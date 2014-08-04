#ifndef RECYCLED_INCLUDE_ROUTER_H
#define RECYCLED_INCLUDE_ROUTER_H
#include <string>
#include <set>
#include <tuple>
#include <pcre.h>
#include "recycled/connection.h"
#include "recycled/handler.h"

namespace recycled {
struct HandlerStruct {
    std::string pattern;
    RequestHandler handler;
    std::set<HTTPMethod> methods;
};

class Router {
    public:
        Router();
        ~Router();
        Router(Router &other) = delete;
        const Router & operator=(Router &other) = delete;
        bool set_error_handler(const ErrorHandler &handler);
        const ErrorHandler & get_error_handler();
        bool add(const std::vector<HandlerStruct> &handlers);
        bool add(const std::string &pattern, const RequestHandler &handler,
                 const std::set<HTTPMethod> &methods);
        RequestHandler route(const std::string &path,
                             HTTPMethod method,
                             std::map<std::string, std::string> &arguments) const;
    private:
        typedef std::tuple<pcre *,
                           RequestHandler,
                           std::set<HTTPMethod>,
                           std::vector<std::string>> HandlerTuple;
        std::vector<HandlerTuple> handlers;
        ErrorHandler error_handler;
        static void default_error_handler(int code, Connection &conn);
};
}
#endif