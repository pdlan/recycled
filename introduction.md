\mainpage
recycled是一个C++11 Web开发框架

编译
====
编译recycled需要支持C++11特性的编译器, 如较新版本的clang, g++和Visual C++.
recycled依赖libevent2和PCRE, 在编译使用recycled的程序时编译参数应加入-lpcre -levent

部署方式
========
目前支持基于libevent的HTTP部署方式, 以后可能增加FastCGI部署方式

示例程序
========
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <recycled.h>

using namespace recycled;

void index_handler(Connection &conn) {
    conn.write("hello, world");
}

int main() {
    Application<HTTPServer> app({
        {"/", index_handler, {HTTPMethod::GET}}
    });
    app.listen(8080);
    IOLoop::get_instance().start();
    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
该程序使用函数处理器, recycled还支持其他多种请求处理器

Lambda处理器
------------
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
auto index_handler = [](Connection &conn) {
    conn.write("hello, world");
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

仿函数处理器
------------
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
struct IndexHandler {
    void operator()(Connection &conn) {
        conn.write("hello, world");
    }
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

基于类的处理器
--------------
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class IndexHandler: public ClassHandler {
    void get(Connection &conn) {
        conn.write("hello, world");
    }
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ClassHandler可以隐式转换为RequestHandler, 因此可以这样使用
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
Application<HTTPServer> app({
    {"/", IndexHandler(), {HTTPMethod::GET}}
});
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~