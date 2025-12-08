/*
 * FreeRTOS+FAT Labs Build 160919 (C) 2016 Real Time Engineers ltd.
 * Authors include James Walmsley, Hein Tibosch and Richard Barry
 *
 *
 * FreeRTOS+FAT can be used under two different free open source licenses.  The
 * license that applies is dependent on the processor on which FreeRTOS+FAT is
 * executed, as follows:
 *
 * If FreeRTOS+FAT is executed on one of the processors listed under the Special
 * License Arrangements heading of the FreeRTOS+FAT license information web
 * page, then it can be used under the terms of the FreeRTOS Open Source
 * License.  If FreeRTOS+FAT is used on any other processor, then it can be used
 * under the terms of the GNU General Public License V2.  Links to the relevant
 * licenses follow:
 *
 * The FreeRTOS+FAT License Information Page: https://www.freertos.org/Documentation/03-Libraries/05-FreeRTOS-labs/04-FreeRTOS-plus-FAT/FreeRTOS_Plus_FAT_License
 * The FreeRTOS Open Source License: https://www.freertos.org/Documentation/03-Libraries/01-Library-overview/04-Licensing
 *
 * FreeRTOS+FAT is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+FAT unless you agree that you use the software 'as is'.
 * FreeRTOS+FAT is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * This is a stub for allowing linux to compile. should just make it save to a file.
 *
 *
 */

#include "ff_sddisk.h"

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portmacro.h"

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_sys.h"

#include "sd_card.h"
#include "hw_config.h"

/* Dummy variables. */
#define HUNDRED_64_BIT (100U)
#define BYTES_PER_MB (1024U * 1024U)
#define SECTORS_PER_MB (BYTES_PER_MB / 512U)

#define FileSECTOR_SIZE 512UL
#define FilePARTITION_NUMBER 0 /* Only a single partition is used. */

#define SECTOR_SIZE 512UL
#define PARTITION_NUMBER 0 /* Only a single partition is used. */
#define BYTES_PER_KB (1024ull)
#define SECTORS_PER_KB (BYTES_PER_KB / 512ull)
static FILE *file;

static int32_t prvReadFile(uint8_t *pucDestination,
                           uint32_t ulSectorNumber,
                           uint32_t ulSectorCount,
                           FF_Disk_t *pxDisk);

static int32_t prvWriteFile(uint8_t *pucSource,
                            uint32_t ulSectorNumber,
                            uint32_t ulSectorCount,
                            FF_Disk_t *pxDisk);
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskDetect(FF_Disk_t *pxDisk)
{
    (void)pxDisk; /* Unused */
    return pdTRUE;
}

/*-----------------------------------------------------------*/

/* Flush changes from the driver's buf to disk */
void FF_SDDiskFlush(FF_Disk_t *pDisk) { FF_FlushCache(pDisk->pxIOManager); }

/*-----------------------------------------------------------*/

FF_Disk_t *FF_SDDiskInitWithSettings(const char *pcName,
                                     const FFInitSettings_t *pxSettings)
{
    (void)pxSettings; /* Unused */

    return FF_SDDiskInit(pcName);
}
/*-----------------------------------------------------------*/
// Doesn't do an automatic mount, since card might need to be formatted first.
// State after return is disk is initialized, but not mounted.
FF_Disk_t *FF_SDDiskInit(const char *pcName)
{
    sd_card_t *sd_card_p = sd_get_by_name(pcName);
    if (!sd_card_p)
    {
        FF_PRINTF("FF_SDDiskInit: unknown name %s\n", pcName);
        return NULL;
    }
    if (disk_init(sd_card_p))
        return &sd_card_p->state.ff_disk;
    else
        return NULL;
}

