//
//  main.m
//  test
//
//  Created by Paul Jara on 2013-09-15.
//
//

#include "coreuv.h"
#include <curl/curl.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <CoreFoundation/CFUUID.h>

#pragma mark Rotate This site scraper

uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out) {
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in , r);
    return(r);
}

void dump_node(TidyDoc doc, TidyNode tnod, int indent, CFMutableArrayRef *all_shows, CFMutableDictionaryRef show) {
    TidyNode child;
    uint32_t gigpress_info_items = 0;

    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child)) {
        ctmbstr name = tidyNodeGetName(child);
        if (name) {
            TidyAttr attr;
            if (strncmp("tbody", name, strlen(name)) == 0) {
                //NSLog(@"%s", name);
                for (attr=tidyAttrFirst(child); attr; attr=tidyAttrNext(attr)) {
                    ctmbstr attr_value = tidyAttrValue(attr);
                    if (strncmp("vevent", attr_value, strlen(attr_value)) == 0) {
                        //NSLog(@"-- %s", attr_value);
                        show = CFDictionaryCreateMutable(NULL, 10, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                        CFDictionarySetValue(show, CFSTR("sold_out"), kCFBooleanFalse);
                        
                        CFUUIDRef uuid_ref = CFUUIDCreate(NULL);
                        CFStringRef uuid_str = CFUUIDCreateString(NULL, uuid_ref);
                        
                        CFDictionarySetValue(show, CFSTR("id"), uuid_str);
                        
                        CFRelease(uuid_str);
                        CFRelease(uuid_ref);
                        
                        gigpress_info_items = 0;
                    }
                }
            }
            if (strncmp("td", name, strlen(name)) == 0) {
                for (attr=tidyAttrFirst(child); attr; attr=tidyAttrNext(attr)) {
                    ctmbstr attr_value = tidyAttrValue(attr);
                    if (attr_value) {
                        if (strncmp("gigpress-date", attr_value, strlen(attr_value)) == 0) {
                            TidyNode date_node = tidyGetChild(child);
                            TidyAttr date_attr;
                            for (date_attr = tidyAttrFirst(date_node); date_attr; date_attr = tidyAttrNext(date_attr)) {
                                if (tidyAttrIsTITLE(date_attr)) {
                                    CFStringRef text_str = CFStringCreateWithBytes(NULL, (const UInt8 *) tidyAttrValue(date_attr), strlen(tidyAttrValue(date_attr)), kCFStringEncodingUTF8, FALSE);
                                   
                                    CFMutableStringRef date_str = CFStringCreateMutable(NULL, 50);
                                    CFStringAppend(date_str, text_str);
                                    CFStringAppend(date_str, CFSTR("Z"));
                                    
                                    CFDictionarySetValue(show, CFSTR("date"), date_str);
                                    CFRelease(text_str);
                                    CFRelease(date_str);
                                }
                            }
                        }
                        
                        if (strncmp("gigpress-artist", attr_value, strlen(attr_value)) == 0) {
                            TidyBuffer buf;
                            tidyBufInit(&buf);
                            tidyNodeGetText(doc, tidyGetChild(child), &buf);
                            CFStringRef text_str = CFStringCreateWithBytes(NULL, buf.bp, strlen((const char *) buf.bp) - 1, kCFStringEncodingUTF8, FALSE);
                            CFDictionarySetValue(show, CFSTR("artist"), text_str);
                            CFRelease(text_str);
                            tidyBufFree(&buf);
                        }
                        
                        if (strncmp("gigpress-venue", attr_value, 14) == 0) {
                            TidyBuffer buf;
                            tidyBufInit(&buf);
                            tidyNodeGetText(doc, tidyGetChild(child), &buf);
                            CFStringRef text_str = CFStringCreateWithBytes(NULL, buf.bp, strlen((const char *) buf.bp) - 1, kCFStringEncodingUTF8, FALSE);
                            CFDictionarySetValue(show, CFSTR("venue"), text_str);
                            CFRelease(text_str);
                            tidyBufFree(&buf);
                            

                        }
                    }
                }
            }
            if (strncmp("span", name, strlen(name)) == 0) {
                for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
                    ctmbstr attr_value = tidyAttrValue(attr);
                    if (attr_value) {
                        if (strncmp("gigpress-info-item", attr_value, strlen(attr_value)) == 0) {
                            gigpress_info_items++;
                            TidyBuffer buf;
                            tidyBufInit(&buf);
                            tidyNodeGetText(doc, tidyGetChild(tidyGetChild(child)), &buf);
                            
                            if (strncmp("Age", (const char *) buf.bp, 3) == 0) {
                                tidyBufFree(&buf);
                                tidyBufInit(&buf);
                                tidyNodeGetText(doc, tidyGetNext(tidyGetChild(child)), &buf);
                                CFStringRef text_str = CFStringCreateWithBytes(NULL, buf.bp + 1, strlen((const char *) buf.bp) - 3, kCFStringEncodingUTF8, FALSE);
                                
                                CFDictionarySetValue(show, CFSTR("age_restrictions"), text_str);
                                CFRelease(text_str);
                                
                                CFArrayAppendValue(*all_shows, show);
                                CFRelease(show);
                                show = nil;
                            }
                            
                            if (strncmp("Price", (const char *) buf.bp, 5) == 0) {
                                tidyBufFree(&buf);
                                tidyBufInit(&buf);
                                tidyNodeGetText(doc, tidyGetNext(tidyGetChild(child)), &buf);
                                CFStringRef text_str = CFStringCreateWithBytes(NULL, buf.bp, strlen((const char *) buf.bp) - 2, kCFStringEncodingUTF8, FALSE);
                                
                                CFDictionarySetValue(show, CFSTR("price"), text_str);
                                CFRelease(text_str);
                            }
                            
                            if (strncmp("Sold Out", (const char *) buf.bp, 8) == 0) {
                                CFMutableDictionaryRef _show = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(*all_shows, CFArrayGetCount(*all_shows) - 1);
                                CFDictionarySetValue(_show, CFSTR("sold_out"), kCFBooleanTrue);
                            }
                            tidyBufFree(&buf);
                        }
                    }
                }
            }
        }
        dump_node(doc, child, indent + 4, all_shows, show);
    }
}

