#ifndef RECYCLED_INCLUDE_FORMAT_H
#define RECYCLED_INCLUDE_FORMAT_H
#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <list>
#include <exception>

namespace recycled {
namespace format {
class FormatException: public std::exception {
    public:
        FormatException(const std::string &msg): msg(msg) {}
        ~FormatException() noexcept{}
        const char * what() const noexcept {return this->msg.c_str();}
    private:
        std::string msg;
};
namespace {
template<typename T>
std::string safe_format(const char *specifier, T value) {
    std::string format = "%";
    format += specifier;
    size_t buf_length = format.length() * 2;
    char *buf = new char[buf_length];
    int need_length = snprintf(buf, buf_length, format.c_str(), value);
    if (need_length >= buf_length) {
        delete buf;
        buf = new char[need_length + 1];
        snprintf(buf, need_length + 1, format.c_str(), value);
    } \
    std::string buf_str(buf);
    delete buf;
    return buf_str;
}

template <>
std::string
safe_format<std::string>(const char *specifier,
                         std::string value) {
    std::string format = "%";
    format += specifier;
    size_t buf_length = format.length() * 2;
    char *buf = new char[buf_length];
    int need_length = snprintf(buf, buf_length, format.c_str(), value.c_str());
    if (need_length >= buf_length) {
        delete buf;
        buf = new char[need_length + 1];
        snprintf(buf, need_length + 1, format.c_str(), value.c_str());
    }
    std::string buf_str(buf);
    delete buf;
    return buf_str;
}

template<size_t i, typename... Arguments>
class FillSpecifiers {
    public:
        static void fill(const std::vector<std::string> &specifiers,
                         const std::tuple<Arguments...> &args,
                         std::list<std::string> &results) {
            const auto &arg = std::get<i>(args);
            const std::string &specifier = specifiers[i];
            std::string result = safe_format(specifier.c_str(), arg);
            results.push_front(result);
            FillSpecifiers<i-1, Arguments...>::fill(specifiers, args, results);
        }
};

template<typename... Arguments>
class FillSpecifiers<0, Arguments...> {
    public:
        static void fill(const std::vector<std::string> &specifiers,
                         const std::tuple<Arguments...> &args,
                         std::list<std::string> &results) {
            const auto &arg = std::get<0>(args);
            const std::string &specifier = specifiers[0];
            std::string result = safe_format(specifier.c_str(), arg);
            results.push_front(result);
        }
};
}
template<typename... Arguments>
std::string format(const std::string &format,
                   const std::tuple<Arguments...> &args) {
    std::ostringstream buf;
    const int TupleSize = std::tuple_size<std::tuple<Arguments...>>::value;
    int state = 1;
    size_t begin_pos = 0;
    size_t length;
    std::vector<std::string> specifiers;
    std::list<std::string> texts;
    std::list<std::string> results;
    std::list<bool> token_types;
    // true: plain text. false: specifier
    for (size_t i = 0; i < format.length() + 1; ++i) {
        char ch = format[i];
        switch (state) {
            case 1:
                switch (ch) {
                    case '%':
                        length = i - begin_pos;
                        texts.push_back(format.substr(begin_pos, length));
                        token_types.push_back(true);
                        begin_pos = i + 1;
                        state = 2;
                        break;
                    case '\0':
                        length = i - begin_pos;
                        texts.push_back(format.substr(begin_pos, length));
                        token_types.push_back(true);
                        begin_pos = i + 1;
                        break;
                }
                break;
            case 2:
                switch (ch) {
                    case '%':
                        buf << '%';
                        begin_pos = i + 1;
                        state = 1;
                        break;
                    case 'd':
                    case 'i':
                    case 'u':
                    case 'o':
                    case 'x':
                    case 'X':
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                    case 'c':
                    case 's':
                    case 'p':
                    case 'n':
                        length = i - begin_pos + 1;
                        std::string spec = format.substr(begin_pos, length);
                        specifiers.push_back(spec);
                        token_types.push_back(false);
                        begin_pos = i + 1;
                        state = 1;
                        break;
                }
        }
    }
    if (TupleSize > specifiers.size()) {
        throw FormatException("too many arguments");
    } else if (TupleSize < specifiers.size()) {
        throw FormatException("too few arguments");
    }
    FillSpecifiers<TupleSize - 1, Arguments...>::fill(specifiers, args, results);
    auto it_type = token_types.begin();
    auto it_text = texts.begin();
    auto it_res = results.begin();
    for (; it_type != token_types.end(); ++it_type) {
        bool type = *it_type;
        if (type) {
            if (it_text != texts.end()) {
                buf << *it_text;
                ++it_text;
            } else {
                throw FormatException("internal error");
            }
        } else {
            if (it_res != results.end()) {
                buf << *it_res;
                ++it_res;
            } else {
                throw FormatException("internal error");
            }
        }
    }
    return buf.str();
}

template<typename... Arguments>
std::string operator%(const std::string &format,
                      const std::tuple<Arguments...> &args) {
    return recycled::format::format(format, args);
}

template<typename... Arguments>
std::tuple<Arguments...> _(Arguments... args) {
    return std::tuple<Arguments...>(args...);
}

template<typename T>
std::string operator%(const std::string &format,
                      const T &arg)  {
    return recycled::format::format(format, _(arg));
}

}
}
#endif
