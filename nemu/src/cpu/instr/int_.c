#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Put the implementations of `int' instructions here.

Special note for `int': please use the instruction name `int_' instead of `int'.
*/

make_instr_func(int_) {


        OPERAND imm;
        imm.type = OPR_IMM;
        imm.sreg = SREG_CS;
        imm.data_size = 8;
        imm.addr = eip + 1; // 立即数在操作码(1字节)之后

        operand_read(&imm);
        uint8_t intr_no = imm.val; // 获取中断号，比如 0x80

        print_asm_1("int", "", 2, &imm);

        // 2. 【极其关键的一步】手动修改 EIP 指向下一条指令
        // 因为 raise_intr 压栈保护的断点 EIP 应该是当前指令之后的地址

        // 3. 触发软中断的硬件响应流程 (查表、压栈、修改 EIP 到处理程序)
        raise_sw_intr(intr_no);

        // 4. 【必须返回 0】
        // 因为 cpu.eip 已经在 raise_sw_intr 中被强行修改为内核入口地址了，
        // 返回 0 是为了告诉外层循环：不要再给 EIP 累加指令长度了！
        return 0;
}