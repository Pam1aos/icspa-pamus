#include "cpu/cpu.h"
#define uint32_MAX 0xFFFFFFFF

void set_CF_ad(uint32_t res, uint32_t src, size_t data_size)
{
    uint32_t mask = uint32_MAX >> (32 - data_size);
    cpu.eflags.CF = (res & mask) < (src & mask);
}

void set_CF_adc(uint32_t dest, uint32_t src,  size_t data_size)
{
    uint32_t mask = 0xFFFFFFFFu >> (32 - data_size);

    dest &= mask;
    src  &= mask;

    uint32_t step1_res = (dest + src) & mask;

    if (step1_res < src) {
        cpu.eflags.CF = 1;
    }
    else if (step1_res == mask && cpu.eflags.CF == 1) {
        cpu.eflags.CF = 1;
    }
    else {
        cpu.eflags.CF = 0;
    }
}

void set_CF_sub(uint32_t dest, uint32_t src, size_t data_size)
{
    uint32_t mask = uint32_MAX >> (32 - data_size);
    cpu.eflags.CF = (dest & mask) < (src & mask);
}

void set_CF_sbb(uint32_t dest, uint32_t src, size_t data_size)
{
    uint32_t mask = 0xFFFFFFFFu >> (32 - data_size);

    dest &= mask;
    src  &= mask;

    if (dest < src) {
        cpu.eflags.CF = 1;
    } else if (dest == src) {
        cpu.eflags.CF = (cpu.eflags.CF == 1);
    } else {
        cpu.eflags.CF = 0;
    }
}

void set_PF(uint32_t res)
{
    uint8_t lo = res & 0xFF;
    lo = lo ^ (lo >> 4);
    lo = lo ^ (lo >> 2);
    lo = lo ^ (lo >> 1);
    cpu.eflags.PF = (~lo) & 0x1;
}

void set_ZF(uint32_t res_masked)
{
    cpu.eflags.ZF = (res_masked == 0);
}

void set_SF(uint32_t res_masked, size_t data_size)
{
    cpu.eflags.SF = (res_masked >> (data_size - 1)) & 1;
}

void set_OF_ad(uint32_t src, uint32_t dest, uint32_t res, size_t data_size)
{
    uint32_t sign_bit = 1u << (data_size - 1);
    uint32_t dest_sign = sign_bit & dest;
    uint32_t src_sign  = sign_bit & src;
    uint32_t res_sign  = sign_bit & res;
    cpu.eflags.OF = ((dest_sign == src_sign) && (dest_sign != res_sign));
}

void set_OF_sb(uint32_t src, uint32_t dest, uint32_t res, size_t data_size)
{
    uint32_t sign_bit = 1u << (data_size - 1);
    uint32_t dest_sign = sign_bit & dest;
    uint32_t src_sign  = sign_bit & src;
    uint32_t res_sign  = sign_bit & res;
    cpu.eflags.OF = ((dest_sign != src_sign) && (dest_sign != res_sign));
}

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_add(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);

    src  &= mask;
    dest &= mask;

    uint32_t res = src + dest;
    uint32_t res_masked = res & mask;

    set_CF_ad(res, src, data_size);
    set_PF(res_masked);
    set_ZF(res_masked);
    set_SF(res_masked, data_size);
    set_OF_ad(src, dest, res_masked, data_size);

    return res_masked;
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_adc(src, dest, data_size);
#else
    uint32_t mask   = uint32_MAX >> (32 - data_size);
    uint8_t  old_cf = cpu.eflags.CF;

    src  &= mask;
    dest &= mask;

    uint32_t res        = src + dest + old_cf;
    uint32_t res_masked = res & mask;

    set_CF_adc(src, dest, data_size);
    set_PF(res_masked);
    set_ZF(res_masked);
    set_SF(res_masked, data_size);
    set_OF_ad(src, dest, res_masked, data_size);

    return res_masked;
#endif
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_sub(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);
    src  &= mask;
    dest &= mask;
    uint32_t res = dest - src;

    uint32_t res_masked = mask & res;
    set_CF_sub(dest, src, data_size);
    set_PF(res_masked);
    set_ZF(res_masked);
    set_SF(res_masked, data_size);
    set_OF_sb(src, dest, res_masked, data_size);

    return res_masked;
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_sbb(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);
    src  &= mask;
    dest &= mask;
    uint32_t res = dest - src - cpu.eflags.CF;

    uint32_t res_masked = mask & res;
    set_CF_sbb(dest, src, data_size);
    set_PF(res_masked);
    set_ZF(res_masked);
    set_SF(res_masked, data_size);
    set_OF_sb(src, dest, res_masked, data_size);

    return res_masked;
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_mul(src, dest, data_size);
#else
    uint32_t mask = 0xFFFFFFFF >> (32 - data_size);

    uint64_t a = (uint64_t)(src  & mask);
    uint64_t b = (uint64_t)(dest & mask);
    uint64_t res = a * b;

    if (res >> data_size) {
        cpu.eflags.CF = 1;
        cpu.eflags.OF = 1;
    } else {
        cpu.eflags.CF = 0;
        cpu.eflags.OF = 0;
    }

    return res;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_imul(src, dest, data_size);
