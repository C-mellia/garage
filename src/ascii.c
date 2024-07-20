#include <garage/ascii.h>

int ch_ws(char ch) {
    return ch == '\011' || ch == '\012' || ch == '\013' || ch == '\015' || ch == '\040';
}

int ch_open_paren(char ch) {
    return ch == '(';
}

int ch_closed_paren(char ch) {
    return ch == ')';
}

int ch_open_brack(char ch) {
    return ch == '[';
}

int ch_closed_brack(char ch) {
    return ch == ']';
}

int ch_open_brace(char ch) {
    return ch == '{';
}

int ch_closed_brace(char ch) {
    return ch == '}';
}

int ch_alph_lower(char ch) {
    return ch >= 'a' && ch <= 'z';
}

int ch_alph_upper(char ch) {
    return ch >= 'A' && ch <= 'Z';
}

int ch_num(char ch) {
    return ch >= '0' && ch <= '9';
}

int ch_ctl(char ch) {
    return ch < '\040' || ch == '\177';
}

char ch_toctl(char ch) {
    return ch & 0x1f;
}

int ch_pun(char ch) {
    return ('\040' < ch && ch < '\60')
    || ('\072' <= ch && ch <= '\100')
    || ('\132' < ch && ch <= '\140')
    || ('\172' < ch && ch <= '\176');
}
