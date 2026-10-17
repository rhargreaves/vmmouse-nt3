#ifndef INSTDRV_PATH_H
#define INSTDRV_PATH_H

int path_get_system_driver(const char *filename, char *out, unsigned long max_len);
int path_get_module_sibling(const char *filename, char *out, unsigned long max_len);

#endif
