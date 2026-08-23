#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `out' instructions here.
*/

make_instr_func(out_a2i_b) {
        // 1. 获取 1 字节端口号
        uint16_t port = instr_fetch(eip + 1, 1);

        // 2. 封装源寄存器 AL (定死为 8 位)
        OPERAND al;
        al.type = OPR_REG;
        al.data_size = 8;
        al.addr = REG_EAX;
        operand_read(&al);

        // 3. 向外设发货 (长度定死为 1 字节)
        pio_write(port, 1, al.val);

        print_asm_0("out", "", 2); // 指令长 2 字节
        return 2;
}

make_instr_func(out_a2d_b) {
        // 1. 从 DX 读取 16 位端口号
        OPERAND dx;
        dx.type = OPR_REG;
        dx.data_size = 16;
        dx.addr = REG_EDX;
        operand_read(&dx);
        uint16_t port = dx.val;

        // 2. 封装源寄存器 AL (定死为 8 位)
        OPERAND al;
        al.type = OPR_REG;
        al.data_size = 8;
        al.addr = REG_EAX;
        operand_read(&al);

        // 3. 向外设发货 (长度定死为 1 字节)
        pio_write(port, 1, al.val);

        print_asm_0("out", "", 1); // 指令长 1 字节
        return 1;
}

make_instr_func(out_a2i_v) {
        // 1. 获取端口号：立即数寻址，读取指令后的 1 个字节
        uint16_t port = instr_fetch(eip + 1, 1);

        // 2. 准备封装源寄存器 (Accumulator)
        OPERAND eax;
        eax.type = OPR_REG;
        eax.data_size = data_size;  // 动态位宽：16 或 32
        eax.addr = REG_EAX;

        // 3. 从源寄存器提取待发货的数据
        // 框架会根据 data_size 自动从 AX 或 EAX 里把数据拿出来放到 eax.val 中
        operand_read(&eax);

        // 4. 发货给外设
        // 将 eax.val 的数据写入指定端口，长度同样由 data_size 决定
        pio_write(port, data_size / 8, eax.val);

        print_asm_0("out", "", 2); // 指令长 2 字节 (1字节 opcode + 1字节端口号)
        return 2;
}

make_instr_func(out_a2d_v) {
        // 1. 封装读取目标端口号 (从 DX 获取)
        OPERAND dx;
        dx.type = OPR_REG;
        dx.data_size = 16;       // 端口寄存器永远是 16 位的 DX
        dx.addr = REG_EDX;

        operand_read(&dx);       // 读取 DX 寄存器
        uint16_t port = dx.val;  // 拿到发货的目标端口号

        // 2. 准备封装源寄存器 (Accumulator)
        OPERAND eax;
        eax.type = OPR_REG;
        eax.data_size = data_size; // 动态位宽：16 或 32
        eax.addr = REG_EAX;

        // 3. 从源寄存器提取数据
        operand_read(&eax);

        // 4. 发货给外设
        pio_write(port, data_size / 8, eax.val);

        print_asm_0("out", "", 1); // 指令长 1 字节
        return 1;
}

