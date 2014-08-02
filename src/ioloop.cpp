#include <functional>
#include <event2/event.h>
#include "recycled/ioloop.h"

using namespace recycled;

IOLoop::IOLoop(): base(NULL) {
    this->base = event_base_new();
}

IOLoop::~IOLoop() {
    if (this->base) {
        event_base_free(this->base);
    }
}

IOLoop & IOLoop::get_instance() {
    static IOLoop loop;
    return loop;
}

bool IOLoop::add_event(EventAddHandler handler) {
    if (!this->base) {
        return false;
    }
    return handler(this->base);
}

bool IOLoop::start() {
    if (!this->base) {
        return false;
    }
    event_base_dispatch(this->base);
    return true;
}