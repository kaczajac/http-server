#include <http_parsers.h>
#include <http_headers.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>


enum HttpRequestLineParseState {
    PARSE_METHOD,       // Expecting http method to be parsed next
    PARSE_TARGET,       // Ditto but expecting target
    PARSE_PROTOCOL,     // Ditto but expecting http protocol
    PARSE_ACCEPT,       // Parsing is finished and is successful
    PARSE_REJECT        // Parsing is finished and is invalid
};


struct MetadataReadInfo {
    u8      *metadata_start;
    u8      *metadata_end;
    u32     total_bytes;
};


struct BodyReadInfo {
    u8      *body_start;
    u32     body_size;
};


static boolean http_parse_requestline(struct HttpRequestLine *request_line, string raw_request_line);
static boolean http_parse_headers(struct HttpHeader *headers, u32 *header_count, string raw_headers);
static struct MetadataReadInfo http_read_metadata(i32 client_fd, u8 *buffer, u32 buffer_size);
static struct BodyReadInfo http_read_body(struct HttpHeader *header, u32 header_count, i64 connection, struct MetadataReadInfo *info);
#define http_parse_body(body, info) (body) = (string) { .str = (info)->body_start, .size = (info)->body_size }


static boolean http_parse_requestline(struct HttpRequestLine *request_line, string raw_request_line) {

    struct StringIterator iterator          = striter_new(raw_request_line, str_fromlit("\r\n "));
    enum HttpRequestLineParseState state    = PARSE_METHOD;

    struct HttpRequestLine temp_requestline;

    while (striter_hasnext(&iterator)) {

        string str = striter_next(&iterator);
        switch (state) {

            case PARSE_METHOD: {

                enum HttpMethod parsed_str = http_method_fromstr(str);
                if (parsed_str == HTTP_INVALID) {
                    state = PARSE_REJECT;
                } else {
                    temp_requestline.method = parsed_str;
                    state = PARSE_TARGET;
                }

                break;

            };

            case PARSE_TARGET: {

                temp_requestline.target = str;
                state = PARSE_PROTOCOL;
                break;

            };

            case PARSE_PROTOCOL: {

                if (str_equals(str_fromlit("HTTP/1.1"), str)) {
                    temp_requestline.protocol = str;
                    state = PARSE_ACCEPT;
                } else {
                    state = PARSE_REJECT;
                }

                break;

            };

            /*
             * If we reach these cases that means
             * we have a string to parse and we've
             * reached a state where no more strings
             * are expected, so we should signal
             * parsing failure
             */
            case PARSE_ACCEPT:
            case PARSE_REJECT:
            default: {
                return FALSE;
            };

        };

    }

    if (state != PARSE_ACCEPT) {
        return FALSE;
    }

    request_line->method    = temp_requestline.method;
    request_line->target    = temp_requestline.target;
    request_line->protocol  = temp_requestline.protocol;

    return TRUE;

}


static boolean http_parse_headers(struct HttpHeader *headers, u32 *header_count, string raw_headers) {
   
    struct StringIterator iterator = striter_new(raw_headers, str_fromlit("\r\n"));
    struct HttpHeader temp_headers[HTTP_MAX_HEADER_COUNT];
    memset(temp_headers, 0, sizeof(temp_headers));

    u32 header_counter = 0;

    while (striter_hasnext(&iterator) && header_counter < HTTP_MAX_HEADER_COUNT) {

        string raw_header = striter_next(&iterator);
        i32 colon = str_findchar(raw_header, ':');
        if (colon == -1) {
            // ignore bad headers
            continue;
        }

        string key      = str_substr(raw_header, 0, colon);
        string value    = str_substr(raw_header, (colon + 1) + 1, raw_header.size);
        httpheaders_set(temp_headers, &header_counter, key, value);

    }

    if (header_counter >= HTTP_MAX_HEADER_COUNT) {
        return FALSE;
    }

    *header_count = header_counter;
    for (u32 i = 0; i < header_counter; i++) {
        headers[i] = temp_headers[i]; 
    }

    return TRUE;

};


static struct MetadataReadInfo http_read_metadata(i32 client_fd, u8 *buffer, u32 buffer_size) {

