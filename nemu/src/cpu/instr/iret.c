#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/
make_instr_func(iret) {
        cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;

        // 2. 弹出 CS (注意：cs 是结构体，要写 .val)
        // 弹出的虽然是 32 位数据，但 CS 寄存器只需低 16 位
        cpu.cs.val = (uint16_t)vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;

        // 3. 弹出 EFLAGS (注意：eflags 是联合体，要写 .val)
        cpu.eflags.val = vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;

        print_asm_0("iret", "", 1);

        return 0;
}