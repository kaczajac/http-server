#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__


#include <core_types.h>
#include <core_string.h>

#include <http_headers.h>


enum HttpStatus {
    STATUS_200,
    STATUS_400,
    STATUS_404,
    STATUS_413,
    STATUS_503,
    STATUS_501
};


struct HttpResponse {
    enum HttpStatus status;
    struct HttpHeader headers[HTTP_MAX_HEADER_COUNT];
    u32 header_count;
    string body;
};


extern void httpresponse_send(struct HttpResponse *response, i64 connection);


#endif /* __HTTP_RESPONSE_H__ */