    memset(buffer, 0, buffer_size);

    u32 offset  = 0;
    i32 bytes   = 0;
    const u32 block_size = 8;
    
    struct pollfd fd;
    fd.fd       = client_fd;
    fd.events   = POLLIN;

    boolean metadata_read_status = FALSE;

    for (;;) {

        i32 availability = poll(&fd, 1, 1000);

        if (availability > 0) {

            bytes = read(client_fd, (buffer + offset), block_size);

            if (bytes > 0) {

                u32 new_offset = offset + bytes;
                if (new_offset > buffer_size) {
                    break;
                }

                offset = new_offset;

                if (strstr((const char *)buffer, "\r\n\r\n") != NULL) {
                    metadata_read_status = TRUE;
                    break;
                }

            } else {

                close(client_fd);
                break;

            }

        } else {
           break; 
        }

    }

    if (metadata_read_status) {
        return (struct MetadataReadInfo) {
            .metadata_start = buffer,
            .metadata_end   = (u8*) strstr((const char *) buffer, "\r\n\r\n") + 4,
            .total_bytes    = strlen((const char *) buffer) 
        };
    }

    return (struct MetadataReadInfo) {};

}


static struct BodyReadInfo http_read_body(struct HttpHeader *header, u32 header_count, i64 connection, struct MetadataReadInfo *info) {

    struct BodyReadInfo body_info = {};

    i32 content_length = httpheaders_get(
        header, 
        header_count, 
        str_fromlit("Content-Length")
    );

    if (content_length == -1) {
        return body_info;
    }

    u8 *body_start  = info->metadata_end;
    u32 body_read   = (info->metadata_start + info->total_bytes) - body_start;
    i32 body_size   = (i32) atoi((const char *) header[content_length].value.str); 
    if (body_size == -1) {
        return body_info;
    }

    u32 body_to_read = body_size - body_read;
    if (body_to_read > HTTP_REQMAXSIZE - info->total_bytes) {
        return body_info;
    }

    i32 bytes = read(connection, body_start + body_read, body_to_read);
    if (bytes > 0) {
        body_info.body_start = body_start;
        body_info.body_size  = body_size;
        return body_info;
    }

    return body_info;

}


boolean http_parse_request(i64 connection, struct HttpRequest *request) {

    /*
     * Read raw request metadata into a buffer
     * (request line and headers).
     * Body will be left for after the metadata
     * is parsed.
     */

    u8 request_lit[HTTP_REQMAXSIZE + 1];

    struct MetadataReadInfo metadata_info = http_read_metadata(connection, request_lit, HTTP_REQMAXSIZE);
    if (metadata_info.total_bytes == 0) {
        return FALSE;
    }

    string metadata_str = (string) {
        .str    = metadata_info.metadata_start,
        .size   = metadata_info.metadata_end - metadata_info.metadata_start
    };

    /*
     * The delimiter sets the end of the request line
     * part and the beginning of the headers (fields).
     */

    i32 delimiter = str_findchar(metadata_str, '\n');
    if (delimiter == -1) {
        return FALSE;
    }

    string request_line_str = str_substr(metadata_str, 0, delimiter + 1);
    string headers_str      = str_substr(metadata_str, delimiter + 1, metadata_str.size);

    /*
     * Parse request metadata into a temporary
     * request object
     */

    struct HttpRequest temp_request = { 0 };

    boolean metadata_parse_status = 
        http_parse_requestline(
            &(temp_request.request_line), 
            request_line_str
        ) & 
        http_parse_headers(
            (struct HttpHeader *) &(temp_request.headers), 
            &(temp_request.header_count), 
            headers_str
        );

    if (metadata_parse_status) {

        request->request_line   = temp_request.request_line;
        request->header_count   = temp_request.header_count;
        memcpy(request->headers, temp_request.headers, request->header_count * sizeof(struct HttpHeader));

        /*
         * Read and parse optional body
         * that was left 
         */

        struct BodyReadInfo body_info = http_read_body(
            request->headers, 
            request->header_count, 
            connection, 
            &metadata_info
        );

        if (body_info.body_size != 0) {
            http_parse_body(request->body, &body_info);
        }

    }
   
    return metadata_parse_status;

}
