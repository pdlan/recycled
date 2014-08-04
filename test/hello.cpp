#include <recycled.h>

using namespace recycled;

// we suggest using it.
void function_handler(Connection &conn) {
    conn.write("hello, function handler.");
}

struct FunctorHandler {
    void operator()(Connection &conn) {
        conn.write("hello, functor handler.");
    }
};

int main() {
    auto lambda_handler = [](Connection &conn) {
        conn.write("hello, lambda handler");
    };
    Application<HTTPServer> app({
        {"/function", function_handler, {HTTPMethod::GET}},
        {"/lambda", lambda_handler, {HTTPMethod::GET}},
        {"/functor", FunctorHandler(), {HTTPMethod::GET}}
    });
    app.listen(8080);
    IOLoop::get_instance().start();
    return 0;
}