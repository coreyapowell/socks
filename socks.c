/* socks - Quickjs socket module 
MIT License

Copyright (c) 2025 Corey A Powell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef QUICKJS_H
/* this file is cross compatible with tacking onto the end of quickjs-libc.c
  or doing it the right way and compiling into a .dll or .so */
#include <inttypes.h>
#include <stdio.h>
#include "quickjs.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#ifdef _WIN32
#include <Direct.h>
#else
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#endif
#else

#endif // INCLUDE_NO_HDRS

#if defined(_WIN32)
#include <winsock2.h>
WSADATA wsaData;

#include <ws2tcpip.h>
#include <ws2ipdef.h>
#define GET_SOCKET_ERROR() (WSAGetLastError())

#else

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  
#include <netinet/tcp.h>  
#include <arpa/inet.h>

#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET (~0)
#define GET_SOCKET_ERROR() (errno)

#define AF_NETBIOS 17
#define AF_BTH 32
#define SD_BOTH 2
#define SD_RECEIVE 0
#define SD_SEND 1

#endif // _WIN32

/*** Macro Section ************************************************************
 * these will be used later, in JSClassID / JSClassDef and Module sections
 * a tool should expand all of these, but for now it's just here!
 * it will likely be a separte include.
 * 
 *********************************************************************************/

#ifndef QJC_CLASS_MACROS
#define QJC_CLASS_MACROS

#define JS_CLASS_DEFINES(JSC_PARAM)\
    static JSClassID js_##JSC_PARAM##_ClassID;\
    static void js_##JSC_PARAM##_finalizer(JSRuntime *rt, JSValue val);\
    static JSClassDef js_##JSC_PARAM##_ClassDef = {\
        #JSC_PARAM,\
        .finalizer = js_##JSC_PARAM##_finalizer,\
    }
    

#define PROTO_FUNCTS(x) x##_proto_functs, (sizeof(x##_proto_functs) / sizeof(x##_proto_functs)[0])
    
#define OS_FLAG(x) JS_PROP_INT32_DEF(#x, x, JS_PROP_CONFIGURABLE )

// IO_NOTHROW returns the errorno instead of triggering a throw/exception
#define QJC_IO_NOTHROW

#endif // QJC_CLASS_MACROS

 
/*  END Macro Section */

/**** Struct Section ********************************************************
 * structures are separated into library / os neutral blocks.
 * this allows independent testing and reuse.
 * 
 * 
 * 
 **************************************************************************/

#define SOCKET_CACHE_SIZE 8192

typedef struct _socketaddress {
    union {
        struct sockaddr_in c_addr;
        struct sockaddr_in6 c_addr6;
    };
} _socketaddress;

#define SOCKETADDRESS_COPY(d,s) \
    memcpy(d, s, sizeof(struct sockaddr_in6))

#define NSLIST_BUFFLEN 256
typedef struct _nslist {
    uint32_t index;
    uint32_t count;
    char *name;
    char *port;
    struct addrinfo *head;
    struct addrinfo *current;
    struct addrinfo hints;
    char buff[NSLIST_BUFFLEN];
} _nslist;

#define NSLIST_ZERO(al) \
    memset(al, 0, sizeof(_nslist))

typedef struct _linecache {
    char *data;
    int cap, first, last;
} _linecache;

#define LINECACHE_ZERO(lc) \
    lc.data = 0; \
    lc.cap = 0;\
    lc.first = lc.last = 0;

#define LINECACHE_CTOR(lc, sz) \
    lc.data = malloc(sz); \
    memset(lc.data, 0, sz); \
    lc.cap = sz + 32; \
    lc.first = lc.last = 0;

#define _SOCKET_LISTEN_BACKLOG 64

struct _socket {
    SOCKET handle;
    int error;
    struct _linecache linecache;
} _socket;

/* End Struct Section */

/********* JSClassID / JSClassDef ******************************************
 * 
 * separation independence is good! naming convention is good!
 * I would prefer a macro! but the linux build was a nightmare. 
 * The best alternative is to use QJS to write headers later.
 * 
 * original x-platform struct: _mystruct;
 * c instance (independent library): mystruct;
 * js module name: MyStruct;
 * c internal data struct of js class: js_MyStruct;
 * 
 * JS_CLASS_DEFINES(MyStruct) - produces the following:
 *  -JSCLassID: js_MyStruct_ClassID;
 *  -JSClassDef: js_MyStruct_ClassDef;
 *  -finalizer: js_MyStruct_finalizer;
 * 
 * laster we have to create the following for the moducle loader:
 *  -Js_MyStruct_ctor - used to create and return a ctor (by other classes)
 *  -Js_MyStruct_ctor_new - used for prototype inheritence (new MyStruct())
 *  -js_MyStruct_proto_functs - prototype function list
 * 
 * ..... later when we register the module:
 * JS_MODULE_CLASS_PROTOS(MyStruct) - produces the following
 *  -js_MsSruct_new: constructor (referencing ctor_new)
 *  -js_MyStruct_proto: contains the protos from the function list
 * 
 * *************************************************************************/
 
typedef struct _nslist js_NsList;
JS_CLASS_DEFINES(NsList);
 
typedef struct _socketaddress js_SocketAddress;
JS_CLASS_DEFINES(SocketAddress);
/*
static JSClassID js_SocketAddress_ClassID;
static void js_SocketAddress_finalizer(JSRuntime *rt, JSValue val);
static JSClassDef js_SocketAddress_ClassDef = {
    "SocketAddress",
    .finalizer = js_SocketAddress_finalizer,
};
*/

typedef struct _socket js_Socket;
JS_CLASS_DEFINES(Socket);
const char* const js_Socket_magic_strings[] = { "bind", "connect", 0 };
/*
static JSClassID js_Socket_ClassID;
static void js_Socket_finalizer(JSRuntime *rt, JSValue val);
static JSClassDef js_Socket_ClassDef = {
    "Socket",
    .finalizer = js_Socket_finalizer,
};
*/

/* End JSClassID / JSClassDef Section */

/***** SocketAddress Section ****************************************************
 *   
 * 
 *******************************************************************************/

static void js_SocketAddress_finalizer(JSRuntime *rt, JSValue val) {
    js_SocketAddress *s = JS_GetOpaque(val, js_SocketAddress_ClassID);
    if (s) {
        js_free_rt(rt, s);
    };
};

static int js_SocketAddress_parse_obj_helper(JSContext *ctx, js_SocketAddress *s,
                             int argc, JSValueConst *argv) {
    if (!s) return 1;
    
    const char* addr = 0; int port = 0; int family = AF_BTH;
    JSValueConst args = argv[0];
    JSValue val;
    
    val = JS_GetPropertyStr(ctx, args, "family");
    if (JS_IsException(val))  return 1;
    if (!JS_IsUndefined(val)) {
        JS_ToInt32(ctx, &family, val);
    };
    if (family == AF_INET6) s->c_addr.sin_family = family;
    else  s->c_addr6.sin6_family = family;

    val  = JS_GetPropertyStr(ctx, args, "address");
    if (JS_IsException(val))  return 1;
    if (!JS_IsUndefined(val)) {
        addr = JS_ToCString(ctx, val);
        if (family == AF_INET6) inet_pton(AF_INET6, addr, &s->c_addr6.sin6_addr);
        else inet_pton(AF_INET, addr, &s->c_addr.sin_addr);
        //s->c_addr.sin_addr.s_addr = inet_addr(addr);
        JS_FreeCString(ctx, addr);
    } else {
        s->c_addr.sin_addr.s_addr = 0;
    };
    JS_FreeValue(ctx, val);
    
    val = JS_GetPropertyStr(ctx, args, "port");
    if (JS_IsException(val))  return 1;
    if (!JS_IsUndefined(val)) {
        JS_ToInt32(ctx, &port, val);
    };
    if (family == AF_INET6) s->c_addr6.sin6_port = htons(port);
    else s->c_addr.sin_port = htons(port);
    JS_FreeValue(ctx, val);
    
    return 0;
                             
};

