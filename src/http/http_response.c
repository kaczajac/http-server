#include <http_response.h>

#include <stdio.h>


void httpresponse_send(struct HttpResponse *response, i64 connection) {

    string response_line;
    boolean bad_status = FALSE;

    switch (response->status) {
        case STATUS_200: {
            response_line = str_fromlit("HTTP/1.1 200 OK");
            break;
        };
        case STATUS_400: {
            response_line = str_fromlit("HTTP/1.1 400 Bad Request");
            break;
        };
        case STATUS_404: {
            response_line = str_fromlit("HTTP/1.1 404 Not Found");
            break;
        };
        case STATUS_413: {
            response_line = str_fromlit("HTTP/1.1 413 Entity Too Large");
            break;
        };
        case STATUS_503: {
            response_line = str_fromlit("HTTP/1.1 503 Service Unavailable");
            break;
        };
        default:
            bad_status = TRUE;
    }

    if (!bad_status) {

        // Response start line
        dprintf(connection, "%.*s\r\n", str_fmt(response_line));

        // Headers (fields)
        for (u8 header_id = 0; header_id < response->header_count; header_id++) {
            dprintf(
                connection,
                "%.*s: %.*s\r\n", 
                str_fmt(response->headers[header_id].key), 
                str_fmt(response->headers[header_id].value)
            );
        }

        // Blank line, body separator
        dprintf(connection, "\r\n");

        // Optional body
        i32 content_length = httpheaders_get(
            response->headers, 
            response->header_count, 
            str_fromlit("Content-Length")
        );
        if (content_length != -1) {
            dprintf(connection, "%.*s", str_fmt(response->body));
        }

    }

}
