#ifndef RECYCLED_INCLUDE_HANDLER_H
#define RECYCLED_INCLUDE_HANDLER_H
#include <functional>
#include "recycled/connection.h"
namespace recycled {
typedef std::function<void (Connection &conn)> RequestHandler;
typedef std::function<void (int code, Connection &conn)> ErrorHandler;
}
#endif