#include <stdio.h>
#include <functional>
#include <recycled.h>

using namespace recycled;

// we suggest using it.
void function_handler(Connection &conn) {
    const UploadFile * file = conn.get_file("test");
    FILE *f = fopen("1.png", "w");
    fwrite(file->data, file->size, sizeof(char), f);
    fclose(f);
}

// you can use std::bind to convert your custom function to handler function.
void custom_handler(const std::string &custom_arg, Connection &conn) {
    conn.write(custom_arg);
}

struct FunctorHandler {
    void operator()(Connection &conn) {
        conn.write("hello, functor handler.");
    }
};

class IndexHandler: public ClassHandler {
    void get(Connection &conn) {
        conn.write("hello, class handler.");
    }
};

int main() {
    auto lambda_handler = [](Connection &conn) {
        conn.write("hello, lambda handler");
    };
    auto custom_handler_binded = std::bind(custom_handler,
                                           "hello, custom handler",
                                           std::placeholders::_1);
    Application<HTTPServer> app({
        {"/", IndexHandler(), {HTTPMethod::GET}},
        {"/function", function_handler, {HTTPMethod::GET, HTTPMethod::POST}},
        {"/lambda", lambda_handler, {HTTPMethod::GET}},
        {"/functor", FunctorHandler(), {HTTPMethod::GET}},
        {"/custom", custom_handler_binded, {HTTPMethod::GET}},
    });
    app.listen(8080);
    IOLoop::get_instance().start();
    return 0;
}