bool disk_init(sd_card_t *sd_card_p)
{

    FF_Disk_t *pxDisk = NULL;
    FF_Error_t xError;
    FF_CreationParameters_t xParameters;
    if (!sd_card_p)
    {
        return false;
    }
    pxDisk = &(sd_card_p->state.ff_disk);

    if (pxDisk->xStatus.bIsInitialised != pdFALSE)
    {
        // Already initialized
        return true;
    }

    // Opening the file to be used as the storage
    sd_card_p->file_if.file = fopen(sd_card_p->file_if.file_name, "r+");
    if (sd_card_p->file_if.file == NULL)
    {
        sd_card_p->file_if.file == fopen(sd_card_p->file_if.file_name, "w+");
    }
    configASSERT(sd_card_p->file_if.file);
    if (!sd_card_p->file_if.file)
    {
        FF_PRINTF("FF_SDDiskInit: Unable to create file%s\n", sd_card_p->file_if.file_name);
        return false;
    }
    fseek(sd_card_p->file_if.file, 0, SEEK_SET);

    sd_card_p->state.ff_disk.pvTag = sd_card_p;

    /* Other stuff here*/

    pxDisk->ulNumberOfSectors = 5UL * 1024UL * 1024UL / FileSECTOR_SIZE;
    /* Create the IO manager that will be used to control the RAM disk. */
    memset(&xParameters, '\0', sizeof(xParameters));
    xParameters.pucCacheMemory = NULL;
    xParameters.ulMemorySize = 15 * FileSECTOR_SIZE;
    xParameters.ulSectorSize = FileSECTOR_SIZE;
    xParameters.fnWriteBlocks = prvWriteFile;
    xParameters.fnReadBlocks = prvReadFile;
    xParameters.pxDisk = pxDisk;

    /* Driver is reentrant so xBlockDeviceIsReentrant can be set to pdTRUE.
     * In this case the semaphore is only used to protect FAT data
     * structures. */
    xParameters.pvSemaphore = (void *)xSemaphoreCreateRecursiveMutex();
    xParameters.xBlockDeviceIsReentrant = pdFALSE;

    pxDisk->pxIOManager = FF_CreateIOManager(&xParameters, &xError);

    configASSERT(FF_isERR(xError) == pdFALSE);

    if ((pxDisk->pxIOManager != NULL) && (FF_isERR(xError) == pdFALSE))
    {
        pxDisk->xStatus.bIsInitialised = pdTRUE;
        return true;
    }
    /* The disk structure was allocated, but the disk’s IO manager could
   not be allocated, so free the disk again. */
    FF_SDDiskDelete(&sd_card_p->state.ff_disk);
    FF_PRINTF("FF_SDDiskInit: FF_CreateIOManger: %s\n",
              (const char *)FF_GetErrMessage(xError));
    configASSERT(!"disk's IO manager could not be allocated!");
    sd_card_p->state.ff_disk.xStatus.bIsInitialised = pdFALSE;

    return false;
}
/*-----------------------------------------------------------*/

/* Format a given partition on an SD-card. */
BaseType_t FF_SDDiskFormat(FF_Disk_t *pxDisk, BaseType_t aPart)
{
    // FF_Error_t FF_Format( FF_Disk_t *pxDisk, BaseType_t xPartitionNumber,
    // BaseType_t xPreferFAT16, BaseType_t xSmallClusters );
    FF_Error_t e = FF_Format(pxDisk, aPart, pdFALSE, pdFALSE);
    if (FF_ERR_NONE != e)
    {
        FF_PRINTF("FF_Format error:%s\n", FF_GetErrMessage(e));
    }
    return e;
}
/*-----------------------------------------------------------*/

/* Unmount the volume */
// FF_SDDiskUnmount() calls FF_Unmount().
BaseType_t FF_SDDiskUnmount(FF_Disk_t *pxDisk)
{
    if (!pxDisk->xStatus.bIsMounted)
        return FF_ERR_NONE;
    sd_card_t *sd_card_p = pxDisk->pvTag;
    const char *name = sd_card_p->device_name;
    FF_PRINTF("Invalidating %s\n", name);
    int32_t rc = FF_Invalidate(pxDisk->pxIOManager);
    if (0 == rc)
        FF_PRINTF("no handles were open\n");
    else if (rc > 0)
        FF_PRINTF("%ld handles were invalidated\n", rc);
    else
        FF_PRINTF("%ld: probably an invalid FF_IOManager_t pointer\n", rc);
    FF_FlushCache(pxDisk->pxIOManager);
    // sd_card_p->sync(sd_card_p);
    FF_PRINTF("Unmounting %s\n", name);
    FF_Error_t e = FF_Unmount(pxDisk);
    if (FF_ERR_NONE != e)
    {
        FF_PRINTF("FF_Unmount error: %s\n", FF_GetErrMessage(e));
    }
    else
    {
        pxDisk->xStatus.bIsMounted = pdFALSE;
    }
    return e;
}
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskReinit(FF_Disk_t *pxDisk)
{
    return disk_init(pxDisk->pvTag) ? pdPASS : pdFAIL;
}
/*-----------------------------------------------------------*/

