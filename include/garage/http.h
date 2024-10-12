#ifndef SERVER_H
#   define SERVER_H 1

#include <regex.h>
#include <stddef.h>
#include <pthread.h>

#include <garage/array.h>
#include <garage/string.h>
#include <garage/slice.h>

typedef struct response_writer *ResponseWriter;
typedef struct request *Request;

typedef enum response_status {
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
    __RESPONSE_COUNT,
} ResponseStatus;

typedef enum request_method {
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

typedef struct handler {
    Array/* HandlePack */ packs;
    handle_func_t default_handle;
} *Handler;

typedef struct server {
    Handler handler; // reference
    Array incoming; // task queue;
    int done, exit_sig, listen_s;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct {
        const char *port;
        size_t worker_threads;
    } config;
} *Server;

struct response_writer {
    int status;
    Array/* String */ headers;
    String body;
};

struct request {
    RequestMethod method;
    Slice url;
    // TODO: headers, body
};

typedef struct handle_pack {
    RequestMethod method;
    regex_t url_reg;
    handle_func_t func;
} *HandlePack;

void handler_init(Handler handler);
Handler handler_new();
void handler_cleanup(Handler handler);
void handler_drop(Handler *handler);

int handler_deb_dprint(int fd, Handler handler);
int handler_deb_print(Handler handler);

void handler_handle_func(Handler h, const char *route, handle_func_t func) __attribute__((nonnull(2, 3)));
void handler_default_handle(Handler h, handle_func_t func) __attribute__((nonnull(2)));

void server_init(Server server, Handler handler, const char *port, size_t worker_thread);
Server server_new(Handler handler, const char *port, size_t worker_threads);
void server_cleanup(Server sv);
void server_drop(Server *sv);

int server_deb_dprint(int fd, Server server);
int server_deb_print(Server server);

int server_listen_and_serve(Server sv);

void req_init(Request req, RequestMethod method, Slice url);
Request req_new(RequestMethod method, Slice url);
void req_cleanup(Request req);
void req_drop(Request *req);

ResponseWriter rw_new(ResponseStatus status);
void rw_status(ResponseWriter rw, int status);
void rw_cleanup(ResponseWriter rw);
void rw_drop(ResponseWriter *rw);

int rw_deb_dprint(int fd, ResponseWriter rw);
int rw_deb_print(ResponseWriter rw);

void rw_write_header(ResponseWriter rw, const char *fmt, ...);
void rw_set_body(ResponseWriter rw, const char *body) __attribute__ ((nonnull(2)));
void rw_write_all(ResponseWriter rw, int fd);

void handle_pack_init(HandlePack pack, Slice method, Slice url, handle_func_t func);
HandlePack handle_pack_new(Slice method, Slice url, handle_func_t func);
void handle_pack_cleanup(HandlePack pack);
void handle_pack_drop(HandlePack *pack);

#endif // SERVER_H
