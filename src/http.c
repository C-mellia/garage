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

#include "./.http.c"

static void *worker(void *args);
static int server_listen(const char *port);

Handler handler_new() {
    Handler h = malloc(sizeof *h);
    memset(h, 0, sizeof *h);
    h->handle_funcs = arr_new(sizeof (HandlePack));
    return h;
}

void handler_handle_func(Handler h, const char *route, handle_func_t func) {
    assert(h, "Handler is not initailized at this point\n");
    Slice Cleanup(slice_drop) route_slice = slice_new((void *)route, strlen(route), 1);

    Slice method = slice_split_at(route_slice, slice_search_item(route_slice, "/"));
    if (method) slice_trim(method, " \t\n", 3);
    slice_trim(route_slice, " \t\n", 3);
    Slice Cleanup(slice_drop) url = slice_split_once(route_slice, " ", 1);
    HandlePack pack = handle_pack_new(method, url, func);
    arr_push_back(h->handle_funcs, &pack);
}

Server server_new(Handler handler, const char *port, size_t worker_threads) {
    if (!handler) return 0;
    Server sv = malloc(sizeof *sv);
    assert(sv, "Failed to allocate memory of size: %zu\n", sizeof *sv);
    memset(sv, 0, sizeof *sv);
    sv->done = 0;
    sv->handler = handler;
    sv->incoming = arr_new(sizeof (int));
    sv->listen_s = -1;
    sv->config.port = port, sv->config.worker_threads = worker_threads;
    pthread_mutex_init(&sv->mutex, 0);
    pthread_cond_init(&sv->cond, 0);
    return sv;
}

static int server_listen(const char *port) {
    int s, res;
    struct addrinfo hint = {0};
    struct addrinfo *ai, *p;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    if (s = getaddrinfo(0, port, &hint, &ai), s != 0) return -1;
    for (p = ai; p; p = p->ai_next) {
        if (s = socket(p->ai_family, p->ai_socktype, p->ai_protocol), s < 0) continue;
        if (bind(s, p->ai_addr, p->ai_addrlen) == 0) break;
        close(s);
    }
    freeaddrinfo(ai);
    if (!p) return -1;
    return res = listen(s, 0x100), res == 0? s: (close(s), -1);
}

// Note: This function has a huge problem, that any interuption happens during
// the loop, will results in a deadlock, due to a design how server cleans up,
// that by decreasing the `done` integer field of the server struct, thereby
// make sure all the thread exits, however one of the exits will be ommited
// when an interuption happens, and the server will be stuck forever.
//
// But, more strangely, not knowing how, interupting the process with C-c still
// kills the server and cleanup all the threads as expected.
//
// So currently, the function still works just to make sure that no
// interuption occurs during the loop, for example null pointer dereference or
// floating point exceptions, and still kind of prune to errors, so this is
// just sort of a Todo list for me to fix this issue maybe in the future.
static void *worker(void *args) {
    Server sv = args;
    Array incoming, handle_funcs;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    pthread_t id = pthread_self();
    int client_s, *p;
    regex_t *regex;
    handle_func_t handle_func, default_handle;

    incoming = sv->incoming;
    handle_funcs = sv->handler->handle_funcs;
    default_handle = sv->handler->default_handle;
    mutex = &sv->mutex;
    cond = &sv->cond;

    for (; !sv->exit_sig;) {
        pthread_mutex_lock(mutex);
        pthread_cond_wait(cond, mutex);
        if (p = arr_pop_front(incoming), p) client_s = *p;
        pthread_mutex_unlock(mutex);
        if (__builtin_expect(!p, 0)) continue; // no client socket available, normally not common at this point though
        Request Cleanup(req_drop) req = req_from_client(client_s);
        if (!req) goto close_connection;

        ResponseWriter Cleanup(rw_drop) rw = rw_new(RESPONSE_OK);
        printf("\n");
        for (size_t i = 0; i < handle_funcs->len; ++i) {
            HandlePack pack = deref(HandlePack, arr_get(handle_funcs, i));
            RequestMethod method = pack->method;
            regex = &pack->url_reg;
            handle_func = pack->func;
            if (!handle_func) goto close_connection;
            char *url = alloca(req->url->len + 1);
            memcpy(url, req->url->mem, req->url->len), url[req->url->len] = 0;
            if (match_url(regex, url) && (method == REQUEST_INVAL || req->method == method)) {
                handle_func(rw, req);
                goto write_all;
            }
        }

        if (default_handle) default_handle(rw, req);

    write_all:
        rw_write_all(rw, client_s);
    close_connection:
        close(client_s), client_s = -1;
    }

    pthread_mutex_lock(mutex);
    sv->done = sv->done > 0? sv->done - 1: 0;
    printf("worker thread %lu exited\n", id);
    pthread_mutex_unlock(mutex);
    return 0;
}

