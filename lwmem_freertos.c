#include "lwmem_os.h"

#if (LWMEM_CONFIG_USE_OS != 0)

uint8_t lwmem_os_create_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex)
{
    *mutex = xSemaphoreCreateMutex();

    return ((*mutex) != NULL) ? 0 : 1;
}

uint8_t lwmem_os_verify_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex)
{
    return ((*mutex) != NULL) ? 0 : 1;
}

uint8_t lwmem_os_take_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex)
{
    return (xSemaphoreTake(*mutex, portMAX_DELAY) != pdTRUE) ? 1 : 0;
}

uint8_t lwmem_os_give_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex)
{
    return (xSemaphoreGive(*mutex) != pdTRUE) ? 1 : 0;
}

#endif
