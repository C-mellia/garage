#ifndef GARAGE_STRING_H
#define GARAGE_STRING_H 1

typedef enum {
    Whitespace,
    OpenBrace,
    ClosedBrace,
    LowercaseAlphabetic,
    UppercaseAlphabetic,
    Numeric,
    ControlCode,
    DelimitedCharacter,
} AsciiCat;

int ch_ws(char ch);
int ch_open_brk(char ch);
int ch_closed_brk(char ch);
int ch_alph_lower(char ch);
int ch_alph_upper(char ch);
int ch_num(char ch);
int ch_ctl(char ch);
char ch_toctl(char ch);
AsciiCat ch_cat(char ch);
const char *ascii_cat_format(AsciiCat cat);

#endif // GARAGE_STRING_H