// Note: I kind of have an idea, that by creating yet another thread for the
// loop, that constantly accepting incomings and queuing up all the
// connections, so that this function is non block at all, and by creating a
// single way pipe, then return the readable side of the pipe, so all the logs
// generated while server is running can be redirected to the main function,
// and the logs can be handled in a more flexible way.
//
// Some technologies, like redirecting between threads with pipes, I've never tested
int server_listen_and_serve(Server sv) {
    pthread_t thread;
    int client_s;
    Array incoming;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;

    if (!sv) return 1;
    incoming = sv->incoming;
    mutex = &sv->mutex;
    cond = &sv->cond;
    if (sv->listen_s = server_listen(sv->config.port), sv->listen_s < 0) return 2; // other process is probably listening at the same port
    sv->done += sv->config.worker_threads;
    for (size_t i = 0; i < sv->config.worker_threads; ++i) {
        pthread_create(&thread, 0, worker, sv);
        pthread_detach(thread);
    }
    for (;;) {
        if (client_s = accept(sv->listen_s, 0, 0), client_s >= 0) {
            pthread_mutex_lock(mutex);
            arr_push_back(incoming, &client_s);
            pthread_cond_signal(cond);
            pthread_mutex_unlock(mutex);
        }
    }
    return 0;
}

void server_cleanup(Server sv) {
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;

    if (!sv) return;
    mutex = &sv->mutex;
    cond = &sv->cond;
    pthread_mutex_unlock(mutex);
    pthread_mutex_lock(mutex);
    close(sv->listen_s), sv->listen_s = -1;
    sv->exit_sig = 1;
    pthread_cond_broadcast(cond);
    printf("waiting for worker threads to exit\n");
    pthread_mutex_unlock(mutex);
    while(sv->done) usleep(1000);
    arr_cleanup(sv->incoming);
    printf("Server cleaned up\n");
}

void handler_cleanup(Handler h) {
    if (!h) return;
    for (size_t i = 0; i < h->handle_funcs->len; ++i) {
        HandlePack *pack = arr_get(h->handle_funcs, i);
        handle_pack_cleanup(*pack);
    }
    arr_cleanup(h->handle_funcs);
    free(h);
}

void handler_drop(Handler *h) {
    if (h && *h) handler_cleanup(*h), *h = 0;
}

void handler_default_handle(Handler h, handle_func_t func) {
    assert(h, "handler_default_handle: null\n");
    h->default_handle = func;
}

ResponseWriter rw_new(ResponseStatus status) {
    ResponseWriter rw = malloc(sizeof *rw);
    assert(rw, "Failed to allocate memory of size: %zu\n", sizeof *rw);
    memset(rw, 0, sizeof *rw);
    rw->headers = arr_new(sizeof (String)), rw->status = status;
    rw->body = string_new();
    return rw;
}

void rw_cleanup(ResponseWriter rw) {
    if (!rw) return;
    for (size_t i = 0; i < rw->headers->len; ++i) {
        String header = deref(String, arr_get(rw->headers, i));
        string_cleanup(header);
    }
    arr_cleanup(rw->headers);
    if (rw->body) free(rw->body);
    free(rw);
}

void rw_drop(ResponseWriter *rw) {
    if (rw && *rw) rw_cleanup(*rw), *rw = 0;
}

void rw_write_header(ResponseWriter rw, const char *fmt, ...) {
    va_list args;

    assert(rw, "Response Writer is not initialized at this point\n");
    va_start(args, fmt);
    String header = string_new();
    string_vfmt(header, fmt, args);
    va_end(args);
    arr_push_back(rw->headers, &header);
}

void rw_set_body(ResponseWriter rw, const char *body) {
    assert(rw, "Response Writer is not initialized at this point\n");
    string_fmt(rw->body, "%s", body);
}

void rw_write_all(ResponseWriter rw, int fd) {
    assert(rw, "Response Writer is not initialized at this point\n");
    dprintf(fd, "HTTP/1.0 %d %.*s\r\n",
            rw->status, (int)res_str[rw->status].len, (char *)res_str[rw->status].mem);
    for (size_t i = 0; i < rw->headers->len; ++i) {
        String header = deref(String, arr_get(rw->headers, i));
        string_dprint(fd, header);
        dprintf(fd, "\r\n");
    }
    dprintf(fd, "\r\n");
    if (string_dprint(fd, rw->body)) dprintf(fd, "\r\n");
}

Request req_new(RequestMethod method, Slice url) {
    Request req = malloc(sizeof *req);
    assert(req, "req_new: Failed to allocate memory of size: %zu\n", sizeof *req);
    req->method = method, req->url = url;
    return req;
}

HandlePack handle_pack_new(Slice method, Slice url, handle_func_t func) {
    if (!url || !func) return 0;
    HandlePack pack = malloc(sizeof *pack);
    assert(pack, "Failed to allocate memory of size: %zu\n", sizeof *pack);
    char *url_str = alloca(url->len + 1);
    memcpy(url_str, url->mem, url->len), url_str[url->len] = 0;
    if (regcomp(&pack->url_reg, url_str, REG_EXTENDED | REG_NOSUB)) {
        free(pack);
        return 0;
    }
    RequestMethod req_method = method? req_method_from_slice(method): REQUEST_INVAL;
    pack->method = req_method, pack->func = func;
    return pack;
}

void handle_pack_cleanup(HandlePack pack) {
    if (!pack) return;
    regfree(&pack->url_reg);
    free(pack);
}

void req_cleanup(Request req) {
    if (!req) return;
    slice_cleanup(req->url);
    free(req);
}

Request req_from_client(int client_s) {
    String Cleanup(string_drop) req_str = string_new();
    string_from_file(client_s, req_str);
    return parse_request(req_str);
}

void req_drop(Request *req) {
    if (req && *req) req_cleanup(*req), *req = 0;
}
