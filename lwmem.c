#include "lwmem.h"
#include <limits.h>
#if (LWMEM_CONFIG_USE_OS != 0)
#include "lwmem_os.h"
#endif

/**
 * Macros for alignment.
 * 
 * LWMEM_BYTE_ALIGNMENT_LOWER: Align a value to the lower boundary of the alignment.
 * LWMEM_BYTE_ALIGNMENT_UPPER: Align a value to the upper boundary of the alignment.
 * LWMEM_BYTE_ALIGNMENT_LOWER_DECREASE: Get the decrement amount to align a value to the lower boundary of the alignment.
 * LWMEM_BYTE_ALIGNMENT_UPPER_INCREASE: Get the increment amount to align a value to the upper boundary of the alignment.
 */
#define LWMEM_BYTE_ALIGNMENT_LOWER(value)           (((size_t)(value)) & ~(((size_t)(LWMEM_CONFIG_BYTE_ALIGNMENT)) - 1))
#define LWMEM_BYTE_ALIGNMENT_UPPER(value)           LWMEM_BYTE_ALIGNMENT_LOWER(((size_t)(value)) + (((size_t)(LWMEM_CONFIG_BYTE_ALIGNMENT)) - 1))
#define LWMEM_BYTE_ALIGNMENT_LOWER_DECREASE(value)  (((size_t)(value)) & (((size_t)(LWMEM_CONFIG_BYTE_ALIGNMENT)) - 1))
#define LWMEM_BYTE_ALIGNMENT_UPPER_INCREASE(value)  (((((size_t)(value)) + (((size_t)(LWMEM_CONFIG_BYTE_ALIGNMENT)) - 1)) & ~(((size_t)(LWMEM_CONFIG_BYTE_ALIGNMENT)) - 1)) - ((size_t)(value)))

/**
 * The bitmask of allocated bits in size in memory block structure.
 * 
 * The highest bit in member size on memory block structure will be set when memory block is allocated.
 */
#define LWMEM_BLOCK_SIZE_ALLOCATED_BITMASK          (((size_t)1) << ((sizeof(size_t) * CHAR_BIT) - 1))

/**
 * Macro to define the minimum size of memory block.
 * 
 * The memory block size must be no less than the size of the memory block structure.
 */
#define LWMEM_BLOCK_MININUM_SIZE                    (LWMEM_BYTE_ALIGNMENT_UPPER(sizeof(lwmem_block_t)) << 1)

/** Macro to get application pointer from memory block handle. */
#define LWMEM_GET_POINTER_FROM_BLOCK(block)         ((void *)((size_t)(block) + LWMEM_BYTE_ALIGNMENT_UPPER(sizeof(lwmem_block_t))))

/** Macro to get memory block handle from application pointer */
#define LWMEM_GET_BLOCK_FROM_POINTER(pointer)       ((lwmem_block_t *)((size_t)(pointer) - LWMEM_BYTE_ALIGNMENT_UPPER(sizeof(lwmem_block_t))))

/** The default lwMEM object. */
static lwmem_object_t lwmem_default_object = {0};

static void *_lwmem_alloc(lwmem_object_t *object, size_t size);
static void _lwmem_insert_block(lwmem_object_t *object, lwmem_block_t *block);

