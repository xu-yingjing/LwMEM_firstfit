#ifndef __LWMEM_CONFIG_EX_H
#define __LWMEM_CONFIG_EX_H

#include "lwmem_config.h"

/**
 * Macro to define memory address alignment size.
 * 
 * @note This macro must be a power of 2.
 */
#ifndef LWMEM_CONFIG_BYTE_ALIGNMENT
#define LWMEM_CONFIG_BYTE_ALIGNMENT 4
#else
#if (((LWMEM_CONFIG_BYTE_ALIGNMENT) & ((LWMEM_CONFIG_BYTE_ALIGNMENT) - 1)) > 0)
#error LWMEM_CONFIG_BYTE_ALIGNMENT must be a power of 2.
#endif
#endif

/**
 * Macro to enable '1' or disable '0' the operating system support for LwMEM.
 * 
 * @note When enabled, the LwMEM API functions are thread-safe.
 * @note When enabled, user must implement the functions witch declaration in lwmem_os.h.
 */
#ifndef LWMEM_CONFIG_USE_OS
#define LWMEM_CONFIG_USE_OS 0
#endif

/**
 * Macro to define mutex handle type.
 * 
 * @note This macro must be defined when LWMEM_CONFIG_USE_OS is enabled.
 */
#if (LWMEM_CONFIG_USE_OS != 0)
#ifndef LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE
#error LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE must de defined when LWMEM_CONFIG_USE_OS is enabled.
#endif
#endif

/**
 * Macro to enable '1' or disable '0' verification of memory region.
 * 
 * @note When enabled, lwMEM API functions will verify the pointer input is within the memory region.
 * @note When enabled, it has little impact on performance and memory usage.
 */
#ifndef LWMEM_CONFIG_USE_VERIFY_REGION
#define LWMEM_CONFIG_USE_VERIFY_REGION 0
#endif

#endif /* __LWMEM_CONFIG_H */