static JSValue js_SocketAddress_parse_obj(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_SocketAddress *s = JS_GetOpaque2(ctx, this_val, js_SocketAddress_ClassID);
    if (!s) return JS_EXCEPTION;
    if (!js_SocketAddress_parse_obj_helper(ctx, s, argc, argv))
        return JS_UNDEFINED;
    return JS_EXCEPTION;
};

static JSValue js_SocketAddress_isnew(JSContext *ctx, JSValueConst new_target,
                             int argc, JSValueConst *argv, const int isnew) {
    js_SocketAddress *s = js_mallocz(ctx, sizeof(*s));
    JSValue obj;
    JSValue proto;
    //const char* str = 0;
    
    if (argc >= 1) 
    if (js_SocketAddress_parse_obj_helper(ctx, s, argc, argv))
      goto addr_parse_err_ctor;
    
    if (isnew) {
        if (JS_IsUndefined(new_target)) {
            proto = JS_GetClassProto(ctx, js_SocketAddress_ClassID);
        } else {
            proto = JS_GetPropertyStr(ctx, new_target, "prototype");
            if (JS_IsException(proto)) goto addr_parse_err_ctor;
        };
    } else {
        proto = JS_GetClassProto(ctx, js_SocketAddress_ClassID);
        if (JS_IsException(proto)) goto addr_parse_err_ctor;
    };   
    obj = JS_NewObjectProtoClass(ctx, proto, js_SocketAddress_ClassID);
    JS_FreeValue(ctx, proto);
    
    JS_SetOpaque(obj, s);
    return obj;
    
  addr_parse_err_ctor:
    if (s) js_free(ctx, s);
    //if (str) JS_FreeCString(ctx, str);
    return JS_EXCEPTION;  
};

static JSValue js_SocketAddress_ctor(JSContext *ctx, JSValueConst new_target,
                                int argc, JSValueConst *argv) {
    return js_SocketAddress_isnew(ctx, new_target, argc, argv, 0);
};

static JSValue js_SocketAddress_ctor_new(JSContext *ctx, JSValueConst new_target,
                             int argc, JSValueConst *argv) {
    return js_SocketAddress_isnew(ctx, new_target, argc, argv, 1);
};

// address, port, family
char FAM_IPV4[] = "IPV4";
static JSValue js_SocketAddress_get(JSContext *ctx, JSValueConst this_val, int magic) { 
    js_SocketAddress *s = JS_GetOpaque2(ctx, this_val, js_SocketAddress_ClassID);
    if (!s) return JS_EXCEPTION;
    if (magic == 0) {
        char mybuff2[64];
        if (s->c_addr.sin_family == AF_INET6) 
            inet_ntop(AF_INET6, &s->c_addr6.sin6_addr, (char*)mybuff2, 64);
        else inet_ntop(AF_INET, &s->c_addr.sin_addr, (char*)mybuff2, 64);
        return JS_NewString(ctx, (char*)mybuff2);
    } else if (magic == 1)  return JS_NewInt32(ctx, htons(s->c_addr.sin_port));
    else if (magic == 2)  return JS_NewInt32(ctx, s->c_addr.sin_family);
    return JS_NewString(ctx, FAM_IPV4); // family
};

static JSValue js_SocketAddress_set(JSContext *ctx, JSValueConst this_val, JSValue val, int magic) {
    js_SocketAddress *s = JS_GetOpaque2(ctx, this_val, js_SocketAddress_ClassID);
    if (!s) return JS_EXCEPTION;
    int v; const char* str;
    if (magic == 0) { // address
        str = JS_ToCString(ctx, val);
        if (!str) return JS_EXCEPTION;
        memset(&s->c_addr6.sin6_addr, 0, sizeof(s->c_addr6.sin6_addr));
        if (s->c_addr.sin_family == AF_INET6) inet_pton(AF_INET6, str, &s->c_addr6.sin6_addr);
        else inet_pton(AF_INET, str, &s->c_addr.sin_addr);
        JS_FreeCString(ctx, str);
    } else if (magic == 1) {  // port
        if (JS_ToInt32(ctx, &v, val)) return JS_EXCEPTION;
        s->c_addr.sin_port = htons(v);
    } else if (magic == 2) {  // family
        if (JS_ToInt32(ctx, &v, val)) return JS_EXCEPTION;
        s->c_addr.sin_family=v;
    } else {
    
    };
    return JS_UNDEFINED;
}

/***** NsList section ********************************************************
 * NsList is an addrinfo list manager. You call the lookup function to get 
 * this list, specifying the address, family, and port you want to use.
 * You can then call first, current, or next to return a record. 
 * If it returns null, then you've gone past the list. It works like some of
 * the old VB lists or SQL calls, where you have to return to first.
 * You can get the index property to see where, or set the index property
 * to skip ahead to a certain record. If you get to the end of the list,
 * index is updated and the last value is returned (as current).
 * **************************************************************************/

static void js_NsList_finalizer(JSRuntime *rt, JSValue val) {
    js_NsList *s = JS_GetOpaque(val, js_Socket_ClassID);
    if (s) {
        if (s->head) {
            freeaddrinfo(s->head);
         };
         js_free_rt(rt, s);
    };
    
};

