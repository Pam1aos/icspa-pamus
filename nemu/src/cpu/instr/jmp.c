#include "cpu/instr.h"

make_instr_func(jmp_short)
{
                OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = 8;     // 【关键修改1】短跳转的偏移量固定为 8 位 (1字节)
        rel.addr = eip + 1;

        operand_read(&rel);

        // 【关键修改2】将读出来的 8 位立即数进行符号扩展
        int offset = sign_ext(rel.val, 8);

        print_asm_1("jmp", "", 2, &rel);

        cpu.eip += offset;

        // 【关键修改3】指令总长度固定为 2 (1字节opcode + 1字节立即数)
        return 2;
}



make_instr_func(jmp_near)
{
                OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = data_size; // 长跳转依赖全局 data_size (32位下是32)
        rel.addr = eip + 1;

        operand_read(&rel);
        int offset = sign_ext(rel.val, data_size);
        print_asm_1("jmp", "", 1 + data_size / 8, &rel);
        cpu.eip += offset;
        return 1 + data_size / 8;
}

make_instr_func(ljmp) {
        OPERAND imm;
        imm.type = OPR_IMM;
        imm.sreg = SREG_CS;

        // ①读 32 位绝对偏移量
        imm.data_size = 32;
        imm.addr = eip + 1;
        operand_read(&imm);
        uint32_t offset = imm.val;        // 绝对地址，不是相对偏移！

        // ②读 16 位段选择符（在 offset 之后）
        imm.data_size = 16;
        imm.addr = eip + 1 + 4;           // 跳过 4 字节 offset
        operand_read(&imm);
        uint16_t selector = imm.val;      // 如 0x0008

        // ③设置 CS 可见部分（写段寄存器 → 必须触发 load_sreg）
        cpu.segReg[SREG_CS].val = selector;

        // ④加载 CS 隐藏部分（查 GDT[1]，缓存 base/limit）
        load_sreg(SREG_CS);

        // ⑤跳转到绝对地址
        cpu.eip = offset;                 // 直接赋值，不是 +=

        print_asm_0("ljmp", "", 7);
        return 0;                         // eip 已改，返回 0 让外层不加 len
}


// [PA-2-1] 间接跳转 JMP r/m32 (0xFF/4)
// [PA-2-1] 间接跳转 JMP r/m32 (0xFF/4)
make_instr_func(jmp_rm_v) {
        OPERAND rm;
        // 【防御性编程】必须清零！防止未初始化的 sreg 等字段引发数组越界段错误
        memset(&rm, 0, sizeof(OPERAND));

        // 【规范化】使用全局的 data_size，不要写死 32
        rm.data_size = data_size;

        int len = 1;
        len += modrm_rm(eip + 1, &rm);
        operand_read(&rm);

        // 【排雷探针】如果等下依然段错误，看终端最后打印的这行：
        // 如果打印出超级大的怪异地址，说明是前面的内核指令有Bug，给了错误的跳转目标！
//         printf("JMP target is: 0x%08x\n", rm.val);

        cpu.eip = rm.val;

        // 兼容 16 位模式（非常重要）
        if (data_size == 16) {
            cpu.eip &= 0xFFFF;
        }

        print_asm_1("jmp", "", len, &rm);
        return 0; // 你原本的逻辑，绝对正确！
}