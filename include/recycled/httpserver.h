#ifndef RECYCLED_INCLUDE_HTTPSERVER_H
#define RECYCLED_INCLUDE_HTTPSERVER_H
#include <stdint.h>
#include <string>
#include <event2/event.h>
#include <event2/http.h>
#include "recycled/handler.h"

namespace recycled {
class HTTPServer {
    public:
        HTTPServer(RequestHandler handler);
        ~HTTPServer() = default;
        bool initialize();
        bool listen(uint16_t port, const std::string &ip = "0.0.0.0");
    private:
        RequestHandler request_handler;
        evhttp *event_http;
        bool event_add_handler(event_base *base);
        static void evhttp_handler(evhttp_request *req, void *arg);
};
}
#endif