uint8_t lwmem_define_region_ex(lwmem_object_t *object, void *start, size_t size)
{
    size_t address;
    lwmem_block_t *block;

    /** If the object is NULL, an internal default object is used. */
    if (object == NULL)
    {
        object = &lwmem_default_object;
    }

    /** Verify that the object has no region defined. */
    if (object->tail != NULL)
    {
        return 1;
    }

#if (LWMEM_CONFIG_USE_OS != 0)
    /** Verify that the mutex of object has not been created. */
    if (lwmem_os_verify_mutex(&(object->mutex)) == 0)
    {
        return 1;
    }
#endif

    /** Set the head memory block. */
    address = (size_t)start;
    address = LWMEM_BYTE_ALIGNMENT_UPPER(address);
    object->head.next = (lwmem_block_t *)address;
    object->head.size = (size_t)0;

    /** Verify the aligned address is within the legal range. */
    if (address > ((size_t)start + size))
    {
        return 1;
    }

    /** 
     * Update the size value.
     * The size must be updated because the address is aligned.
     */
    size -= address - (size_t)start;

    /** Verify that the size is large enough to hold the first free and tail memory blocks. */
    if (LWMEM_BYTE_ALIGNMENT_LOWER(size) < (2 * LWMEM_BYTE_ALIGNMENT_UPPER(sizeof(lwmem_block_t))))
    {
        return 1;
    }

    /** Set the tail memory block. */
    address += size;
    address -= LWMEM_BYTE_ALIGNMENT_UPPER(sizeof(lwmem_block_t));
    address = LWMEM_BYTE_ALIGNMENT_LOWER(address);
    object->tail = (lwmem_block_t *)address;
    object->tail->next = NULL;
    object->tail->size = (size_t)0;

    /** Set the first free memory block. */
    block = object->head.next;
    block->next = object->tail;
    block->size = (size_t)(object->tail) - (size_t)block;

    /** Verify that the bitmask in member size is not used. */
    if ((block->size & LWMEM_BLOCK_SIZE_ALLOCATED_BITMASK) != 0)
    {
        object->tail = NULL;
        return 1;
    }

#if (LWMEM_CONFIG_USE_OS != 0)
    /** Create a mutex for the object. */
    if (lwmem_os_create_mutex(&(object->mutex)) != 0)
    {
        object->tail = NULL;
        return 1;
    }
#endif

#if (LWMEM_CONFIG_USE_VERIFY_REGION != 0)
    /** Set the first memory block. */
    object->start = block;
#endif

    return 0;
}

void *lwmem_malloc_ex(lwmem_object_t *object, size_t size)
{
    void *pointer;

    /** If the object is NULL, an internal default object is used. */
    if (object == NULL)
    {
        object = &lwmem_default_object;
    }

    /** Verify that the object has a region defined. */
    if (object->tail == NULL)
    {
        return NULL;
    }

#if (LWMEM_CONFIG_USE_OS != 0)
    /**
     * Take the mutex of the object.
     * 
     * To protect the object from concurrent access.
     */
    if (lwmem_os_take_mutex(&(object->mutex)) != 0)
    {
        return NULL;
    }
#endif

    pointer = _lwmem_alloc(object, size);

#if (LWMEM_CONFIG_USE_OS != 0)
    /**
     * Give the mutex of the object.
     * 
     * To protect the object from concurrent access.
     */
    lwmem_os_give_mutex(&(object->mutex));
#endif

    return pointer;
}

void lwmem_free_ex(lwmem_object_t *object, void *pointer)
{
    lwmem_block_t *block;

    /** Verify that the pointer is not NULL. */
    if (pointer == NULL)
    {
        return;
    }

    /** If the object is NULL, an internal default object is used. */
    if (object == NULL)
    {
        object = &lwmem_default_object;
    }

    /** Verify that the object has a region defined. */
    if (object->tail == NULL)
    {
        return;
    }

#if (LWMEM_CONFIG_USE_OS != 0)
    /**
     * Take the mutex of the object.
     * 
     * To protect the object from concurrent access.
     */
    if (lwmem_os_take_mutex(&(object->mutex)) != 0)
    {
        return;
    }
#endif

    /** Get the memory block from the application pointer. */
    block = LWMEM_GET_BLOCK_FROM_POINTER(pointer);

#if (LWMEM_CONFIG_USE_VERIFY_REGION != 0)
    /** Verify that the block is within the legal region. */
    if (((size_t)block >= (size_t)(object->start)) && ((size_t)block < (size_t)(object->tail)))
#endif
    {
        /** Verify that the memory block is allocated. */
        if ((block->size & LWMEM_BLOCK_SIZE_ALLOCATED_BITMASK) != 0)
        {
            /* Mark the memory block as unallocated. */
            block->size &= ~LWMEM_BLOCK_SIZE_ALLOCATED_BITMASK;

            /** Insert the memory block to be freed into the linked list. */
            _lwmem_insert_block(object, block);
        }
    }

#if (LWMEM_CONFIG_USE_OS != 0)
    /**
     * Give the mutex of the object.
     * 
     * To protect the object from concurrent access.
     */
    lwmem_os_give_mutex(&(object->mutex));
#endif
}

