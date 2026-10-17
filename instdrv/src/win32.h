#ifndef INSTDRV_WIN32_H
#define INSTDRV_WIN32_H

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define MAX_PATH 260

#define ERROR_SERVICE_MARKED_FOR_DELETE 1072L

#define STD_OUTPUT_HANDLE ((unsigned long)-11)
#define SC_MANAGER_ALL_ACCESS 0xF003F
#define SERVICE_ALL_ACCESS 0xF01FF
#define SERVICE_KERNEL_DRIVER 0x00000001
#define SERVICE_SYSTEM_START 0x00000001
#define SERVICE_ERROR_NORMAL 0x00000001
#define SERVICE_NO_CHANGE 0xFFFFFFFF
#define MOVEFILE_DELAY_UNTIL_REBOOT 0x00000004
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200

extern void *__stdcall GetStdHandle(unsigned long nStdHandle);
extern int __stdcall WriteFile(void *hFile, const void *lpBuffer,
                               unsigned long nNumberOfBytesToWrite,
                               unsigned long *lpNumberOfBytesWritten, void *lpOverlapped);
extern unsigned long __stdcall GetWindowsDirectoryA(char *lpBuffer, unsigned long uSize);
extern int __stdcall CopyFileA(const char *lpExistingFileName, const char *lpNewFileName,
                               int bFailIfExists);
extern int __stdcall DeleteFileA(const char *lpFileName);
extern int __stdcall MoveFileExA(const char *lpExistingFileName, const char *lpNewFileName,
                                 unsigned long dwFlags);
extern int __stdcall SetFileAttributesA(const char *lpFileName, unsigned long dwFileAttributes);
extern char *__stdcall GetCommandLineA(void);
extern unsigned long __stdcall GetLastError(void);
extern unsigned long __stdcall GetModuleFileNameA(void *hModule, char *lpFilename,
                                                  unsigned long nSize);
extern unsigned long __stdcall FormatMessageA(unsigned long dwFlags, const void *lpSource,
                                              unsigned long dwMessageId, unsigned long dwLanguageId,
                                              char *lpBuffer, unsigned long nSize, void *Arguments);
extern void __stdcall ExitProcess(unsigned int uExitCode);

extern void *__stdcall OpenSCManagerA(const char *lpMachineName, const char *lpDatabaseName,
                                      unsigned long dwDesiredAccess);
extern void *__stdcall CreateServiceA(void *hSCManager, const char *lpServiceName,
                                      const char *lpDisplayName, unsigned long dwDesiredAccess,
                                      unsigned long dwServiceType, unsigned long dwStartType,
                                      unsigned long dwErrorControl, const char *lpBinaryPathName,
                                      const char *lpLoadOrderGroup, unsigned long *lpdwTagId,
                                      const char *lpDependencies, const char *lpServiceStartName,
                                      const char *lpPassword);
extern void *__stdcall OpenServiceA(void *hSCManager, const char *lpServiceName,
                                    unsigned long dwDesiredAccess);
extern int __stdcall ChangeServiceConfigA(void *hService, unsigned long dwServiceType,
                                          unsigned long dwStartType, unsigned long dwErrorControl,
                                          const char *lpBinaryPathName,
                                          const char *lpLoadOrderGroup, unsigned long *lpdwTagId,
                                          const char *lpDependencies,
                                          const char *lpServiceStartName, const char *lpPassword,
                                          const char *lpDisplayName);
extern int __stdcall DeleteService(void *hService);
extern int __stdcall CloseServiceHandle(void *hSCObject);

#endif
