//
//  coreuv-utils.c
//  coreuv
//
//  Created by Paul Jara on 2013-09-19.
//
//

#include <CoreFoundation/CoreFoundation.h>
#include "coreuv.h"
#include "coreuv-utils.h"

#define HASH #
#define HASHIFY() HASH
#define Q(x) #x
#define QUOTE(x) Q(x)

#define HTML_ENTITY(ENTITY_NAME, ENTITY_NUMBER, VALUE) \
CFDictionarySetValue(entity_names, CFSTR(QUOTE(&ENTITY_NAME;)), CFSTR(#VALUE)); \
CFDictionarySetValue(entity_numbers, CFSTR(QUOTE(&HASHIFY()ENTITY_NUMBER;)), CFSTR(#VALUE));

typedef _Bool boolean_t;

typedef struct {
  CFStringRef entity_name;
  CFStringRef entity_number;
  CFStringRef character;
} entity_transform_t;

static CFMutableDictionaryRef entity_names = NULL;
static CFMutableDictionaryRef entity_numbers = NULL;

void __init__() {
 
  if (entity_names || entity_numbers) {
    return;
  }
  
  entity_names = CFDictionaryCreateMutable(NULL, 129, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  entity_numbers = CFDictionaryCreateMutable(NULL, 129, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  HTML_ENTITY(nbsp, 160, " ");
  HTML_ENTITY(iexcl, 161, \u00A1);
  HTML_ENTITY(cent, 162, \u00A2);
  HTML_ENTITY(pound, 163, \u00A3);
  HTML_ENTITY(curren, 164, \u00A4);
  HTML_ENTITY(yen, 165, \u00A5);
  HTML_ENTITY(brvbar, 166, \u00A6);
  HTML_ENTITY(sect, 167, \u00A7);
  HTML_ENTITY(uml, 168, \u00A8);
  HTML_ENTITY(copy, 169, \u00A9);
  HTML_ENTITY(ordf, 170, \u00AA);
  HTML_ENTITY(laquo, 171, \u00AB);
  HTML_ENTITY(not, 172, \u00AC);
  HTML_ENTITY(shy, 173, \u00AD);
  HTML_ENTITY(reg, 174, \u00AE);
  HTML_ENTITY(macr, 175, \u00AF);
  HTML_ENTITY(deg, 176, \u00B0);
  HTML_ENTITY(plusmn, 177, \u00B1);
  HTML_ENTITY(sup2, 178, \u00B2);
  HTML_ENTITY(sup3, 179, \u00B3);
  HTML_ENTITY(acute, 180, \u00B4);
  HTML_ENTITY(micro, 181, \u00B5);
  HTML_ENTITY(para, 182, \u00B6);
  HTML_ENTITY(middot, 183, \u00B7);
  HTML_ENTITY(cedil, 184, \u00B8);
  HTML_ENTITY(sup1, 185, \u00B9);
  HTML_ENTITY(ordm, 186, \u00BA);
  HTML_ENTITY(raquo, 187, \u00BB);
  HTML_ENTITY(frac14, 188, \u00BC);
  HTML_ENTITY(frac12, 189, \u00BD);
  HTML_ENTITY(frac34, 190, \u00BE);
  HTML_ENTITY(iquest, 191, \u00BF);
  HTML_ENTITY(Agrave, 192, \u00C0);
  HTML_ENTITY(Aacute, 193, \u00C1);
  HTML_ENTITY(Acirc, 194, \u00C2);
  HTML_ENTITY(Atilde, 195, \u00C3);
  HTML_ENTITY(Auml, 196, \u00C4);
  HTML_ENTITY(aring, 197, \u00C5);
  HTML_ENTITY(AElig, 198, \u00C6);
  HTML_ENTITY(Ccedil, 199, \u00C7);
  HTML_ENTITY(Egrave, 200, \u00C8);
  HTML_ENTITY(Eacute, 201, \u00C9);
  HTML_ENTITY(Ecirc, 202, \u00CA);
  HTML_ENTITY(Euml, 203, \u00CB);
  HTML_ENTITY(Igrave, 204, \u00CC);
  HTML_ENTITY(Iacute, 205, \u00CD);
  HTML_ENTITY(Icirc, 206, \u00CE);
  HTML_ENTITY(Iuml, 207, \u00CF);
  HTML_ENTITY(ETH, 208, \u00D0);
  HTML_ENTITY(Ntilde, 209, \u00D1);
  HTML_ENTITY(Ograve, 210, \u00D2);
  HTML_ENTITY(Oacute, 211, \u00D3);
  HTML_ENTITY(Ocirc, 212, \u00D4);
  HTML_ENTITY(Otilde, 213, \u00D5);
  HTML_ENTITY(Ouml, 214, \u00D6);
  HTML_ENTITY(times, 215, \u00D7);
  HTML_ENTITY(Oslash, 216, \u00D8);
  HTML_ENTITY(Ugrave, 217, \u00D9);
  HTML_ENTITY(Uacute, 218, \u00DA);
  HTML_ENTITY(Ucirc, 219, \u00DB);
  HTML_ENTITY(Uuml, 220, \u00DC);
  HTML_ENTITY(Yacute, 221, \u00DD);
  HTML_ENTITY(THORN, 222, \u00DE);
  HTML_ENTITY(szlig, 223, \u00DF);
  HTML_ENTITY(agrave, 224, \u00E0);
  HTML_ENTITY(aacute, 225, \u00E1);
  HTML_ENTITY(acirc, 226, \u00E2);
  HTML_ENTITY(atilde, 227, \u00E3);
  HTML_ENTITY(auml, 228, \u00E4);
  HTML_ENTITY(aring, 229, \u00E5);
  HTML_ENTITY(aelig, 230, \u00E6);
  HTML_ENTITY(cedil, 231, \u00E7);
  HTML_ENTITY(egrave, 232, \u00E8);
  HTML_ENTITY(eacute, 233, \u00E9);
  HTML_ENTITY(ecirc, 234, \u00EA);
  HTML_ENTITY(euml, 235, \u00EB);
  HTML_ENTITY(igrave, 236, \u00EC);
  HTML_ENTITY(iacute, 237, \u00ED);
  HTML_ENTITY(icirc, 238, \u00EE);
  HTML_ENTITY(iuml, 239, \u00EF);
  HTML_ENTITY(eth, 240, \u00F0);
  HTML_ENTITY(ntilde, 241, \u00F1);
  HTML_ENTITY(ograve, 242, \u00F2);
  HTML_ENTITY(oacute, 243, \u00F3);
  HTML_ENTITY(ocirc, 244, \u00F4);
  HTML_ENTITY(otilde, 245, \u00F5);
  HTML_ENTITY(ouml, 246, \u00F6);
  HTML_ENTITY(divide, 247, \u00F7);
  HTML_ENTITY(oslash, 248, \u00F8);
  HTML_ENTITY(ugrave, 249, \u00F9);
  HTML_ENTITY(uacute, 250, \u00FA);
  HTML_ENTITY(ucirc, 251, \u00FB);
  HTML_ENTITY(uuml, 252, \u00FC);
  HTML_ENTITY(yacute, 253, \u00FD);
  HTML_ENTITY(thorn, 254, \u00FE);
  HTML_ENTITY(yuml, 255, \u00FF);
  CFDictionarySetValue(entity_names, CFSTR("&quot;"), CFSTR("\""));
  CFDictionarySetValue(entity_numbers, CFSTR("&#34;"), CFSTR("\""));
  CFDictionarySetValue(entity_names, CFSTR("&amp;"), CFSTR("&"));
  CFDictionarySetValue(entity_numbers, CFSTR("&#38;"), CFSTR("&"));
  CFDictionarySetValue(entity_names, CFSTR("&lt;"), CFSTR("<"));
  CFDictionarySetValue(entity_numbers, CFSTR("&#60;"), CFSTR("<"));
  CFDictionarySetValue(entity_names, CFSTR("&gt;"), CFSTR(">"));
  CFDictionarySetValue(entity_numbers, CFSTR("&#62;"), CFSTR(">"));
  HTML_ENTITY(OElig, 338, \u0152);
  HTML_ENTITY(oelig, 339, \u0153);
  HTML_ENTITY(Scaron, 352, \u0160);
  HTML_ENTITY(scaron, 353, \u0161);
  HTML_ENTITY(Yuml, 376, \u0178);
  HTML_ENTITY(circ, 710, \u02C6);
  HTML_ENTITY(tilde, 732, \u02DC);
  CFDictionarySetValue(entity_names, CFSTR("&ensp;"), CFSTR(" "));
  CFDictionarySetValue(entity_numbers, CFSTR("&#8194;"), CFSTR(" "));
  CFDictionarySetValue(entity_names, CFSTR("&emsp;"), CFSTR(" "));
  CFDictionarySetValue(entity_numbers, CFSTR("&#8195;"), CFSTR(" "));
  CFDictionarySetValue(entity_names, CFSTR("&thinsp;"), CFSTR(" "));
  CFDictionarySetValue(entity_numbers, CFSTR("&#8201;"), CFSTR(" "));
  HTML_ENTITY(zwnj, 8204, \u200C);
  HTML_ENTITY(zwj, 8205, \u200D);
  HTML_ENTITY(lrm, 8206, \u200E);
  HTML_ENTITY(rlm, 8207, \u200F);
  HTML_ENTITY(ndash, 8211, \u2013);
  HTML_ENTITY(mdash, 8212, \u2014);
  HTML_ENTITY(lsquo, 8216, \u2018);
  HTML_ENTITY(rsquo, 8217, \u2019);
  HTML_ENTITY(sbquo, 8218, \u201A);
  HTML_ENTITY(ldquo, 8220, \u201C);
  HTML_ENTITY(rdquo, 8221, \u201D);
  HTML_ENTITY(bdquo, 8222, \u201E);
  HTML_ENTITY(dagger, 8224, \u2020);
  HTML_ENTITY(Dagger, 8225, \u2021);
  HTML_ENTITY(permil, 8240, \u2030);
  HTML_ENTITY(lsaquo, 8249, \u2039);
  HTML_ENTITY(rsaquo, 8250, \u203A);
  HTML_ENTITY(euro, 8364, \u20AC);
  
  HTML_ENTITY(prime, 8242, \u2032);
}

CFStringRef __CoreUVStringCreateFromEntityString(CFStringRef source, CUVResponseReplacements replace) {
  __init__();

  CFIndex start = 0;
  CFIndex end = CFStringGetLength(source);
  CFMutableStringRef dest = CFStringCreateMutableCopy(NULL, end, source);
  CFRange matches;
  CFRange entity = { 0, 0 };
  CFMutableCharacterSetRef entity_set = CFCharacterSetCreateMutable(NULL);
  CFCharacterSetAddCharactersInString(entity_set, CFSTR("&"));
  CFCharacterSetAddCharactersInString(entity_set, CFSTR(";"));
  boolean_t status;

  do {
    status = CFStringFindCharacterFromSet(dest, entity_set, CFRangeMake(start, end - start), 0, &matches);
    
    if (status) {
      if (entity.location == 0 && entity.length == 0) {
        entity.location = matches.location;
      }
      else {
        entity.length = matches.location - entity.location + 1;
        CFStringRef entity_match = CFStringCreateWithSubstring(NULL, dest, entity);
        CFStringRef entity_replacement;
        
        if (replace == kCUVResponseReplaceHTMLEntityNames) {
          entity_replacement = CFDictionaryGetValue(entity_names, entity_match);
        }
        else {
          entity_replacement = CFDictionaryGetValue(entity_numbers, entity_match);
        }
        
        if (entity_replacement) {
          CFStringFindAndReplace(dest, entity_match, entity_replacement, CFRangeMake(0, end), 0);
          end = CFStringGetLength(dest);
          if (start >= end) {
            CFRelease(entity_match);
            break;
          }
        }
        
        CFRelease(entity_match);
        entity.location = 0;
        entity.length = 0;
      }
      start = matches.location + matches.length;
    }
  } while (status);

  CFRelease(entity_set);
  
  return dest;
}

/**
 * Returns a CFStringRef that has the HTML entities from source converted
 * to a UTF8 text representation.
 */
CFStringRef CUVStringCreateFromHTMLEntityNameString(CFStringRef source) {
  return __CoreUVStringCreateFromEntityString(source, kCUVResponseReplaceHTMLEntityNames);
}

/**
 * Returns a CFStringRef that has the XML entities from source converted
 * to a UTF8 text representation.
 */
CFStringRef CUVStringCreateFromHTMLEntityNumberString(CFStringRef source) {
  return __CoreUVStringCreateFromEntityString(source, kCUVResponseReplaceHTMLEntityNumbers);
}


CFStringRef __CUVStringCreateHTMLFromArray(CFArrayRef source, CFStringRef className) {
  CFMutableStringRef html = CFStringCreateMutable(NULL, 100000);
  CFStringRef div_string = CFStringCreateWithFormat(NULL, NULL, CFSTR("<div class=\"%@\">"), className);
  
  CFIndex idx = CFArrayGetCount(source);
  
  for (CFIndex i = 0; i < idx; i++) {
    CFTypeRef elem = CFArrayGetValueAtIndex(source, i);
    CFTypeID type_id = CFGetTypeID(elem);
    boolean_t is_string = (type_id == CFStringGetTypeID());
    boolean_t is_array = (type_id == CFArrayGetTypeID());
    boolean_t is_dict = (type_id == CFDictionaryGetTypeID());
    
    if (is_string) {
      CFStringAppend(html, (CFStringRef) elem);
    }
    else if (is_dict) {
      CFDictionaryRef dict = elem;
      CFStringRef nested_object = CUVStringCreateHTMLFromDictionary(dict);
      CFStringAppend(html, div_string);
      CFStringAppend(html, nested_object);
      CFStringAppend(html, CFSTR("</div>"));
      CFRelease(nested_object);
    }
    else if (is_array) {
      CFArrayRef arr = elem;
      CFStringRef nested_class_name = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@_item"), className);
      CFStringRef nested_object = __CUVStringCreateHTMLFromArray(arr, nested_class_name);
      CFStringAppend(html, nested_object);
      CFRelease(nested_class_name);
      CFRelease(nested_object);
    }
  }
  
  CFRelease(div_string);
  return html;
}

CFStringRef CUVStringCreateHTMLFromDictionary(CFDictionaryRef source) {
  CFMutableStringRef html = CFStringCreateMutable(NULL, 100000);
  CFIndex idx = CFDictionaryGetCount(source);
  CFTypeRef *keys = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);
  CFTypeRef *values = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);
  
  CFDictionaryGetKeysAndValues(source, (const void **) keys, (const void **) values);
  
  for (CFIndex i = 0; i < idx; i++) {
    CFTypeID type_id = CFGetTypeID(values[i]);
    boolean_t is_string = (type_id == CFStringGetTypeID());
    boolean_t is_array = (type_id == CFArrayGetTypeID());
    boolean_t is_dict = (type_id == CFDictionaryGetTypeID());
    CFStringRef div_string = CFStringCreateWithFormat(NULL, NULL, CFSTR("<div id=\"%@\">"), keys[i]);
    CFStringAppend(html, div_string);
    
    if (is_string) {
      CFStringAppend(html, values[i]);
    }
    else if (is_dict) {
      CFDictionaryRef dict = values[i];
      CFStringRef nested_object = CUVStringCreateHTMLFromDictionary(dict);
      CFStringAppend(html, nested_object);
      CFRelease(nested_object);
    }
    else if (is_array) {
      CFArrayRef arr = values[i];
      CFStringRef nested_class_name = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@_item"), keys[i]);
      CFStringRef nested_object = __CUVStringCreateHTMLFromArray(arr, nested_class_name);
      CFStringAppend(html, nested_object);
      CFRelease(nested_class_name);
      CFRelease(nested_object);
    }
    
    CFStringAppend(html, CFSTR("</div>"));
    CFRelease(div_string);
  }
  
  free(keys);
  free(values);
  
  return html;
}

CFStringRef __CUVStringCreateXMLFromArray(CFArrayRef source, CFStringRef nodeName) {
  CFMutableStringRef html = CFStringCreateMutable(NULL, 100000);
  
  CFIndex idx = CFArrayGetCount(source);
  
  for (CFIndex i = 0; i < idx; i++) {
    CFTypeRef elem = CFArrayGetValueAtIndex(source, i);
    CFTypeID type_id = CFGetTypeID(elem);
    boolean_t is_string = (type_id == CFStringGetTypeID());
    boolean_t is_array = (type_id == CFArrayGetTypeID());
    boolean_t is_dict = (type_id == CFDictionaryGetTypeID());
    
    if (is_string) {
      CFStringAppend(html, (CFStringRef) elem);
    }
    else if (is_dict) {
      CFDictionaryRef dict = elem;
      CFStringRef nested_object = CUVStringCreateXMLFromDictionary(dict);
      CFStringAppend(html, CFSTR("<"));
      CFStringAppend(html, nodeName);
      CFStringAppend(html, CFSTR(">"));
      CFStringAppend(html, nested_object);
      CFStringAppend(html, CFSTR("</"));
      CFStringAppend(html, nodeName);
      CFStringAppend(html, CFSTR(">"));
      CFRelease(nested_object);
    }
    else if (is_array) {
      CFArrayRef arr = elem;
      CFStringRef nested_node_name = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@_item"), nodeName);
      CFStringRef nested_object = __CUVStringCreateXMLFromArray(arr, nested_node_name);
      CFStringAppend(html, nested_object);
      CFRelease(nested_node_name);
      CFRelease(nested_object);
    }
  }
  
  return html;
}

CFStringRef CUVStringCreateXMLFromDictionary(CFDictionaryRef source) {
  CFMutableStringRef html = CFStringCreateMutable(NULL, 100000);
  CFIndex idx = CFDictionaryGetCount(source);
  CFTypeRef *keys = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);
  CFTypeRef *values = (CFTypeRef *) malloc(sizeof(CFTypeRef) * idx);
  
  CFDictionaryGetKeysAndValues(source, (const void **) keys, (const void **) values);
  
  for (CFIndex i = 0; i < idx; i++) {
    CFTypeID type_id = CFGetTypeID(values[i]);
    boolean_t is_string = (type_id == CFStringGetTypeID());
    boolean_t is_array = (type_id == CFArrayGetTypeID());
    boolean_t is_dict = (type_id == CFDictionaryGetTypeID());
    CFStringRef open_tag = CFStringCreateWithFormat(NULL, NULL, CFSTR("<%@>"), keys[i]);
    CFStringRef close_tag = CFStringCreateWithFormat(NULL, NULL, CFSTR("</%@>"), keys[i]);
    CFStringAppend(html, open_tag);
    
    if (is_string) {
      CFStringAppend(html, values[i]);
    }
    else if (is_dict) {
      CFDictionaryRef dict = values[i];
      CFStringRef nested_object = CUVStringCreateXMLFromDictionary(dict);
      CFStringAppend(html, nested_object);
      CFRelease(nested_object);
    }
    else if (is_array) {
      CFArrayRef arr = values[i];
      CFStringRef nested_node_name = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@_item"), keys[i]);
      CFStringRef nested_object = __CUVStringCreateXMLFromArray(arr, nested_node_name);
      CFStringAppend(html, nested_object);
      CFRelease(nested_node_name);
      CFRelease(nested_object);
    }
    
    CFStringAppend(html, close_tag);
    CFRelease(open_tag);
    CFRelease(close_tag);
  }
  
  free(keys);
  free(values);
  
  return html;
}
