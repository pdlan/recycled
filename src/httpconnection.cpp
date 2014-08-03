#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <sys/queue.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include "recycled/httpconnection.h"

using namespace recycled;

const std::string EmptyString = "";

bool evkeyvalq_to_map(evkeyvalq *src, std::multimap<std::string, std::string> &dest) {
    if (!src)
        return false;
    evkeyval *first = src->tqh_first;
    if (!first)
        return false;
    for (evkeyval *i = first; i != NULL && &i != src->tqh_last; i = i->next.tqe_next) {
        const char *key = i->key;
        const char *value = i->value;
        if (key && value)
            dest.insert(std::make_pair(key, value));
    }
    return true;
}

HTTPConnection::HTTPConnection(evhttp_request *evreq):
    evreq(evreq), input_buffer(NULL), output_buffer(NULL), input_headers(NULL),
    output_headers(NULL) {
}

HTTPConnection::~HTTPConnection() {
    if (this->output_buffer)
        evbuffer_free(this->output_buffer);
}

bool HTTPConnection::initialize() {
    if (!this->evreq)
        return false;
    this->output_buffer = evbuffer_new();
    if (!this->output_buffer)
        return false;
    this->input_buffer = evhttp_request_get_input_buffer(this->evreq);
    this->input_headers = evhttp_request_get_input_headers(this->evreq);
    this->output_headers = evhttp_request_get_output_headers(this->evreq);
    const char *uri = evhttp_request_get_uri(this->evreq);
    evhttp_uri *decoded = evhttp_uri_parse(uri);
    if (!decoded)
        return false;
    const char *path = evhttp_uri_get_path(decoded);
    if (!path) {
        if (decoded)
            evhttp_uri_free(decoded);
        return false;
    }
    const char *decoded_path = evhttp_uridecode(path, 1, NULL);
    if (!decoded_path) {
        if (decoded)
            evhttp_uri_free(decoded);
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
    if (decoded)
        evhttp_uri_free(decoded);
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
    this->set_status(200);
    auto it = Methods.find(evhttp_request_get_command(this->evreq));
    if (it != Methods.end())
        this->method = it->second;
    else
        this->method = HTTPMethod::Other;
    return true;
}

bool HTTPConnection::write(const char *data, size_t length) {
    if (!this->output_buffer)
        return false;
    if (evbuffer_add(this->output_buffer, data, length) != 0)
        return false;
    return true;
}

bool HTTPConnection::write(const std::string &str) {
    return this->write(str.c_str(), str.length());
}

const std::string & HTTPConnection::get_query_argument(const std::string &key) const {
    size_t count = this->query_arguments.count(key);
    if (!count)
        return EmptyString;
    auto it = this->query_arguments.find(key);
    for (size_t i = 0; i < (count - 1); ++i, ++it);
    return it->second;
}

std::vector<std::string> HTTPConnection::get_query_arguments(const std::string &key) const {
    std::vector<std::string> arguments;
    auto begin = this->query_arguments.lower_bound(key);
    auto end = this->query_arguments.upper_bound(key);
    for (auto it = begin; it != end; ++it)
        arguments.push_back(it->second);
    return arguments;
}

const std::string & HTTPConnection::get_body_argument(const std::string &key) const {
    size_t count = this->body_arguments.count(key);
    if (!count)
        return EmptyString;
    auto it = this->body_arguments.find(key);
    for (size_t i = 0; i < (count - 1); ++i, ++it);
    return it->second;
}

std::vector<std::string> HTTPConnection::get_body_arguments(const std::string &key) const {
    std::vector<std::string> arguments;
    auto begin = this->body_arguments.lower_bound(key);
    auto end = this->body_arguments.upper_bound(key);
    for (auto it = begin; it != end; ++it)
        arguments.push_back(it->second);
    return arguments;
}

const std::string & HTTPConnection::get_argument(const std::string &key) const {
    const std::string &query_argument = this->get_query_argument(key);
    if (query_argument != EmptyString)
        return query_argument;
    const std::string &body_argument = this->get_body_argument(key);
    if (body_argument != EmptyString)
        return body_argument;
    return EmptyString;
}

std::vector<std::string> HTTPConnection::get_arguments(const std::string &key) const {
    const std::vector<std::string> &query_arguments = this->get_query_arguments(key);
    const std::vector<std::string> &body_arguments = this->get_body_arguments(key);
    std::vector<std::string> arguments;
    for (const std::string &s: query_arguments)
        arguments.push_back(s);
    for (const std::string &s: body_arguments)
        arguments.push_back(s);
    return arguments;
}

bool HTTPConnection::set_status(int status_code, const std::string &reason) {
    if (!StatusCodes.count(status_code))
        return false;
    this->status_code = status_code;
    if (reason == EmptyString) {
        auto it = StatusReasons.find(status_code);
        if (it != StatusReasons.end())
            this->status_reason = it->second;
        else
            return false;
    } else {
        this->status_reason = reason;
    }
    return true;
}

void HTTPConnection::finish() {
    if (!this->output_buffer)
        return;
    evhttp_send_reply(this->evreq, this->status_code,
                      this->status_reason.c_str(), this->output_buffer);
}