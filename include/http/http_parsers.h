#ifndef __HTTP_PARSERS_H__
#define __HTTP_PARSERS_H__

#include <core_types.h>
#include <core_string.h>

#include <http_request.h>

extern boolean http_parse_request(i64 connection, struct HttpRequest *request);

#endif /* __HTTP_PARSERS_H__ */
