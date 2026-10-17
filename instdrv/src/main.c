#include "console.h"
#include "errors.h"
#include "path.h"
#include "service.h"
#include "win32.h"

static int str_has_flag(const char *s, char flag) {
    while (*s) {
        if ((*s == '/' || *s == '-') && (s[1] == flag || s[1] == (char)(flag - 32))) {
            return 1;
        }
        s++;
    }
    return 0;
}

void __stdcall mainCRTStartup(void) {
    char src[MAX_PATH];
    char dst[MAX_PATH];
    char *cmdline = GetCommandLineA();
    int is_uninstall = (cmdline && str_has_flag(cmdline, 'u'));

    if (!path_get_system_driver(DRIVER_FILENAME, dst, sizeof(dst))) {
        console_print("Error: Failed to find Windows directory.\r\n");
        ExitProcess(EXIT_FAILURE);
    }

    if (is_uninstall) {
        int err = service_uninstall(dst);
        if (err != ERR_NONE) {
            ExitProcess((unsigned int)err);
        }
        ExitProcess(EXIT_SUCCESS);
    }

    if (!path_get_module_sibling(DRIVER_FILENAME, src, sizeof(src))) {
        console_print("Error: Failed to determine source driver path.\r\n");
        ExitProcess(EXIT_FAILURE);
    }

    {
        int err = service_install(src, dst);
        if (err != ERR_NONE) {
            ExitProcess((unsigned int)err);
        }
        ExitProcess(EXIT_SUCCESS);
    }
}
