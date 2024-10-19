#include ".reg.c"

extern const struct slice res_str[__RESPONSE_COUNT];
extern const struct slice req_str[__REQUEST_COUNT];

static inline RequestMethod req_method_from_slice(Slice slice) {
    for (size_t i = REQUEST_GET; i < __REQUEST_COUNT; ++i) {
        if (req_str[i].len != slice->len) continue;
        void *mem_pos = slice_search_mem(slice, req_str[i].mem, req_str[i].len);
        if (mem_pos == slice->mem) return i;
    }
    return REQUEST_INVAL;
}

// HTTP Request:
// METHOD URL HTTP_VERSION
// HEADER_NAME: VALUE
// BODY
// HTTP Response:
// HTTP_VERSION RESPONSE_CODE CODE_NAME
// HEADER_NAME: VALUE
// BODY
static Request parse_request(Slice req_slice) {
    Slice Cleanup(slice_drop) method = slice_split_at(req_slice, slice_search_item(req_slice, "/"));
    if (!method) return 0;
    slice_trim(method, " \t\r\n", 4), slice_trim(req_slice, " \t\r\n", 4);
    RequestMethod req_method = req_method_from_slice(method);
    if (req_method == REQUEST_INVAL) return 0;
    Slice Cleanup(slice_drop) url = slice_split_once(req_slice, " ", 1);
    slice_trim(url, " ", 1);
    return req_new(req_method, url);
}

// route_pattern: GET /; GET /home; POST /api/*; GET /api/*
// req_route: GET /; GET /home; POST /api/login; GET /api/token
static inline int match_url(regex_t *url_pattern, Slice url) {
    regmatch_t match = reg_match(url_pattern, url->mem, 0, url->len);
    return match.rm_so == 0 && match.rm_eo == (regoff_t)url->len;
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
    Server server = args;
    Array incoming = server->incoming, packs = server->handler->packs;
    pthread_mutex_t *mutex = &server->mutex;
    pthread_cond_t *cond = &server->cond;
    // pthread_t id = pthread_self();
    int *incoming_fd;
    handle_func_t default_handle = server->handler->default_handle;

    for (; server->listen_s != -1;) {
        int Cleanup(fd_drop) client_s = -1;
        pthread_mutex_lock(mutex);
        pthread_cond_wait(cond, mutex);
        // using invalid fd as exit signal instead
        // if (server->exit_sig) {
        //     pthread_mutex_unlock(mutex);
        //     break;
        // }
        // printf("%p\n", incoming_fd);
        if (incoming_fd = arr_pop_front(incoming), incoming_fd) client_s = *incoming_fd;
        pthread_mutex_unlock(mutex);
        if (__builtin_expect(!incoming_fd, 0)) continue; // no client socket available, normally not common at this point though
        String Cleanup(string_drop) msg = string_new();

        // TODO: reading incoming messaged with timeout or only read when necessary
        string_from_file(client_s, msg);

        Slice Cleanup(slice_drop) req_slice = slice_from_arr((void *)msg->arr);
        Request Cleanup(req_drop) req = parse_request(req_slice);
        if (!req) continue;

        ResponseWriter Cleanup(rw_drop) rw = rw_new(RESPONSE_OK);
        for (size_t i = 0; i < arr_len(packs); ++i) {
            HandlePack pack = deref(HandlePack, arr_get(packs, i));
            RequestMethod method = pack->method;
            register regex_t *regex = &pack->url_reg;
            handle_func_t handle_func = pack->func;
            if (!handle_func) continue;
            if (match_url(regex, req->url) && (method == REQUEST_INVAL || req->method == method)) {
                handle_func(rw, req);
                goto write_all;
            }
        }

        if (default_handle) default_handle(rw, req);
        else continue;

    write_all:
        rw_deb_print(rw), printf("\n");
        rw_write_all(rw, client_s);
    }

    pthread_mutex_lock(mutex);
    server->done = server->done > 0? server->done - 1: 0;
    // printf("worker thread %lu exited\n", id);
    pthread_mutex_unlock(mutex);
    return 0;
}

static inline void server_threads_exit_wait(Server server) {
    pthread_mutex_t *mutex = &server->mutex;
    pthread_cond_t *cond = &server->cond;
    pthread_mutex_lock(mutex);
    server->exit_sig = 1;
    pthread_cond_broadcast(cond);
    pthread_mutex_unlock(mutex);
    while(server->done) usleep(1000);
}

#undef SLICE_CONST