static JSValue js_socks_lookup(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    const char* name = 0;
    size_t len;
    int family = AF_INET;
    JSValue val;
    
    if (argc >=3) {
        val = argv[2];
        JS_ToInt32(ctx, &family, val);
    };
    if (argc <= 0) {
        JS_ThrowPlainError(ctx,"requires name.");
        return JS_EXCEPTION;
    };
        js_NsList *s = js_mallocz(ctx, sizeof(js_NsList));
        if (!s) return JS_EXCEPTION;
        //NSLIST_ZERO(s);
        
        // name
        val = argv[0];
        name = JS_ToCStringLen(ctx, &len, val);
        if (len > NSLIST_BUFFLEN - 1) {
            JS_ThrowPlainError(ctx,"name parameter is way too long!");
            goto nslist_parse_error;
        };
        if (len == 0) {
            JS_ThrowPlainError(ctx,"name is blank. hostname is required.");
            goto nslist_parse_error;
        };
        for (int i = 0; i < len; i++) s->buff[i] = name[i];
        s->buff[len] = 0; len++;
        s->name = s->buff;
        JS_FreeCString(ctx, name); name = 0;
        
        // port / service
        if (argc>=2) {
            val = argv[1];
            int port;
            JS_ToInt32(ctx, &port, val);
            if (!port) {
                name = JS_ToCString(ctx, val);
                if (name) {
                    int i = 0;
                    while(name[i] != 0) {
                        if (len + i + 1 >= NSLIST_BUFFLEN) {
                            JS_ThrowPlainError(ctx,"combined name and service port parameters exceed buffer size of %d", NSLIST_BUFFLEN);
                            goto nslist_parse_error;
                        };
                        s->buff[len + i] = name[i];
                        i++;
                    };
                    s->buff[len + i] = 0;
                    s->port = &(s->buff[len]);
                    JS_FreeCString(ctx, name); name = 0;                             
                };
            } else {
                if (port > 65535) {
                    JS_ThrowPlainError(ctx,"requested port %d. they don't stack ports that high!", port);
                    goto nslist_parse_error;
                };
                sprintf(&s->buff[len], "%d", port);
                s->port = &s->buff[len];
            };
        } else {
            s->port = 0;
        };
        
    s->hints.ai_family = family;
    int result = getaddrinfo( s->name, s->port, &s->hints, &s->head );
    if (result) {
        if (s) js_free(ctx, s);
        return JS_NULL;
    }; 
    if (!s->head) {
        if (s) js_free(ctx, s);
        return JS_NULL;
    };  
    
    js_SocketAddress *t =  js_mallocz(ctx, sizeof(js_SocketAddress));
    if (!t) return JS_EXCEPTION;
    SOCKETADDRESS_COPY(t, s->head->ai_addr);    
    freeaddrinfo(s->head); s->head = 0; 
    js_free(ctx, s); s = 0;
    
    JSValue proto = JS_GetClassProto(ctx, js_SocketAddress_ClassID);
    if (JS_IsException(proto)) {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };  
    JSValue obj = JS_NewObjectProtoClass(ctx, proto, js_SocketAddress_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };
    JS_SetOpaque(obj, t);
    return obj;
    
  nslist_parse_error:
    if (name != 0) { JS_FreeCString(ctx, name); name = 0; };
    if (s) { js_free(ctx, s); s= 0; };
    return JS_EXCEPTION;       
};

