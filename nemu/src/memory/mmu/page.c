#include "cpu/cpu.h"
#include "memory/memory.h"

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
    uint32_t dir    = laddr >> 22;
    uint32_t page   = (laddr >> 12) & 0x3FF;
    uint32_t offset = laddr & 0xFFF;

    // 1. 读取页目录项 (PDE)
    uint32_t dir_base = cpu.cr3.pdbr << 12;
    uint32_t pde_addr = dir_base + (dir * 4);
    uint32_t pde = paddr_read(pde_addr, 4);


    if ((pde & 0x1) == 0) {
        printf("\n================ 致命缺页异常 ================\n");
        printf("发生故障的线性地址 (laddr): 0x%08x\n", laddr);
        printf("当前指令指针 (EIP): 0x%08x\n", cpu.eip);
        printf("当前 CR3 寄存器值: 0x%08x\n", cpu.cr3.val);
        printf("PDE 读取的物理地址: 0x%08x\n", pde_addr);
        printf("==============================================\n");
        assert(0);
    }

    // 2. 读取页表项 (PTE)
    uint32_t table_base = pde & 0xFFFFF000;
    uint32_t pte_addr = table_base + (page * 4);
    uint32_t pte = paddr_read(pte_addr, 4);
    assert((pte & 0x1) == 1); // 确保页表项存在

    // 3. 计算最终物理地址
    uint32_t paddr = (pte & 0xFFFFF000) + offset;
    return paddr;


#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
