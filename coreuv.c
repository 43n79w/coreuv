//
//  coreuv.c
//  CoreUV
//
//  Created by Paul Jara on 2013-09-17.
//
//

#include <dispatch/dispatch.h>
#include <unistd.h>
#include <stdlib.h>
// #include "CoreJSON.h"
#include "coreuv.h"
#include "coreuv-utils.h"

typedef struct {
  CFMutableDictionaryRef headers;
  CUVResponseTransform transform;
  CUVResponseReplacements replacements;
  CUVHTTPResponseStatus status;
} http_response_t;

struct client_t {
  uv_tcp_t handle;
  http_parser parser;
  uv_write_t write_req;
  http_request_t request;
  http_response_t response;
  uv_buf_t response_buffer;
};

typedef struct client_t client_t;

typedef int (*handle_request_t)(uv_work_t *work);

void after_write(uv_write_t *req, int status);
void on_response_send(void *context);

#pragma mark globals
static uv_loop_t *default_loop;
static uv_tcp_t server;
static http_parser_settings parser_settings;
static uint64_t request_num = 1;
static uv_async_t async;
static dispatch_queue_t response_queue;

void (*on_response_begin_ptr)(CUVResponseTransform * const transform, CUVResponseReplacements * const replace);
void (*on_response_end_ptr)(CUVHTTPResponseStatus status);
CUVHTTPResponseStatus (*on_response_status_ptr)();

CFDictionaryRef (*headers_ptr)(http_request_t const * const request, CFIndex contentLength);
CFDictionaryRef (*body_ptr)(http_request_t const * const request, CUVHTTPResponseStatus * const status);

void begin_response(client_t *c) {
  on_response_begin_ptr(&c->response.transform, &c->response.replacements);
}

void end_response(CUVHTTPResponseStatus status) {
  on_response_end_ptr(status);
}

#pragma mark worker functionality

void handle_async_response(void *context) {
  CFMutableArrayRef responses = (CFMutableArrayRef) async.data;
  CFArrayAppendValue(responses, context);
  uv_async_send(&async);
}

void __handle_request_dispatch_json(void *context) {

  client_t *c = (client_t *) context;

  CFMutableStringRef response_buffer;
  CFMutableStringRef headers_buffer;
  
  /**
   * We generate the body before the headers so we can set the Content-Length header (though this isn't mandatory).
   * Actually setting the Content-Length is left to the PJAPIResponseDataSource functionality to accomplish.
   */
  CFDictionaryRef body = (*body_ptr)(&c->request, &c->response.status);
  response_buffer = CFStringCreateMutable(NULL, 1000);
  
  CFErrorRef error = NULL;
  CFStringRef body_replace;
  // CFStringRef json = JSONCreateString(NULL, body, kJSONWriteOptionsDefault, &error);
  CFStringRef json = CFSTR("{\"foo\":\"bar\"}");

  if ((c->response.replacements & kCUVResponseReplaceHTMLEntityNames) == kCUVResponseReplaceHTMLEntityNames) {
    body_replace = CUVStringCreateFromHTMLEntityNameString(json);
    //CFRelease(json);
  }
  else if ((c->response.replacements & kCUVResponseReplaceHTMLEntityNumbers) == kCUVResponseReplaceHTMLEntityNumbers) {
    body_replace = CUVStringCreateFromHTMLEntityNumberString(json);
    //CFRelease(json);
  }
  else {
    body_replace = json;
  }
  
  if (body_replace) {
      CFStringAppend(response_buffer, body_replace);
      CFRelease(body_replace);
  }
  
  CFIndex len_chars = CFStringGetLength(response_buffer);
  CFIndex len_bytes_body;
  CFIndex num_converted_chars;
  num_converted_chars = CFStringGetBytes(response_buffer, CFRangeMake(0, len_chars), kCFStringEncodingUTF8, '?', FALSE, NULL, 0, &len_bytes_body);
  
  if (num_converted_chars == 0) {
      // converted nothing
  }
  
  CFDictionaryRef headers = (*headers_ptr)(&c->request, len_bytes_body);
  
  CFIndex idx = CFDictionaryGetCount(headers);
  CFTypeRef *keys = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);
  CFTypeRef *values = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);

  CFDictionaryGetKeysAndValues(headers, (const void **) keys, (const void **) values);
  headers_buffer = CFStringCreateMutable(NULL, 1000);

  CFStringRef statusString = CFStringCreateWithFormat(NULL, NULL, CFSTR("HTTP/1.1 %llu \r\n"), c->response.status);
  CFStringAppend(headers_buffer, statusString);
  CFRelease(statusString);
  
  for (int i = 0; i < idx; i++) {
      CFTypeRef *key = (CFTypeRef *) keys[i];
      CFTypeRef *value = (CFTypeRef *) values[i];
      CFStringAppend(headers_buffer, (CFStringRef) key);
      CFStringAppend(headers_buffer, CFSTR(": "));
      CFStringAppend(headers_buffer, (CFStringRef) value);
      CFStringAppend(headers_buffer, CFSTR("\r\n"));
  }
  
  free(keys);
  free(values);
  
  CFStringAppend(headers_buffer, CFSTR("\r\n"));
  CFStringAppend(headers_buffer, response_buffer);
  
  CFIndex len_bytes_header;
  len_chars = CFStringGetLength(headers_buffer);
  num_converted_chars = CFStringGetBytes(headers_buffer, CFRangeMake(0, len_chars), kCFStringEncodingUTF8, '?', FALSE, NULL, 0, &len_bytes_header);
  
  c->response_buffer = uv_buf_init((char *) malloc(len_bytes_header + 1), (uint32_t) len_bytes_header + 1);
  num_converted_chars = CFStringGetBytes(headers_buffer, CFRangeMake(0, len_chars), kCFStringEncodingUTF8, '?', FALSE, (UInt8 *) c->response_buffer.base, len_bytes_header, &len_bytes_header);
  
  if (num_converted_chars == 0) {
      // converted nothing
  }

  dispatch_async_f(response_queue, c, handle_async_response);
 
  CFRelease(headers_buffer);
  CFRelease(response_buffer);
  CFRelease(headers);
  CFRelease(body);
}

