//
//  coreuv-utils.h
//  coreuv
//
//  Created by Paul Jara on 2013-09-19.
//
//

#ifndef coreuv_coreuv_utils_h
#define coreuv_coreuv_utils_h

/**
 * Returns a CFStringRef that has the HTML entities from source converted
 * to a UTF8 text representation.
 */
CFStringRef CUVStringCreateFromHTMLEntityNameString(CFStringRef source);
CFStringRef CUVStringCreateFromHTMLEntityNumberString(CFStringRef source);

CFStringRef CUVStringCreateHTMLFromDictionary(CFDictionaryRef source);
CFStringRef CUVStringCreateXMLFromDictionary(CFDictionaryRef source);
#endif
