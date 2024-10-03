#include <garage/ascii.h>
#include <garage/input_tok.h>
#include <garage/slice.h>
#include <garage/http.h>

#define SLICE_CONST(LIT) { .mem = LIT, .align = 1, .len = sizeof LIT - 1 }

const char *const input_tok_type_str[] = {
    [INPUT_TOK_NONE] = "Input Token None",
    [INPUT_TOK_EOF] = "Input Token End of file",
    [INPUT_TOK_TEXT] = "Input Token Texts",
    [INPUT_TOK_EQ_SPLIT] = "Input Token Equals Splitter",
    [INPUT_TOK_MINUS_SPLIT] = "Input Token Minus Splitter",
};

const char *const ch_type_str[] = {
    [CH_NONE] = "^@",
    [CH_CTL] = "Control character set",
    [CH_WS] = "Whitespace character set",
    [CH_PUN] = "Punctuators",
    [CH_NUM] = "Digits",
    [CH_ALPHA] = "Alphabetics",
};

// from `man ascii`
const char *const ch_str[] = {
    ['\000'] = "NUL",
    ['\001'] = "SOH",
    ['\002'] = "STX",
    ['\003'] = "ETX",
    ['\004'] = "EOT",
    ['\005'] = "ENQ",
    ['\006'] = "ACK",
    ['\007'] = "BEL",
    ['\010'] = "BS",
    ['\011'] = "HT",
    ['\012'] = "LF",
    ['\013'] = "VT",
    ['\014'] = "FF",
    ['\015'] = "CR",
    ['\016'] = "SO",
    ['\017'] = "SI",
    ['\020'] = "DLE",
    ['\021'] = "DC1",
    ['\022'] = "DC2",
    ['\023'] = "DC3",
    ['\024'] = "DC4",
    ['\025'] = "NAK",
    ['\026'] = "SYN",
    ['\027'] = "ETB",
    ['\030'] = "CAN",
    ['\031'] = "EM",
    ['\032'] = "SUB",
    ['\033'] = "ESC",
    ['\034'] = "FS",
    ['\035'] = "GS",
    ['\036'] = "RS",
    ['\037'] = "US",
    ['\040'] = "SPA",
    ['\041'] = "!",
    ['\042'] = "\"",
    ['\043'] = "#",
    ['\044'] = "$",
    ['\045'] = "%",
    ['\046'] = "&",
    ['\047'] = "'",
    ['\050'] = "(",
    ['\051'] = ")",
    ['\052'] = "*",
    ['\053'] = "+",
    ['\054'] = ",",
    ['\055'] = "-",
    ['\056'] = ".",
    ['\057'] = "/",
    ['\060'] = "0",
    ['\061'] = "1",
    ['\062'] = "2",
    ['\063'] = "3",
    ['\064'] = "4",
    ['\065'] = "5",
    ['\066'] = "6",
    ['\067'] = "7",
    ['\070'] = "8",
    ['\071'] = "9",
    ['\072'] = ":",
    ['\073'] = ";",
    ['\074'] = "<",
    ['\075'] = "=",
    ['\076'] = ">",
    ['\077'] = "?",
    ['\100'] = "@",
    ['\101'] = "A",
    ['\102'] = "B",
    ['\103'] = "C",
    ['\104'] = "D",
    ['\105'] = "E",
    ['\106'] = "F",
    ['\107'] = "G",
    ['\110'] = "H",
    ['\111'] = "I",
    ['\112'] = "J",
    ['\113'] = "K",
    ['\114'] = "L",
    ['\115'] = "M",
    ['\116'] = "N",
    ['\117'] = "O",
    ['\120'] = "P",
    ['\121'] = "Q",
    ['\122'] = "R",
    ['\123'] = "S",
    ['\124'] = "T",
    ['\125'] = "U",
    ['\126'] = "V",
    ['\127'] = "W",
    ['\130'] = "X",
    ['\131'] = "Y",
    ['\132'] = "Z",
    ['\133'] = "[",
    ['\134'] = "\\",
    ['\135'] = "]",
    ['\136'] = "^",
    ['\137'] = "_",
    ['\140'] = "`",
    ['\141'] = "a",
    ['\142'] = "b",
    ['\143'] = "c",
    ['\144'] = "d",
    ['\145'] = "e",
    ['\146'] = "f",
    ['\147'] = "g",
    ['\150'] = "h",
    ['\151'] = "i",
    ['\152'] = "j",
    ['\153'] = "k",
    ['\154'] = "l",
    ['\155'] = "m",
    ['\156'] = "n",
    ['\157'] = "o",
    ['\160'] = "p",
    ['\161'] = "q",
    ['\162'] = "r",
    ['\163'] = "s",
    ['\164'] = "t",
    ['\165'] = "u",
    ['\166'] = "v",
    ['\167'] = "w",
    ['\170'] = "x",
    ['\171'] = "y",
    ['\172'] = "z",
    ['\173'] = "{",
    ['\174'] = "|",
    ['\175'] = "}",
    ['\176'] = "~",
    ['\177'] = "DEL",
};

const struct slice res_str[] = {
    [RESPONSE_OK] = SLICE_CONST("OK"),
    [RESPONSE_NOT_FOUND] = SLICE_CONST("NOT FOUND"),
    [RESPONSE_BAD_REQUEST] = SLICE_CONST("BAD REQUEST"),
};

const struct slice req_str[] = {
    [REQUEST_GET] = SLICE_CONST("GET"),
    [REQUEST_POST] = SLICE_CONST("POST"),
    [REQUEST_PUT] = SLICE_CONST("PUT"),
    [REQUEST_DELETE] = SLICE_CONST("DELETE"),
    [REQUEST_HEAD] = SLICE_CONST("HEAD"),
    [REQUEST_OPTIONS] = SLICE_CONST("OPTIONS"),
    [REQUEST_CONNECT] = SLICE_CONST("CONNECT"),
    [REQUEST_TRACE] = SLICE_CONST("TRACE"),
    [REQUEST_PATCH] = SLICE_CONST("PATCH"),
};
