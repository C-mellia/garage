#ifndef SERVER_H
#   define SERVER_H 1

#include <regex.h>
#include <stddef.h>
#include <pthread.h>

#include <garage/array.h>
#include <garage/string.h>
#include <garage/slice.h>

typedef struct ResponseWriter *ResponseWriter;
typedef struct Request *Request;

typedef enum ResponseStatus {
    RESPONSE_INVAL=-1,
    RESPONSE_OK=200,
    RESPONSE_NOT_FOUND=404,
    RESPONSE_BAD_REQUEST=400,
    RESPONSE_INTERNAL_SERVER_ERROR=500,
    RESPONSE_METHOD_NOT_ALLOWED=405,
    RESPONSE_NOT_IMPLEMENTED=501,
    RESPONSE_HTTP_VERSION_NOT_SUPPORTED=505,
    RESPONSE_SERVICE_UNAVAILABLE=503,
    RESPONSE_GATEWAY_TIMEOUT=504,
} ResponseStatus;

typedef enum RequestMethod {
    REQUEST_INVAL=-1,
    REQUEST_GET,
    REQUEST_POST,
    REQUEST_PUT,
    REQUEST_DELETE,
    REQUEST_HEAD,
    REQUEST_OPTIONS,
    REQUEST_CONNECT,
    REQUEST_TRACE,
    REQUEST_PATCH,
    __REQUEST_COUNT,
} RequestMethod;

typedef void (*handle_func_t) (ResponseWriter rw, Request req);

struct Request {
    RequestMethod method;
    Slice url;
    // TODO: headers, body
};

struct ResponseWriter {
    int status;
    Array/* String */ headers;
    String body;
};

typedef struct HandlePack {
    RequestMethod method;
    regex_t url_reg;
    handle_func_t func;
} *HandlePack;

typedef struct Handler {
    Array/* HandlePack */ handle_funcs;
    handle_func_t default_handle;
} *Handler;

typedef struct Server {
    Handler handler;
    Array incoming; // task queue;
    int done, exit_sig;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int listen_s;
    struct {
        const char *port;
        size_t worker_threads;
    } config;
} *Server;

Handler handler_new();
void handler_cleanup(Handler h);
void handler_drop(Handler *h);
void handler_handle_func(Handler h, const char *route, handle_func_t func) __attribute__((nonnull(2, 3)));
void handler_default_handle(Handler h, handle_func_t func) __attribute__((nonnull(2)));

Server server_new(Handler handler, const char *port, size_t worker_threads);
void server_cleanup(Server sv);
void server_drop(Server *sv);
int server_listen_and_serve(Server sv);

Request req_new(RequestMethod method, Slice url);
void req_cleanup(Request req);
void req_drop(Request *req);
Request req_from_client(int client_s);

ResponseWriter rw_new(ResponseStatus status);
void rw_status(ResponseWriter rw, int status);
void rw_cleanup(ResponseWriter rw);
void rw_drop(ResponseWriter *rw);
void rw_write_header(ResponseWriter rw, const char *fmt, ...);
void rw_set_body(ResponseWriter rw, const char *body) __attribute__ ((nonnull(2)));
void rw_write_all(ResponseWriter rw, int fd);

HandlePack handle_pack_new(Slice method, Slice url, handle_func_t func);
void handle_pack_cleanup(HandlePack pack);

#endif // SERVER_H
