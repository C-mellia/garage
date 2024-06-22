#ifndef SERVER_H
#define SERVER_H 1

#include <stddef.h>
#include <pthread.h>

#include <garage/array.h>

typedef struct Request {
    const char *url, *method;
    // headers, body
} Request;

typedef struct {
    int status;
    Array headers;
    const char *body;
} ResponseWriter;

typedef void (*handle_func_t) (ResponseWriter *rw, Request *req);

typedef struct Handler {
    Array handle_funcs;
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
void handler_handle_func(Handler h, const char *route, handle_func_t func) __attribute__((nonnull(2, 3)));
void handler_default_handle(Handler h, handle_func_t func) __attribute__((nonnull(2)));
void handler_cleanup(Handler h);

Server server_new(Handler handler, const char *port, size_t worker_threads);
void server_cleanup(Server sv);
int server_listen_and_serve(Server sv);

void rw_init(ResponseWriter *rw);
void rw_status(ResponseWriter *rw, int status);
void rw_cleanup(ResponseWriter *rw);
void rw_write_header(ResponseWriter *rw, const char *fmt, ...);
void rw_set_body(ResponseWriter *rw, const char *body) __attribute__ ((nonnull(2)));
void rw_write_all(ResponseWriter *rw, int fd);

#endif // SERVER_H
