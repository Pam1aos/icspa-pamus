#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/mmu/cache.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>

uint8_t hw_mem[MEM_SIZE_B];

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len)
{
#ifdef CACHE_ENABLED
    return cache_read(paddr, len);
#else
    return hw_mem_read(paddr, len);
#endif

}

void paddr_write(paddr_t paddr, size_t len, uint32_t data)
{
#ifdef CACHE_ENABLED
    cache_write(paddr, len, data);
#else
    hw_mem_write(paddr, len, data);
#endif

}
uint32_t laddr_read(laddr_t laddr, size_t len) {
    if (cpu.cr0.pg == 1) {
        uint32_t page_offset = laddr & 0xFFF;
        if (page_offset + len > 0x1000) {
            size_t first_len = 0x1000 - page_offset;
            size_t second_len = len - first_len;

            uint32_t paddr1 = page_translate(laddr);
            uint32_t paddr2 = page_translate(laddr + first_len);

            uint32_t data1 = paddr_read(paddr1, first_len);
            uint32_t data2 = paddr_read(paddr2, second_len);

            return data1 | (data2 << (first_len * 8));
        } else {
            return paddr_read(page_translate(laddr), len);
        }
    } else {
        return paddr_read(laddr, len);
    }
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data) {

    if (cpu.cr0.pg == 1) {
        uint32_t page_offset = laddr & 0xFFF;
        if (page_offset + len > 0x1000) {
            size_t first_len = 0x1000 - page_offset;
            size_t second_len = len - first_len;

            uint32_t paddr1 = page_translate(laddr);
            uint32_t paddr2 = page_translate(laddr + first_len);

            paddr_write(paddr1, first_len, data);
            uint32_t data2 = data >> (first_len * 8);
            paddr_write(paddr2, second_len, data2);
        } else {
            paddr_write(page_translate(laddr), len, data);
        }
    } else {
        paddr_write(laddr, len, data);
    }
}



uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG
    if (cpu.cr0.pe == 1) {
          vaddr = segment_translate(vaddr, sreg);
      }
#endif

    return laddr_read(vaddr, len);
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG
    if (cpu.cr0.pe == 1) {
          vaddr = segment_translate(vaddr, sreg);
      }
#endif

    laddr_write(vaddr, len, data);
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);
#ifdef CACHE_ENABLED
    init_cache();
#endif

#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif



}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}
