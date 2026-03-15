#include <core_allocator.h>

struct MemoryPool memory_pool = { .mem_mutex = PTHREAD_MUTEX_INITIALIZER };

void *memblock_alloc(struct MemoryBlock *block, u64 alloc_size) {

    if (block == NULL || 
        block->size + alloc_size > MEMPOOL_BLKCAP ||
        !block->is_reserved) {
        return NULL;
    }

    void *pointer = (void *) block->data + block->size;
    block->size += alloc_size;

    return pointer;

}

struct MemoryBlock *memorypool_reserveblock() {
    
    struct MemoryBlock *block = NULL;

    pthread_mutex_lock(&(memory_pool.mem_mutex));

    for (u64 block_id = 0; block_id < MEMPOOL_BLKCNT; block_id++) {
        if (!memory_pool.mem_blocks[block_id].is_reserved) {
            memory_pool.mem_blocks[block_id].is_reserved = TRUE;
            block = &memory_pool.mem_blocks[block_id];
            break;
        }
    }

    pthread_mutex_unlock(&(memory_pool.mem_mutex));

    return block;

}

void memorypool_returnblock(struct MemoryBlock *block) {
    
    if (block == NULL)
        return;

    pthread_mutex_lock(&(memory_pool.mem_mutex));

    memblock_clear(block);
    block->is_reserved = FALSE;
    block = NULL;

    pthread_mutex_unlock(&(memory_pool.mem_mutex));

}
