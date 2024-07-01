#include <garage/ascii.h>

static inline int str_scan(const char *str, char ch) {
    for (; *str; ++str) {
        if (ch == *str) return !0;
    }
    return 0;
}

int ch_ws(char ch) {
    static const char *whitespace = "\011\012\013\015\040";
    return str_scan(whitespace, ch);
}

int ch_open_brk(char ch) {
    static const char *open_brace = "\050\074\133\173";
    return str_scan(open_brace, ch);
}

int ch_closed_brk(char ch) {
    static const char *closed_brace = "\051\076\135\175";
    return str_scan(closed_brace, ch);
}

int ch_alph_lower(char ch) {
    return ch > '\140' && ch < '\173';
}

int ch_alph_upper(char ch) {
    return ch > '\100' && ch < '\133';
}

int ch_num(char ch) {
    return ch >= '\060' && ch < '\072';
}

int ch_ctl(char ch) {
    return ch < '\040';
}

char ch_toctl(char ch) {
    return ch & 0xf1;
}

AsciiCat ch_cat(char ch) {
    if (ch_ws(ch)) {
        return Whitespace;
    } else if (ch_open_brk(ch)) {
        return OpenBrace;
    } else if (ch_closed_brk(ch)) {
        return ClosedBrace;
    } else if (ch_alph_lower(ch)) {
        return LowercaseAlphabetic;
    } else if (ch_alph_upper(ch)) {
        return UppercaseAlphabetic;
    } else if (ch_num(ch)) {
        return Numeric;
    } else if (ch_ctl(ch)) {
        return ControlCode;
    } else {
        return DelimitedCharacter;
    }
}

const char *ascii_cat_format(AsciiCat cat) {
    switch (cat) {
        case Whitespace: return "Whitespace";
        case OpenBrace: return "OpenBrace";
        case ClosedBrace: return "ClosedBrace";
        case LowercaseAlphabetic: return "LowercaseAlphabetic";
        case UppercaseAlphabetic: return "UppercaseAlphabetic";
        case Numeric: return "Numeric";
        case ControlCode: return "ControlCode";
        case DelimitedCharacter: return "DelimitedCharacter";
        default: return "Uncategorized";
    }
}
