#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <pthread.h>

#include <core_types.h>

#ifndef MEMPOOL_BLKCAP
#   define MEMPOOL_BLKCAP KiB(5)
#endif

#ifndef MEMPOOL_BLKCNT
#   define MEMPOOL_BLKCNT 5
#endif 

struct MemoryBlock {
    u8 data[MEMPOOL_BLKCAP];
    u64 size;
    boolean is_reserved;
};

struct MemoryPool {
    struct MemoryBlock mem_blocks[MEMPOOL_BLKCNT];
    pthread_mutex_t mem_mutex;     
};

extern struct MemoryPool memory_pool;

void *memblock_alloc(struct MemoryBlock *block, u64 alloc_size);
#define memblock_clear(block) (block)->size = 0

struct MemoryBlock *memorypool_reserveblock();
void memorypool_returnblock(struct MemoryBlock *block);

#endif /* __ALLOCATOR_H__ */
