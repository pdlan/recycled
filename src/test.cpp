#include <string.h>
#include <string>
#include <recycled/connection.h>
#include <recycled/httpserver.h>
#include <recycled/ioloop.h>

using namespace recycled;

void handler(Connection &conn) {
    conn.write("test");
    conn.finish();
}

int main() {
    HTTPServer server(handler);
    server.initialize();
    server.listen(8080);
    IOLoop::get_instance().start();
    return 0;
}