#include "cpu/instr.h"
/*
Put the implementations of `adc' instructions here.
*/
static void instr_execute_2op()
{
    operand_read(&opr_src);
    operand_read(&opr_dest);

    // 2. 处理 bv 后缀的符号扩展问题
    if (opr_src.data_size == 8 && opr_dest.data_size != 8) {
        opr_src.val = sign_ext(opr_src.val, 8);
    }

    opr_dest.val = alu_adc(opr_src.val, opr_dest.val, opr_dest.data_size);
    operand_write(&opr_dest);
}


make_instr_impl_2op(adc, r, rm, b)
make_instr_impl_2op(adc, r, rm, v)
make_instr_impl_2op(adc, rm, r, b)
make_instr_impl_2op(adc, rm, r, v)
make_instr_impl_2op(adc, i, rm, b)
make_instr_impl_2op(adc, i, rm, v)
make_instr_impl_2op(adc, i, rm, bv)    // group_1_bv 需要 bv 后缀
make_instr_impl_2op(adc, i, r, b)
make_instr_impl_2op(adc, i, r, v)
make_instr_impl_2op(adc, i, a, b)      // ADD AL, imm8
make_instr_impl_2op(adc, i, a, v)      // ADD EAX, imm32