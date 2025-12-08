/**
 * @file sensors.h
 * @brief Header file for WiFi functionality.
 */

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
    void my_assert_func_panic(const char *file, int line, const char *func, const char *pred);

    void my_assert_func(const char *file, int line, const char *func, const char *pred);

#ifdef __cplusplus
}
#endif