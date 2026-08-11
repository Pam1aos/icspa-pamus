#include "cpu/instr.h"
/*
Put the implementations of `and' instructions here.
*/
static void instr_execute_2op()
{
    operand_read(&opr_src);
    operand_read(&opr_dest);

    // 2. 处理 bv 后缀的符号扩展问题
    if (opr_src.data_size == 8 && opr_dest.data_size != 8) {
        opr_src.val = sign_ext(opr_src.val, 8);
    }


    // 执行与运算（假设你的 ALU 函数叫 alu_and）
    opr_dest.val = alu_and(opr_src.val, opr_dest.val, opr_dest.data_size);

    operand_write(&opr_dest); // 💣 算完一定要记得写回结果！
}

make_instr_impl_2op(and, r, rm, b)
make_instr_impl_2op(and, r, rm, v)
make_instr_impl_2op(and, rm, r, b)
make_instr_impl_2op(and, rm, r, v)
make_instr_impl_2op(and, i, rm, b)
make_instr_impl_2op(and, i, rm, v)
make_instr_impl_2op(and, i, rm, bv)
make_instr_impl_2op(and, i, a, b)      // ADD AL, imm8
make_instr_impl_2op(and, i, a, v)      // ADD EAX, imm32
