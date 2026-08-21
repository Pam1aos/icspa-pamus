#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op()
{
    operand_read(&opr_src);                                    // 读操作数值
    cpu.esp -= opr_src.data_size / 8;                          // ① ESP 下移
    vaddr_write (cpu.esp, SREG_SS, opr_src.data_size / 8,       // ② 写入栈顶
                opr_src.val);
}

#include "cpu/instr.h"

make_instr_func(pusha) {
        // 1. 极其关键：提前缓存执行 pusha 之前的原始 ESP 值
        uint32_t temp_esp = cpu.esp;


        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, cpu.eax);

        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, cpu.ecx);

        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, cpu.edx);

        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, cpu.ebx);


        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, temp_esp);

        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, cpu.ebp);

        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, cpu.esi);

        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, cpu.edi);

        print_asm_0("pusha", "", 1);

        return 1; // 单字节指令
}

make_instr_func(push_i_b)
{
        int len = 1;
        opr_src.data_size = 8;
        opr_src.type = OPR_IMM;
        opr_src.sreg = SREG_CS;
        opr_src.addr = eip +1;
        len +=1;

        operand_read(&opr_src);
        int32_t val = sign_ext(opr_src.val,8);
        cpu.esp -= 4;                          // ① ESP 下移
        vaddr_write(cpu.esp, SREG_SS, 4, val);

        return  len;
}
make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, rm, v)
make_instr_impl_1op(push, i, v)
