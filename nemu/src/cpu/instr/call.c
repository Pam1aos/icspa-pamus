#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/

make_instr_func(call_rel32) {
        int len = 5;                              // opcode(1) + rel32(4) = 5
        int32_t rel32 = instr_fetch(eip + 1, 4);  // 读偏移量

        cpu.esp -= 4;                              // ①压栈
        vaddr_write(cpu.esp, SREG_SS, 4, eip + len); // ②存返回地址
        cpu.eip = eip + len + rel32;               // ③跳转到下一条+偏移

        print_asm_0("call", "", len);
        return 0;  // call 不返回正常长度——因为 eip 已被修改
}


make_instr_func(call_rm_v){
        OPERAND rm;

        rm.data_size = 32;
        int len = 1;                    // opcode 长度1字节
        len += modrm_rm(eip + 1, &rm);
        operand_read(&rm);

        cpu.esp -= 4;                              // ①压栈
        vaddr_write(cpu.esp, SREG_SS, 4, eip + len); // ②存返回地址
        cpu.eip = rm.val;                           // ③跳转到操作数值(绝对地址)

        print_asm_1("call", "", len, &rm);
        return 0;  // call 不返回正常长度——因为 eip 已被修改
}


