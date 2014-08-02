#ifndef RECYCLED_INCLUDE_HTTPCONNECTION_H
#define RECYCLED_INCLUDE_HTTPCONNECTION_H
#include <string>
#include <map>
#include <event2/event.h>
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
    {EVHTTP_REQ_PATCH,   HTTPMethod::PATCH},
}

class HTTPConnection: public Connection {
    public:
        HTTPConnection(evhttp_request *evreq);
        ~HTTPConnection();
        bool write(const char *data, size_t length);
        bool write(const std::string &str);
        bool set_status(int status_code, const std::string &reason = "");
        void finish();
    private:
        evhttp_request *evreq;
        evbuffer *input_buffer, *output_buffer;
        int status_code;
        std::string status_reason;
        HTTPMethod method;
};
}
#endif