/**
 * Allocate memory by requested size from a lwMEM object for internal use.
 * 
 * @note This function is used internally by the lwMEM library.
 * 
 * @param object Pointer to the lwMEM object.
 * @param size Requested size in bytes to allocate.
 * 
 * @return Pointer to the allocated memory if successful, otherwise NULL.
 */
static void *_lwmem_alloc(lwmem_object_t *object, size_t size)
{
    size_t size_increase;
    lwmem_block_t *block_previous;
    lwmem_block_t *block_current;
    lwmem_block_t *block_new;
    void *pointer_return;

    /** Verify that the size is not zero. */
    if (size == 0)
    {
        return NULL;
    }

    /** 
     * Update the size value.
     * 
     * The size must be increased to contain a memory block structure
     * and to align the size to the upper boundary of the alignment value,
     * then verify that the increased size is not larger than the maximum value of a size_t type.
     */
    size_increase = LWMEM_BYTE_ALIGNMENT_UPPER(sizeof(lwmem_block_t)) + LWMEM_BYTE_ALIGNMENT_UPPER_INCREASE(size);
    if (((~((size_t)0)) - size) >= size_increase)
    {
        size += size_increase;
    }
    else
    {
        return NULL;
    }

    /** Verify that the memory block is not allocated. */
    if ((size & LWMEM_BLOCK_SIZE_ALLOCATED_BITMASK) != 0)
    {
        return NULL;
    }

    /** Find a free memory block that is large enough to hold the requested size. */
    block_previous = &object->head;
    block_current = block_previous->next;
    while ((block_current->size < size) && (block_current != object->tail))
    {
        block_previous = block_current;
        block_current = block_current->next;
    }
    if (block_current == object->tail)
    {
        return NULL;
    }

    /** Application pointer of memory block will be returned. */
    pointer_return = LWMEM_GET_POINTER_FROM_BLOCK(block_current);
    
    /** Remove the eligible memory block from the linked list. */
    block_previous->next = block_current->next;

    /**
     * If the size of the memory block exceeds the required size by too much,
     * then split the memory block into two memory blocks.
     */
    if ((block_current->size - size) >= LWMEM_BLOCK_MININUM_SIZE)
    {
        /** Create a new memory block from excess memory space. */
        block_new = (lwmem_block_t *)(((size_t)block_current) + size);
        block_new->size = block_current->size - size;
        block_current->size = size;

        /** Insert the new memory block into the linked list. */
        _lwmem_insert_block(object, block_new);
    }

    /* Mark the memory block as allocated. */
    block_current->size |= LWMEM_BLOCK_SIZE_ALLOCATED_BITMASK;

    return pointer_return;
}

/**
 * Insert a memory block into the linked list.
 * 
 * @note This function is used internally by the lwMEM library.
 * 
 * @param object Pointer to the lwMEM object.
 * @param block Pointer to the memory block to be inserted.
 * 
 * @return None.
 */
static void _lwmem_insert_block(lwmem_object_t *object, lwmem_block_t *block)
{
    lwmem_block_t *block_index;

    /** Find the last memory block with a lower address than the memory block to be inserted. */
    block_index = &(object->head);
    while ((size_t)(block_index->next) < (size_t)block)
    {
        block_index = block_index->next;
    }

    /**
     * If the memory block to be inserted is adjacent to the low-address memory block,
     * merge them.
     */
    if ((size_t)block_index + block_index->size == (size_t)block)
    {
        block_index->size += block->size;
        block = block_index;
    }

    /**
     * If the memory block to be inserted is adjacent to the high-address memory block,
     * merge them.
     */
    if ((size_t)block + block->size == (size_t)(block_index->next))
    {
        if (block_index->next == object->tail)
        {
            block->next = object->tail;
        }
        else
        {
            block->size += block_index->next->size;
            block->next = block_index->next->next;
        }
    }
    else
    {
        /**
         * If the memory block to be inserted is not adjacent to the higher address memory block,
         * insert the memory block to be inserted before the high-address memory block.
         */
        block->next = block_index->next;
    }

    /**
     * If the memory block to be inserted is not adjacent to the lower address memory block,
     * insert the memory block to be inserted after the low-address memory block.
     */
    if (block_index != block)
    {
        block_index->next = block;
    }
}
