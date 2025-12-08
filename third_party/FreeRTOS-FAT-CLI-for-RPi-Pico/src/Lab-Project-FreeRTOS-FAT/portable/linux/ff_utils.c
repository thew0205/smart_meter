#include "ff_utils.h"
#include "ff_sddisk.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
//
#include "FreeRTOS.h"
#include "ff_headers.h"
#include "ff_sddisk.h"
#include "ff_stdio.h"

#include "FreeRTOS_strerror.h"
#include "file_stream.h"
#include "hw_config.h"
#include "my_debug.h"

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_sys.h"


#define TRACE_PRINTF(fmt, args...)
// #define TRACE_PRINTF printf

bool format(const char *name) {
    FF_Disk_t *pxDisk = FF_SDDiskInit(name);
    if (!pxDisk) {
        return false;
    }
    FF_Error_t e = prvPartitionAndFormatDisk(pxDisk);
    return FF_ERR_NONE == e ? true : false;
}


bool mount(const char *name) {
    TRACE_PRINTF("> %s\n", __FUNCTION__);
    FF_Disk_t *pxDisk = FF_SDDiskInit(name);
    if (!pxDisk) return false;
    if (pxDisk->xStatus.bIsMounted) return true;
    FF_Error_t xError = FF_SDDiskMount(pxDisk);
    if (FF_isERR(xError) != pdFALSE) return false;
    sd_card_t *sd_card_p = pxDisk->pvTag;
    configASSERT(sd_card_p);
    return FF_FS_Add(sd_card_p->mount_point, pxDisk);
}
void unmount(const char *name) {
    TRACE_PRINTF("> %s\n", __FUNCTION__);
    sd_card_t *sd_card_p = sd_get_by_name(name);
    if (!sd_card_p) {
        return;
    }
    FF_FS_Remove(sd_card_p->mount_point);
    FF_Disk_t *pxDisk = &sd_card_p->state.ff_disk;

    /*Unmount the partition. */
    FF_Error_t xError = FF_SDDiskUnmount(pxDisk);
    if (FF_isERR(xError) != pdFALSE) {
        FF_PRINTF("FF_SDDiskUnmount: %s (0x%08x)\n", (const char *)FF_GetErrMessage(xError),
                  (unsigned)xError);
    }
    FF_SDDiskDelete(pxDisk);
}

void getFree(FF_Disk_t *pxDisk, uint64_t *pFreeMB, unsigned *pFreePct) {
    FF_Error_t xError;
    uint64_t ullFreeSectors, ulFreeSizeKB;
    int iPercentageFree;

    configASSERT(pxDisk);
    FF_IOManager_t *pxIOManager = pxDisk->pxIOManager;

    FF_GetFreeSize(pxIOManager, &xError);

    ullFreeSectors = pxIOManager->xPartition.ulFreeClusterCount *
                     pxIOManager->xPartition.ulSectorsPerCluster;
    if (pxIOManager->xPartition.ulDataSectors == 0) {
        iPercentageFree = 0;
    } else {
        iPercentageFree =
            (int)((100ULL * ullFreeSectors + pxIOManager->xPartition.ulDataSectors / 2) /
                  ((uint64_t)pxIOManager->xPartition.ulDataSectors));
    }

    const int SECTORS_PER_KB = 2;
    ulFreeSizeKB = (uint32_t)(ullFreeSectors / SECTORS_PER_KB);

    *pFreeMB = ulFreeSizeKB / 1024;
    *pFreePct = iPercentageFree;
}

// Make Filesize equal to the FilePointer
FF_Error_t FF_UpdateDirEnt(FF_FILE *pxFile) {
    FF_DirEnt_t xOriginalEntry;
    FF_Error_t xError;

    /* Get the directory entry and update it to show the new file size */
    xError = FF_GetEntry(pxFile->pxIOManager, pxFile->usDirEntry, pxFile->ulDirCluster,
                         &xOriginalEntry);

    /* Now update the directory entry */
    if ((FF_isERR(xError) == pdFALSE) &&
        ((pxFile->ulFileSize != xOriginalEntry.ulFileSize) || (pxFile->ulFileSize == 0UL))) {
        if (pxFile->ulFileSize == 0UL) {
            xOriginalEntry.ulObjectCluster = 0;
        }

        xOriginalEntry.ulFileSize = pxFile->ulFileSize;
        xError = FF_PutEntry(pxFile->pxIOManager, pxFile->usDirEntry, pxFile->ulDirCluster,
                             &xOriginalEntry, NULL);
    }
    return xError;
}

int prvFFErrorToErrno(FF_Error_t xError);  // In ff_stdio.c

FF_Error_t ff_set_fsize(FF_FILE *pxStream) {
    FF_Error_t iResult;
    int iReturn, ff_errno;

    iResult = FF_UpdateDirEnt(pxStream);

    ff_errno = prvFFErrorToErrno(iResult);

    if (ff_errno == 0) {
        iReturn = 0;
    } else {
        iReturn = -1;
    }

    /* Store the errno to thread local storage. */
    stdioSET_ERRNO(ff_errno);

    return iReturn;
}

