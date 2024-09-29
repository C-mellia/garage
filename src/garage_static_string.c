#include <garage/ascii.h>

const char *const ch_type_str[] = {
    [CH_NONE] = "^@",
    [CH_CTL] = "Control character set",
    [CH_WS] = "Whitespace character set",
    [CH_PUN] = "Punctuators",
    [CH_NUM] = "Digits",
    [CH_ALPHA] = "Alphabetics",
};