static JSValue js_NsList_ctor_new(JSContext *ctx, JSValueConst new_target,
                             int argc, JSValueConst *argv) {
    js_NsList *s;  
    JSValue obj, proto, val, args;
    const char* name = 0;
    size_t len;
    
    if (argc <= 0) {
        JS_ThrowPlainError(ctx,"options argument not supplied. requires name.");
        return JS_EXCEPTION;
    };
    
        s = js_mallocz(ctx, sizeof(js_NsList));
        if (!s) return JS_EXCEPTION;
        NSLIST_ZERO(s);
        args = argv[0];
        
        // flags           
        val = JS_GetPropertyStr(ctx, args, "flags");
        if (JS_IsException(val))  goto nslist_parse_error;
        if (!JS_IsUndefined(val)) {
            JS_ToInt32(ctx, &(s->hints.ai_flags), val);
        } else {
            s->hints.ai_flags = AI_PASSIVE;
        };
        
        // family           
        val = JS_GetPropertyStr(ctx, args, "family");
        if (JS_IsException(val))  goto nslist_parse_error;
        if (!JS_IsUndefined(val)) {
            JS_ToInt32(ctx, &s->hints.ai_family, val);
        } else {
            s->hints.ai_family = AF_INET;
        };
        
        // socktype           
        val = JS_GetPropertyStr(ctx, args, "socktype");
        if (JS_IsException(val))  goto nslist_parse_error;
        if (!JS_IsUndefined(val)) {
            JS_ToInt32(ctx, &s->hints.ai_socktype, val);
        } else {
            s->hints.ai_socktype = SOCK_STREAM;
        };
                   
        // protocol
        val = JS_GetPropertyStr(ctx, args, "protocol");
        if (JS_IsException(val))  goto nslist_parse_error;
        if (!JS_IsUndefined(val)) {
            JS_ToInt32(ctx, &s->hints.ai_protocol, val);
        } else {
            s->hints.ai_protocol = IPPROTO_TCP;
        };
        
        // name
        val = JS_GetPropertyStr(ctx, args, "name");
        if (JS_IsException(val))  goto nslist_parse_error;
        if (JS_IsUndefined(val)) {
            JS_ThrowPlainError(ctx,"name parameter is required for hostname.");
            goto nslist_parse_error;
        };
        name = JS_ToCStringLen(ctx, &len, val);
        if (len > NSLIST_BUFFLEN - 1) {
            JS_ThrowPlainError(ctx,"name parameter is way too long!");
            goto nslist_parse_error;
        };
        if (len == 0) {
            JS_ThrowPlainError(ctx,"name is blank. hostname is required.");
            goto nslist_parse_error;
        };
        for (int i = 0; i < len; i++) s->buff[i] = name[i];
        s->buff[len] = 0; len++;
        s->name = s->buff;
        JS_FreeCString(ctx, name); name = 0;
        
        // port / service
        val = JS_GetPropertyStr(ctx, args, "port");
        if (JS_IsException(val))  goto nslist_parse_error;
        if (JS_IsUndefined(val)) {
            val = JS_GetPropertyStr(ctx, args, "service");
            if (JS_IsException(val))  goto nslist_parse_error;          
        };
        if (!JS_IsUndefined(val)) {
            int port;
            JS_ToInt32(ctx, &port, val);
            if (!port) {
                name = JS_ToCString(ctx, val);
                if (name) {
                    int i = 0;
                    while(name[i] != 0) {
                        if (len + i + 1 >= NSLIST_BUFFLEN) {
                            JS_ThrowPlainError(ctx,"combined name and service port parameters exceed buffer size of %d", NSLIST_BUFFLEN);
                            goto nslist_parse_error;
                        };
                        s->buff[len + i] = name[i];
                        i++;
                    };
                    s->buff[len + i] = 0;
                    s->port = &(s->buff[len]);
                    JS_FreeCString(ctx, name); name = 0;                             
                };
            } else {
                if (port > 65535) {
                    JS_ThrowPlainError(ctx,"requested port %d. they don't stack ports that high!", port);
                    goto nslist_parse_error;
                };
                sprintf(&s->buff[len], "%d", port);
                s->port = &s->buff[len];
            };
        } else {
            s->port = 0;
        };
        
    int result = getaddrinfo( s->name, s->port, &s->hints, &s->head );
    if (result) {
        JS_ThrowPlainError(ctx,"getaddrinfo failed with error: %d", result);
        goto nslist_parse_error;
    }; 
    if (!s->head) {
        JS_ThrowPlainError(ctx,"getaddrinfo failed to supply an nsLookup record.");
         goto nslist_parse_error;
    };   // from here the s->head structure needs to be managed.
    
    // prototype
    if (JS_IsUndefined(new_target)) {
        proto = JS_GetClassProto(ctx, js_NsList_ClassID);
    } else {
        proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    };
    if (JS_IsException(proto)) goto nslist_proto_error;   
    obj = JS_NewObjectProtoClass(ctx, proto, js_NsList_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  goto nslist_obj_error;
    
    // return JS Object
    s->current = s->head; s->index = 0;
    JS_SetOpaque(obj, s);
    return obj;
    
  nslist_obj_error:
    JS_FreeValue(ctx, obj);  
  nslist_proto_error:
    if (s->head) { freeaddrinfo(s->head); s->head = 0; };
  nslist_parse_error:
    if (name != 0) { JS_FreeCString(ctx, name); name = 0; };
    if (s) { js_free(ctx, s); s= 0; };
    return JS_EXCEPTION;       
};

static JSValue js_NsList_first_current(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv, int magic) {
    js_NsList *s = JS_GetOpaque2(ctx, this_val, js_NsList_ClassID);
    if (!s) return JS_EXCEPTION;      
    struct addrinfo *taddr;
    if (magic==1) {
        taddr = s->current;  
    } else {
        return JS_NULL;
    }; 
    //s->index = 0; s->current = taddr;
    if (!taddr->ai_addr) {
        JS_ThrowInternalError(ctx,"record is missing it's address.");
        return JS_EXCEPTION;
    };
    
    js_SocketAddress *t =  js_mallocz(ctx, sizeof(js_SocketAddress));
    if (!t) return JS_EXCEPTION;
    SOCKETADDRESS_COPY(t, taddr->ai_addr);
    JSValue proto = JS_GetClassProto(ctx, js_SocketAddress_ClassID);
    if (JS_IsException(proto)) {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };  
    JSValue obj = JS_NewObjectProtoClass(ctx, proto, js_SocketAddress_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };
    JS_SetOpaque(obj, t);
    return obj;
};

/*
static JSValue js_NsList_first(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_NsList *s = JS_GetOpaque2(ctx, this_val, js_NsList_ClassID);
    if (!s) return JS_EXCEPTION;      
    struct addrinfo *taddr = s->head;    
    s->index = 0; s->current = taddr;
    if (!taddr->ai_addr) {
        JS_ThrowInternalError(ctx,"record is missing it's address.");
        return JS_EXCEPTION;
    };
    
    js_SocketAddress *t =  js_mallocz(ctx, sizeof(js_SocketAddress));
    if (!t) return JS_EXCEPTION;
    SOCKETADDRESS_COPY(t, taddr->ai_addr);
    JSValue proto = JS_GetClassProto(ctx, js_SocketAddress_ClassID);
    if (JS_IsException(proto)) {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };  
    JSValue obj = JS_NewObjectProtoClass(ctx, proto, js_SocketAddress_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };
    JS_SetOpaque(obj, t);
    return obj;
};
*/

static JSValue js_NsList_next_shift(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv, int magic) {
    js_NsList *s = JS_GetOpaque2(ctx, this_val, js_NsList_ClassID);
    if (!s) return JS_EXCEPTION;      
    struct addrinfo *current = s->current;
    if (!current) {
        return JS_NULL;
    };
    struct addrinfo *result;
    if (magic==1) {
        result = current;
    } else {
        result = current->ai_next;
    };
        // advance anyway! for next time
        s->current = s->current->ai_next;
        s->index++;
          
    if (!result) {
        return JS_NULL;
    };
    if (!result->ai_addr) {
        JS_ThrowInternalError(ctx,"record is missing it's address.");
        return JS_EXCEPTION;
    };

    js_SocketAddress *t =  js_mallocz(ctx, sizeof(js_SocketAddress));
    if (!t) return JS_EXCEPTION;
    SOCKETADDRESS_COPY(t, result->ai_addr);

    JSValue proto = JS_GetClassProto(ctx, js_SocketAddress_ClassID);
    if (JS_IsException(proto)) {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };  
    JSValue obj = JS_NewObjectProtoClass(ctx, proto, js_SocketAddress_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  {
        js_free(ctx, t);
        return JS_EXCEPTION;  
    };
    JS_SetOpaque(obj, t);
    return obj;
};

static JSValue js_NsList_get(JSContext *ctx, JSValueConst this_val, int magic) {
    js_NsList *s = JS_GetOpaque2(ctx, this_val, js_NsList_ClassID);
    if (!s) return JS_EXCEPTION; 
    if (magic==0) { // index
        return JS_NewInt32(ctx, s->index);
    };     
    
    return JS_EXCEPTION;
};

static JSValue js_NsList_set(JSContext *ctx, JSValueConst this_val,
                             JSValue argv, int magic) {
    js_NsList *s = JS_GetOpaque2(ctx, this_val, js_NsList_ClassID);
    if (!s) return JS_EXCEPTION;      
    if (magic==0) { // index
        JSValue val = argv; 
        int v = 0, i = 0;
        JS_ToInt32(ctx, &v, val);
        struct addrinfo *taddr = s->head;
        if (!taddr) {
            JS_ThrowInternalError(ctx,"NsList address list is lost.");
            return JS_EXCEPTION;        
        };  
        while (i < v) {
            if (!taddr->ai_next) {
                break;
            };
            i++;
            taddr = taddr->ai_next;
        };
        s->index = i;
        s->current = taddr;
        return JS_NULL;
    };
    return JS_EXCEPTION;
};

/* leaving this signture in the queue to copy-paste until tools are up
static JSValue js_NsList_first(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_NsList *s = JS_GetOpaque2(ctx, this_val, js_NsList_ClassID);
    if (!s) return JS_EXCEPTION;      
    return JS_EXCEPTION;
};
*/

/***** Socket section *******************************************************
 * 
 * 
 * 
*******************************************************************************/

static void js_Socket_finalizer(JSRuntime *rt, JSValue val) {
    js_Socket *s = JS_GetOpaque(val, js_Socket_ClassID);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    if (s) { 
        if (s->handle) {
            //shutdown(s->handle, SD_BOTH);
#if defined(_WIN32)
            closesocket(s->handle);
#else
            close(s->handle);
#endif
           // s->handle = 0;
        };
        if (s->linecache.data != 0) { 
            free(s->linecache.data);
        };
        js_free_rt(rt, s);
    };
      
};

static JSValue js_Socket_from_handle(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_Socket *s;
    JSValue proto = JS_UNDEFINED, obj = JS_UNDEFINED;
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0])) return JS_EXCEPTION;
    s = js_mallocz(ctx, sizeof(js_Socket));
    if (!s) return JS_EXCEPTION;
    proto = JS_GetClassProto(ctx, js_Socket_ClassID);
    if (JS_IsException(proto)) goto clone_socket_fail;  
    obj = JS_NewObjectProtoClass(ctx, proto, js_Socket_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  goto clone_socket_fail;
    LINECACHE_ZERO(s->linecache);
    s->handle = (SOCKET) handle;
    s->error = 0;
    JS_SetOpaque(obj, s);
    return obj;
 clone_socket_fail:
    if (s) js_free(ctx, s);
    //JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
                             
};

