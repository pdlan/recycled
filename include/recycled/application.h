#ifndef RECYCLED_INCLUDE_APPLICATION_H
#define RECYCLED_INCLUDE_APPLICATION_H
#include <string>
#include <functional>
#include "recycled/handler.h"
#include "recycled/router.h"

namespace recycled {
class ApplicationException: public std::exception {
    public:
        ApplicationException(const std::string &msg): msg(msg) {}
        ~ApplicationException() noexcept {}
        const char * what() const noexcept {return this->msg.c_str();}
    private:
        std::string msg;
};

template<typename T>
class Application {
    public:
        /**
         * 构造一个Application
         * @param handlers Application需要的请求处理器
         *
         * @param args 这些参数将传给Server
         */
        template<typename... Arguments>
        Application(const std::vector<HandlerStruct> &handlers, Arguments... args);
        Application(const Application &other) = delete;
        ~Application();
        const Application & operator=(const Application &other) = delete;
        /**
         * 调用Server的listen方法
         *
         * @param args 这些参数将传给Server
         */
        template<typename... Arguments>
        void listen(Arguments... args);
    private:
        T *server;
        Router *router;
        void server_handler(Connection &conn);
};

template<typename T> template<typename... Arguments>
Application<T>::Application(const std::vector<HandlerStruct> &handlers,
                            Arguments... args) {
    auto handler = std::bind(&Application<T>::server_handler,
                             this, std::placeholders::_1);
    this->router = new Router();
    for (auto &i: handlers) {
        if (!router->add(i.pattern, i.handler, i.methods)) {
            delete this->router;
            std::string msg = "invalid pattern: " + i.pattern;
            throw ApplicationException(msg);
        }
    }
    this->server = new T(handler, args...);
    if (!server->initialize()) {
        delete this->server;
        throw ApplicationException("cannot initialize server.");
    }
}

template<typename T>
Application<T>::~Application() {
    delete this->server;
    delete this->router;
}

template<typename T> template<typename... Arguments>
void Application<T>::listen(Arguments... args) {
    if (!this->server->listen(args...)) {
        throw ApplicationException("cannot listen.");
    }
}

template<typename T>
void Application<T>::server_handler(Connection &conn) {
    const std::string &path = conn.get_path();
    HTTPMethod method = conn.get_method();
    SSMap &path_arguments = conn.get_path_arguments();
    const ErrorHandler &error_handler = this->router->get_error_handler();
    const RequestHandler &handler =
        this->router->route(path, method, path_arguments);
    conn.set_error_handler(error_handler);
    handler(conn);
    if (!conn.is_finished()) {
        conn.finish();
    }
}
}
#endif