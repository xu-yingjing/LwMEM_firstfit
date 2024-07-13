#ifndef __LWMEM_OS_H
#define __LWMEM_OS_H

#include "lwmem_config_ex.h"

#if (LWMEM_CONFIG_USE_OS != 0)

/** 
 * Create a mutex.
 * 
 * @param mutex Pointer to the mutex handle.
 * 
 * @return 0 if successful, otherwise 1.
 * 
 * @example
 * LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE mutex;
 * if (lwmem_os_create_mutex(&mutex) != 0)
 * {
 *     // Mutex created error.
 * }
 * else
 * {
 *     // Mutex created successfully.
 * }
 */
uint8_t lwmem_os_create_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex);

/**
 * Verify mutex has been created.
 * 
 * @param mutex Pointer to the mutex handle.
 * 
 * @return 0 if successful, otherwise 1.
 * 
 * @example
 * LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE mutex;
 * if (lwmem_os_verify_mutex(&mutex) == 0)
 * {
 *     // Verify successfully, mutex has been created.
 * }
 * else
 * {
 *     // Verify error, mutex has not been created.
 * }
 */
uint8_t lwmem_os_verify_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex);

/**
 * Obtain a mutex.
 * 
 * @param mutex Pointer to the mutex handle.
 * 
 * @return 0 if successful, otherwise 1.
 * 
 * @example
 * LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE mutex;
 * if (lwmem_os_take_mutex(&mutex) != 0)
 * {
 *     // Mutex obtained error.
 * }
 * else
 * {
 *     // Mutex obtained successfully.
 * }
 */
uint8_t lwmem_os_take_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex);

/**
 * Release a mutex.
 * 
 * @param mutex Pointer to the mutex handle.
 * 
 * @return 0 if successful, otherwise 1.
 * 
 * @example
 * LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE mutex;
 * if (lwmem_os_give_mutex(&mutex) != 0)
 * {
 *     // Mutex released error.
 * }
 * else
 * {
 *     // Mutex released successfully.
 * }
 */
uint8_t lwmem_os_give_mutex(LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE *mutex);

#endif

#endif /* __LWMEM_OS_H */