// sock.socket(AF_INET, SOCK_STREAM, 0)
static JSValue js_Socket_isnew(JSContext *ctx, JSValueConst new_target,
                             int argc, JSValueConst *argv, const int magic) {
    js_Socket *s;
    JSValue proto = JS_UNDEFINED, obj = JS_UNDEFINED;
    int af, stype, pr;
    SOCKET handle;
    
    if (argc <= 0) af = 2; // AF_INET
    else if (JS_ToInt32(ctx, &af, argv[0])) return JS_EXCEPTION;
    if (argc <= 1) stype = 1; // SOCK_STREAM
    else if (JS_ToInt32(ctx, &stype, argv[1])) return JS_EXCEPTION;
    if (argc <= 2) pr = 0; // choose protocol for me
    else if (JS_ToInt32(ctx, &pr, argv[2])) return JS_EXCEPTION;
    
    handle = socket(af, stype, pr); // actual c call
    if (handle == INVALID_SOCKET) {
        int e = GET_SOCKET_ERROR ();
        JS_FreeValue(ctx, obj);
        JS_ThrowTypeError(ctx, "failed to create socket with error: %d", e);
        return JS_EXCEPTION;
    };
    if (magic == 2) return JS_NewInt64(ctx, (int64_t) handle);

    s = js_mallocz(ctx, sizeof(js_Socket ));
    if (!s) return JS_EXCEPTION;
    s->error = 0;
    s->handle = handle;
    if (magic == 1) {  // real proto ... new()
        if (JS_IsUndefined(new_target)) {
            proto = JS_GetClassProto(ctx, js_Socket_ClassID);
        } else {
            proto = JS_GetPropertyStr(ctx, new_target, "prototype");
        };
    } else if (magic == 0) { 
        proto = JS_GetClassProto(ctx, js_Socket_ClassID);
    };   
    if (JS_IsException(proto)) {
        goto socket_fail;
    };
    obj = JS_NewObjectProtoClass(ctx, proto, js_Socket_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  {
        goto socket_fail;
    };
    LINECACHE_ZERO(s->linecache);
    JS_SetOpaque(obj, s);
    return obj;
 socket_fail:
    if (s) js_free(ctx, s);
    //JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
};

static JSValue js_Socket_ctor(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    return js_Socket_isnew(ctx, this_val, argc, argv, 0);
};

static JSValue js_Socket_ctor_new(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    return js_Socket_isnew(ctx, this_val, argc, argv, 1);
};
                             
static JSValue js_socks_socket_handle(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    return js_Socket_isnew(ctx, this_val, argc, argv, 2);
};
                             
// other member functions
static JSValue js_Socket_get(JSContext *ctx, JSValueConst this_val, int magic) { 
    js_Socket *s = JS_GetOpaque2(ctx, this_val, js_Socket_ClassID);
    if (!s) return JS_EXCEPTION;
    if (magic == 0) return JS_NewInt64(ctx, s->handle);
    else if (magic == 1)  return JS_NewInt64(ctx, s->error);
    else return JS_UNDEFINED;
};

static JSValue js_Socket_set(JSContext *ctx, JSValueConst this_val, JSValue val, int magic) {
    js_Socket *s = JS_GetOpaque2(ctx, this_val, js_Socket_ClassID);
    if (!s) return JS_EXCEPTION;
    int64_t v; 
    if (magic == 0) {  // port
        if (JS_ToInt64(ctx, &v, val)) return JS_EXCEPTION;
        s->handle = v;
    } else if (magic == 1) {  // port
        if (JS_ToInt64(ctx, &v, val)) return JS_EXCEPTION;
        s->error = v;
    };
    return JS_UNDEFINED;
};

static JSValue js_Socket_getopt_setopt(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv, const int magic) {
                             
    js_Socket *s = JS_GetOpaque(this_val, js_Socket_ClassID);
    if (!s) return JS_EXCEPTION;
    if (!s->handle) {
      JS_ThrowTypeError(ctx, "setopts failed. socket was lost!");
       return JS_EXCEPTION; 
    };
    int64_t optval = 0, result = 0;
    int level, optname = 0; 
    int32_t optlen = 8;
    if (magic >= 2) {
        if (argc < 3) {
           JS_ThrowTypeError(ctx, "setsockopt function requires 4 params");
            return JS_EXCEPTION;
        };
    } else {
        if (argc < 2) {
            JS_ThrowTypeError(ctx, "getsockopt . getsockoptlen function requires 2");
            return JS_EXCEPTION;
        };
    }
    if (JS_ToInt32(ctx, &level, argv[0])) return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &optname, argv[1])) return JS_EXCEPTION;
    if ((argc >= 3) && (JS_ToInt64(ctx, &optval, argv[2]) != 0)) return JS_EXCEPTION;
    if ((argc >= 4) && (JS_ToInt32(ctx, &optlen, argv[3]) != 0)) return JS_EXCEPTION;
    
    if (magic==2) { //setsockopt
        s->error = 0;
        result = setsockopt(s->handle, level, optname, (char*) &optval, optlen);
        if (result) {
            s->error = GET_SOCKET_ERROR();
            JS_ThrowPlainError(ctx, "setsockopt returned error: %d", s->error);
            return JS_EXCEPTION;
        };
        return JS_NULL;
    }; 
    s->error = 0;
    result = getsockopt(s->handle, level, optname, (char*) &optval, (socklen_t *) &optlen);
    if (result) {
        s->error = GET_SOCKET_ERROR();
        if (magic == 2) { JS_ThrowPlainError(ctx, "getsockopt returned error: %d", s->error); }
        else { JS_ThrowPlainError(ctx,"(from setsockopt) getsockopt returned error: %d", s->error); };
        return JS_EXCEPTION;
    };
    if (magic == 1) {
        return JS_NewInt32(ctx, optlen);
     } else {
        return JS_NewInt64(ctx, optval);
     };
    return JS_EXCEPTION;
};

static JSValue js_Socket_recvline(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_Socket *s = JS_GetOpaque(this_val, js_Socket_ClassID);
    if (!s) return JS_EXCEPTION;
    if (!s->handle) {
      JS_ThrowTypeError(ctx, "get request failed. socket was lost!");
       return JS_EXCEPTION; 
    };

    int e = 0, len = 0, state = 0; 
    char * buff = s->linecache.data;
    if (buff == 0)  {
      LINECACHE_CTOR(s->linecache, SOCKET_CACHE_SIZE); 
      buff = s->linecache.data;
    };
    int start = s->linecache.first, end = s->linecache.last;
    JSValue tmpstring; // output string
    
    if (start == end) { // newly created buffer or data was pulled
#ifdef WIN321 
        DWORD bread;
        ReadFile((HANDLE) s->handle, buff, s->linecache.cap, &bread, 0);
        len = bread;
#else
        len = recv(s->handle, buff, s->linecache.cap, 0);
#endif
        if (len == SOCKET_ERROR ) goto socket_exception;
        if (len == 0) return JS_NULL;
        start = 0; end = len; 
        s->linecache.last = len; 
    } else { // return to current point
    };

    state = 1; int i = start, terminator = 0;
    recvline_scan:
    while (state==1) {
        if (i >= end) {
          state = 2; break;  // need more data, string is pending
        } else if (buff[i] == '\r') {
            terminator = i; state = 4;  // found string, data hanging on \r          
            i++;
            if (i >= end) {
                break;  // need more data hanging on \r
            };
            if (buff[i] == '\n') {
                i++;
                buff[terminator] = 0;          
                state = 3; // finished string
                break;
            } else {
                state = 1; terminator = 0;
            };
        } else if (buff[i] == '\n') {
            terminator = i;
            buff[i] = 0; i++; 
            state = 3; 
        } else i++;
    };
        
    if (state != 3) { // string is not found yet
        i = 0; len = end - start;
        while (i < len) { buff[i] = buff[i+start]; i++; }; // copy old data
        start = 0; len = i - s->linecache.cap;
        // lets' be safe! store the linecache just in case we crash. it's changed.
        s->linecache.first = 0; s->linecache.last = i;        
        len = recv(s[0].handle, &buff[i], len, 0);
        if (len == SOCKET_ERROR ) goto socket_exception;
        if (len == 0) goto socket_failed_read;
        end = len + i;  s->linecache.last = end; 
        if (state == 4) { 
            if (buff[i] == '\n') {
                i++;
                buff[terminator] = 0;
            }; 
            state = 3; // finished string
        };
    };  
    
    if (state == 3) { // string is found
        tmpstring = JS_NewString(ctx, &buff[start]);
        if (JS_IsException(tmpstring)) return JS_EXCEPTION;
        //JS_SetOpaque(ctx, t);
        s->linecache.first = i; // consume the string
        return tmpstring;
    };
    
    goto recvline_scan;        
    socket_exception:
        e = GET_SOCKET_ERROR();
        JS_ThrowTypeError(ctx, "get request failed to read with error: %d", e);
        return JS_EXCEPTION;
    socket_failed_read:
       // JS_FreeValue(obj);
        JS_ThrowTypeError(ctx, "get request failed to read.");
        return JS_EXCEPTION;
};