#pragma mark PJHTTPRequestDelegate functions

CFDictionaryRef CreateHeaders(http_request_t *request, CFIndex contentLength) {
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 30, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFStringRef contentLengthValue = CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld"), contentLength);
    
    CFDictionarySetValue(dict, CFSTR("Content-Type"), CFSTR("application/json; charset=UTF-8"));
    CFDictionarySetValue(dict, CFSTR("Content-Length"), contentLengthValue);
    CFDictionarySetValue(dict, CFSTR("Transfer-Encoding"), CFSTR("Identity"));
    CFDictionarySetValue(dict, CFSTR("X-Frame-Options"), CFSTR("SAMEORIGIN"));
    CFDictionarySetValue(dict, CFSTR("X-XSS-Protection"), CFSTR("1; mode=block"));
    
    CFRelease(contentLengthValue);
    
    return dict;
}

CFDictionaryRef CreateBody(http_request_t *request) {
  /*CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 30, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(dict, CFSTR("Foo"), CFSTR("Bar"));
  return dict;*/
  
    CURL *curl;
    CURLcode res;
    
    TidyDoc tdoc;
    TidyBuffer docbuf = {0};
    TidyBuffer tidy_errbuf = {0};
    int err;
    
    CFMutableArrayRef all_shows = CFArrayCreateMutable(NULL, 200, &kCFTypeArrayCallBacks);
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.rotate.com/content/tickets/");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        
        tdoc = tidyCreate();
        tidyOptSetBool(tdoc, TidyForceOutput, yes);
        tidyOptSetInt(tdoc, TidyWrapLen, 4096);
        tidySetErrorBuffer(tdoc, &tidy_errbuf);
        tidyBufInit(&docbuf);
        
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
        
        res = curl_easy_perform(curl);
        
        if (CURLE_OK != res) {
            //error
            NSLog(@"Error");
        }
        else {
            err = tidyParseBuffer(tdoc, &docbuf);
            if (err >= 0) {
                err = tidyCleanAndRepair(tdoc);
                if (err >= 0) {
                    err = tidyRunDiagnostics(tdoc);
                    if (err >= 0) {
                        dump_node(tdoc, tidyGetRoot(tdoc), 0, &all_shows, nil);
                        tidyBufFree(&docbuf);
                        tidyBufFree(&tidy_errbuf);
                        tidyRelease(tdoc);
                    }
                }
            }
           
        }
        
        curl_easy_cleanup(curl);
    }
    
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 30, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(dict, CFSTR("shows"), all_shows);
    
    CFRelease(all_shows);
    
    return dict;
}

#pragma mark PJHTTPResponseDelegate functions

void BeginResponse(CUVResponseTransform *transform, CUVResponseReplacements *replace) {
  *replace = kCUVResponseReplaceHTMLEntityNames;
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

void EndResponse() {
    curl_global_cleanup();
}

#pragma mark main
int main(int argc, const char * argv[])
{
    CFMutableDictionaryRef settings = CFDictionaryCreateMutable(NULL, 10, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(settings, CFSTR("listen_address"), CFSTR("0.0.0.0"));
    CFDictionarySetValue(settings, CFSTR("listen_port"), CFSTR("9000"));

    CoreUVResponseDataSourceInit(&CreateHeaders, &CreateBody);
    CoreUVResponseDelegateInit(&BeginResponse, &EndResponse);

    int exit_code = CoreUVInit(settings);
    
    CFRelease(settings);
    
    exit(exit_code);
}