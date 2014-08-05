#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <sys/queue.h>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <map>
#include <functional>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include "recycled/httpconnection.h"

using namespace recycled;

template<typename T>
bool evkeyvalq_to_map(evkeyvalq *src, T &dest) {
    if (!src) {
        return false;
    }
    evkeyval *first = src->tqh_first;
    if (!first) {
        return false;
    }
    for (evkeyval *i = first; i != NULL && &i != src->tqh_last; i = i->next.tqe_next) {
        const char *key = i->key;
        const char *value = i->value;
        if (key && value) {
            dest.insert(std::make_pair(key, value));
        }
    }
    return true;
}

bool parse_cookie(const std::string &str, SSMap &dest) {
    SSMap cookies;
    std::ostringstream key_buf, value_buf;
    int state = 1;
    for (size_t i = 0; i <= str.length(); ++i) {
        char ch = str[i];
        switch (state) {
            case 1:
                switch (ch) {
                    case '=':
                        state = 2;
                        break;
                    case '\0':
                    case ';':
                        return false;
                    default:
                        key_buf << ch;
                }
                break;
            case 2:
                switch (ch) {
                    case ';':
                        state = 3;
                        break;
                    case '=':
                        return false;
                    case '\0': {
                        const std::string &key = key_buf.str();
                        const std::string &value = value_buf.str();
                        cookies.insert(std::make_pair(key, value));
                        state = 1;
                    }
                    default:
                        value_buf << ch;
                }
                break;
            case 3:
                switch (ch) {
                    case ' ':
                        break;
                    case '=':
                    case ';':
                        return false;
                    default: {
                        const std::string &key = key_buf.str();
                        const std::string &value = value_buf.str();
                        cookies.insert(std::make_pair(key, value));
                        key_buf.str("");
                        value_buf.str("");
                        key_buf << ch;
                        state = 1;
                    }
                }
        }
    }
    dest = cookies;
    return true;
}

std::string make_cookie_header(const std::string &key, const CookieInfo &info) {
    const std::string &value = std::get<0>(info);
    bool secure = std::get<1>(info);
    time_t expires = std::get<2>(info);
    const std::string &domain = std::get<3>(info);
    const std::string &path = std::get<4>(info);
    bool http_only = std::get<5>(info);
    const int BufferSize = 128;
    char expires_buf[BufferSize];
    std::ostringstream header;
    header << key << "=" << value;
    if (!domain.empty()) {
        header << "; " << "Domain=" << domain;
    }
    if (!path.empty()) {
        header << "; " << "Path=" << path;
    }
    time_t now = time(NULL);
    time_t expires_stamp = now + expires;
    tm *expires_time = gmtime(&expires_stamp);
    strftime(expires_buf, BufferSize, "%a, %d-%b-%Y %H:%M:%S GMT", expires_time);
    header << "; Expires=" << expires_buf;
    if (secure) {
        header << "; Secure";
    }
    if (http_only) {
        header << "; HttpOnly";
    }
    return header.str();
}

HTTPConnection::HTTPConnection(evhttp_request *evreq):
    evreq(evreq), input_buffer(NULL), output_buffer(NULL),
    output_headers(NULL), finished(false), chunked(false) {
}

HTTPConnection::~HTTPConnection() {
    if (this->output_buffer) {
        evbuffer_free(this->output_buffer);
    }
    if (this->output_headers) {
        evhttp_clear_headers(this->output_headers);
    }
}

