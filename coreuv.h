//
//  coreuv.h
//  CoreUV
//
//  Created by Paul Jara on 2013-09-17.
//
//

#ifndef obj_http_obj_http_h
#define obj_http_obj_http_h

#include <CoreFoundation/CoreFoundation.h>
#include <uv.h>
#include "http_parser.h"

typedef struct {
  uint64_t id;
  CFMutableArrayRef header_keys;
  CFMutableArrayRef header_values;
  CFMutableDictionaryRef headers;
  CFStringRef url;
} http_request_t;

enum {
  kCUVResponseTransformJSON = 0,
  kCUVResponseTransformHTML = 1,
  kCUVResponseTransformXML  = 2
};
typedef uint64_t CUVResponseTransform;

enum {
  kCUVResponseReplaceNothing      = 0,
  kCUVResponseReplaceHTMLEntityNames    = 1 << 0,
  kCUVResponseReplaceHTMLEntityNumbers  = 1 << 1,
};
typedef uint64_t CUVResponseReplacements;

int CoreUVInit(CFDictionaryRef settings);
void CoreUVResponseDataSourceInit(CFDictionaryRef (*CreateHeaders)(http_request_t *request, CFIndex contentLength), CFDictionaryRef (*CreateBody)(http_request_t *request));
void CoreUVResponseDelegateInit(void (*BeginResponse)(CUVResponseTransform *transform, CUVResponseReplacements *replace), void (*EndResponse)());

#endif