void __handle_request_dispatch_html(void *context) {
  client_t *c = (client_t *) context;
  
  CFMutableStringRef response_buffer;
  CFIndex num_chars_response_buffer;
  CFIndex num_bytes_response_buffer;
  
  CFMutableStringRef headers_buffer;
  CFIndex num_chars_headers_buffer;
  CFIndex num_bytes_headers_buffer;
  
  CFStringRef body_replace;
  CFDictionaryRef body;
  CFStringRef xml;
  
  CFIndex num_chars_converted;
  CFIndex max_bytes_buf_converted = 100;
  UInt8 buf_converted[max_bytes_buf_converted];
  CFIndex num_bytes_buf_converted;
  CFIndex total_num_bytes_converted;
  
  CFRange range_to_convert;
  
  CFDictionaryRef headers;
  CFIndex num_headers;
  CFTypeRef *keys;
  CFTypeRef *values;
  CFTypeRef *key;
  CFTypeRef *value;
  
  CFStringRef statusString;
  
  /**
   * We generate the body before the headers so we can set the Content-Length header (though this isn't mandatory).
   * Actually setting the Content-Length is left to the PJAPIResponseDataSource functionality to accomplish.
   */
  body = (*body_ptr)(&c->request, &c->response.status);
  response_buffer = CFStringCreateMutable(NULL, 0);

  CFStringRef html = CUVStringCreateHTMLFromDictionary(body);
  
  if ((c->response.replacements & kCUVResponseReplaceHTMLEntityNames) == kCUVResponseReplaceHTMLEntityNames) {
    body_replace = CUVStringCreateFromHTMLEntityNameString(html);
    CFRelease(html);
  }
  else if ((c->response.replacements & kCUVResponseReplaceHTMLEntityNumbers) == kCUVResponseReplaceHTMLEntityNumbers) {
    body_replace = CUVStringCreateFromHTMLEntityNumberString(html);
    CFRelease(html);
  }
  else {
    body_replace = html;
  }
  
  if (body_replace) {
    CFStringAppend(response_buffer, body_replace);
    CFRelease(body_replace);
  }
  
  num_chars_response_buffer = CFStringGetLength(response_buffer);
  
  total_num_bytes_converted = 0;
  
  range_to_convert = CFRangeMake(0, num_chars_response_buffer);
  
  while (range_to_convert.length > 0) {
    
    num_chars_converted = CFStringGetBytes(response_buffer, range_to_convert, kCFStringEncodingUTF8, '?', false, buf_converted, max_bytes_buf_converted, &num_bytes_buf_converted);
    total_num_bytes_converted += num_bytes_buf_converted;
    
    if (num_chars_converted == 0) break;
    
    range_to_convert.location += num_chars_converted;
    range_to_convert.length -= num_chars_converted;
  };
  
  num_bytes_response_buffer = total_num_bytes_converted;
  
  headers = (*headers_ptr)(&c->request, num_bytes_response_buffer);
  
  num_headers = CFDictionaryGetCount(headers);
  keys = (CFTypeRef *) malloc(sizeof(CFTypeRef) * num_headers);
  values = (CFTypeRef *) malloc(sizeof(CFTypeRef) * num_headers);
  
  CFDictionaryGetKeysAndValues(headers, (const void **) keys, (const void **) values);
  
  headers_buffer = CFStringCreateMutable(NULL, 0);
  
  statusString = CFStringCreateWithFormat(NULL, NULL, CFSTR("HTTP/1.1 %llu \r\n"), c->response.status);
  CFStringAppend(headers_buffer, statusString);
  CFRelease(statusString);
  
  for (int i = 0; i < num_headers; i++) {
    key = (CFTypeRef *) keys[i];
    value = (CFTypeRef *) values[i];
    CFStringAppend(headers_buffer, (CFStringRef) key);
    CFStringAppend(headers_buffer, CFSTR(": "));
    CFStringAppend(headers_buffer, (CFStringRef) value);
    CFStringAppend(headers_buffer, CFSTR("\r\n"));
  }
  
  free(keys);
  free(values);
  
  CFStringAppend(headers_buffer, CFSTR("\r\n"));
  CFStringAppend(headers_buffer, response_buffer);
  
  num_chars_headers_buffer = CFStringGetLength(headers_buffer);
  
  total_num_bytes_converted = 0;
  range_to_convert = CFRangeMake(0, num_chars_headers_buffer);
  
  while (range_to_convert.length > 0) {
    
    num_chars_converted = CFStringGetBytes(headers_buffer, range_to_convert, kCFStringEncodingUTF8, '?', false, buf_converted, max_bytes_buf_converted, &num_bytes_buf_converted);
    
    total_num_bytes_converted += num_bytes_buf_converted;
    
    if (num_chars_converted == 0) break;
    
    range_to_convert.location += num_chars_converted;
    range_to_convert.length -= num_chars_converted;
  };
  
  num_bytes_headers_buffer = total_num_bytes_converted;
  
  c->response_buffer = uv_buf_init((char *) malloc(sizeof(UInt8) * (num_bytes_headers_buffer + num_bytes_response_buffer)), num_bytes_headers_buffer + num_bytes_response_buffer);
  
  num_chars_converted = CFStringGetBytes(headers_buffer, CFRangeMake(0, CFStringGetLength(headers_buffer)), kCFStringEncodingUTF8, '?', false, (UInt8 *) c->response_buffer.base, num_bytes_headers_buffer, NULL);
  num_chars_converted = CFStringGetBytes(response_buffer, CFRangeMake(0, CFStringGetLength(response_buffer)), kCFStringEncodingUTF8, '?', false, (UInt8 *) &c->response_buffer.base[num_bytes_headers_buffer], num_bytes_response_buffer, NULL);
  
  dispatch_async_f(response_queue, c, handle_async_response);
  
  CFRelease(headers_buffer);
  CFRelease(response_buffer);
  CFRelease(headers);
  CFRelease(body);
}

