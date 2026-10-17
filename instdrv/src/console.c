#include "console.h"
#include "win32.h"

void console_print(const char *s) {
    void *out = GetStdHandle(STD_OUTPUT_HANDLE);
    unsigned long written;
    unsigned long len = 0;

    while (s[len]) {
        len++;
    }
    WriteFile(out, s, len, &written, 0);
}

static void console_print_num(unsigned long n) {
    char buf[16];
    int idx = 0;

    if (n == 0) {
        buf[idx++] = '0';
    } else {
        char tmp[16];
        int t = 0;
        while (n > 0) {
            tmp[t++] = (char)('0' + (n % 10));
            n /= 10;
        }
        while (t > 0) {
            buf[idx++] = tmp[--t];
        }
    }
    buf[idx] = 0;
    console_print(buf);
}

void console_error(const char *prefix, unsigned long err) {
    char msg[256];
    unsigned long len;

    console_print(prefix);
    console_print(" (error ");
    console_print_num(err);
    console_print("): ");

    len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, err, 0, msg,
                         sizeof(msg), 0);
    if (len > 0) {
        console_print(msg);
    } else {
        console_print("Unknown error.\r\n");
    }
}
