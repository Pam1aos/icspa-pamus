#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

//void raise_intr(uint8_t intr_no)
//{
//#ifdef IA32_INTR
//	printf("Please implement raise_intr()");
//	fflush(stdout);
//
//    uint32_t idt_dsc_head = cpu.idtr.base + intr_no * 8;
//
//    cpu.cs.val =     laddr_read(idt_dsc_head,2);
//    uint32_t eip_low =laddr_read(idt_dsc_head +2,2);
//    uint32_t eip_high =laddr_read(idt_dsc_head +4,2);
//    ccpu.eip =  eip_high<< 8 |eip_high;
//
//    cpu.eflags.IF = 0
//
//#endif
//}

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
    // 1. 保护案发现场：依次将 EFLAGS, CS, EIP 压栈
    // 务必注意顺序！压入 32 位数据，每次 ESP 减 4
    cpu.esp -= 4;
    vaddr_write(cpu.esp, SREG_SS, 4, cpu.eflags.val);

    cpu.esp -= 4;
    // 往栈里写 4 字节，但 CS 其实只用到低 16 位
    vaddr_write(cpu.esp, SREG_SS, 4, cpu.cs.val);

    cpu.esp -= 4;
    vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip);

    // 2. 找到门描述符在内存中的首地址
    uint32_t idt_dsc_head = cpu.idtr.base + intr_no * 8;

    // 3. 极其精密的解析门描述符
    // +0 读取 2 字节：EIP 的低 16 位
    uint32_t eip_low = laddr_read(idt_dsc_head, 2);
    // +2 读取 2 字节：目标 CS 选择子
    cpu.cs.val = laddr_read(idt_dsc_head + 2, 2);
    // +6 读取 2 字节：EIP 的高 16 位 (跳过 +4 处的属性)
    uint32_t eip_high = laddr_read(idt_dsc_head + 6, 2);

    // 4. 拼图完成，修改指令指针跳转！
    cpu.eip = (eip_high << 16) | eip_low;

    // 5. 关中断，屏蔽外部干扰
    cpu.eflags.IF = 0;
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
