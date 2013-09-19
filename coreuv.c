//
//  coreuv.c
//  obj-http
//
//  Created by Paul Jara on 2013-09-17.
//
//

#include <dispatch/dispatch.h>
#include <unistd.h>
#include "CoreJSON.h"
#include "coreuv.h"

/*typedef int (*response_send_t)();*/

typedef struct {
    CFMutableDictionaryRef headers;
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

void (*on_response_begin_ptr)();
void (*on_response_end_ptr)();

CFDictionaryRef (*headers_ptr)(http_request_t *request, CFIndex contentLength);
CFDictionaryRef (*body_ptr)(http_request_t *request);

void begin_response() {
  on_response_begin_ptr();
};

void end_response() {
  on_response_end_ptr();
};

#pragma mark worker functionality

void handle_async_response(void *context) {
  CFMutableArrayRef responses = (CFMutableArrayRef) async.data;
  CFArrayAppendValue(responses, context);
  uv_async_send(&async);
}

void handle_request_dispatch(void *context) {

  client_t *c = (client_t *) context;

  CFMutableStringRef response_buffer;
  
  /**
   * We generate the body before the headers so we can set the Content-Length header (though this isn't mandatory).
   * Actually setting the Content-Length is left to the PJAPIResponseDataSource functionality to accomplish.
   */
  CFDictionaryRef body = (*body_ptr)(&c->request);
  response_buffer = CFStringCreateMutable(NULL, 1000);
  
  CFErrorRef error = NULL;
  CFStringRef json = JSONCreateString(NULL, body, kJSONWriteOptionsDefault, &error);
  
  CFIndex content_length = CFStringGetLength(json);
  
  CFDictionaryRef headers = (*headers_ptr)(&c->request, content_length);
  
  CFIndex idx = CFDictionaryGetCount(headers);
  CFTypeRef *keys = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);
  CFTypeRef *values = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);

  CFDictionaryGetKeysAndValues(headers, (const void **) keys, (const void **) values);
  
  CFStringAppend(response_buffer, CFSTR("HTTP/1.1 200 OK\r\n"));
  
  for (int i = 0; i < idx; i++) {
      CFTypeRef *key = (CFTypeRef *) keys[i];
      CFTypeRef *value = (CFTypeRef *) values[i];
      CFStringAppend(response_buffer, (CFStringRef) key);
      CFStringAppend(response_buffer, CFSTR(": "));
      CFStringAppend(response_buffer, (CFStringRef) value);
      CFStringAppend(response_buffer, CFSTR("\r\n"));
  }
  
  free(keys);
  free(values);
  
  CFStringAppend(response_buffer, CFSTR("\r\n"));
 
  if (json) {
      CFStringAppend(response_buffer, json);
      CFRelease(json);
  }
  
  CFRelease(headers);
  CFRelease(body);
  
  CFIndex len = CFStringGetLength(response_buffer);
  CFIndex usedBufferLength;
  CFIndex numChars;
  c->response_buffer = uv_buf_init((char *) malloc(len), (uint32_t) len);
  numChars = CFStringGetBytes(response_buffer, CFRangeMake(0, len), kCFStringEncodingUTF8, '?', FALSE, (UInt8 *) c->response_buffer.base, len, &usedBufferLength);
  
  if (numChars == 0) {
      // converted nothing
  }
  
  CFRelease(response_buffer);
  dispatch_async_f(response_queue, c, handle_async_response);
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
    end_response();
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
  
  begin_response();
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

int PJAPIServerInit(CFDictionaryRef settings) {
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

void PJAPIResponseDataSourceInit(CFDictionaryRef (*CreateHeaders)(), CFDictionaryRef (*CreateBody)()) {
  headers_ptr = CreateHeaders;
  body_ptr = CreateBody;
}

void PJAPIResponseDelegateInit(void (*BeginResponse)(), void (*EndResponse)()) {
  on_response_begin_ptr = BeginResponse;
  on_response_end_ptr = EndResponse;
}