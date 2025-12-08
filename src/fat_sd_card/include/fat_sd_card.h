#include "ff_stdio.h"

bool fat_sd_card_init(bool should_format);
bool fat_sd_card_deinit();
FF_FILE *fat_sd_card_open(const char *pcFile, const char *pcMode);
bool fat_sd_card_close(FF_FILE *pxStream);
size_t fat_sd_card_write(const void *pvBuffer, size_t xItems, FF_FILE *pxStream);
size_t fat_sd_card_read(void *pvBuffer, size_t xItems, FF_FILE *pxStream);