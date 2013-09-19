//
//  obj-http.h
//  obj-http
//
//  Created by Paul Jara on 2013-09-17.
//
//

#ifndef obj_http_obj_http_h
#define obj_http_obj_http_h

#include <uv.h>
#include "http_parser.h"

typedef struct {
    uint64_t id;
    CFMutableArrayRef header_keys;
    CFMutableArrayRef header_values;
    CFMutableDictionaryRef headers;
    CFStringRef url;
} http_request_t;

int PJAPIServerInit(CFDictionaryRef settings);
void PJAPIResponseDataSourceInit(CFDictionaryRef (*CreateHeaders)(http_request_t *request, CFIndex contentLength), CFDictionaryRef (*CreateBody)(http_request_t *request));
void PJAPIResponseDelegateInit(void (*BeginResponse)(), void (*EndResponse)());

#endif