void __handle_request_dispatch_xml(void *context) {
  client_t *c = (client_t *) context;

  CFMutableStringRef response_buffer;
  CFIndex num_chars_response_buffer;
  CFIndex num_bytes_response_buffer;

  CFMutableStringRef headers_buffer;
  CFIndex num_chars_headers_buffer;
  CFIndex num_bytes_headers_buffer;

  CFStringRef body_replace;
  CFDictionaryRef body;
  CFStringRef xml;
  
  CFIndex num_chars_converted;
  CFIndex max_bytes_buf_converted = 100;
  UInt8 buf_converted[max_bytes_buf_converted];
  CFIndex num_bytes_buf_converted;
  CFIndex total_num_bytes_converted;

  CFRange range_to_convert;

  CFDictionaryRef headers;
  CFIndex num_headers;
  CFTypeRef *keys;
  CFTypeRef *values;
  CFTypeRef *key;
  CFTypeRef *value;
  
  CFStringRef statusString;
  
  /**
   * We generate the body before the headers so we can set the Content-Length header (though this isn't mandatory).
   * Actually setting the Content-Length is left to the PJAPIResponseDataSource functionality to accomplish.
   */
  body = (*body_ptr)(&c->request, &c->response.status);
  response_buffer = CFStringCreateMutable(NULL, 0);
  
  /*CFErrorRef error = NULL;*/
  xml = CUVStringCreateXMLFromDictionary(body);
  
  if ((c->response.replacements & kCUVResponseReplaceHTMLEntityNames) == kCUVResponseReplaceHTMLEntityNames) {
    body_replace = CUVStringCreateFromHTMLEntityNameString(xml);
    CFRelease(xml);
  }
  else if ((c->response.replacements & kCUVResponseReplaceHTMLEntityNumbers) == kCUVResponseReplaceHTMLEntityNumbers) {
    body_replace = CUVStringCreateFromHTMLEntityNumberString(xml);
    CFRelease(xml);
  }
  else {
    body_replace = xml;
  }
  
  if (body_replace) {
    CFStringAppendCString(response_buffer, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n", kCFStringEncodingUTF8);
    CFStringAppend(response_buffer, body_replace);
    CFStringFindAndReplace(response_buffer, CFSTR("&"), CFSTR("&amp;"), CFRangeMake(0, CFStringGetLength(response_buffer)), 0);
    CFRelease(body_replace);
  }
  
  num_chars_response_buffer = CFStringGetLength(response_buffer);

  total_num_bytes_converted = 0;
  
  range_to_convert = CFRangeMake(0, num_chars_response_buffer);

  while (range_to_convert.length > 0) {

    num_chars_converted = CFStringGetBytes(response_buffer, range_to_convert, kCFStringEncodingUTF8, '?', false, buf_converted, max_bytes_buf_converted, &num_bytes_buf_converted);
    total_num_bytes_converted += num_bytes_buf_converted;
   
    if (num_chars_converted == 0) break;

    range_to_convert.location += num_chars_converted;
    range_to_convert.length -= num_chars_converted;
  };

  num_bytes_response_buffer = total_num_bytes_converted;

  headers = (*headers_ptr)(&c->request, num_bytes_response_buffer);
  
  num_headers = CFDictionaryGetCount(headers);
  keys = (CFTypeRef *) malloc(sizeof(CFTypeRef) * num_headers);
  values = (CFTypeRef *) malloc(sizeof(CFTypeRef) * num_headers);
  
  CFDictionaryGetKeysAndValues(headers, (const void **) keys, (const void **) values);

  headers_buffer = CFStringCreateMutable(NULL, 0);
  
  statusString = CFStringCreateWithFormat(NULL, NULL, CFSTR("HTTP/1.1 %llu \r\n"), c->response.status);
  CFStringAppend(headers_buffer, statusString);
  CFRelease(statusString);
  
  for (int i = 0; i < num_headers; i++) {
    key = (CFTypeRef *) keys[i];
    value = (CFTypeRef *) values[i];
    CFStringAppend(headers_buffer, (CFStringRef) key);
    CFStringAppend(headers_buffer, CFSTR(": "));
    CFStringAppend(headers_buffer, (CFStringRef) value);
    CFStringAppend(headers_buffer, CFSTR("\r\n"));
  }
  
  free(keys);
  free(values);
  
  CFStringAppend(headers_buffer, CFSTR("\r\n"));
  CFStringAppend(headers_buffer, response_buffer);
  
  num_chars_headers_buffer = CFStringGetLength(headers_buffer);

  total_num_bytes_converted = 0;
  range_to_convert = CFRangeMake(0, num_chars_headers_buffer);

  while (range_to_convert.length > 0) {

    num_chars_converted = CFStringGetBytes(headers_buffer, range_to_convert, kCFStringEncodingUTF8, '?', false, buf_converted, max_bytes_buf_converted, &num_bytes_buf_converted);

    total_num_bytes_converted += num_bytes_buf_converted;
   
    if (num_chars_converted == 0) break;

    range_to_convert.location += num_chars_converted;
    range_to_convert.length -= num_chars_converted;
  };
  
  num_bytes_headers_buffer = total_num_bytes_converted;

  c->response_buffer = uv_buf_init((char *) malloc(sizeof(UInt8) * (num_bytes_headers_buffer + num_bytes_response_buffer)), num_bytes_headers_buffer + num_bytes_response_buffer);

  num_chars_converted = CFStringGetBytes(headers_buffer, CFRangeMake(0, CFStringGetLength(headers_buffer)), kCFStringEncodingUTF8, '?', false, (UInt8 *) c->response_buffer.base, num_bytes_headers_buffer, NULL);
  num_chars_converted = CFStringGetBytes(response_buffer, CFRangeMake(0, CFStringGetLength(response_buffer)), kCFStringEncodingUTF8, '?', false, (UInt8 *) &c->response_buffer.base[num_bytes_headers_buffer], num_bytes_response_buffer, NULL);

  dispatch_async_f(response_queue, c, handle_async_response);
 
  CFRelease(headers_buffer);
  CFRelease(response_buffer);
  CFRelease(headers);
  CFRelease(body);
}

void handle_request_dispatch(void *context) {
  client_t *c = (client_t *) context;
  
  switch(c->response.transform) {
    case kCUVResponseTransformXML:
      __handle_request_dispatch_xml(c);
      break;
    case kCUVResponseTransformHTML:
      __handle_request_dispatch_html(c);
      break;
    default:
      __handle_request_dispatch_json(c);
      break;
  }
}

void handle_request(client_t *client) {
  dispatch_async_f(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), client, handle_request_dispatch);
}