bool HTTPConnection::initialize() {
    if (!this->evreq) {
        return false;
    }
    this->output_buffer = evbuffer_new();
    if (!this->output_buffer) {
        return false;
    }
    this->input_buffer = evhttp_request_get_input_buffer(this->evreq);
    evkeyvalq *input_headers_ev = evhttp_request_get_input_headers(this->evreq);
    if (!evkeyvalq_to_map(input_headers_ev, this->input_headers)) {
        return false;
    }
    evhttp_clear_headers(input_headers_ev);
    this->output_headers = evhttp_request_get_output_headers(this->evreq);
    const char *uri = evhttp_request_get_uri(this->evreq);
    evhttp_uri *decoded = evhttp_uri_parse(uri);
    if (!decoded) {
        return false;
    }
    const char *path = evhttp_uri_get_path(decoded);
    if (!path) {
        if (decoded) {
            evhttp_uri_free(decoded);
        }
        return false;
    }
    const char *decoded_path = evhttp_uridecode(path, 1, NULL);
    if (!decoded_path) {
        if (decoded) {
            evhttp_uri_free(decoded);
        }
        return false;
    }
    this->path = decoded_path;
    const char *query_str = evhttp_uri_get_query(decoded);
    if (query_str) {
        evkeyvalq arguments_ev;
        if (evhttp_parse_query_str(query_str, &arguments_ev) == 0) {
            evkeyvalq_to_map(&arguments_ev, this->query_arguments);
            evhttp_clear_headers(&arguments_ev);
        }
    }
    if (decoded) {
        evhttp_uri_free(decoded);
    }
    size_t body_length = evbuffer_get_length(this->input_buffer);
    if (body_length) {
        char *buffer = new char[body_length + 1];
        memcpy(buffer, evbuffer_pullup(this->input_buffer, body_length), body_length);
        buffer[body_length] = '\0'; //regard body as string.
        evkeyvalq arguments_ev;
        if (evhttp_parse_query_str(buffer, &arguments_ev) == 0) {
            evkeyvalq_to_map(&arguments_ev, this->body_arguments);
            evhttp_clear_headers(&arguments_ev);
        }
        delete buffer;
    }
    const std::string &cookie_header = this->get_header("Cookie");
    parse_cookie(cookie_header, this->input_cookies);
    this->set_status(200);
    auto it = Methods.find(evhttp_request_get_command(this->evreq));
    if (it != Methods.end()) {
        this->method = it->second;
    } else {
        this->method = HTTPMethod::Other;
    }
    return true;
}

bool HTTPConnection::write(const char *data, size_t size) {
    if (!this->output_buffer) {
        return false;
    }
    if (evbuffer_add(this->output_buffer, data, size) != 0) {
        return false;
    }
    return true;
}

bool HTTPConnection::write(const std::string &str) {
    return this->write(str.c_str(), str.length());
}

bool HTTPConnection::set_status(int status_code, const std::string &reason) {
    if (!StatusCodes.count(status_code) || this->finished || this->chunked) {
        return false;
    }
    this->status_code = status_code;
    if (reason.empty()) {
        auto it = StatusReasons.find(status_code);
        if (it != StatusReasons.end()) {
            this->status_reason = it->second;
        } else {
            return false;
        }
    } else {
        this->status_reason = reason;
    }
    return true;
}

HTTPMethod HTTPConnection::get_method() const {
    return this->method;
}

std::string HTTPConnection::get_path() const {
    return this->path;
}

std::string HTTPConnection::get_uri() const {
    return this->uri;
}

std::string HTTPConnection::get_query_argument(const std::string &key) const {
    size_t count = this->query_arguments.count(key);
    if (!count) {
        return "";
    }
    auto it = this->query_arguments.find(key);
    for (size_t i = 0; i < (count - 1); ++i, ++it);
    return it->second;
}

std::string HTTPConnection::get_body_argument(const std::string &key) const {
    size_t count = this->body_arguments.count(key);
    if (!count) {
        return "";
    }
    auto it = this->body_arguments.find(key);
    for (size_t i = 0; i < (count - 1); ++i, ++it);
    return it->second;
}

std::string HTTPConnection::get_argument(const std::string &key) const {
    const std::string &query_argument = this->get_query_argument(key);
    if (!query_argument.empty()) {
        return query_argument;
    }
    const std::string &body_argument = this->get_body_argument(key);
    if (!body_argument.empty()) {
        return body_argument;
    }
    return "";
}

std::string HTTPConnection::get_path_argument(const std::string &key) const {
    auto it = this->path_arguments.find(key);
    if (it != this->path_arguments.end()) {
        return it->second;
    } else {
        return "";
    }
}

std::string HTTPConnection::get_header(const std::string &key) const {
    auto it = this->input_headers.find(key);
    if (it != this->input_headers.end()) {
        return it->second;
    } else {
        return "";
    }
}

std::string HTTPConnection::get_cookie(const std::string &key) const {
    auto it = this->input_cookies.find(key);
    if (it != this->input_cookies.end()) {
        return it->second;
    } else {
        return "";
    }
}

SVector HTTPConnection::get_query_arguments(const std::string &key) const {
    SVector arguments;
    auto begin = this->query_arguments.lower_bound(key);
    auto end = this->query_arguments.upper_bound(key);
    for (auto it = begin; it != end; ++it) {
        arguments.push_back(it->second);
    }
    return arguments;
}

SVector HTTPConnection::get_body_arguments(const std::string &key) const {
    SVector arguments;
    auto begin = this->body_arguments.lower_bound(key);
    auto end = this->body_arguments.upper_bound(key);
    for (auto it = begin; it != end; ++it) {
        arguments.push_back(it->second);
    }
    return arguments;
}

