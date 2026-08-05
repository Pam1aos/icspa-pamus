#include "cpu/instr.h"
/*
Put the implementations of `not' instructions here.
*/

static void instr_execute_1op()
{
    operand_read(&opr_src); // 读取

    // 取反（假设 ALU 里有对应逻辑，或者直接位运算）
    opr_src.val = ~opr_src.val;

    operand_write(&opr_src); // 写回
}

make_instr_impl_1op(not, rm, b)       // 0xF6/2
make_instr_impl_1op(not, rm, v)       // 0xF7/2
