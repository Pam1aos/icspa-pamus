#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */
	return cpu.segReg[sreg].base + offset;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
    // 通过 hw_mem 数组访问模拟内存中的 GDT
    SegDesc *description = (SegDesc *)(hw_mem + cpu.gdtr.base
                                       + cpu.segReg[sreg].index * 8);

    uint32_t base = (description->base_31_24 << 24) |
                    (description->base_23_16 << 16) |
                    description->base_15_0;

    uint32_t limit = (description->limit_19_16 << 16) | description->limit_15_0;

    assert(base == 0);
    assert(limit == 0xFFFFF);
    assert(description->granularity == 1);

    cpu.segReg[sreg].base            = base;
    cpu.segReg[sreg].limit           = limit;
    cpu.segReg[sreg].type            = description->type;
    cpu.segReg[sreg].privilege_level = description->privilege_level;
    cpu.segReg[sreg].soft_use        = description->soft_use;
}