static JSValue js_Socket_sendstring(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_Socket *s = JS_GetOpaque(this_val, js_Socket_ClassID);
    if (!s) return JS_EXCEPTION;
    if (!s->handle) {
      JS_ThrowTypeError(ctx, "missing socket handle.");
       return JS_EXCEPTION; 
    };
    int start = 0, len = 0, bytes = 0;
    size_t end = 0;
    const char* data = JS_ToCStringLen(ctx, &end, argv[0]);
    if (!data) {
        JS_ThrowTypeError(ctx, "expected data buffer.");
        return JS_EXCEPTION;
    };
    len = end;
    
    while (start < end) {
        bytes = send(s->handle, &data[start], len, 0);
        if (bytes == INVALID_SOCKET) {
            int e = GET_SOCKET_ERROR();
            JS_FreeCString(ctx, data);
#ifdef QJC_IO_NOTHROW
            return JS_NewInt32(ctx, e);
#else
            JS_ThrowPlainError(ctx, "sendstring failed with error: %d", e);
            return JS_EXCEPTION;
#endif            
        };
        len = end - start;
        start += bytes; 
        if (bytes == 0) {
            JS_FreeCString(ctx, data);
#ifdef QJC_IO_NOTHROW
            return JS_NewInt32(ctx, start);
#else
            JS_ThrowRangeError(ctx, "sendstring failed to write data.");
            return JS_EXCEPTION;
#endif            
        };    
    };
    JS_FreeCString(ctx, data);
    return JS_NULL;
};

static JSValue js_Socket_bind_connect(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv, int magic) {
    js_Socket *s = JS_GetOpaque(this_val, js_Socket_ClassID);
    if (!s->handle) {
      JS_ThrowTypeError(ctx, "missing socket handle.");
       return JS_EXCEPTION; 
    };
    if (!s) return JS_EXCEPTION;
    js_SocketAddress *sa; //, *saddr = &s->socketaddress;
//    if (argc >= 1) {
        sa = JS_GetOpaque(argv[0], js_SocketAddress_ClassID);
        if (!sa) {
            JS_ThrowTypeError(ctx, "bind/connect[%d]: expected a SocketAddress.", magic);
            return JS_EXCEPTION;
        };
        //SOCKETADDRESS_COPY(saddr[0], sa[0]);
        //memcpy (&s->c_addr6, &sa->c_addr6, sizeof(s->c_addr6));                    
//    };
    int len = sizeof(sa->c_addr), result = 0;
    if (sa->c_addr.sin_family == AF_INET6) len = sizeof(sa->c_addr6);
    if (magic) {
        result = connect(s->handle, (struct sockaddr*) &(sa->c_addr), len);
    } else {
    /* setsockopts 
          int optval = 1;
          setsockopt(s->handle, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));
          setsockopt(s->handle, SO_RCVTIMEO, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));
          setsockopt(s->handle, SO_SNDTIMEO, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));
    //S*/
        result = bind(s->handle, (struct sockaddr*) &(sa->c_addr), len);
    };
    if (result < 0) {
    int e = GET_SOCKET_ERROR();
#ifdef QJC_IO_NOTHROW
            return JS_NewInt32(ctx, e);
#else      
      JS_ThrowPlainError(ctx, "bind/connect[%d] failed with error: %d", magic, e);
      return JS_EXCEPTION;
#endif
    };
    return JS_NULL;
};

static JSValue js_Socket_listen(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_Socket *s = JS_GetOpaque2(ctx, this_val, js_Socket_ClassID);
    if (!s) {
        return JS_EXCEPTION;
    };
    int n = _SOCKET_LISTEN_BACKLOG;
    if ((argc>=1) && (JS_ToInt32(ctx, &n, argv[0]))) return JS_EXCEPTION;
    if (listen(s->handle, n)) {
        int e = GET_SOCKET_ERROR();
#ifdef QJC_IO_NOTHROW
        return JS_NewInt32(ctx, e);
#else
        JS_ThrowPlainError(ctx, "listen failed with error: %d", e);
        return JS_EXCEPTION;
#endif            
    };
    return JS_UNDEFINED;    
};
                            
static JSValue js_Socket_accept(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_Socket *s = JS_GetOpaque2(ctx, this_val, js_Socket_ClassID);
    //JSValue obj = JS_UNDEFINED, proto = JS_UNDEFINED;
    if (!s) {
        return JS_EXCEPTION;
    };
    js_SocketAddress *arg0 = JS_GetOpaque2(ctx, argv[0], js_SocketAddress_ClassID);
    if (!s) {
        JS_ThrowTypeError( ctx, "arg 0 requires SocketAddress.");
        return JS_EXCEPTION;
    };    
#if defined(_WIN32)
    int l = sizeof(arg0->c_addr6);
#else
    socklen_t l = sizeof(arg0->c_addr6);
#endif
    SOCKET tmp = accept(s->handle, (struct sockaddr *) &arg0->c_addr, &l);  
    if (tmp == INVALID_SOCKET) {
        int e = GET_SOCKET_ERROR();
        s->error = e;
#ifdef QJC_IO_NOTHROW
        return JS_NewInt64(ctx, INVALID_SOCKET);
#else
        JS_ThrowPlainError(ctx, "accept failed with error: %d", e);
        return JS_EXCEPTION;
#endif            
    };
/*
    proto = JS_GetClassProto(ctx, js_Socket_ClassID);
    if (JS_IsException(proto)) goto accept_fail;   
    obj = JS_NewObjectProtoClass(ctx, proto, js_Socket_ClassID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))  goto accept_fail;
    JS_SetOpaque(obj, tmp);
*/
    return JS_NewInt64(ctx, tmp);
};
                            
