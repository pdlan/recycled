/**
 * @file
 * @author Falconly members
 * @version 0.1
 *
 * @section DESCRIPTION
 *
 * 定义HTTP连接的接口
 */
#ifndef RECYCLED_INCLUDE_CONNECTION_H
#define RECYCLED_INCLUDE_CONNECTION_H
#include <string>
#include <vector>
#include <set>
#include <map>
#include "recycled/handler.h"

namespace recycled {
/**
 * HTTP方法类型枚举
 */
enum class HTTPMethod {GET, POST, PUT, PATCH, DELETE, HEAD, OPTIONS, Other};

static const std::set<int> StatusCodes = {
    100, 101,
    200, 201, 202, 203, 204, 205, 206,
    300, 301, 302, 303, 304, 305, 307,
    400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414,
    415, 416, 417,
    500, 501, 502, 503, 504, 505
};

static const std::map<int, const char *> StatusReasons = {
    {100, "Continue"},
    {101, "Switching Protocols"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Time-out"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Request Entity Too Large"},
    {414, "Request-URI Too Large"},
    {415, "Unsupported Media Type"},
    {416, "Requested range not satisfiable"},
    {417, "Expectation Failed"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Time-out"},
    {505, "HTTP Version not supported"}
};

typedef std::vector<std::string> SVector;
typedef std::map<std::string, std::string> SSMap;
typedef std::multimap<std::string, std::string> SSMultiMap;

struct UploadFile {
    std::string filename; /**< 文件名 */
    std::string content_type; /**< 文件的Content-Type */
    const char *data; /**< 文件数据 */
    size_t size; /**< 文件大小 */
};

/**
 * 规范HTTP Connection的借口
 */
class Connection {
    public:
        /**
         * 向响应Body输出数据
         *
         * @param data 要输出的数据
         *
         * @param size 输出数据的大小, 以sizeof(char)为单位
         *
         * @return 输出成功返回true, 否则返回false
         */
        virtual bool write(const char *data, size_t size) = 0;
         /**
         * 向响应Body输出字符串
         *
         * @param str 要输出的字符串
         *
         * @return 输出成功返回true, 否则返回false
         */
        virtual bool write(const std::string &str) = 0;
        /**
         * 设置HTTP响应状态
         *
         * @param status_code 状态码
         *
         * @param reason 原因短语
         *
         * @return 设置状态成功返回true, 否则返回false
         */
        virtual bool set_status(int status_code, const std::string &reason="") = 0;
        /**
         * 取得HTTP请求方法
         *
         * @return HTTP请求方法
         */
        virtual HTTPMethod get_method() const = 0;
        /**
         * 取得请求Body
         *
         * @return 请求Body的指针
         */
        virtual const char * get_body() const = 0;
        /**
         * 取得请求Body的大小
         *
         * @return 请求Body的大小(以sizeof(char)为单位)
         */
        virtual size_t get_body_size() const = 0;
        /**
         * 取得上传的文件
         *
         * @param name 上传文件的名字(即<input>的name, 非文件名)
         *
         * @return 上传文件(若无此文件返回空指针)
         */
        virtual const UploadFile * get_file(const std::string &name) const = 0;
        /**
         * 取得HTTP请求路径
         *
         * @return HTTP请求路径
         */
        virtual std::string get_path() const = 0;
         /**
         * 取得HTTP请求URI
         *
         * @return HTTP请求URI
         */
        virtual std::string get_uri() const = 0;
         /**
         * 取得URL Query参数
         *
         * @param key 参数名
         *
         * @return 参数值(若一个参数名有多个参数值返回最后一个, 无此参数返回空字符串)
         */
        virtual std::string get_query_argument(const std::string &key) const = 0;
        /**
         * 取得Body参数
         *
         * @param key 参数名
         *
         * @return 参数值(若一个参数名有多个参数值返回最后一个, 无此参数返回空字符串)
         */
        virtual std::string get_body_argument(const std::string &key) const = 0;
        /**
         * 取得URL Query及Body参数
         * 不包含Path参数!
         *
         * @param key 参数名
         *
         * @return 参数值(优先考虑URL Query参数, 无此参数返回空字符串)
         */
        virtual std::string get_argument(const std::string &key) const = 0;
        /**
         * 取得Path参数.
         * 如请求路径为/article/hello, 路由匹配模式为/article/<id>,
         * 则get_path_argument("id")返回值为"hello"
         *
         * @param key 参数名
         *
         * @return 参数值(无此参数返回空字符串)
         */
        virtual std::string get_path_argument(const std::string &key) const = 0;
        /**
         * 取得HTTP请求头
         *
         * @param key 请求头名(若一个请求头名有多个值返回最后一个)
         *
         * @return 请求头值(无此请求头返回空字符串)
         */
        virtual std::string get_header(const std::string &key) const = 0;
        /**
         * 取得Cookie
         *
         * @param key Cookie名
         *
         * @return Cookie值(无此Cookie返回空字符串)
         */
        virtual std::string get_cookie(const std::string &key) const = 0;
        /**
         * 取得所有指定名字的URL Query参数.
         * 如URL Query为?arg=val1&arg=val2, 则get_query_arguments("arg")返回值为
         * {"val1", "val2"}
         *
         * @param key 参数名
         *
         * @return 参数值(0个或多个)
         */
        virtual SVector get_query_arguments(const std::string &key) const = 0;
        /**
         * 取得所有指定名字的Body参数
         *
         * @param key 参数名
         *
         * @return 参数值(0个或多个)
         */
        virtual SVector get_body_arguments(const std::string &key) const = 0;
        /**
         * 取得所有指定名字的URL Query及Body参数
         *
         * @param key 参数名
         *
         * @return 参数值(0个或多个, 同时包含URL Query及Body参数值)
         */
        virtual SVector get_arguments(const std::string &key) const = 0;
        /**
         * 取得所有Path参数
         *
         * @return Path参数Map的引用(不可修改)
         */
        virtual const SSMap & get_path_arguments() const = 0;
        /**
         * 取得所有HTTP请求头
         *
         * @return 请求头Map
         */
        virtual const SSMap & get_headers() const = 0;
        /**
         * 取得所有Cookie
         *
         * @return Cookies Map
         */
        virtual const SSMap & get_cookies() const = 0;
        /**
         * 取得所有Path参数
         *
         * @return Path参数Map的引用(可修改)
         */
        virtual SSMap & get_path_arguments() = 0;
        /**
         * 设置错误处理器
         *
         * @param handler 错误处理器
         *
         * @return 设置成功返回true, 否则返回false
         */
        virtual bool set_error_handler(const ErrorHandler &handler) = 0;
        /**
        * 设置Cookie
        *
        * @param key Cookie名字
        *
        * @param value Cookie值
        *
        * @param secure 是否启用安全Cookie
        *
        * @param expires 过期时间(以秒为单位, 默认为1小时)
        *
        * @param domain 域名(可为空)
        *
        * @param path 路径(默认为"/")
        *
        * @param http_only 是否仅限HTTP(s)使用Cookie
        * 若为true则javascript不能读写此Cookie
        *
        * @return 设置成功返回true, 否则返回false
         */
        virtual bool set_cookie(const std::string &key,
                                const std::string &value,
                                bool secure = false,
                                time_t expires=3600,
                                const std::string &domain="",
                                const std::string &path="/",
                                bool http_only = false) = 0;
        /**
        * 删除指定Cookie
        *
        * @param key Cookie名字
        *
        * @param domain 域名(若为空则删除所有)
        *
        * @param path 路径(若为空则删除所有)
        *
        * @return 删除成功返回true, 否则返回false
         */
        virtual bool remove_cookie(const std::string &key,
                                   const std::string &domain="",
                                   const std::string &path="") = 0;
        /**
        * 删除所有Cookies
         */
        virtual void clear_cookies() = 0;
        /**
         * 增加HTTP响应头
         *
         * @param key 响应头名
         *
         * @param value 响应头值
         *
         * @return 增加成功返回true, 否则返回false
         */
        virtual bool add_header(const std::string &key, const std::string &value) = 0;
         /**
         * 删除HTTP响应头
         *
         * @param key 要删除的响应头名
         *
         * @return 删除成功返回true, 否则返回false
         */
        virtual bool remove_header(const std::string &key) = 0;
        /**
         * 删除所有HTTP响应头
         */
        virtual void clear_headers() = 0;
        /**
         * 将缓冲区的内容全部发送并清空缓冲区.
         * flush后将不能增加, 删除响应头,
         * 也不能使用重定向、发送错误或修改响应状态
         *
         * @return 发送成功返回true, 否则返回false
         */
        virtual bool flush() = 0;
        /**
         * 完成响应.
         * finish后将不能增加, 删除响应头, 不能向Body输出数据
         * 也不能使用重定向、发送错误或修改响应状态
         *
         * @return 完成响应成功返回true, 否则返回false
         */
        virtual void finish() = 0;
        /**
         * 发送错误
         *
         * @param status 错误代码, 默认为500
         *
         * @return 发送成功返回true, 否则返回false
         */
        virtual bool send_error(int status=500) = 0;
         /**
         * 重定向到指定的URL
         *
         * @param url 重定向的目标URL
         *
         * @param status 重定向代码, 默认为302
         *
         * @return 重定向成功返回true, 否则返回false
         */
        virtual bool redirect(const std::string &url, int status=302) = 0;
         /**
         * 判断是否已经完成相应
         *
         * @return 响应已完成返回true, 否则返回false
         */
        virtual bool is_finished() const = 0;
};
}
#endif