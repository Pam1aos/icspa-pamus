#ifndef __CACHE_H__
#define __CACHE_H__

#include "nemu.h"

#ifdef CACHE_ENABLED

#define CACHE_BLOCK_SIZE  64       // 每块 64 字节
#define CACHE_WAY         8        // 8 路组相联
#define CACHE_SET         128      // 128 个组 (64KB / 64B / 8 = 128)
#define CACHE_SIZE        (CACHE_SET * CACHE_WAY * CACHE_BLOCK_SIZE)  // 64KB


#define CACHE_OFFSET_MASK  0x3F            // 低 6 位
#define CACHE_SET_MASK     0x7F            // 中间 7 位
#define CACHE_OFFSET(addr) ((addr) & CACHE_OFFSET_MASK)
#define CACHE_SET_IDX(addr) (((addr) >> 6) & CACHE_SET_MASK)
#define CACHE_TAG(addr)     ((addr) >> 13) // 高 19 位


typedef struct {
    bool  valid;
    uint32_t tag;
    uint8_t  data[CACHE_BLOCK_SIZE];   // 64 字节
} CacheBlock;

typedef  struct {
    CacheBlock cacheblocks[CACHE_WAY];
}CacheSet;

typedef struct {
    CacheSet cachesets[CACHE_SET];
}Cache;

// init the cache
void init_cache();

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data);

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len);

#endif

#endif