/*
** mkdirhier() - create all directories in a given path
** returns:
**	0			success
**	1			all directories already exist
**	-1 (and sets errno)	error
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
int mkdirhier(char *path) {
    char src[ffconfigMAX_FILENAME], dst[ffconfigMAX_FILENAME] = "";
    char *dirp, *nextp = src;
    int retval = 1;

    if (strlcpy(src, path, sizeof(src)) > sizeof(src)) {
        stdioSET_ERRNO(pdFREERTOS_ERRNO_ENAMETOOLONG);
        return -1;
    }

    if (path[0] == '/') strcpy(dst, "/");

    while ((dirp = strsep(&nextp, "/")) != NULL) {
        if (*dirp == '\0') continue;

        if ((dst[0] != '\0') && !(dst[0] == '/' && dst[1] == '\0')) strcat(dst, "/");
        // size_t strlcat(char *dst, const char *src, size_t size);
        strlcat(dst, dirp, sizeof dst);

        //		DBG_PRINTF("Creating directory dst = %s\n",dst);
        if (ff_mkdir(dst) == -1) {
            if (stdioGET_ERRNO() != pdFREERTOS_ERRNO_EEXIST) {
                int error = stdioGET_ERRNO();
                DBG_PRINTF("%s: %s (%d)\n", __FUNCTION__, FreeRTOS_strerror(error), error);
                return -1;
            }
        } else
            retval = 0;
    }

    return retval;
}
#pragma GCC diagnostic pop

void ls(const char *path) {
    char pcWriteBuffer[128] = {0};

    FF_FindData_t xFindStruct;
    memset(&xFindStruct, 0x00, sizeof(FF_FindData_t));

    if (!path) ff_getcwd(pcWriteBuffer, sizeof(pcWriteBuffer));
    IMSG_PRINTF("Directory Listing: %s\n", path ? path : pcWriteBuffer);

    int iReturned = ff_findfirst(path ? path : "", &xFindStruct);
    if (FF_ERR_NONE != iReturned) {
        FF_PRINTF("ff_findfirst error: %s (%d)\n", FreeRTOS_strerror(stdioGET_ERRNO()),
                  -stdioGET_ERRNO());
        return;
    }
    do {
        const char *pcWritableFile = "writable file", *pcReadOnlyFile = "read only file",
                   *pcDirectory = "directory";
        const char *pcAttrib;

        /* Point pcAttrib to a string that describes the file. */
        if ((xFindStruct.ucAttributes & FF_FAT_ATTR_DIR) != 0) {
            pcAttrib = pcDirectory;
        } else if (xFindStruct.ucAttributes & FF_FAT_ATTR_READONLY) {
            pcAttrib = pcReadOnlyFile;
        } else {
            pcAttrib = pcWritableFile;
        }
        /* Create a string that includes the file name, the file size and the
         attributes string. */
        IMSG_PRINTF("%s\t[%s]\t[size=%lu]\n", xFindStruct.pcFileName, pcAttrib,
                    xFindStruct.ulFileSize);
    } while (FF_ERR_NONE == ff_findnext(&xFindStruct));
}

sd_card_t *get_current_sd_card_p() {
    char buf[256];
    char *ret = ff_getcwd(buf, sizeof buf);
    if (!ret) {
        FF_PRINTF("ff_getcwd failed\n");
        return NULL;
    }
    IMSG_PRINTF("Working directory: %s\n", buf);
    if (strlen(buf) < 2) {
        FF_PRINTF("Can't write to current working directory: %s\n", buf);
        return NULL;
    }
    configASSERT('/' == buf[0]);
    size_t i;
    for (i = 1; i < sizeof buf; ++i) {
        if (0 == buf[i]) break;
        if ('/' == buf[i]) {
            buf[i] = 0;
            break;
        }
    }
    if (sizeof buf == i) {
        FF_PRINTF("Couldn't find mount point in %s\n", buf);
        return NULL;
    }
    sd_card_t *sd_card_p = sd_get_by_mount_point(buf);
    if (!sd_card_p) {
        FF_PRINTF("Unknown device at mount point %s\n", buf);
        return NULL;
    }
    return sd_card_p;
}

FILE *mk_tmp_fil(const char *prefix, size_t pathname_sz, char *pathname) {
    int nw = snprintf(pathname, pathname_sz, "%s/tmp", prefix);
    // Only when this returned value is non-negative and less than n,
    //    the string has been completely written.
    configASSERT(0 <= nw && nw < (int)pathname_sz);

    ff_mkdir(pathname);

    //    char *tempnam(char *dir, char *pfx);
    //    char *_tempnam_r(struct _reent *reent, char *dir, char *pfx);
    // static struct _reent reent;
    char *pn = tempnam( "", NULL);
    int nw2 = snprintf(pathname + nw, pathname_sz - nw, "%s", pn);
    // Only when this returned value is non-negative and less than n,
    //    the string has been completely written.
    configASSERT(0 <= nw2 && nw2 < (int)pathname_sz);

    FILE *fil;
    fil = open_file_stream(pathname, "w+");
    if (!fil) FF_FAIL("ff_fopen", pathname);
    return fil;
}

/* [] END OF FILE */