/* Mount the volume */
// FF_SDDiskMount() calls FF_Mount().
BaseType_t FF_SDDiskMount(FF_Disk_t *pDisk)
{
    if (pDisk->xStatus.bIsMounted)
        return FF_ERR_NONE;
    if (pdFALSE == pDisk->xStatus.bIsInitialised)
    {
        bool ok = disk_init(pDisk->pvTag);
        if (!ok)
            return FF_ERR_DEVICE_DRIVER_FAILED;
    }
    // FF_Error_t FF_Mount( FF_Disk_t *pxDisk, BaseType_t xPartitionNumber );
    FF_Error_t e = FF_Mount(pDisk, PARTITION_NUMBER);
    if (FF_ERR_NONE != e)
    {
        FF_PRINTF("FF_Mount error: %s\n", FF_GetErrMessage(e));
    }
    else
    {
        pDisk->xStatus.bIsMounted = pdTRUE;
    }
    return e;
}

/*-----------------------------------------------------------*/

/* Get a pointer to IOMAN, which can be used for all FreeRTOS+FAT functions */
FF_IOManager_t *sddisk_ioman(FF_Disk_t *pxDisk)
{
    FF_IOManager_t *pxReturn;

    if ((pxDisk != NULL) && (pxDisk->xStatus.bIsInitialised != pdFALSE))
    {
        pxReturn = pxDisk->pxIOManager;
    }
    else
    {
        pxReturn = NULL;
    }

    return pxReturn;
}
/*-----------------------------------------------------------*/

/* Release all resources */

BaseType_t FF_SDDiskDelete(FF_Disk_t *pxDisk)
{
    if (pxDisk)
    {
        if (pxDisk->xStatus.bIsInitialised)
        {
            FILE *file = ((sd_card_t *)(pxDisk->pvTag))->file_if.file;
            if (file)
            {
                fclose(file);
            }
            if (pxDisk->pxIOManager)
            {
                FF_DeleteIOManager(pxDisk->pxIOManager);
            }
            pxDisk->ulSignature = 0;
            pxDisk->xStatus.bIsInitialised = pdFALSE;
        }
        return pdPASS;
    }
    else
    {
        return pdFAIL;
    }
}
/*-----------------------------------------------------------*/

