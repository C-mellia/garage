#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/time.h>

#include <garage/garage.h>
#include <garage/http.h>
#include <garage/log.h>
#include <garage/slice.h>
#include <garage/string.h>

#include "./.http.c"
#include "./.reg.c"

static void *worker(void *args);
static int server_listen(const char *port);
static inline void __handler_init(Handler handler);
static inline void __server_init(Server server, Handler handler, const char *port, size_t worker_threads);
static inline void __rw_init(ResponseWriter rw, ResponseStatus status);
static inline void __req_init(Request req, RequestMethod method, Slice url);
static inline void __handle_pack_init(HandlePack pack, Slice method, Slice url, handle_func_t func);

void handler_init(Handler handler) {
    nul_check(Handler, handler), __handler_init(handler);
}

Handler handler_new() {
    Handler handler = malloc(sizeof *handler);
    alloc_check(malloc, handler, sizeof *handler);
    return __handler_init(handler), handler;
}

void handler_cleanup(Handler handler) {
    if (!handler) return;
    for (size_t i = 0; i < handler->packs->_len; ++i) {
        handle_pack_drop(arr_get(handler->packs, i));
    }
    arr_drop(&handler->packs);
}

void handler_drop(Handler *handler) {
    if (handler && *handler) handler_cleanup(*handler), free(*handler), *handler = 0;
}

