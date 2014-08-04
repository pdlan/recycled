#include <stdio.h>
#include <string>
#include <set>
#include <vector>
#include <recycled/connection.h>
#include <recycled/httpconnection.h>
#include <recycled/httpserver.h>
#include <recycled/ioloop.h>
#include <recycled/router.h>
#include <recycled/application.h>

using namespace recycled;

void index_handler(Connection &conn) {
    conn.send_error(500);
}

int main() {
    Application<HTTPServer> app({
        {"/test", index_handler, {HTTPMethod::GET}}
    });
    if (!app.listen(8080)) {
        fprintf(stderr, "Cannot listen port 8080.\n");
        return -1;
    }
    IOLoop::get_instance().start();
    return 0;
}