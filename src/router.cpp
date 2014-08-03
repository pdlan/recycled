#include <string>
#include <sstream>
#include <set>
#include <tuple>
#include <pcre.h>
#include "recycled/connection.h"
#include "recycled/handler.h"
#include "recycled/router.h"

using namespace recycled;

Router::Router() {
    this->error_handler = default_error_handler;
}

bool Router::add(std::vector<HandlerStruct> handlers) {
    for (auto &i: handlers) {
        if (!this->add(i.pattern, i.handler, i.methods)) {
            return false;
        }
    }
    return true;
}

bool Router::add(const std::string &pattern, const RequestHandler &handler,
                 const std::set<HTTPMethod> &methods) {
    const char *pattern_string = "(\\w+)";
    const char *pattern_int = "(\\d+)";
    const char *pattern_float = "(\\d*.\\d+|\\d+.\\d*)";
    std::ostringstream pattern_buf, arg_buf1, arg_buf2;
    std::vector<std::string> arg_names;
    int state = 1;
    bool escape = false;
    for (size_t i = 0; i < pattern.length(); ++i) {
        char ch = pattern[i];
        switch (state) {
            case 1:
                switch (ch) {
                    case '<':
                        if (escape) {
                            pattern_buf << '<';
                            escape = false;
                        } else {
                            state = 2;
                        }
                        break;
                    case '>':
                        if (escape) {
                            pattern_buf << '>';
                            escape = false;
                            break;
                        } else {
                            return false;
                        }
                    case '\\':
                        if (escape) {
                            pattern_buf << '\\';
                            escape = false;
                        }
                        else {
                            escape = true;
                        }
                        break;
                    case '/':
                        if (escape)
                            pattern_buf << '/';
                        else
                            pattern_buf << "\\/";
                        break;
                    default:
                        if (escape)
                            return false;
                        pattern_buf << ch;
                }
                break;
            case 2:
                switch (ch) {
                    case '<':
                    case ':':
                    case '>':
                        if (escape) {
                            arg_buf1 << ch;
                            escape = false;
                            break;
                        } else {
                            return false;
                        }
                    case '\\':
                        if (escape) {
                            arg_buf1 << '\\';
                            escape = false;
                        }
                        else {
                            escape = true;
                        }
                        break;
                    default:
                        if (escape)
                            return false;
                        arg_buf1 << ch;
                        state = 3;
                }
                break;
            case 3:
                switch (ch) {
                    case '<':
                        if (escape) {
                            pattern_buf << ch;
                            escape = false;
                            break;
                        } else {
                            return false;
                        }
                    case ':':
                        if (escape) {
                            arg_buf1 << ch;
                            escape = false;
                            break;
                        }
                        state = 4;
                        break;
                    case '>': {
                        if (escape) {
                            arg_buf1 << ch;
                            escape = false;
                            break;
                        }
                        pattern_buf << pattern_string;
                        const std::string &arg_name = arg_buf1.str();
                        arg_buf1.str("");
                        arg_buf2.str("");
                        arg_names.push_back(arg_name);
                        state = 1;
                        break;
                    }
                    case '\\':
                        if (escape) {
                            arg_buf1 << '\\';
                            escape = false;
                        }
                        else {
                            escape = true;
                        }
                        break;
                    default:
                        if (escape)
                            return false;
                        arg_buf1 << ch;
                }
                break;
            case 4:
                switch (ch) {
                    case '<':
                    case '>':
                    case ':':
                        if (escape) {
                            arg_buf2 << ch;
                            escape = false;
                            break;
                        } else {
                            return false;
                        }
                    case '\\':
                        if (escape) {
                            arg_buf2 << '\\';
                            escape = false;
                        }
                        else {
                            escape = true;
                        }
                        break;
                    default:
                        if (escape)
                            return false;
                        arg_buf2 << ch;
                        state = 5;
                }
                break;
            case 5:
                switch (ch) {
                    case '<':
                    case ':':
                        if (escape) {
                            arg_buf2 << ch;
                            escape = false;
                            break;
                        } else {
                            return false;
                        }
                    case '>': {
                        if (escape) {
                            arg_buf2 << ch;
                            escape = false;
                            break;
                        }
                        const std::string &arg_type = arg_buf1.str();
                        const std::string &arg_name = arg_buf2.str();
                        arg_buf1.str("");
                        arg_buf2.str("");
                        if (arg_type == "string")
                            pattern_buf << pattern_string;
                        else if (arg_type == "int")
                            pattern_buf << pattern_int;
                        else if (arg_type == "float")
                            pattern_buf << pattern_float;
                        else
                            pattern_buf << '(' << arg_type << ')';
                        arg_names.push_back(arg_name);
                        state = 1;
                        break;
                    }
                    case '\\':
                        if (escape) {
                            arg_buf2 << '\\';
                            escape = false;
                        }
                        else {
                            escape = true;
                        }
                        break;
                    default:
                        if (escape)
                            return false;
                        arg_buf2 << ch;
                }
                break;
        }
    }
    if (state != 1)
        return false;
    const char *errmsg = NULL;
    int offset = 0;
    pcre *re = pcre_compile(pattern_buf.str().c_str(),
                            0,
                            &errmsg,
                            &offset,
                            NULL);
    if (!re)
        return false;
    this->handlers.push_back(std::forward_as_tuple(re, handler, methods, arg_names));
    return true;
}

RequestHandler Router::route(const std::string &path,
                             HTTPMethod method,
                             std::map<std::string, std::string> &arguments) const {
    const size_t OVecCount = 128;
    for (auto &i: this->handlers) {
        pcre *re = std::get<0>(i);
        if (!re)
            continue;
        const RequestHandler &handler = std::get<1>(i);
        const std::set<HTTPMethod> &methods = std::get<2>(i);
        const std::vector<std::string> &arg_names = std::get<3>(i);
        if (methods.count(method)) {
            int ovector[OVecCount] = {0};
            int rc = pcre_exec(re,
                               0,
                               path.c_str(),
                               path.length(),
                               0,
                               0,
                               ovector,
                               OVecCount);
            if (rc <= 0)
                continue;
            if ((rc - 1) != arg_names.size())
                continue;
            for (size_t i = 1; i < rc; ++i) {
                size_t length = ovector[2*i+1] - ovector[2*i];
                const std::string &arg_value = path.substr(ovector[2*i], length);
                const std::string &arg_name = arg_names[i-1];
                arguments.insert(std::make_pair(arg_name, arg_value));
            }
            return handler;
        }
    }
    auto handler = std::bind(this->error_handler, 404, std::placeholders::_1);
    return handler;
}

void Router::default_error_handler(int code, Connection &conn) {
    printf("HTTP ERROR %d\n", code);
}