#define STD_OUTPUT_HANDLE ((unsigned long)-11)
#define SC_MANAGER_ALL_ACCESS 0xF003F
#define SERVICE_ALL_ACCESS 0xF01FF
#define SERVICE_KERNEL_DRIVER 0x00000001
#define SERVICE_SYSTEM_START 0x00000001
#define SERVICE_ERROR_NORMAL 0x00000001
#define SERVICE_NO_CHANGE 0xFFFFFFFF

extern void *__stdcall GetStdHandle(unsigned long nStdHandle);
extern int __stdcall WriteFile(void *hFile, const void *lpBuffer,
                              unsigned long nNumberOfBytesToWrite,
                              unsigned long *lpNumberOfBytesWritten,
                              void *lpOverlapped);
extern unsigned long __stdcall GetWindowsDirectoryA(char *lpBuffer, unsigned long uSize);
extern int __stdcall CopyFileA(const char *lpExistingFileName,
                              const char *lpNewFileName,
                              int bFailIfExists);
extern int __stdcall DeleteFileA(const char *lpFileName);
extern int __stdcall MoveFileExA(const char *lpExistingFileName,
                                 const char *lpNewFileName,
                                 unsigned long dwFlags);
extern int __stdcall SetFileAttributesA(const char *lpFileName, unsigned long dwFileAttributes);
extern char *__stdcall GetCommandLineA(void);
extern unsigned long __stdcall GetLastError(void);
extern unsigned long __stdcall GetModuleFileNameA(void *hModule, char *lpFilename,
                                                  unsigned long nSize);
extern unsigned long __stdcall FormatMessageA(unsigned long dwFlags,
                                              const void *lpSource,
                                              unsigned long dwMessageId,
                                              unsigned long dwLanguageId,
                                              char *lpBuffer,
                                              unsigned long nSize,
                                              void *Arguments);
extern void __stdcall ExitProcess(unsigned int uExitCode);

extern void *__stdcall OpenSCManagerA(const char *lpMachineName,
                                    const char *lpDatabaseName,
                                    unsigned long dwDesiredAccess);
extern void *__stdcall CreateServiceA(void *hSCManager,
                                     const char *lpServiceName,
                                     const char *lpDisplayName,
                                     unsigned long dwDesiredAccess,
                                     unsigned long dwServiceType,
                                     unsigned long dwStartType,
                                     unsigned long dwErrorControl,
                                     const char *lpBinaryPathName,
                                     const char *lpLoadOrderGroup,
                                     unsigned long *lpdwTagId,
                                     const char *lpDependencies,
                                     const char *lpServiceStartName,
                                     const char *lpPassword);
extern void *__stdcall OpenServiceA(void *hSCManager,
                                   const char *lpServiceName,
                                   unsigned long dwDesiredAccess);
extern int __stdcall ChangeServiceConfigA(void *hService,
                                         unsigned long dwServiceType,
                                         unsigned long dwStartType,
                                         unsigned long dwErrorControl,
                                         const char *lpBinaryPathName,
                                         const char *lpLoadOrderGroup,
                                         unsigned long *lpdwTagId,
                                         const char *lpDependencies,
                                         const char *lpServiceStartName,
                                         const char *lpPassword,
                                         const char *lpDisplayName);
extern int __stdcall DeleteService(void *hService);
extern int __stdcall CloseServiceHandle(void *hSCObject);

static void print_str(const char *s) {
    void *out = GetStdHandle(STD_OUTPUT_HANDLE);
    unsigned long written;
    unsigned long len = 0;
    while (s[len]) len++;
    WriteFile(out, s, len, &written, 0);
}

static void print_num(unsigned long n) {
    char buf[16];
    int idx = 0;
    if (n == 0) {
        buf[idx++] = '0';
    } else {
        char tmp[16];
        int t = 0;
        while (n > 0) {
            tmp[t++] = '0' + (n % 10);
            n /= 10;
        }
        while (t > 0) buf[idx++] = tmp[--t];
    }
    buf[idx] = 0;
    print_str(buf);
}

static void print_error(const char *prefix, unsigned long err) {
    char msg[256];
    unsigned long len;

    print_str(prefix);
    print_str(" (error ");
    print_num(err);
    print_str("): ");

    len = FormatMessageA(0x00001200, 0, err, 0, msg, sizeof(msg), 0);
    if (len > 0) {
        print_str(msg);
    } else {
        print_str("Unknown error.\r\n");
    }
}

static int str_contains_u(const char *s) {
    while (*s) {
        if ((*s == '/' || *s == '-') && (s[1] == 'u' || s[1] == 'U'))
            return 1;
        s++;
    }
    return 0;
}

static void build_driver_path(const char *windir, unsigned long wlen,
                              const char *filename, char *out) {
    unsigned long i = 0, j = 0;
    const char *prefix = "\\system32\\drivers\\";
    for (i = 0; i < wlen; i++) out[i] = windir[i];
    while (prefix[j]) { out[i + j] = prefix[j]; j++; }
    i += j;
    j = 0;
    while (filename[j]) { out[i + j] = filename[j]; j++; }
    out[i + j] = 0;
}

