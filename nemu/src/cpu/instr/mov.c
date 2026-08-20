#include "cpu/instr.h"



make_instr_func(mov_rm2sreg) {
        OPERAND rm;
        uint8_t sreg_no;
        rm.data_size = 16;
        int len = 1;
        len += modrm_opcode_rm(eip + 1, &sreg_no, &rm);
        //     ↑ sreg_no 自动取 reg 字段的值（0~5），
        //       是 ES 还是 DS 由 ModR/M 决定，不需要分支

        operand_read(&rm);
        cpu.segReg[sreg_no].val = rm.val;
        load_sreg(sreg_no);

        return len;
}

make_instr_func(mov_cr2r) {
        OPERAND cr, r;
        uint8_t cr_no;
        r.data_size = 32;

        int len = 1;
        // reg 字段 = 控制寄存器编号，r/m = 通用寄存器
        len += modrm_opcode_rm(eip + 1, &cr_no, &r);
        //         ↑ 返回 reg 字段（CR 编号 0/2/3）

        cr.type      = OPR_CREG;
        cr.addr      = cr_no;
        cr.data_size = 32;

        operand_read(&cr);              // 读控制寄存器
        r.val = cr.val;
        operand_write(&r);              // 写通用寄存器

        return len;
}

make_instr_func(mov_r2cr) {
        OPERAND cr, r;
        uint8_t cr_no;
        r.data_size = 32;

        int len = 1;
        len += modrm_opcode_rm(eip + 1, &cr_no, &r);

        cr.type      = OPR_CREG;
        cr.addr      = cr_no;
        cr.data_size = 32;

        operand_read(&r);               // 读通用寄存器
        cr.val = r.val;
        operand_write(&cr);             // 写控制寄存器

        return len;
}



static void instr_execute_2op() 
{
    operand_read(&opr_src);
    operand_read(&opr_dest);

    // 2. 处理 bv 后缀的符号扩展问题
    if (opr_src.data_size == 8 && opr_dest.data_size != 8) {
        opr_src.val = sign_ext(opr_src.val, 8);
    }

	opr_dest.val = opr_src.val;
	operand_write(&opr_dest);
}

make_instr_impl_2op(mov, r, rm, b)
make_instr_impl_2op(mov, r, rm, v)
make_instr_impl_2op(mov, rm, r, b)
make_instr_impl_2op(mov, rm, r, v)
make_instr_impl_2op(mov, i, rm, b)
make_instr_impl_2op(mov, i, rm, v)
make_instr_impl_2op(mov, i, r, b)
make_instr_impl_2op(mov, i, r, v)
make_instr_impl_2op(mov, a, o, b)
make_instr_impl_2op(mov, a, o, v)
make_instr_impl_2op(mov, o, a, b)
make_instr_impl_2op(mov, o, a, v)

make_instr_func(mov_zrm82r_v) {
	int len = 1;
	OPERAND r, rm;
	r.data_size = data_size;
	rm.data_size = 8;
	len += modrm_r_rm(eip + 1, &r, &rm);
	
	operand_read(&rm);
	r.val = rm.val;
	operand_write(&r);

	print_asm_2("mov", "", len, &rm, &r);
	return len;
}

make_instr_func(mov_zrm162r_l) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);

        operand_read(&rm);
        r.val = rm.val;
        operand_write(&r);
	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm82r_v) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = data_size;
        rm.data_size = 8;
        len += modrm_r_rm(eip + 1, &r, &rm);
        
	operand_read(&rm);
        r.val = sign_ext(rm.val, 8);
        operand_write(&r);
	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm162r_l) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);
        operand_read(&rm);
        r.val = sign_ext(rm.val, 16);
        operand_write(&r);

	print_asm_2("mov", "", len, &rm, &r);
        return len;
}