/* Show some partition information */
BaseType_t FF_SDDiskShowPartition(FF_Disk_t *pxDisk)
{
    FF_Error_t xError;
    uint64_t ullFreeSectors;
    uint32_t ulTotalSizeKB, ulFreeSizeKB;
    int iPercentageFree;
    FF_IOManager_t *pxIOManager;
    const char *pcTypeName = "unknown type";
    BaseType_t xReturn = pdPASS;

    if (pxDisk == NULL)
    {
        xReturn = pdFAIL;
    }
    else
    {
        pxIOManager = pxDisk->pxIOManager;

        FF_PRINTF("Reading FAT and calculating Free Space\n");

        switch (pxIOManager->xPartition.ucType)
        {
        case FF_T_FAT12:
            pcTypeName = "FAT12";
            break;

        case FF_T_FAT16:
            pcTypeName = "FAT16";
            break;

        case FF_T_FAT32:
            pcTypeName = "FAT32";
            break;

        default:
            pcTypeName = "UNKOWN";
            break;
        }

        FF_GetFreeSize(pxIOManager, &xError);

        ullFreeSectors = pxIOManager->xPartition.ulFreeClusterCount *
                         pxIOManager->xPartition.ulSectorsPerCluster;
        if (pxIOManager->xPartition.ulDataSectors == (uint32_t)0)
        {
            iPercentageFree = 0;
        }
        else
        {
            iPercentageFree = (int)((HUNDRED_64_BIT * ullFreeSectors +
                                     pxIOManager->xPartition.ulDataSectors / 2) /
                                    ((uint64_t)pxIOManager->xPartition.ulDataSectors));
        }

        ulTotalSizeKB = pxIOManager->xPartition.ulDataSectors / SECTORS_PER_KB;
        ulFreeSizeKB = (uint32_t)(ullFreeSectors / SECTORS_PER_KB);

        FF_PRINTF("Partition Nr   %8u\n", pxDisk->xStatus.bPartitionNumber);
        FF_PRINTF("Type           %8u (%s)\n", pxIOManager->xPartition.ucType, pcTypeName);
        FF_PRINTF("VolLabel       '%8s' \n", pxIOManager->xPartition.pcVolumeLabel);
        FF_PRINTF("TotalSectors   %8lu\n",
                  (unsigned long)pxIOManager->xPartition.ulTotalSectors);
        FF_PRINTF("SecsPerCluster %8lu\n",
                  (unsigned long)pxIOManager->xPartition.ulSectorsPerCluster);
        FF_PRINTF("Size           %8lu KB\n", (unsigned long)ulTotalSizeKB);
        FF_PRINTF("FreeSize       %8lu KB ( %d perc free )\n", (unsigned long)ulFreeSizeKB,
                  iPercentageFree);
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskInserted(BaseType_t xDriveNr)
{
    (void)xDriveNr;
    sd_card_t *sd_card_p = sd_get_by_num(xDriveNr);
    if (!sd_card_p)
        return false;
    return fopen(sd_card_p->file_if.file_name, "r") != NULL;
}

static int32_t prvWriteFile(uint8_t *pucSource,
                            uint32_t ulSectorNumber,
                            uint32_t ulSectorCount,
                            FF_Disk_t *pxDisk)
{
    long position = FileSECTOR_SIZE * ulSectorNumber; // write starting at byte 10
    fseek(((sd_card_t *)(pxDisk->pvTag))->file_if.file, position, SEEK_SET);
    fwrite(pucSource, 1, ulSectorCount * FileSECTOR_SIZE, (FILE *)((sd_card_t *)(pxDisk->pvTag))->file_if.file);
    return FF_ERR_NONE;
}

static int32_t prvReadFile(uint8_t *pucDestination,
                           uint32_t ulSectorNumber,
                           uint32_t ulSectorCount,
                           FF_Disk_t *pxDisk)
{
    long position = FileSECTOR_SIZE * ulSectorNumber; // write starting at byte 10
    fseek(((sd_card_t *)(pxDisk->pvTag))->file_if.file, position, SEEK_SET);
    fread(pucDestination, 1, ulSectorCount * FileSECTOR_SIZE, ((sd_card_t *)(pxDisk->pvTag))->file_if.file);
    return FF_ERR_NONE;
}

FF_Error_t prvPartitionAndFormatDisk(FF_Disk_t *pxDisk)
{
    FF_PartitionParameters_t xPartition;
    FF_Error_t xError;

    /* Create a single partition that fills all available space on the disk. */
    memset(&xPartition, '\0', sizeof(xPartition));
    xPartition.ulSectorCount = pxDisk->ulNumberOfSectors;
    xPartition.ulHiddenSectors = 8;
    xPartition.xPrimaryCount = 1;
    xPartition.eSizeType = eSizeIsQuota;

    /* Partition the disk */
    xError = FF_Partition(pxDisk, &xPartition);
    FF_PRINTF("FF_Partition: %s\n", (const char *)FF_GetErrMessage(xError));

    if (FF_isERR(xError) == pdFALSE)
    {
        /* Format the partition. */
        xError = FF_Format(pxDisk, 0, pdTRUE, pdTRUE);
        FF_PRINTF("FF_RAMDiskInit: FF_Format: %s\n", (const char *)FF_GetErrMessage(xError));
    }

    return xError;
}