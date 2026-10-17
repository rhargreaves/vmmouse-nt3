#include "path.h"
#include "win32.h"

int path_get_system_driver(const char *filename, char *out, unsigned long max_len) {
    char windir[MAX_PATH];
    const char *prefix = "\\system32\\drivers\\";
    unsigned long wlen = GetWindowsDirectoryA(windir, sizeof(windir));
    unsigned long i = 0, j = 0;

    if (wlen == 0 || wlen > 200) {
        return 0;
    }

    for (i = 0; i < wlen && i < max_len - 1; i++) {
        out[i] = windir[i];
    }
    while (prefix[j] && (i + j < max_len - 1)) {
        out[i + j] = prefix[j];
        j++;
    }
    i += j;
    j = 0;
    while (filename[j] && (i + j < max_len - 1)) {
        out[i + j] = filename[j];
        j++;
    }
    out[i + j] = '\0';
    return 1;
}

int path_get_module_sibling(const char *filename, char *out, unsigned long max_len) {
    char mod[MAX_PATH];
    unsigned long len = GetModuleFileNameA(0, mod, sizeof(mod));
    unsigned long last_slash = 0;
    unsigned long i = 0, j = 0;

    if (len == 0 || len >= sizeof(mod)) {
        while (filename[j] && j < max_len - 1) {
            out[j] = filename[j];
            j++;
        }
        out[j] = '\0';
        return 0;
    }

    while (i < len) {
        if (mod[i] == '\\' || mod[i] == '/') {
            last_slash = i;
        }
        i++;
    }

    for (i = 0; i <= last_slash && i < max_len - 1; i++) {
        out[i] = mod[i];
    }
    while (filename[j] && (last_slash + 1 + j < max_len - 1)) {
        out[last_slash + 1 + j] = filename[j];
        j++;
    }
    out[last_slash + 1 + j] = '\0';
    return 1;
}
