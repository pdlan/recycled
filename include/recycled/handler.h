#ifndef RECYCLED_INCLUDE_HANDLER_H
#define RECYCLED_INCLUDE_HANDLER_H
#include <functional>
namespace recycled {
class Connection;
/**
 * 请求处理器函数对象
 */
typedef std::function<void (Connection &conn)> RequestHandler;
/**
 * 错误处理器函数对象
 * 可通过std::bind转换为RequestHandler
 */
typedef std::function<void (int code, Connection &conn)> ErrorHandler;
/**
 * 基于类的请求处理器
 * 可以隐式转换为ReuestHandler
 */
class ClassHandler {
    public:
        virtual void handle(Connection &conn);
        virtual void get(Connection &conn) {}
        virtual void post(Connection &conn) {}
        virtual void put(Connection &conn) {}
        virtual void patch(Connection &conn) {}
        virtual void delet(Connection &conn) {}
        virtual void head(Connection &conn) {}
        virtual void options(Connection &conn) {}
        operator RequestHandler();
        
};
}
#endif