SVector HTTPConnection::get_arguments(const std::string &key) const {
    const SVector &query_arguments = this->get_query_arguments(key);
    const SVector &body_arguments = this->get_body_arguments(key);
    SVector arguments;
    for (const std::string &s: query_arguments) {
        arguments.push_back(s);
    }
    for (const std::string &s: body_arguments) {
        arguments.push_back(s);
    }
    return arguments;
}

const SSMap & HTTPConnection::get_path_arguments() const {
    return this->path_arguments;
}

const SSMap & HTTPConnection::get_headers() const {
    return this->input_headers;
}

const SSMap & HTTPConnection::get_cookies() const {
    return this->input_cookies;
}

SSMap & HTTPConnection::get_path_arguments() {
    return this->path_arguments;
}

bool HTTPConnection::set_error_handler(const ErrorHandler &handler) {
    if (!handler) {
        return false;
    }
    this->error_handler = handler;
    return true;
}

bool HTTPConnection::set_cookie(const std::string &key,
                                const std::string &value,
                                bool secure,
                                time_t expires,
                                const std::string &domain,
                                const std::string &path,
                                bool http_only) {
    auto begin = this->output_cookies.lower_bound(key);
    auto end = this->output_cookies.upper_bound(key);
    for (auto it = begin; it != end; ++it) {
        const CookieInfo &info = it->second;
        if (std::get<3>(info) == domain && std::get<4>(info) == path) {
            return false;
        }
    }
    auto p = std::make_pair(key, std::forward_as_tuple(value, secure, expires,
                                                       domain, path, http_only));
    this->output_cookies.insert(p);
    return true;
}

bool HTTPConnection::remove_cookie(const std::string &key,
                                   const std::string &domain,
                                   const std::string &path) {
    bool found = false;
    for (auto it = this->output_cookies.begin();
         it != this->output_cookies.end(); ++it) {
        const std::string &k = it->first;
        if (k == key) {
            const CookieInfo &info = it->second;
            const std::string &d = std::get<3>(info);
            const std::string &p = std::get<4>(info);
            if (domain.empty() || d == domain) {
                if (path.empty() || p == path) {
                    this->output_cookies.erase(it);
                    found = true;
                }
            }
        }
    }
    return found;
}

void HTTPConnection::clear_cookies() {
    this->output_cookies.clear();
}

bool HTTPConnection::add_header(const std::string &key, const std::string &value) {
    if (!this->output_headers || this->finished || this->chunked) {
        return false;
    }
    return evhttp_add_header(this->output_headers, key.c_str(), value.c_str()) == 0;
}

bool HTTPConnection::remove_header(const std::string &key) {
    if (!this->output_headers || this->finished || this->chunked) {
        return false;
    }
    return evhttp_remove_header(this->output_headers, key.c_str()) == 0;
}

void HTTPConnection::clear_headers() {
    if (!this->output_headers) {
        return;
    }
    evhttp_clear_headers(this->output_headers);
}

bool HTTPConnection::flush() {
    if (this->finished || !this->output_buffer) {
        return false;
    }
    if (!this->chunked) {
        for (auto &p: this->output_cookies) {
            this->add_header("Set-Cookie", make_cookie_header(p.first, p.second));
        }
        evhttp_send_reply_start(this->evreq, this->status_code,
                                this->status_reason.c_str());
        this->chunked = true;
    }
    evhttp_send_reply_chunk(this->evreq, this->output_buffer);
    size_t length = evbuffer_get_length(this->output_buffer);
    evbuffer_drain(this->output_buffer, length);
    return true;
}

bool HTTPConnection::send_error(int status) {
    if (this->finished || this->chunked || !this->error_handler) {
        return false;
    }
    this->error_handler(status, *this);
    return true;
}

void HTTPConnection::finish() {
    if (this->finished) {
        return;
    }
    if (!this->chunked) {
        if (!this->output_buffer) {
            return;
        }
        for (auto &p: this->output_cookies) {
            this->add_header("Set-Cookie", make_cookie_header(p.first, p.second));
        }
        evhttp_send_reply(this->evreq, this->status_code,
                        this->status_reason.c_str(), this->output_buffer);
    } else {
        size_t length = evbuffer_get_length(this->output_buffer);
        if (length) {
            evhttp_send_reply_chunk(this->evreq, this->output_buffer);
        }
        evhttp_send_reply_end(this->evreq);
    }
    finished = true;
}

bool HTTPConnection::redirect(const std::string &key, int status) {
    if (this->finished || this->chunked) {
        return false;
    }
    
    this->clear_headers();
    if (!this->set_status(status)) {
        return false;
    }
    if (!this->add_header("Location", key)) {
        return false;
    }
    this->finish();
    return true;
}

bool HTTPConnection::is_finished() const {
    return this->finished;
}