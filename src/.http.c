#define SLICE_CONST(__str, __len) { .mem = __str, .align = 1, .len = __len }

static struct Slice res_str[] = {
    [RESPONSE_OK] = SLICE_CONST("OK", 2),
    [RESPONSE_NOT_FOUND] = SLICE_CONST("NOT FOUND", 9),
    [RESPONSE_BAD_REQUEST] = SLICE_CONST("BAD REQUEST", 11),
};

static struct Slice req_str[] = {
    [REQUEST_GET] = SLICE_CONST("GET", 3),
    [REQUEST_POST] = SLICE_CONST("POST", 4),
    [REQUEST_PUT] = SLICE_CONST("PUT", 3),
    [REQUEST_DELETE] = SLICE_CONST("DELETE", 6),
    [REQUEST_HEAD] = SLICE_CONST("HEAD", 4),
    [REQUEST_OPTIONS] = SLICE_CONST("OPTIONS", 7),
    [REQUEST_CONNECT] = SLICE_CONST("CONNECT", 7),
    [REQUEST_TRACE] = SLICE_CONST("TRACE", 5),
    [REQUEST_PATCH] = SLICE_CONST("PATCH", 5),
};

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
static Request parse_request(String req_str) {
    Slice Cleanup(slice_drop) slice = slice_from_arr(req_str);
    Slice Cleanup(slice_drop) method = slice_split_at(slice, slice_search_item(slice, "/"));
    if (!method) return 0;
    slice_trim(method, " \t\r\n", 4), slice_trim(slice, " \t\r\n", 4);
    RequestMethod req_method = req_method_from_slice(method);
    if (req_method == REQUEST_INVAL) return 0;
    Slice url = slice_split_once(slice, " ");
    slice_trim(url, " ", 1);
    return req_new(req_method, url);
}

// route_pattern: GET /; GET /home; POST /api/*; GET /api/*
// req_route: GET /; GET /home; POST /api/login; GET /api/token
static inline int match_url(regex_t *url_pattern, const char *req_url) {
    regmatch_t pmatch[1];

    if (regexec(url_pattern, req_url, sizeof pmatch / sizeof pmatch[0], pmatch, 0)) return 0;
    return (size_t) (pmatch[0].rm_eo - pmatch[0].rm_so) == strlen(req_url);
}

#undef SLICE_CONST
