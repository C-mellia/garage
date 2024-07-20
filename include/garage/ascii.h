#ifndef GARAGE_STRING_H
#define GARAGE_STRING_H 1

int ch_ws(char ch);
int ch_open_paren(char ch);
int ch_closed_paren(char ch);
int ch_open_brack(char ch);
int ch_closed_brack(char ch);
int ch_open_brace(char ch);
int ch_closed_brace(char ch);
int ch_alph_lower(char ch);
int ch_alph_upper(char ch);
int ch_num(char ch);
int ch_ctl(char ch);
int ch_pun(char ch);

char ch_toctl(char ch);

#endif // GARAGE_STRING_H
