#include "assert_panic.h"

#include "stdio.h"
void my_assert_func_panic(const char *file, int line, const char *func, const char *pred)
{
    // printf(
    //     "%s: assertion \"%s\" failed: file \"%s\", line %d, function: %s\n",
    //     pcTaskGetName(NULL), pred, file, line, func);
    // panic("");
}

void my_assert_func(const char *file, int line, const char *func, const char *pred)
{
    // error_message_printf_plain(
    //     "%s: assertion \"%s\" failed: file \"%s\", line %d, function: %s\n",
    //     pcTaskGetName(NULL), pred, file, line, func);
    // stdio_flush();
    // vTaskSuspendAll();
    // __disable_irq(); /* Disable global interrupts. */
    // capture_assert(file, line, func, pred);
}