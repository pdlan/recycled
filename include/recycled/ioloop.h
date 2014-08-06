#ifndef RECYCLED_INCLUDE_IOLOOP_H
#define RECYCLED_INCLUDE_IOLOOP_H
#include <functional>
#include <event2/event.h>

namespace recycled {
/**
 * 基于libevent的事件循环类
 */
class IOLoop {
    public:
        typedef std::function<bool (event_base *base)> EventAddHandler;
        /**
         *取得IOLoop示例
         *
         * @return IOLoop实例
         */
        static IOLoop & get_instance();
        /**
         * 增加一个事件循环
         *
         * @param handler 增加时调用的回调函数, 参数中包含event_base *
         *
         * @return 增加成功返回true, 否则返回false
         */
        bool add_event(EventAddHandler handler);
        /**
         * 开始事件循环
         *
         * @return 开始循环成功返回true, 否则返回false
         */
        bool start();
    private:
        IOLoop();
        ~IOLoop();
        event_base *base;
};
}
#endif