void aggregate_responses(void *context) {
  CFMutableArrayRef responses = (CFMutableArrayRef) context;
  CFIndex num_responses = CFArrayGetCount(responses);
  client_t *clients[num_responses];
  CFArrayGetValues(responses, CFRangeMake(0, num_responses), (const void **) clients);

  for (uint32_t i = 0; i < num_responses; i++) {
    client_t *client = clients[i];
    uv_write(&client->write_req, (uv_stream_t *)&client->handle, &client->response_buffer, 1, after_write);
    end_response(client->response.status);
  }
    
  CFArrayRemoveAllValues(responses);
}

void on_request_complete(uv_async_t *async, int status) {
  CFMutableArrayRef responses = (CFMutableArrayRef) async-> data;
  dispatch_sync_f(response_queue, responses, aggregate_responses);
}

#pragma mark http_parser callbacks
int on_url(http_parser *parser, const char *url, size_t url_length) {
  client_t *client = (client_t *) parser->data;
  client->request.url = CFStringCreateWithBytes(NULL, (const UInt8 *) url, url_length, kCFStringEncodingUTF8, FALSE);
  
  return 0;
}

int on_header_field(http_parser *parser, const char *header_field, size_t header_field_length) {
  client_t *client = (client_t *) parser->data;
  CFStringRef header = CFStringCreateWithBytes(NULL, (const UInt8 *) header_field, header_field_length, kCFStringEncodingUTF8, TRUE);
  
  CFArrayAppendValue(client->request.header_keys, header);
  
  CFRelease(header);
  return 0;
}

