#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret) {
        cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;

        print_asm_0("ret", "", 1);
        return 0;
}

// [PA-2-1] RET imm16 (0xC2): 返回后额外清理 imm16 字节栈空间
make_instr_func(ret_imm16) {
        cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;
        uint16_t imm = instr_fetch(eip + 1, 2);
        cpu.esp += imm;

        print_asm_0("ret", "", 3);
        return 0;
}
