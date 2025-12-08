#include "ff_sddisk.h"
#include "ff_stdio.h"

#include "ff_utils.h"

#include "sd_card.h"
#include "hw_config.h"


FF_Error_t prvPartitionAndFormatDisk(FF_Disk_t *pxDisk);
bool fat_sd_card_init(bool should_format)
{
    sd_card_t *sd_card_p = sd_get_by_num(0);
    bool noError = true;
    BaseType_t xError = FF_ERR_NONE;
    noError &= FF_SDDiskInit("sd0") != nullptr;
    configASSERT(noError);

    if (noError && should_format)
    {
        noError &= format("sd0");
    }
    if (noError)
    {
        xError = FF_SDDiskMount(&(sd_card_p->state.ff_disk));
        configASSERT(!FF_isERR(xError));
        noError &= !FF_isERR(xError);
    }

    if (noError)
    {
        noError &= FF_FS_Add("/sd0", &(sd_card_p->state.ff_disk));
    }

    return noError;
}

bool fat_sd_card_deinit()
{
    bool noError = true;
    sd_card_t *sd_card_p = sd_get_by_num(0);

    FF_FS_Remove("/sd0");
    noError &= (FF_SDDiskUnmount(&(sd_card_p->state.ff_disk)) == FF_ERR_NONE);
    FF_SDDiskDelete(&(sd_card_p->state.ff_disk));
    return noError;
}

FF_FILE *fat_sd_card_open(const char *pcFile, const char *pcMode)
{
    return ff_fopen(pcFile, pcMode);
}

bool fat_sd_card_close(FF_FILE *pxStream)
{
    return !FF_isERR(ff_fclose(pxStream));
}

size_t fat_sd_card_write(const void *pvBuffer, size_t xItems, FF_FILE *pxStream)
{
    return ff_fwrite(pvBuffer,
                     xItems,
                     1,
                     pxStream);
}

size_t fat_sd_card_read(void *pvBuffer, size_t xItems, FF_FILE *pxStream)
{
    return ff_fread(pvBuffer,
                    xItems,
                    1,
                    pxStream);
}