int on_header_value(http_parser *parser, const char *header_value, size_t header_value_length) {
  client_t *client = (client_t *) parser->data;
  CFStringRef key = CFArrayGetValueAtIndex(client->request.header_keys, CFArrayGetCount(client->request.header_keys) - 1);
  CFStringRef value = CFStringCreateWithBytes(NULL, (const UInt8 *) header_value, header_value_length, kCFStringEncodingUTF8, FALSE);
  
  CFArrayAppendValue(client->request.header_values, value);
  CFDictionarySetValue(client->request.headers, key, value);
  
  CFRelease(value);
  return 0;
}

int on_headers_complete(http_parser *parser) {
  client_t *client = (client_t *) parser->data;
  
  begin_response(client);
  handle_request(client);
  
  return 1;
}

#pragma mark uv_tcp callbacks
uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
    return uv_buf_init((char *) malloc(suggested_size), (uint32_t) suggested_size);
}

void on_close(uv_handle_t *handle) {
  client_t *client = (client_t *) handle;
  uv_buf_t buf = client->response_buffer;
  
  CFRelease(client->request.header_keys);
  CFRelease(client->request.header_values);
  CFRelease(client->request.headers);
  CFRelease(client->request.url);
  
  free(buf.base);
  free(client);
}

void after_write(uv_write_t *req, int status)  {
  uv_close((uv_handle_t *)req->handle, on_close);
}


