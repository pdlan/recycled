#include <string>
#include <set>
#include <vector>
#include <recycled/connection.h>
#include <recycled/httpconnection.h>
#include <recycled/httpserver.h>
#include <recycled/ioloop.h>
#include <recycled/router.h>

using namespace recycled;

void handler(Connection &conn) {
    conn.finish();
}

int main() {
    HTTPServer server(handler);
    server.initialize();
    server.listen(8080);
    IOLoop::get_instance().start();
    return 0;
}