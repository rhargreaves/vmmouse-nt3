#ifndef INSTDRV_SERVICE_H
#define INSTDRV_SERVICE_H

#define SERVICE_NAME "vmmouse"
#define DISPLAY_NAME "VMMouse for NT 3.x"
#define DRIVER_FILENAME "vmmouse.sys"
#define DRIVER_PATH_SYS32 "System32\\drivers\\" DRIVER_FILENAME
#define DRIVER_GROUP "Pointer Port"

int service_install(const char *src, const char *dst);
int service_uninstall(const char *dst);

#endif
