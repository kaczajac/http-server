#ifndef __HTTP_REQUEST_H_
#define __HTTP_REQUEST_H_

#include <core_types.h>
#include <core_allocator.h>

#include <http_headers.h>

#ifndef HTTP_REQMAXSIZE
#   define HTTP_REQMAXSIZE KiB(4)
#endif

enum HttpMethod {
    HTTP_GET = 1,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_INVALID // reserved for parsing when method wasn't recognized
};

struct HttpRequestLine {
    enum HttpMethod method;
    string target;
    string protocol;
};

struct HttpRequest {
    struct HttpRequestLine request_line;
    struct HttpHeader headers[HTTP_MAX_HEADER_COUNT];
    u32 header_count;
    string body;
};

extern string http_method_tostr(enum HttpMethod method);
extern enum HttpMethod http_method_fromstr(string str);

#endif /* __HTTP_REQUEST_H_ */
