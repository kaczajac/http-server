#ifndef __HTTP_HEADERS_H__
#define __HTTP_HEADERS_H__


#include <core_types.h>
#include <core_string.h>


#ifndef HTTP_MAX_HEADER_COUNT
#   define HTTP_MAX_HEADER_COUNT 20
#endif


struct HttpHeader {
    string key;
    string value;
};


extern i32 httpheaders_get(struct HttpHeader *header, u32 header_count, string header_key);
extern void httpheaders_set(struct HttpHeader *header, u32 *header_count, string header_key, string header_val);


#endif /* __HTTP_HEADERS_H__ */
