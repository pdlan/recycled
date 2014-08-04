#include <stdint.h>
#include <string>
#include <functional>
#include <event2/event.h>
#include <event2/http.h>
#include "recycled/httpserver.h"
#include "recycled/httpconnection.h"
#include "recycled/ioloop.h"

using namespace recycled;

HTTPServer::HTTPServer(RequestHandler request_handler):
    request_handler(request_handler), event_http(nullptr) {}

bool HTTPServer::initialize() {
    IOLoop & loop = IOLoop::get_instance();
    IOLoop::EventAddHandler add_handler =
        std::bind(&HTTPServer::event_add_handler, this, std::placeholders::_1);
    if (!loop.add_event(add_handler)) {
        return false;
    }
    evhttp_set_gencb(this->event_http, evhttp_handler, (void *)this);
    return true;
}

bool HTTPServer::listen(uint16_t port, const std::string &ip) {
    if (!this->event_http) {
        return false;
    }
    evhttp_bound_socket *handle;
    handle = evhttp_bind_socket_with_handle(this->event_http, ip.c_str(), port);
    if (!handle) {
        return false;
    }
    return true;
}

bool HTTPServer::event_add_handler(event_base *base) {
    if (!base) {
        return false;
    }
    event_http = evhttp_new(base);
    if (!this->event_http) {
        return false;
    }
    return true;
}

void HTTPServer::evhttp_handler(evhttp_request *req, void *arg) {
    HTTPServer *server = (HTTPServer *)arg;
    HTTPConnection conn(req);
    conn.initialize();
    server->request_handler(conn);
}