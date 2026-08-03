#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
//ESP=EBP, POP EBP →恢复旧帧




make_instr_func(leave) {
        cpu.esp = cpu.ebp;                             // ① ESP 跳回 EBP — 撤销局部变量
        cpu.ebp = vaddr_read(cpu.esp, SREG_SS, 4);     // ② 从栈顶弹出旧 EBP
        cpu.esp += 4;                                   // ③ ESP +4 (POP 的副作用)

        print_asm_0("leave", "", 1);
        return 1;
}
