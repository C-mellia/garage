#include <stdio.h>

#include <garage/ascii.h>
#include <garage/garage.h>

extern const char *const ch_type_str[__CH_COUNT];

CharType ch_type(char *ch) {
    if (ch_is_ws(ch)) {
        return CH_WS;
    } else if (ch_is_pun(ch)) {
        return CH_PUN;
    } else if (ch_is_alpha(ch)) {
        return CH_ALPHA;
    } else if (ch_is_num(ch)) {
        return CH_NUM;
    } else if (ch_is_ctl(ch)) {
        return CH_CTL;
    } else {
        return CH_NONE;
    }
}

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
    // parenthesis are included
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

int ch_deb_dprint(int fd, char *ch) {
    #define get_type_str (ch_type_str[type]? : "Inval CharType")
    if (!ch) return dprintf(fd, "(nil)");
    CharType type = ch_type(ch);
    switch(type) {
        case CH_NONE ... CH_WS: {
            return dprintf(fd, "{type: '%s'}",
                           get_type_str);
        } break;
        case CH_PUN ... CH_ALPHA: {
            return dprintf(fd, "{type: '%s', sym: '%c'}",
                           get_type_str, deref(char, ch));
        } break;
        default: return dprintf(fd, "(inval CharType)");
    }
    #undef get_type_str
}

int ch_deb_print(char *ch) {
    return ch_deb_dprint(1, ch);
}

char ch_toctl(char ch) {
    if (!ch) return 0;
    return ch & 0x1f;
}
