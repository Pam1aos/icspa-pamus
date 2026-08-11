#include "memory/mmu/cache.h"
#include "memory/memory.h"

#include <string.h>
#include <stdlib.h>

// 全局 Cache 变量，不是局部指针
static Cache cache;

void init_cache()
{
    for (int i = 0; i < CACHE_SET; i++) {
        for (int j = 0; j < CACHE_WAY; j++) {
            cache.cachesets[i].cacheblocks[j].valid = false;
            cache.cachesets[i].cacheblocks[j].tag   = 0;
        }
    }
}

uint32_t cache_read(paddr_t paddr, size_t len)
{
    uint32_t cache_offset  = CACHE_OFFSET(paddr);
    uint32_t cache_set_idx = CACHE_SET_IDX(paddr);
    uint32_t cache_tag     = CACHE_TAG(paddr);
    CacheSet *set = &cache.cachesets[cache_set_idx];

    // 跨 Block：分两次读
    if (cache_offset + len > CACHE_BLOCK_SIZE) {
        uint32_t first_len = CACHE_BLOCK_SIZE - cache_offset;
        uint32_t lo = cache_read(paddr, first_len);
        uint32_t hi = cache_read(paddr + first_len, len - first_len);
        return lo | (hi << (first_len * 8));
    }

    // 8 路并行比较
    int way = -1;
    for (int i = 0; i < CACHE_WAY; i++) {
        if (set->cacheblocks[i].valid && set->cacheblocks[i].tag == cache_tag) {
            way = i;
            break;
        }
    }

    if (way >= 0) {
        // 命中
        uint32_t res = 0;
        memcpy(&res, set->cacheblocks[way].data + cache_offset, len);
        return res;
    }

    // 缺失：随机替换
    int victim = rand() % CACHE_WAY;
    CacheBlock *blk = &set->cacheblocks[victim];

    uint32_t block_addr = paddr & ~(uint32_t)(CACHE_BLOCK_SIZE - 1);
    memcpy(blk->data, hw_mem + block_addr, CACHE_BLOCK_SIZE);

    blk->valid = true;
    blk->tag   = cache_tag;

    uint32_t res = 0;
    memcpy(&res, blk->data + cache_offset, len);
    return res;
}

void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
    // write-through: 直接写内存
    hw_mem_write(paddr, len, data);

    uint32_t cache_offset  = CACHE_OFFSET(paddr);
    uint32_t cache_set_idx = CACHE_SET_IDX(paddr);
    uint32_t cache_tag     = CACHE_TAG(paddr);
    CacheSet *set = &cache.cachesets[cache_set_idx];

    // 跨 Block
    if (cache_offset + len > CACHE_BLOCK_SIZE) {
        uint32_t first_len = CACHE_BLOCK_SIZE - cache_offset;
        cache_write(paddr, first_len, data);
        cache_write(paddr + first_len, len - first_len, data >> (first_len * 8));
        return;
    }

    hw_mem_write(paddr, len, data);
    // 命中则更新 Cache
    for (int i = 0; i < CACHE_WAY; i++) {
        if (set->cacheblocks[i].valid && set->cacheblocks[i].tag == cache_tag) {
            memcpy(set->cacheblocks[i].data + cache_offset, &data, len);
            return;
        }
    }
    // no-write-allocate: 缺失不加载
}
