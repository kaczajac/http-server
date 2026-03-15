#include <http_headers.h>


i32 httpheaders_get(struct HttpHeader *header, u32 header_count, string header_key) {

    i32 result = -1;
    for (u32 header_id = 0; header_id < header_count; header_id++) {
        if (str_equals(header[header_id].key, header_key)) {
            result = header_id;
            break;
        }
    }
    return result;

}


void httpheaders_set(struct HttpHeader *header, u32 *header_count, string header_key, string header_val) {
   
    i32 header_idx = httpheaders_get(header, *header_count, header_key);
    if (header_idx != -1) {

        header[header_idx].value = header_val;

    } else if (*header_count <= HTTP_MAX_HEADER_COUNT) {

        (*header_count)++;
        header[*header_count - 1].key     = header_key;
        header[*header_count - 1].value   = header_val;

    }

}
