#ifndef RECYCLED_INCLUDE_HANDLER_H
#define RECYCLED_INCLUDE_HANDLER_H
#include <functional>
namespace recycled {
class Connection;
typedef std::function<void (Connection &conn)> RequestHandler;
typedef std::function<void (int code, Connection &conn)> ErrorHandler;

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