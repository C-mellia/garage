#define SLICE_CONST(__str, __len) { .mem = __str, .align = 1, .len = __len }

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
static Request parse_request(String req_str) {
    // Array req_str_arr = (void *)req_str->arr;
    Slice Cleanup(slice_drop) slice = slice_from_arr((void *)req_str->arr);
    Slice Cleanup(slice_drop) method = slice_split_at(slice, slice_search_item(slice, "/"));
    if (!method) return 0;
    slice_trim(method, " \t\r\n", 4), slice_trim(slice, " \t\r\n", 4);
    RequestMethod req_method = req_method_from_slice(method);
    if (req_method == REQUEST_INVAL) return 0;
    Slice url = slice_split_once(slice, " ", 1);
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
