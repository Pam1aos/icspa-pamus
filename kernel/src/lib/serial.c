#include "common.h"
#include "x86.h"

#define SERIAL_PORT 0x3F8

void init_serial(void) {}

static inline int
serial_idle(void)
{
	return (in_byte(SERIAL_PORT + 5) & 0x20) != 0;
}

void serial_printc(char ch) {
    // 调用内联汇编封装好的 out 指令，把 1 字节的 ch 发送到 0x3F8 端口
    out_byte(SERIAL_PORT, ch);
}