#else
    // 根据 data_size 符号扩展后做 64 位有符号乘法
    int64_t s = 0, d = 0;
    if (data_size == 8) {
        s = (int64_t)(int8_t)src;
        d = (int64_t)(int8_t)dest;
    } else if (data_size == 16) {
        s = (int64_t)(int16_t)src;
        d = (int64_t)(int16_t)dest;
    } else {
        s = (int64_t)src;
        d = (int64_t)dest;
    }

    int64_t res = s * d;

    // 检查高半部分是否是符号位的完整延伸（安全判断有符号乘法溢出）
    int64_t high_part = res >> data_size;
    if (data_size == 32) {
        // 32位乘法结果由 edx:eax 表示，imul 的 OF/CF 判断标准
        // 若高 32 位不全是符号位（即不等于最高位的扩展），则溢出
        int32_t low = (int32_t)res;
        int64_t sign_ext_check = (low < 0) ? -1 : 0;
        if (high_part != sign_ext_check) {
            cpu.eflags.CF = 1;
            cpu.eflags.OF = 1;
        } else {
            cpu.eflags.CF = 0;
            cpu.eflags.OF = 0;
        }
    } else {
        if (high_part != 0 && high_part != -1) {
            cpu.eflags.CF = 1;
            cpu.eflags.OF = 1;
        } else {
            cpu.eflags.CF = 0;
            cpu.eflags.OF = 0;
        }
    }

    return res;
#endif
}

uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_div(src, dest, data_size);
#else
    assert(src != 0);
    uint32_t res = dest / src;
    return res;
#endif
}

int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_idiv(src, dest, data_size);
#else
    assert(src != 0);
    int32_t res = dest / src;
    return res;
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_mod(src, dest);
#else
    assert(src != 0);
    uint32_t res = dest % src;
    return res;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_imod(src, dest);
#else
    assert(src != 0);
    int32_t res = dest % src;
    return res;
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_and(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);
    uint32_t res = (src & dest) & mask;

    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
    set_PF(res);
    set_ZF(res);
    set_SF(res, data_size);

    return res;
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_xor(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);
    uint32_t res = (src ^ dest) & mask;

    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
    set_PF(res);
    set_ZF(res);
    set_SF(res, data_size);

    return res;
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_or(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);
    uint32_t res = (src | dest) & mask;

    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
    set_PF(res);
    set_ZF(res);
    set_SF(res, data_size);

    return res;
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_shl(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);
    dest &= mask;

    if (src == 0) {
        return dest;
    }

    if (src <= data_size) {
        cpu.eflags.CF = (dest >> (data_size - src)) & 1;
    } else {
        cpu.eflags.CF = 0;
    }

    uint32_t res = (dest << src) & mask;

    set_PF(res);
    set_ZF(res);
    set_SF(res, data_size);

    return res;
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_shr(src, dest, data_size);
#else
    uint32_t mask = uint32_MAX >> (32 - data_size);
    dest &= mask;

    if (src == 0) {
        return dest;
    }

    if (src <= data_size) {
        cpu.eflags.CF = (dest >> (src - 1)) & 1;
    } else {
        cpu.eflags.CF = 0;
    }

    uint32_t res = (dest >> src) & mask;

    set_PF(res);
    set_ZF(res);
    set_SF(res, data_size);

    return res;
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_sar(src, dest, data_size);
#else
    uint32_t mask     = uint32_MAX >> (32 - data_size);
    uint32_t sign_bit = 1u << (data_size - 1);
    uint32_t sign     = dest & sign_bit;

    dest &= mask;

    if (src == 0) {
        return dest;
    }

    if (src <= data_size) {
        cpu.eflags.CF = (dest >> (src - 1)) & 1;
    } else {
        cpu.eflags.CF = (sign != 0);
    }

    uint32_t res = dest >> src;

    if (sign) {
        if (src >= data_size) {
            res = mask;
        } else {
            uint32_t top_fill = mask ^ (mask >> src);
            res |= top_fill;
        }
    }

    res &= mask;
    set_PF(res);
    set_ZF(res);
    set_SF(res, data_size);
    return res;
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
    return __ref_alu_sal(src, dest, data_size);
#else
    return alu_shl(src, dest, data_size);
#endif
}