int handler_deb_dprint(int fd, Handler handler) {
    if (!handler) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{packs: ");
    string_fmt_func(string, (void *)arr_deb_dprint, handler->packs);
    string_fmt(string, ", default_handle: %p}", handler->default_handle);
    return string_dprint(fd, string);
}

int handler_deb_print(Handler handler) {
    return fflush(stdout), handler_deb_dprint(1, handler);
}

void handler_handle_func(Handler handler, const char *route, handle_func_t func) {
    nul_check(Handler, handler);
    Slice Cleanup(slice_drop) route_slice = slice_new((void *)route, 1, strlen(route));

    Slice method = slice_split_at(route_slice, slice_search_item(route_slice, "/"));
    if (method) slice_trim(method, " \t\n", 3);
    slice_trim(route_slice, " \t\n", 3);
    Slice Cleanup(slice_drop) url = slice_split_once(route_slice, " ", 1);
    HandlePack pack = handle_pack_new(method, url, func);
    arr_push_back(handler->packs, &pack);
}

void handler_default_handle(Handler handler, handle_func_t func) {
    nul_check(Handler, handler);
    handler->default_handle = func;
}

void server_init(Server server, Handler handler, const char *port, size_t worker_thread) {
    nul_check(Server, server), nul_check(Handler, handler), __server_init(server, handler, port, worker_thread);
}

Server server_new(Handler handler, const char *port, size_t worker_threads) {
    nul_check(Handler, handler);
    Server server = malloc(sizeof *server);
    alloc_check(malloc, server, sizeof *server);
    return __server_init(server, handler, port, worker_threads), server;
}

void server_cleanup(Server server) {
    if (!server) return;
    fd_drop(&server->listen_s);
    server_threads_exit_wait(server);
    server->handler = 0;
    arr_drop(&server->incoming);
}

void server_drop(Server *server) {
    if (server && *server) server_cleanup(*server), free(*server), *server = 0;
}

int server_deb_dprint(int fd, Server server) {
    if (!server) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{handler: ");
    string_fmt_func(string, (void *)handler_deb_dprint, server->handler);
    if (!server->done) string_fmt(string, ", (done)");
    string_fmt(string, ", listen_s: %d, config: {port: %s, worker_threads: %zu}}",
               server->listen_s, server->config.port, server->config.worker_threads);
    return string_dprint(fd, string);
}

int server_deb_print(Server server) {
    return fflush(stdout), server_deb_dprint(1, server);
}

// Note: I kind of have an idea, that by creating yet another thread for the
// loop, that constantly accepting incomings and queuing up all the
// connections, so that this function is non block at all, and by creating a
// single way pipe, then return the readable side of the pipe, so all the logs
// generated while server is running can be redirected to the main function,
// and the logs can be handled in a more flexible way.
//
// Some technologies, like redirecting between threads with pipes, I've never tested
int server_listen_and_serve(Server server) {
    nul_check(Server, server);

    pthread_t thread;
    int client_s;
    Array incoming = server->incoming;
    pthread_mutex_t *mutex = &server->mutex;
    pthread_cond_t *cond = &server->cond;

    server->done += server->config.worker_threads;
    if (server->listen_s = server_listen(server->config.port), server->listen_s < 0) return 2; // other process is probably listening at the same port
    for (size_t i = 0; i < server->config.worker_threads; ++i) {
        pthread_create(&thread, 0, worker, server);
        pthread_detach(thread);
    }
    for (;;) {
        if (client_s = accept(server->listen_s, 0, 0), client_s >= 0) {
            pthread_mutex_lock(mutex);
            arr_push_back(incoming, &client_s);
            pthread_cond_signal(cond);
            pthread_mutex_unlock(mutex);
        } else {
            usleep(1000);
        }
    }
    return 0;
}

void rw_init(ResponseWriter rw, ResponseStatus status) {
    nul_check(ResponseWriter, rw), __rw_init(rw, status);
}

ResponseWriter rw_new(ResponseStatus status) {
    ResponseWriter rw = malloc(sizeof *rw);
    alloc_check(malloc, rw, sizeof *rw);
    return __rw_init(rw, status), rw;
}

void rw_cleanup(ResponseWriter rw) {
    if (!rw) return;
    for (size_t i = 0; i < rw->headers->_len; ++i) {
        String Cleanup(string_drop) header = deref(String, arr_get(rw->headers, i));
    }
    arr_drop(&rw->headers), string_drop(&rw->body);
}

void rw_drop(ResponseWriter *rw) {
    if (rw && *rw) rw_cleanup(*rw), free(*rw), *rw = 0;
}

int rw_deb_dprint(int fd, ResponseWriter rw) {
    if (!rw) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "ResponseWriter: {status: %d, headers: ", rw->status);
    string_fmt_func(string, (void *)arr_deb_dprint, rw->headers);
    string_fmt(string, ", body: ");
    string_fmt_func(string, (void *)string_deb_dprint, rw->body);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int rw_deb_print(ResponseWriter rw) {
    return fflush(stdout), rw_deb_dprint(1, rw);
}

void rw_write_header(ResponseWriter rw, const char *fmt, ...) {
    nul_check(ResponseWriter, rw);
    va_list args;
    va_start(args, fmt);
    String header = string_new();
    string_vfmt(header, fmt, args);
    va_end(args);
    arr_push_back(rw->headers, &header);
}

void rw_set_body(ResponseWriter rw, const char *body) {
    nul_check(ResponseWriter, rw);
    string_fmt(rw->body, "%s", body);
}

void rw_write_all(ResponseWriter rw, int fd) {
    nul_check(ResponseWriter, rw);
    dprintf(fd, "HTTP/1.0 %d %.*s\r\n",
            rw->status, (int)res_str[rw->status].len, (char *)res_str[rw->status].mem);
    for (size_t i = 0; i < rw->headers->_len; ++i) {
        String header = deref(String, arr_get(rw->headers, i));
        string_dprint(fd, header);
        dprintf(fd, "\r\n");
    }
    dprintf(fd, "\r\n");
    if (string_dprint(fd, rw->body)) dprintf(fd, "\r\n");
}

void req_init(Request req, RequestMethod method, Slice url) {
    nul_check(Request, req), __req_init(req, method, url);
}

Request req_new(RequestMethod method, Slice url) {
    Request req = malloc(sizeof *req);
    alloc_check(malloc, req, sizeof *req);
    return __req_init(req, method, url), req;
}

void req_cleanup(Request req) {
    if (!req) return;
    slice_cleanup(req->url);
}

void req_drop(Request *req) {
    if (req && *req) req_cleanup(*req), free(*req), *req = 0;
}

void handle_pack_init(HandlePack pack, Slice method, Slice url, handle_func_t func) {
    nul_check(HandlerPack, pack), __handle_pack_init(pack, method, url, func);
}

HandlePack handle_pack_new(Slice method, Slice url, handle_func_t func) {
    if (!url || !func) return 0;
    HandlePack pack = malloc(sizeof *pack);
    alloc_check(malloc, pack, sizeof *pack);
    return __handle_pack_init(pack, method, url, func), pack;
}

void handle_pack_cleanup(HandlePack pack) {
    if (!pack) return;
    regfree(&pack->url_reg);
}

void handle_pack_drop(HandlePack *pack) {
    if (pack && *pack) handle_pack_cleanup(*pack), free(*pack), *pack = 0;
}

static inline void __handler_init(Handler handler) {
    memset(handler, 0, sizeof *handler);
    handler->packs = arr_new(sizeof (HandlePack));
}

static inline void __server_init(Server server, Handler handler, const char *port, size_t worker_threads) {
    memset(server, 0, sizeof *server);
    server->done = 0, server->listen_s = -1;
    server->handler = handler;
    server->incoming = arr_new(sizeof (int));
    server->config.port = port, server->config.worker_threads = worker_threads;
    pthread_mutex_init(&server->mutex, 0);
    pthread_cond_init(&server->cond, 0);
}

static inline void __rw_init(ResponseWriter rw, ResponseStatus status) {
    memset(rw, 0, sizeof *rw);
    rw->headers = arr_new(sizeof (String)), rw->status = status;
    rw->body = string_new();
}

static inline void __req_init(Request req, RequestMethod method, Slice url) {
    req->method = method, req->url = slice_clone(url);
}

static inline void __handle_pack_init(HandlePack pack, Slice method, Slice url, handle_func_t func) {
    char *url_str = alloca(url->len + 1);
    memcpy(url_str, url->mem, url->len), url_str[url->len] = 0;
    assert(!reg_from_slice(&pack->url_reg, url), "Failed to compile regex for pattern: '%s'", url_str);
    pack->method = method? req_method_from_slice(method): REQUEST_INVAL, pack->func = func;
}
