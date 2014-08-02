#include <stdio.h>
#include <stdint.h>
#include <string>
#include <map>
#include <functional>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include "recycled/httpconnection.h"

using namespace recycled;

HTTPConnection::HTTPConnection(evhttp_request *evreq):
    evreq(evreq), input_buffer(NULL), output_buffer(NULL) {
    this->output_buffer = evbuffer_new();
    this->set_status(200);
}

HTTPConnection::~HTTPConnection() {
    if (this->input_buffer)
        evbuffer_free(this->input_buffer);
    if (this->output_buffer)
        evbuffer_free(this->output_buffer);
}

bool HTTPConnection::write(const char *data, size_t length) {
    if (!this->output_buffer)
        return false;
    if (evbuffer_add(this->output_buffer, data, length) != 0)
        return false;
    return true;
}

bool HTTPConnection::write(const std::string &str) {
    return this->write(str.c_str(), str.length());
}

bool HTTPConnection::set_status(int status_code, const std::string &reason) {
    if (!StatusCodes.count(status_code)) {
        return false;
    }
    this->status_code = status_code;
    if (reason == "") {
        auto it = StatusReasons.find(status_code);
        if (it != StatusReasons.end()) {
            this->status_reason = it->second;
        } else {
            return false;
        }
    } else {
        this->status_reason = reason;
    }
    return true;
}

void HTTPConnection::finish() {
    if (!this->output_buffer)
        return;
    evhttp_send_reply(this->evreq, this->status_code,
                      this->status_reason.c_str(), this->output_buffer);
}