static void get_source_path(char *out, unsigned long max_len) {
    char mod[260];
    unsigned long len = GetModuleFileNameA(0, mod, sizeof(mod));
    unsigned long last_slash = 0;
    unsigned long i = 0;
    const char *fn = "vmmouse.sys";
    unsigned long j = 0;

    if (len == 0 || len >= sizeof(mod)) {
        while (fn[j] && j < max_len - 1) { out[j] = fn[j]; j++; }
        out[j] = 0;
        return;
    }
    while (i < len) {
        if (mod[i] == '\\' || mod[i] == '/') last_slash = i;
        i++;
    }
    for (i = 0; i <= last_slash && i < max_len - 1; i++) out[i] = mod[i];
    while (fn[j] && (last_slash + 1 + j < max_len - 1)) {
        out[last_slash + 1 + j] = fn[j];
        j++;
    }
    out[last_slash + 1 + j] = 0;
}

void __stdcall mainCRTStartup(void) {
    char src[260];
    char windir[260];
    char dst[260];
    char cleanup[260];
    unsigned long wlen;
    void *scm;
    void *svc;
    int is_uninstall;
    char *cmdline = GetCommandLineA();

    is_uninstall = (cmdline && str_contains_u(cmdline));

    wlen = GetWindowsDirectoryA(windir, sizeof(windir));
    if (wlen == 0 || wlen > 200) {
        print_str("Error: Failed to find Windows directory.\r\n");
        ExitProcess(1);
    }

    build_driver_path(windir, wlen, "vmmouse.sys", dst);

    scm = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        unsigned long err = GetLastError();
        print_error("Error: OpenSCManager failed", err);
        ExitProcess(2);
    }

    svc = OpenServiceA(scm, "vmmouse", SERVICE_ALL_ACCESS);

    if (is_uninstall) {
        print_str("Uninstalling VMMouse for NT 3.x...\r\n");
        if (svc) {
            if (DeleteService(svc)) {
                print_str("Service marked for deletion.\r\n");
            } else {
                unsigned long err = GetLastError();
                if (err == 1072) {
                    print_str("Service is already marked for deletion.\r\n");
                } else {
                    print_error("DeleteService failed", err);
                }
            }
            CloseServiceHandle(svc);
        } else {
            print_str("Service not found.\r\n");
        }
        SetFileAttributesA(dst, 0x80);
        if (!DeleteFileA(dst)) {
            MoveFileExA(dst, 0, 4);
        }
        print_str("Uninstall complete. Restart Windows NT to restore standard PS/2 mouse.\r\n");
        CloseServiceHandle(scm);
        ExitProcess(0);
    }

    print_str("Installing VMMouse for NT 3.x...\r\n");

    build_driver_path(windir, wlen, "vmmouse2.sys", cleanup);
    SetFileAttributesA(cleanup, 0x80); DeleteFileA(cleanup);
    build_driver_path(windir, wlen, "vmmouse3.sys", cleanup);
    SetFileAttributesA(cleanup, 0x80); DeleteFileA(cleanup);
    build_driver_path(windir, wlen, "vmmouse4.sys", cleanup);
    SetFileAttributesA(cleanup, 0x80); DeleteFileA(cleanup);

    get_source_path(src, sizeof(src));
    print_str("Copying ");
    print_str(src);
    print_str(" -> ");
    print_str(dst);
    print_str("...\r\n");

    SetFileAttributesA(dst, 0x80);
    if (!CopyFileA(src, dst, 0)) {
        unsigned long err = GetLastError();
        print_error("CopyFile failed", err);
        if (svc) CloseServiceHandle(svc);
        CloseServiceHandle(scm);
        ExitProcess(3);
    }
    SetFileAttributesA(dst, 0x80);
    print_str("CopyFile successful.\r\n");

    if (!svc) {
        svc = CreateServiceA(
            scm,
            "vmmouse",
            "VMMouse for NT 3.x",
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_SYSTEM_START,
            SERVICE_ERROR_NORMAL,
            "System32\\drivers\\vmmouse.sys",
            "Pointer Port",
            0, 0, 0, 0
        );
    } else {
        ChangeServiceConfigA(
            svc,
            SERVICE_NO_CHANGE,
            SERVICE_SYSTEM_START,
            SERVICE_ERROR_NORMAL,
            "System32\\drivers\\vmmouse.sys",
            "Pointer Port",
            0, 0, 0, 0,
            "VMMouse for NT 3.x"
        );
    }

    if (svc) {
        print_str("SUCCESS: Installation complete!\r\n");
        print_str("Restart Windows NT to activate VMMouse.\r\n");
        CloseServiceHandle(svc);
    } else {
        unsigned long err = GetLastError();
        print_error("Error: Could not configure service", err);
        if (err == 1072) {
            print_str("Note: Reboot required before service can be reinstalled.\r\n");
        }
    }

    CloseServiceHandle(scm);
    ExitProcess(0);
}
