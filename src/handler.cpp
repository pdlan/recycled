#include <functional>
#include "recycled/handler.h"
#include "recycled/connection.h"

using namespace recycled;

void ClassHandler::handle(Connection &conn) {
    HTTPMethod method = conn.get_method();
    switch (method) {
        case HTTPMethod::GET:
            this->get(conn);
            break;
        case HTTPMethod::POST:
            this->post(conn);
            break;
        case HTTPMethod::PUT:
            this->put(conn);
            break;
        case HTTPMethod::PATCH:
            this->patch(conn);
            break;
        case HTTPMethod::DELETE:
            this->delet(conn);
            break;
        case HTTPMethod::HEAD:
            this->head(conn);
            break;
        case HTTPMethod::OPTIONS:
            this->options(conn);
            break;
        case HTTPMethod::Other:
            conn.send_error(404);
            break;
    }
}

ClassHandler::operator RequestHandler() {
    auto binded = std::bind(&ClassHandler::handle, this, std::placeholders::_1);
    return binded;
}