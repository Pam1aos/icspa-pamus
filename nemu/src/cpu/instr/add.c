#include "cpu/instr.h"
/*
Put the implementations of `add' instructions here.
*/
static void instr_execute_2op()
{
    // 1. 读取源操作数和目的操作数
    operand_read(&opr_src);
    operand_read(&opr_dest);

    // 2. 处理 bv 后缀的符号扩展问题
    if (opr_src.data_size == 8 && opr_dest.data_size != 8) {
        opr_src.val = sign_ext(opr_src.val, 8);
    }

    // 3. 执行 ALU 加法
    opr_dest.val = alu_add(opr_src.val, opr_dest.val, opr_dest.data_size);

    // 4. 写回结果
    operand_write(&opr_dest);
}

make_instr_impl_2op(add, r, rm, b)
make_instr_impl_2op(add, r, rm, v)
make_instr_impl_2op(add, rm, r, b)
make_instr_impl_2op(add, rm, r, v)
make_instr_impl_2op(add, i, rm, b)
make_instr_impl_2op(add, i, rm, v)
make_instr_impl_2op(add, i, rm, bv)    // group_1_bv 需要 bv 后缀
make_instr_impl_2op(add, i, r, b)
make_instr_impl_2op(add, i, r, v)
make_instr_impl_2op(add, i, a, b)      // ADD AL, imm8
make_instr_impl_2op(add, i, a, v)      // ADD EAX, imm32

