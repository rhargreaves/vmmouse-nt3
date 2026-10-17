#include "service.h"
#include "console.h"
#include "errors.h"
#include "win32.h"

int service_uninstall(const char *dst) {
    void *scm;
    void *svc;

    console_print("Uninstalling " DISPLAY_NAME "...\r\n");

    scm = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        console_error("Error: OpenSCManager failed", GetLastError());
        return ERR_SCM_OPEN;
    }

    svc = OpenServiceA(scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (svc) {
        if (DeleteService(svc)) {
            console_print("Service marked for deletion.\r\n");
        } else {
            unsigned long err = GetLastError();
            if (err == ERR_SERVICE_MARKED_FOR_DELETE) {
                console_print("Service is already marked for deletion.\r\n");
            } else {
                console_error("DeleteService failed", err);
            }
        }
        CloseServiceHandle(svc);
    } else {
        console_print("Service not found.\r\n");
    }

    SetFileAttributesA(dst, FILE_ATTRIBUTE_NORMAL);
    if (!DeleteFileA(dst)) {
        MoveFileExA(dst, 0, MOVEFILE_DELAY_UNTIL_REBOOT);
    }

    console_print("Uninstall complete. Restart Windows NT to restore standard PS/2 mouse.\r\n");
    CloseServiceHandle(scm);
    return ERR_NONE;
}

int service_install(const char *src, const char *dst) {
    void *scm;
    void *svc;

    console_print("Installing " DISPLAY_NAME "...\r\n");
    console_print("Copying ");
    console_print(src);
    console_print(" -> ");
    console_print(dst);
    console_print("...\r\n");

    SetFileAttributesA(dst, FILE_ATTRIBUTE_NORMAL);
    if (!CopyFileA(src, dst, 0)) {
        console_error("CopyFile failed", GetLastError());
        return ERR_FILE_COPY;
    }
    SetFileAttributesA(dst, FILE_ATTRIBUTE_NORMAL);
    console_print("CopyFile successful.\r\n");

    scm = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        console_error("Error: OpenSCManager failed", GetLastError());
        return ERR_SCM_OPEN;
    }

    svc = OpenServiceA(scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!svc) {
        svc = CreateServiceA(scm, SERVICE_NAME, DISPLAY_NAME, SERVICE_ALL_ACCESS,
                             SERVICE_KERNEL_DRIVER, SERVICE_SYSTEM_START, SERVICE_ERROR_NORMAL,
                             DRIVER_PATH_SYS32, DRIVER_GROUP, 0, 0, 0, 0);
    } else {
        ChangeServiceConfigA(svc, SERVICE_NO_CHANGE, SERVICE_SYSTEM_START, SERVICE_ERROR_NORMAL,
                             DRIVER_PATH_SYS32, DRIVER_GROUP, 0, 0, 0, 0, DISPLAY_NAME);
    }

    if (svc) {
        console_print("SUCCESS: Installation complete!\r\n");
        console_print("Restart Windows NT to activate VMMouse.\r\n");
        CloseServiceHandle(svc);
    } else {
        unsigned long err = GetLastError();
        console_error("Error: Could not configure service", err);
        if (err == ERR_SERVICE_MARKED_FOR_DELETE) {
            console_print("Note: Reboot required before service can be reinstalled.\r\n");
        }
        CloseServiceHandle(scm);
        return ERR_SERVICE_CONFIG;
    }

    CloseServiceHandle(scm);
    return ERR_NONE;
}
