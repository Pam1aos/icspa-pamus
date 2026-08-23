#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `in' instructions here.
*/
make_instr_func(in_i2a_b) {

        uint16_t port = instr_fetch(eip + 1, 1);
        uint32_t data = pio_read(port, 1);

        OPERAND al;
        al.type = OPR_REG;
        al.data_size = 8;
        al.addr = REG_EAX;
        al.val = data & 0xFF;

        operand_write(&al);

        print_asm_0("in", "", 2);
        return 2;
}

make_instr_func(in_i2a_v) {
        // 1. 获取端口号：立即数寻址直接从内存取 1 字节即可，没必要对这 1 个字节套用复杂封装
        uint16_t port = instr_fetch(eip + 1, 1);

        OPERAND eax;
        eax.type = OPR_REG;            // 操作数类型：寄存器
        eax.data_size = data_size;     // 动态位宽：16 或 32
        eax.addr = REG_EAX;            // 目标地址：EAX 的寄存器索引编号

        // 3. 收货并交给封装结构
        // pio_read 读出来的返回值，直接赋给 eax.val
        eax.val = pio_read(port, data_size / 8);

        // 4. 执行封装写入
        // 框架会根据 eax.data_size 的值，自动决定是写 AX 还是 EAX
        operand_write(&eax);

        print_asm_0("in", "", 2);
        return 2;
}

make_instr_func(in_d2a_b) {
        // 1. 封装读取源寄存器 (DX)
        OPERAND dx;
        dx.type = OPR_REG;
        dx.data_size = 16;
        dx.addr = REG_EDX;

        operand_read(&dx);
        uint16_t port = dx.val;

        // 2. 准备封装目的寄存器 (AL)
        OPERAND al;
        al.type = OPR_REG;
        al.data_size = 8;        // 定死为 8 位
        al.addr = REG_EAX;       // 对应 EAX 的低 8 位 (AL)

        // 3. 向外设收货 (长度定死为 1 字节)
        al.val = pio_read(port, 1) & 0xFF;

        // 4. 执行封装写入
        operand_write(&al);

        print_asm_0("in", "", 1);
        return 1;
}

make_instr_func(in_d2a_v) {
        // 1. 封装读取源寄存器 (DX)
        OPERAND dx;
        dx.type = OPR_REG;
        dx.data_size = 16;       // 铁律：不管传多少数据，端口寄存器死死绑定为 16 位的 DX
        dx.addr = REG_EDX;       // 传入 EDX 的基准编号，配合 data_size=16 框架会自动索引到 DX

        operand_read(&dx);       // 调用框架接口读取 DX 的值
        uint16_t port = dx.val;  // 拿到端口号

        // 2. 准备封装目的寄存器 (Accumulator)
        OPERAND eax;
        eax.type = OPR_REG;
        eax.data_size = data_size; // 动态位宽：16 或 32
        eax.addr = REG_EAX;

        // 3. 向外设收货
        eax.val = pio_read(port, data_size / 8);

        // 4. 执行封装写入
        operand_write(&eax);

        print_asm_0("in", "", 1);
        return 1;
}