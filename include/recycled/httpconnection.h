#ifndef RECYCLED_INCLUDE_HTTPCONNECTION_H
#define RECYCLED_INCLUDE_HTTPCONNECTION_H
#include <sys/queue.h>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <event2/event.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include "recycled/connection.h"

namespace recycled {
static const std::map<evhttp_cmd_type, HTTPMethod> Methods = {
    {EVHTTP_REQ_GET,     HTTPMethod::GET},
    {EVHTTP_REQ_POST,    HTTPMethod::POST},
    {EVHTTP_REQ_HEAD,    HTTPMethod::HEAD},
    {EVHTTP_REQ_PUT,     HTTPMethod::PUT},
    {EVHTTP_REQ_DELETE,  HTTPMethod::DELETE},
    {EVHTTP_REQ_OPTIONS, HTTPMethod::OPTIONS},
    {EVHTTP_REQ_PATCH,   HTTPMethod::PATCH}
};

typedef std::tuple<std::string,
                   bool,
                   time_t,
                   std::string,
                   std::string,
                   bool> CookieInfo;

class HTTPConnection: public Connection {
    public:
        HTTPConnection(evhttp_request *evreq);
        HTTPConnection(const HTTPConnection &other) = delete;
        ~HTTPConnection();
        const HTTPConnection & operator=(const HTTPConnection &other) = delete;
        bool initialize();
        bool write(const char *data, size_t size);
        bool write(const std::string &str);
        bool set_status(int status_code, const std::string &reason = "");
        HTTPMethod get_method() const;
        std::string get_path() const;
        std::string get_uri() const;
        std::string get_query_argument(const std::string &key) const;
        std::string get_body_argument(const std::string &key) const;
        std::string get_argument(const std::string &key) const;
        std::string get_path_argument(const std::string &key) const;
        std::string get_header(const std::string &key) const;
        std::string get_cookie(const std::string &key) const;
        SVector get_query_arguments(const std::string &key) const;
        SVector get_body_arguments(const std::string &key) const;
        SVector get_arguments(const std::string &key) const;
        const SSMap & get_path_arguments() const;
        const SSMap & get_headers() const;
        const SSMap & get_cookies() const;
        SSMap & get_path_arguments();
        bool set_error_handler(const ErrorHandler &handler);
        bool set_cookie(const std::string &key,
                        const std::string &value,
                        bool secure = false,
                        time_t expires=3600,
                        const std::string &domain="",
                        const std::string &path="/",
                        bool http_only = false);
        bool remove_cookie(const std::string &key,
                           const std::string &domain="",
                           const std::string &path="");
        void clear_cookies();
        bool add_header(const std::string &key, const std::string &value);
        bool remove_header(const std::string &key);
        void clear_headers();
        bool flush();
        bool send_error(int status=500);
        void finish();
        bool redirect(const std::string &url, int status=302);
        bool is_finished() const;
    private:
        evhttp_request *evreq;
        ErrorHandler error_handler;
        std::string uri, path;
        evbuffer *input_buffer, *output_buffer;
        evkeyvalq *output_headers;
        SSMap input_headers;
        SSMultiMap query_arguments, body_arguments;
        SSMap path_arguments;
        SSMap input_cookies;
        std::multimap<std::string, CookieInfo> output_cookies;
        int status_code;
        std::string status_reason;
        HTTPMethod method;
        bool finished;
        bool chunked;
};
}
#endif