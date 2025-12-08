#pragma once

#include <stdbool.h>

#include "FreeRTOS.h"
#include "ff_headers.h"
#include "sd_card.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool format(const char *devName);
bool mount(const char *devName);
void unmount(const char *devName);
void eject(const char *name);
void getFree(FF_Disk_t *pxDisk, uint64_t *pFreeMB, unsigned *pFreePct);
FF_Error_t ff_set_fsize( FF_FILE *pxFile ); // Make Filesize equal to the FilePointer
int mkdirhier(char *path);
void ls(const char *path);

#ifdef __cplusplus
}
#endif
