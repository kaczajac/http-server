#include <http_request.h>


string http_method_tostr(enum HttpMethod method) {
    
    string result;
    switch (method) {

        case HTTP_GET: {
            result = str_fromlit("GET");
            break;
        }
        
        case HTTP_POST: {
            result = str_fromlit("POST");
            break;
        }

        case HTTP_PUT: {
            result = str_fromlit("PUT");
            break;
        }

        case HTTP_DELETE: {
            result = str_fromlit("DELETE");
            break;
        }

        default:
            result = (string) {};

    }
    
    return result;

}


enum HttpMethod http_method_fromstr(string str) {

    if (str_equals(str, str_fromlit("GET"))) { return HTTP_GET; }

    else if (str_equals(str, str_fromlit("POST"))) { return HTTP_POST; }

    else if (str_equals(str, str_fromlit("PUT"))) { return HTTP_PUT; }

    else if (str_equals(str, str_fromlit("DELETE"))) { return HTTP_DELETE; }

    return HTTP_INVALID;

}
