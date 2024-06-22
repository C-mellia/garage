#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <regex.h>
#include <stdarg.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/time.h>

#include <garage/garage.h>
#include <garage/http.h>

typedef struct {
    const char *method;
    regex_t url_reg;
    handle_func_t func;
} HandlePack;

extern StackAllocator sa;

static const char *status_code[] = {
    [200] = "OK",
    [404] = "NOT FOUND",
};

static void *worker(void *args);
static int server_listen(const char *port);

// HTTP Request:
// METHOD URL HTTP_VERSION
// HEADER_NAME: VALUE
// BODY
// HTTP Response:
// HTTP_VERSION RESPONSE_CODE CODE_NAME
// HEADER_NAME: VALUE
// BODY
static inline int parse_request(char *msg, Request *req) {
    char *ws;
    req->method = msg;
    if (ws = strchr(msg, ' '), !ws) return 1;
    *ws++ = 0;
    req->url = ws;
    if (ws = strchr(ws, ' '), !ws) return 1;
    *ws++ = 0;
    return 0;
}

static inline int read_request(int client_s, Request *req) {
    char buf[0x1000] = {0};
    size_t msg_len;
    msg_len = read(client_s, buf, sizeof buf), buf[msg_len] = 0;
    return parse_request(buf, req);
}

// route_pattern: GET /; GET /home; POST /api/*; GET /api/*
// req_route: GET /; GET /home; POST /api/login; GET /api/token
static int match_url(regex_t *url_pattern, const char *req_url) {
    regmatch_t pmatch[1];

    if (regexec(url_pattern, req_url, sizeof pmatch / sizeof pmatch[0], pmatch, 0)) return 0;
    return (size_t) (pmatch[0].rm_eo - pmatch[0].rm_so) == strlen(req_url);
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

Handler handler_new() {
    Handler h = malloc(sizeof *h);
    memset(h, 0, sizeof *h);
    h->handle_funcs = arr_new(sizeof (HandlePack));
    return h;
}

void handler_handle_func(Handler h, const char *route, handle_func_t func) {
    char *method = 0, *url = 0;
    char route_buf[strlen(route) + 1];
    HandlePack pack = {0};

    code_trap(h, "handler_handle_func: null\n");
    memcpy(route_buf, route, sizeof route_buf);
    do {
        method = route_buf;
        if (url = strchr(method, ' '), !url) break;
        *url++ = 0;
    } while(0);
    if (!url) url = method, method = 0;
    pack.method = strdup(method), pack.func = func;
    if (regcomp(&pack.url_reg, url, 0)) return;
    arr_push_back(h->handle_funcs, &pack);
}

Server server_new(Handler handler, const char *port, size_t worker_threads) {
    Server sv = sa_alloc(sa, sizeof *sv);
    code_trap(sv && handler, "server_new: null\n");
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

static void *worker(void *args) {
    Server sv;
    Array incoming, handle_funcs;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    pthread_t id;
    int client_s, *p;
    const char *method;
    regex_t *regex;
    handle_func_t handle_func, default_handle;
    HandlePack *pack;
    Request req;
    ResponseWriter rw = {0};

    sv = args;
    id = pthread_self();
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
        if (!p) continue; // no client socket, though normally not possible at this point
        if (read_request(client_s, &req)) goto close_connection;
        // printf("%s, %s\n", req.method, req.url);

        rw_init(&rw);
        for (size_t i = 0; i < handle_funcs->len; ++i) {
            if (pack = arr_get(handle_funcs, i), !pack) continue;
            method = pack->method;
            regex = &pack->url_reg;
            handle_func = pack->func;
            if (match_url(regex, req.url) && (!method || strcmp(method, req.method) == 0)) {
                handle_func(&rw, &req);
                goto write_all;
            }
        }

        // Unreachable if handled correctly
        default_handle(&rw, &req);

    write_all:
        rw_write_all(&rw, client_s);
        rw_cleanup(&rw);
    close_connection:
        close(client_s), client_s = -1;
    }

    pthread_mutex_lock(mutex);
    sv->done = sv->done > 0? sv->done - 1: 0;
    printf("worker thread %lu exited\n", id);
    pthread_mutex_unlock(mutex);
    return 0;
}

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
    pthread_mutex_lock(mutex);
    sv->exit_sig = 1;
    pthread_cond_broadcast(cond);
    printf("waiting for worker threads to exit\n");
    pthread_mutex_unlock(mutex);
    while(sv->done) usleep(1000);
    close(sv->listen_s), sv->listen_s = -1;
    arr_cleanup(sv->incoming);
    handler_cleanup(sv->handler);
    printf("Server cleanup\n");
}

void handler_cleanup(Handler h) {
    if (!h) return;
    for (size_t i = 0; i < h->handle_funcs->len; ++i) {
        HandlePack *pack = arr_get(h->handle_funcs, i);
        free((void *)pack->method);
        regfree(&pack->url_reg);
    }
    arr_cleanup(h->handle_funcs);
    free(h);
}

void handler_default_handle(Handler h, handle_func_t func) {
    code_trap(h, "handler_default_handle: null\n");
    h->default_handle = func;
}

void rw_init(ResponseWriter *rw) {
    code_trap(rw, "rw_init: null\n");
    memset(rw, 0, sizeof *rw);
    rw->headers = arr_new(sizeof (const char *));
    rw->status = 200;
}

void rw_status(ResponseWriter *rw, int status) {
    code_trap(rw, "rw_status: null\n");
    rw->status = status;
}

void rw_cleanup(ResponseWriter *rw) {
    if (!rw) return;
    for (size_t i = 0; i < rw->headers->len; ++i) {
        free((void *) *(const char **)arr_get(rw->headers, i));
    }
    arr_cleanup(rw->headers);
    if (rw->body) free((void *)rw->body);
}

void rw_write_header(ResponseWriter *rw, const char *fmt, ...) {
    char buf[0x1000];
    char *header;
    size_t len;
    va_list args;

    code_trap(rw, "rw_write_header: null\n");
    va_start(args, fmt);
    len = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    header = malloc(len + 1);
    memcpy(header, buf, len + 1);
    arr_push_back(rw->headers, &header);
}

void rw_set_body(ResponseWriter *rw, const char *body) {
    code_trap(rw, "rw_set_body: null\n");
    rw->body = strdup(body);
}

void rw_write_all(ResponseWriter *rw, int fd) {
    code_trap(rw, "rw_write_all: null\n");
    dprintf(fd, "HTTP/1.0 %d %s\r\n", rw->status, status_code[rw->status]);
    for (size_t i = 0; i < rw->headers->len; ++i) {
        const char *header = *(const char **)arr_get(rw->headers, i);
        if (i + 1 == rw->headers->len) {
            dprintf(fd, "%s\r\n\r\n", header);
        } else {
            dprintf(fd, "%s\r\n", header);
        }
    }
    if (rw->body) dprintf(fd, "%s\n", rw->body);
}
