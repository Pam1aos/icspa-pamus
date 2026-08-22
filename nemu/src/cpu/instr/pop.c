#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op()
{
    opr_src.val = vaddr_read(cpu.esp, SREG_SS, opr_src.data_size / 8);
    cpu.esp += opr_src.data_size / 8;
    operand_write(&opr_src);
}

make_instr_func(popa) {



        cpu.edi = vaddr_read(cpu.esp, SREG_SS,  4);
        cpu.esp += 4;

        cpu.esi = vaddr_read(cpu.esp, SREG_SS,  4);
        cpu.esp += 4;

        cpu.ebp = vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;

        cpu.esp += 4;

        cpu.ebx = vaddr_read(cpu.esp, SREG_SS,  4);
        cpu.esp += 4;

        cpu.edx = vaddr_read(cpu.esp, SREG_SS,  4);
        cpu.esp += 4;

        cpu.ecx = vaddr_read(cpu.esp, SREG_SS,  4);
        cpu.esp += 4;

        cpu.eax = vaddr_read(cpu.esp, SREG_SS,  4);
        cpu.esp += 4;

        print_asm_0("popa", "", 1);

        return 1; // 单字节指令
}

make_instr_impl_1op(pop, r, v)    // POP r32 (0x58~0x5F)
make_instr_impl_1op(pop, rm, v)