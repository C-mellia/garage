#include <garage/ascii.h>

int ch_is_ws(char *ch) {
    if (!ch) return 0;
    return *ch == '\011'
    || *ch == '\012'
    || *ch == '\013'
    || *ch == '\015'
    || *ch == '\040';
}

int ch_is_open_paren(char *ch) {
    if (!ch) return 0;
    return *ch == '(';
}

int ch_is_closed_paren(char *ch) {
    if (!ch) return 0;
    return *ch == ')';
}

int ch_is_open_brack(char *ch) {
    if (!ch) return 0;
    return *ch == '[';
}

int ch_is_closed_brack(char *ch) {
    if (!ch) return 0;
    return *ch == ']';
}

int ch_is_open_brace(char *ch) {
    if (!ch) return 0;
    return *ch == '{';
}

int ch_is_closed_brace(char *ch) {
    if (!ch) return 0;
    return *ch == '}';
}

int ch_is_alpha_lower(char *ch) {
    if (!ch) return 0;
    return *ch >= 'a' && *ch <= 'z';
}

int ch_is_alpha_upper(char *ch) {
    if (!ch) return 0;
    return *ch >= 'A' && *ch <= 'Z';
}

int ch_is_alpha(char *ch) {
    if (!ch) return 0;
    return ch_is_alpha_lower(ch)
    || ch_is_alpha_upper(ch);
}

int ch_is_num(char *ch) {
    if (!ch) return 0;
    return *ch >= '0' && *ch <= '9';
}

int ch_is_ctl(char *ch) {
    if (!ch) return 0;
    return *ch < '\040' || *ch == '\177';
}

int ch_is_pun(char *ch) {
    if (!ch) return 0;
    return ('\040' < *ch && *ch < '\60')
    || ('\072' <= *ch && *ch <= '\100')
    || ('\132' < *ch && *ch <= '\140')
    || ('\172' < *ch && *ch <= '\176');
}

int ch_is(char *ch, char *oth, int len) {
    if (!ch || !oth) return !ch && !oth;
    while (len--) if (*ch == *oth++) return 1;
    return 0;
}

char ch_toctl(char ch) {
    if (!ch) return 0;
    return ch & 0x1f;
}
