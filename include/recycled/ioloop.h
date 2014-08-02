#ifndef RECYCLED_INCLUDE_IOLOOP_H
#define RECYCLED_INCLUDE_IOLOOP_H
#include <functional>
#include <event2/event.h>

namespace recycled {
class IOLoop {
    public:
        typedef std::function<bool (event_base *base)> EventAddHandler;
        static IOLoop & get_instance();
        bool add_event(EventAddHandler handler);
        bool start();
    private:
        IOLoop();
        ~IOLoop();
        event_base *base;
};
}
#endif