static JSValue js_Socket_shutdown(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_Socket *s = JS_GetOpaque2(ctx, this_val, js_Socket_ClassID);
    if (!s) {
        return JS_EXCEPTION;
    };
    if (shutdown(s->handle, SD_BOTH)) {
        int e = GET_SOCKET_ERROR();
#ifdef QJC_IO_NOTHROW
        return JS_NewInt32(ctx, e);
#else
        JS_ThrowPlainError(ctx, "shutdown failed with error: %d", e);
        return JS_EXCEPTION;
#endif            
    };
    return JS_UNDEFINED;
};

                       
// (len) recv / send (*buffer, int len)   
static JSValue js_Socket_recv_send(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv, int magic) {
    js_Socket *s = JS_GetOpaque2(ctx, this_val, js_Socket_ClassID);
    if (!s) {
        return JS_EXCEPTION;
    };
    if (!s->handle)  {
        JS_ThrowTypeError(ctx, "socket shutdown failed. no socket.");
        return JS_EXCEPTION;
    };
    
    int len = 0, offset = 0; size_t size = 0; 
    uint8_t *buf;    
    buf = JS_GetArrayBuffer(ctx, &size, argv[0]);
    if (!buf) return JS_EXCEPTION;
    if (magic < 2) {
        if (argc <= 1) len = size;
        else if (JS_ToInt32(ctx, &len, argv[1])) return JS_EXCEPTION;
    } else {
        if (argc <= 1) offset = 0;
        else if (JS_ToInt32(ctx, &offset, argv[1])) return JS_EXCEPTION;
        if (argc <= 2) len = size;
        else if (JS_ToInt32(ctx, &len, argv[2])) return JS_EXCEPTION;
    };
    if ((offset + len) > size) {
        JS_ThrowRangeError(ctx, "recv/send[%d] array buffer overflow", magic);
        return JS_EXCEPTION;
    };
    if (magic & 1) {
        len = send(s->handle, (char*) buf + offset, len, 0);
        if (len == SOCKET_ERROR ) {
            int e = GET_SOCKET_ERROR();
#ifdef QJC_IO_NOTHROW
            return JS_NewInt32(ctx, e);
#else
            JS_ThrowPlainError(ctx, "send failed with error: %d.", e);
            return JS_EXCEPTION;
#endif
        };
    } else {
        //if (magic == 3) buf = &buf[offset];
        int total = 0;
        if (s->linecache.data != 0) {
            int start = s->linecache.first;
            int end = s->linecache.last; 
            const char * cache = &s->linecache.data[start];
            int incache = end - start;
            if (len < incache) {  // we can get it all out of cache
                for (int i = 0; i < len; i++) buf[i + offset] = cache[i];
                total += len;
                s->linecache.first = start + len;                
            } else {
                for (int i = 0; i < incache; i++) buf[i + offset] = cache[i];
                total += incache;
                s->linecache.first = s->linecache.last;
                len -= incache;
              if(len) {
                int result = recv(s->handle, (char*) &buf[incache + offset], len, 0);
                if (result == SOCKET_ERROR ) {
                    int e = GET_SOCKET_ERROR();
#ifdef QJC_IO_NOTHROW
                    return JS_NewInt32(ctx, e);
#else
                    JS_ThrowPlainError(ctx, "recv failed with error: %d.", e);
                    return JS_EXCEPTION;
#endif
                };
                total += result;
              }; 
              
            };
            return JS_NewInt64(ctx, total);
        };
    };
    return JS_NewInt64(ctx, len);
};
                            
static JSValue js_Socket_closesocket(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
    js_Socket *s = JS_GetOpaque2(ctx, this_val, js_Socket_ClassID);
    if (!s) {
        return JS_EXCEPTION;
    };
    if (!s->handle)  {
        JS_ThrowTypeError(ctx, "socket shutdown failed. no socket.");
        return JS_EXCEPTION;
    };
#if defined(_WIN32)
    if (closesocket(s->handle)) {
#else
    if (close(s->handle)) {
#endif
        int e = GET_SOCKET_ERROR();
#ifdef QJC_IO_NOTHROW
        return JS_NewInt32(ctx, e);
#else
        JS_ThrowPlainError(ctx, "socket shutdown failed with error: %d.", e);
        return JS_EXCEPTION;
#endif
    };
    s->handle = 0;  
    return JS_UNDEFINED;
};


/************************ sock module functions *********************/

static JSValue js_socks_init(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
#if defined(_WIN32)
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        JS_ThrowTypeError(ctx, "sock failed to initialize.");
        return JS_EXCEPTION;
    };
#endif
    return JS_UNDEFINED;
};


static JSValue js_socks_cleanup(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv) {
#if defined(_WIN32)
    WSACleanup();
#endif
    return JS_UNDEFINED;
};

static JSValue js_socks_recv_send(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv, int magic) {
    int64_t handle;
    int64_t ret = 0;
/*
#if defined(WIN32)
    int64_t ret = 0;
#else
    int ret = 0;
#endif
*/
    size_t size = 0;
    int32_t pos, len;
    uint8_t *buff = JS_GetArrayBuffer(ctx, &size, argv[1]);
    if (!buff) return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &pos, argv[2])) return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &len, argv[3])) return JS_EXCEPTION;
    if (JS_ToInt64(ctx, &handle, argv[0])) return JS_EXCEPTION;
    
    if ((pos + len) > size) {
        JS_ThrowRangeError(ctx, "recv/send[%d] array buffer overflow", magic);
        return JS_EXCEPTION;
    };
#if defined(WIN32)
    if (magic) {
        ret = send((SOCKET) handle, ( char*) buff + pos, len, 0);
    } else {
        ret = recv((SOCKET) handle, ( char*) buff + pos, len,  0);    
    };
#elif defined(WIN321) 
    if (magic) {
        WriteFile((HANDLE) handle, buff + pos, len, (DWORD*) &ret, 0);
    } else {
        ReadFile((HANDLE) handle, buff + pos, len, (DWORD*) &ret, 0);    
    };
#else
    if (magic) {
        ret = send(handle, buff + pos, (int) len, 0);
    } else {
        ret = recv(handle, buff + pos, (int) len, 0);    
    };
#endif
    return JS_NewInt64(ctx, ret);
};


/**** TBL / Interface ********************************************************/

static const JSCFunctionListEntry js_socks_proto_functs[] = {
    JS_CFUNC_DEF("init", 0, js_socks_init),
    JS_CFUNC_DEF("cleanup", 0, js_socks_cleanup),
    JS_CFUNC_DEF("lookup", 2, js_socks_lookup),
    JS_CFUNC_DEF("createSocket", 3, js_Socket_ctor),
    JS_CFUNC_DEF("socket", 3, js_socks_socket_handle),
    JS_CFUNC_DEF("cloneSocket", 1, js_Socket_from_handle),
    JS_CFUNC_DEF("parseSocketAddress", 1, js_SocketAddress_ctor),
    JS_CFUNC_MAGIC_DEF("recv", 4, js_socks_recv_send, 0 ),
    JS_CFUNC_MAGIC_DEF("send", 4, js_socks_recv_send, 1 ),
// socket family
    OS_FLAG(AF_UNSPEC),
    OS_FLAG(AF_INET),
    OS_FLAG(AF_IPX),
    OS_FLAG(AF_APPLETALK),
    OS_FLAG(AF_NETBIOS),
    OS_FLAG(AF_INET6),
    OS_FLAG(AF_IRDA),
    OS_FLAG(AF_BTH),
// socket stream
    OS_FLAG(SOCK_STREAM),
    OS_FLAG(SOCK_DGRAM),
    OS_FLAG(SOCK_RAW),
    OS_FLAG(SOCK_RDM),
    OS_FLAG(SOCK_SEQPACKET),
// socket protocol
    OS_FLAG(IPPROTO_ICMP),
    OS_FLAG(IPPROTO_IGMP),
    OS_FLAG(IPPROTO_TCP),
    OS_FLAG(IPPROTO_UDP),
    OS_FLAG(IPPROTO_ICMPV6),
// NsList - ai_flags
    OS_FLAG(AI_PASSIVE),
    OS_FLAG(AI_CANONNAME),
    OS_FLAG(AI_NUMERICHOST),
    OS_FLAG(AI_NUMERICSERV),
    OS_FLAG(AI_ALL),
    OS_FLAG(AI_ADDRCONFIG),
    // OS_FLAG(AI_V4MAPPED),
    // OS_FLAG(AI_NON_AUTHORITATIVE),
    // OS_FLAG(AI_SECURE),
    // OS_FLAG(AI_RETURN_PREFERRED_NAMES),
    // OS_FLAG(AI_FQDN),
    // OS_FLAG(AI_FILESERVER),
// setsockopt - level / optname
    OS_FLAG(SOL_SOCKET),
    OS_FLAG(SO_REUSEADDR),
    OS_FLAG(TCP_NODELAY),
    OS_FLAG(SO_SNDTIMEO),
    OS_FLAG(SO_RCVTIMEO),
};


