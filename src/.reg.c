#ifndef _GARAGE_REG_H
#define _GARAGE_REG_H 1

static inline regmatch_t reg_match(regex_t *reg, const char *str, regoff_t start, regoff_t end) {
    regmatch_t pmatch[1] = { [0] = (regmatch_t) {.rm_so = start, .rm_eo = end}, };
    int res = regexec(reg, str, sizeof pmatch / sizeof pmatch[0], pmatch, REG_STARTEND);
    if (res == REG_NOMATCH) return (regmatch_t) {.rm_so = -1, .rm_eo = -1};
    return pmatch[0];
}

static inline int reg_from_slice(regex_t *reg, Slice slice) {
    char *str = alloca(slice->len + 1);
    memcpy(str, slice->mem, slice->len), str[slice->len] = 0;
    return regcomp(reg, str, REG_EXTENDED);
}

#endif // _GARAGE_REG_H
