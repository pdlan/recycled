#include <stdio.h>
#include <string>
#include <set>
#include <vector>
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