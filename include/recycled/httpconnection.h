#ifndef RECYCLED_INCLUDE_HTTPCONNECTION_H
#define RECYCLED_INCLUDE_HTTPCONNECTION_H
#include <sys/queue.h>
#include <string>
#include <vector>
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

class HTTPConnection: public Connection {
    public:
        HTTPConnection(evhttp_request *evreq);
        ~HTTPConnection();
        bool initialize();
        bool write(const char *data, size_t length);
        bool write(const std::string &str);
        bool set_status(int status_code, const std::string &reason = "");
        const std::string & get_query_argument(const std::string &key) const;
        std::vector<std::string> get_query_arguments(const std::string &key) const;
        const std::string & get_body_argument(const std::string &key) const;
        std::vector<std::string> get_body_arguments(const std::string &key) const;
        const std::string & get_argument(const std::string &key) const;
        std::vector<std::string> get_arguments(const std::string &key) const;
        void finish();
    private:
        evhttp_request *evreq;
        std::string uri, path;
        evbuffer *input_buffer, *output_buffer;
        evkeyvalq *input_headers, *output_headers;
        std::multimap<std::string, std::string> query_arguments, body_arguments;
        int status_code;
        std::string status_reason;
        HTTPMethod method;
};
}
#endif