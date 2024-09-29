#ifndef GARAGE_ASCII_H
#   define GARAGE_ASCII_H 1

typedef enum CharType {
    CH_NONE=0,
    CH_CTL,
    CH_WS,
    CH_PUN,
    CH_NUM,
    CH_ALPHA,

    __CH_COUNT,
} CharType;

CharType ch_type(char *ch);

int ch_is_ws(char *ch);
int ch_is_open_paren(char *ch);
int ch_is_closed_paren(char *ch);
int ch_is_open_brack(char *ch);
int ch_is_closed_brack(char *ch);
int ch_is_open_brace(char *ch);
int ch_is_closed_brace(char *ch);
int ch_is_alpha_lower(char *ch);
int ch_is_alpha_upper(char *ch);
int ch_is_alpha(char *ch);
int ch_is_num(char *ch);
int ch_is_ctl(char *ch);
int ch_is_pun(char *ch);

int ch_is(char *ch, char *oth, int len);

int ch_deb_dprint(int fd, char *ch);
int ch_deb_print(char *ch);

char ch_toctl(char ch);

#endif // GARAGE_ASCII_H
