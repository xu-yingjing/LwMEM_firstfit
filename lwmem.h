#ifndef __LWMEM_H
#define __LWMEM_H

#include <stdint.h>
#include <stddef.h>
#include "lwmem_config_ex.h"

/**
 * Memory block structure used in a linked list to manage the memory blocks.
 */
typedef struct lwmem_block
{
    /** Pointer to the next memory block in the linked list. */  
    struct lwmem_block *next;

    /**
     * Size in bytes of the memory block(Contains memory block structure).
     * 
     * The highest bit will be set when memory block is allocated.
     */
    size_t size;
} lwmem_block_t;

/**
 * lwMEM object structure used to abstraction a lwMEM object.
 */
typedef struct lwmem_object
{
    /** The head memory block on the linked list. */
    lwmem_block_t head;

    /** Pointer to the tail memory block on the linked list. */
    lwmem_block_t *tail;

#if (LWMEM_CONFIG_USE_OS != 0)
    /** Mutex used to protect access to lwMEM object. */
    LWMEM_CONFIG_OS_MUTEX_HANDLE_TYPE mutex;
#endif

#if (LWMEM_CONFIG_USE_VERIFY_REGION != 0)
    /** Pointer to the first memory block in the memory region all the time. */
    lwmem_block_t *start;
#endif
} lwmem_object_t;

/**
 * Defines a memory region for a lwMEM object.
 * 
 * @note This function can be called only once for each lwMEM object.
 *
 * @param object Pointer to the lwMEM object.
 * @param start Pointer to the starting address of the memory region.
 * @param size Size in bytes of the memory region.
 * 
 * @return 0 if memory region was defined successfully, otherwise 1.
 * 
 * @example
 * @note This example shows how to define a memory region for a lwMEM object by array.
 * lwmem_object_t lwmem_object;
 * uint8_t region[4096];
 * lwmem_define_region_ex(&lwmem_object, region, sizeof(region));
 * 
 * @note This example shows how to define a memory region for a lwMEM object by pointer.
 * lwmem_object_t lwmem_object;
 * lwmem_define_region_ex(&lwmem_object, (void *)0x20000000, 0x00001000);
 */
uint8_t lwmem_define_region_ex(lwmem_object_t *object, void *start, size_t size);
#define lwmem_define_region(start, size) lwmem_define_region_ex(NULL, start, size)

/**
 * Allocate memory by requested size from a lwMEM object.
 * 
 * @param object Pointer to the lwMEM object.
 * @param size Requested size in bytes to allocate.
 * 
 * @return Pointer to the allocated memory if successful, otherwise NULL.
 * 
 * @example
 * uint8_t *ptr;
 * ptr = lwmem_malloc_ex(&lwmem_object, 10);
 */
void *lwmem_malloc_ex(lwmem_object_t *object, size_t size);
#define lwmem_malloc(size) lwmem_malloc_ex(NULL, size)

/**
 * Free allocated memory by it's pointer.
 * 
 * @param object Pointer to the lwMEM object.
 * @param pointer Pointer to the allocated memory to free.
 * 
 * @return None.
 * 
 * @example
 * lwmem_free_ex(&lwmem_object, ptr);
 */
void lwmem_free_ex(lwmem_object_t *object, void *pointer);
#define lwmem_free(pointer) lwmem_free_ex(NULL, pointer)

#endif /* __LWMEM_H */
