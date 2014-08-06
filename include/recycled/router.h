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

/**
 * URL路由类
 */
class Router {
    public:
        Router();
        ~Router();
        Router(Router &other) = delete;
        const Router & operator=(Router &other) = delete;
        /**
         * 设置错误处理器
         *
         * @param handler 错误处理器
         *
         * @return 设置成功返回true, 否则返回false
         */
        bool set_error_handler(const ErrorHandler &handler);
        /**
         * 得到错误处理器
         *
         * @return 错误处理器
         */
        const ErrorHandler & get_error_handler();
        /**
         * 增加多个请求处理器
         *
         * @param handlers 请求处理器
         *
         * @return 增加成功返回true, 否则返回false
         */
        bool add(const std::vector<HandlerStruct> &handlers);
         /**
         * 增加一个请求处理器
         *
         * @param pattern URL模式(类似于Flask)的路由模式
         * 参数形式: <[int:/string:/float:/Regex:]argument_name>
         * 参数类型默认为字符串, 也可以填入正则表达式
         * 请不要在URL模式中出现捕获组, 不然将无法取得参数
         * 如/page/<int:page> /<[\\dA-Fa-f]+:hex_arg> /article/<id>
         *
         * @param handler 请求处理器
         *
         * @param methods 该处理器允许的HTTP请求方法
         *
         * @return 增加成功返回true, 否则返回false
         */
        bool add(const std::string &pattern, const RequestHandler &handler,
                 const std::set<HTTPMethod> &methods);
        /**
         * 通过提供的路径和HTTP请求方法路由到请求处理器
         *
         * @param path 路径
         *
         * @param method HTTP请求方法
         *
         * @param arguments Path参数的输出Map
         *
         * @return 若成功返回请求处理器,
         * 否则返回一个通过std::bind由错误处理器转换而成的请求处理器
         */
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