void on_read(uv_stream_t *tcp, ssize_t nread, uv_buf_t buf) {
  size_t parsed;
  
  client_t *client = (client_t *) tcp->data;
  
  client->request.headers = CFDictionaryCreateMutable(NULL, 30, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  client->request.header_keys = CFArrayCreateMutable(NULL, 30, &kCFTypeArrayCallBacks);
  client->request.header_values = CFArrayCreateMutable(NULL, 30, &kCFTypeArrayCallBacks);
  
  if (nread >= 0) {
      parsed = http_parser_execute(&client->parser, &parser_settings, buf.base, nread);
      
      if (parsed < nread) {
          uv_close((uv_handle_t *) &client->handle, on_close);
      }
  }
  else {
      uv_err_t err = uv_last_error(default_loop);
      if (err.code != UV_EOF) {
      }
  }
  
  free(buf.base);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (-1 == status) {
      // error
      return;
  }
  
  int r;
  
  client_t *client = malloc(sizeof(client_t));
  client->request.id = request_num++;
  
  uv_tcp_init(default_loop, &client->handle);
  http_parser_init(&client->parser, HTTP_REQUEST);
  
  client->parser.data = client;
  client->handle.data = client;
  
  r = uv_accept(server, (uv_stream_t *)&client->handle);
  
  uv_read_start((uv_stream_t *)&client->handle, alloc_buffer, on_read);
}

int CUVInit(CFDictionaryRef settings) {
  default_loop = uv_default_loop();

  parser_settings.on_headers_complete = on_headers_complete;
  parser_settings.on_header_field = on_header_field;
  parser_settings.on_header_value = on_header_value;
  parser_settings.on_url = on_url;
  
  uv_tcp_init(default_loop, &server);
  
  CFStringRef listen_address = CFDictionaryGetValue(settings, CFSTR("listen_address"));
  CFStringRef listen_port = CFDictionaryGetValue(settings, CFSTR("listen_port"));

  const char *uv_listen_address = CFStringGetCStringPtr(listen_address, kCFStringEncodingMacRoman);
  const char *uv_listen_port = CFStringGetCStringPtr(listen_port, kCFStringEncodingMacRoman);
  
  if (! uv_listen_address) {
      uv_listen_address = CFStringGetCStringPtr(listen_address, kCFStringEncodingASCII);
  }
  
  if (! uv_listen_port) {
      uv_listen_port = CFStringGetCStringPtr(listen_port, kCFStringEncodingASCII);
  }

  struct sockaddr_in bind_addr = uv_ip4_addr(uv_listen_address, atoi(uv_listen_port));
  uv_tcp_bind(&server, bind_addr);
  int r = uv_listen((uv_stream_t *) &server, 128, on_new_connection);
  
  if (r) {
      return 1;
  }

  response_queue = dispatch_queue_create("com.43n79w.coreuv-response", NULL);
  async.data = CFArrayCreateMutable(NULL, 1000, NULL);
  
  uv_async_init(default_loop, &async, on_request_complete);
  uv_run(default_loop, UV_RUN_DEFAULT);
  
  CFRelease(async.data);

  return 0;
}

void CUVResponseDataSourceInit(CFDictionaryRef (*CreateHeaders)(http_request_t const * const request, CFIndex contentLength), CFDictionaryRef (*CreateBody)(http_request_t const * const request, CUVHTTPResponseStatus * const status)) {
  headers_ptr = CreateHeaders;
  body_ptr = CreateBody;
}

void CUVResponseDelegateInit(void (*BeginResponse)(CUVResponseTransform * const transform, CUVResponseReplacements * const replace), void (*EndResponse)(CUVHTTPResponseStatus status)) {
  on_response_begin_ptr = BeginResponse;
  on_response_end_ptr = EndResponse;
}