static const JSCFunctionListEntry js_Socket_proto_functs[] = {
    JS_CFUNC_DEF("shutdown", 0, js_Socket_shutdown),
    JS_CFUNC_DEF("closesocket", 0, js_Socket_closesocket),
    JS_CFUNC_MAGIC_DEF("bind", 1, js_Socket_bind_connect, 0 ),
    JS_CFUNC_MAGIC_DEF("connect", 1, js_Socket_bind_connect, 1 ),
    JS_CFUNC_DEF("listen", 1, js_Socket_listen),
    JS_CFUNC_DEF("accept", 0, js_Socket_accept),
    JS_CFUNC_DEF("recvline", 0, js_Socket_recvline),
    JS_CFUNC_DEF("sendstring", 1, js_Socket_sendstring),
    JS_CFUNC_MAGIC_DEF("recv", 2, js_Socket_recv_send, 0 ),
    JS_CFUNC_MAGIC_DEF("send", 2, js_Socket_recv_send, 1 ),
    JS_CFUNC_MAGIC_DEF("read", 3, js_Socket_recv_send, 2 ),
    JS_CFUNC_MAGIC_DEF("write", 3, js_Socket_recv_send, 3 ),
    JS_CFUNC_MAGIC_DEF("setsockopt", 2, js_Socket_getopt_setopt, 2 ),
    JS_CFUNC_MAGIC_DEF("getsockopt", 3, js_Socket_getopt_setopt, 0 ),
    JS_CFUNC_MAGIC_DEF("getsockoptlen", 3, js_Socket_getopt_setopt, 1 ),
    JS_CGETSET_MAGIC_DEF("handle", js_Socket_get, js_Socket_set, 0),
    JS_CGETSET_MAGIC_DEF("error",  js_Socket_get, js_Socket_set, 1),
    JS_CGETSET_MAGIC_DEF("errno",  js_Socket_get, js_Socket_set, 1),
    JS_CGETSET_MAGIC_DEF("result",  js_Socket_get, js_Socket_set, 1),
    OS_FLAG(SD_BOTH),
};

static const JSCFunctionListEntry js_NsList_proto_functs[] = {
    JS_CFUNC_MAGIC_DEF("next", 0, js_NsList_next_shift, 0),
    JS_CFUNC_MAGIC_DEF("shift", 0, js_NsList_next_shift, 1),
    JS_CFUNC_MAGIC_DEF("first", 0, js_NsList_first_current, 0),
    JS_CFUNC_MAGIC_DEF("current", 0, js_NsList_first_current, 1),
    JS_CGETSET_MAGIC_DEF("index", js_NsList_get, js_NsList_set, 0),
};

static const JSCFunctionListEntry js_SocketAddress_proto_functs[] = {
    JS_CGETSET_MAGIC_DEF("address", js_SocketAddress_get, js_SocketAddress_set, 0),
    JS_CGETSET_MAGIC_DEF("port",    js_SocketAddress_get, js_SocketAddress_set, 1),
    JS_CGETSET_MAGIC_DEF("family",  js_SocketAddress_get, js_SocketAddress_set, 2),
    JS_CFUNC_DEF("parse", 1, js_SocketAddress_parse_obj),
    OS_FLAG(INADDR_ANY),
};

/* ********* Module section *******************************************************
 * initialize module via proto information 
 * 
 */
static int js_module_init_socks(JSContext *ctx, JSModuleDef *m) {
    JSRuntime *rt = JS_GetRuntime(ctx);
    JSValue js_SocketAddress_proto, js_SocketAddress_new;    
    /* create the SocketAddress class */
    JS_NewClassID(rt, &js_SocketAddress_ClassID);
    JS_NewClass(rt, js_SocketAddress_ClassID, &js_SocketAddress_ClassDef);
    js_SocketAddress_proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, js_SocketAddress_proto, 
        js_SocketAddress_proto_functs, countof(js_SocketAddress_proto_functs));
    /* set proto.constructor and ctor.prototype */        
    js_SocketAddress_new = JS_NewCFunction2(ctx, js_SocketAddress_ctor_new, "SocketAddress", 
                                                1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, js_SocketAddress_new, js_SocketAddress_proto);
    JS_SetClassProto(ctx, js_SocketAddress_ClassID, js_SocketAddress_proto);
    JS_SetModuleExport(ctx, m, "SocketAddress", js_SocketAddress_new);
    
    /* now do the same thing with NsList */
    JSValue js_NsList_proto, js_NsList_new;
    /* create the NsList class */
    JS_NewClassID(rt, &js_NsList_ClassID);
    JS_NewClass(rt, js_NsList_ClassID, &js_NsList_ClassDef);
    js_NsList_proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, js_NsList_proto, js_NsList_proto_functs, 
        countof(js_NsList_proto_functs));
    /* set proto.constructor and ctor.prototype */        
    js_NsList_new = JS_NewCFunction2(ctx, js_NsList_ctor_new, "NsList", 
                                                1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, js_NsList_new, js_NsList_proto);
    JS_SetClassProto(ctx, js_NsList_ClassID, js_NsList_proto);
    JS_SetModuleExport(ctx, m, "NsList", js_NsList_new);

    /* now do the same thing with Socket */
    JSValue js_Socket_proto, js_Socket_new;
    /* create the Socket class */
    JS_NewClassID(rt, &js_Socket_ClassID);
    JS_NewClass(rt, js_Socket_ClassID, &js_Socket_ClassDef);
    js_Socket_proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, js_Socket_proto, js_Socket_proto_functs, 
        countof(js_Socket_proto_functs));
    /* set proto.constructor and ctor.prototype */        
    js_Socket_new = JS_NewCFunction2(ctx, js_Socket_ctor_new, "Socket", 
                                                1, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, js_Socket_new, js_Socket_proto);
    JS_SetClassProto(ctx, js_Socket_ClassID, js_Socket_proto);
    JS_SetModuleExport(ctx, m, "Socket", js_Socket_new);

    return JS_SetModuleExportList(ctx, m, PROTO_FUNCTS(js_socks));
};

#ifndef TACK_ON_MODULE

#ifndef JS_EXTERN
#ifdef _WIN32
#define JS_EXTERN __declspec(dllexport)
#else
#define JS_EXTERN
#endif
#endif

JS_EXTERN JSModuleDef *js_init_module(JSContext *ctx, const char *module_name)
{
#else
JSModuleDef *js_init_module_socks(JSContext *ctx, const char *module_name) {
#endif
    //_socks_clogger = dup(fileno(stderr));
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_module_init_socks);
    if (!m)  return NULL;
    JS_AddModuleExport(ctx, m, "SocketAddress");
    JS_AddModuleExport(ctx, m, "NsList");
    JS_AddModuleExport(ctx, m, "Socket");
    JS_AddModuleExportList(ctx, m, PROTO_FUNCTS(js_socks));
    return m;
};

