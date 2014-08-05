/**
 * @file
 * @author Falconly members
 * @version 0.1
 *
 * @section DESCRIPTION
 *
 * 基于libevent的HTTP服务器
 */
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
        /**
         * 构造一个服务器
         *
         * @param request_handler 请求处理器
         */
        HTTPServer(const RequestHandler &request_handler);
        HTTPServer(const HTTPServer &other) = delete;
        ~HTTPServer() = default;
        const HTTPServer & operator=(const HTTPServer &other) = delete;
        /**
         * 初始化服务器
         *
         * @return 初始化成功则返回true, 否则返回false
         */
        bool initialize();
        /**
         * 指定绑listen的端口和IP
         *
         * @param port 端口
         * @param ip 绑定的IP, 默认为0.0.0.0
         *
         * @return listen成功返回true, 否则返回false
         */
        bool listen(uint16_t port, const std::string &ip = "0.0.0.0");
    private:
        RequestHandler request_handler;
        evhttp *event_http;
        bool event_add_handler(event_base *base);
        static void evhttp_handler(evhttp_request *req, void *arg);
};
}
#endif