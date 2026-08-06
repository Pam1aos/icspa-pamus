# PA 2-1 指令实现修改汇总

## 一、ALU 运算 (`nemu/src/cpu/alu.c`)

### 修复：CF 计算避免未定义行为
| 函数 | 之前 | 之后 |
|------|------|------|
| `set_CF_ad` | `(res >> data_size) & 1` | `(res & mask) < (src & mask)` |
| `set_CF_ad` 签名 | `(uint32_t res, size_t data_size)` | `(uint32_t res, uint32_t src, size_t data_size)` |

**原因**: `res >> 32` 在 C 中是未定义行为，当 `data_size=32` 时编译器的优化会导致 CF 结果不确定。

### 新增：减法专用借位标志
| 函数 | 实现 |
|------|------|
| `set_CF_sb` | `cpu.eflags.CF = (dest & mask) < (src & mask)` |
| `set_CF_sbb` | SBB 借位判断（dest < src 或 dest == src 且已有借位） |

### 新增：所有标志位辅助函数
```c
void set_PF(uint32_t res);      // XOR折叠法计算奇偶标志
void set_ZF(uint32_t res_masked); // 零标志
void set_SF(uint32_t res_masked, size_t data_size); // 符号标志
void set_OF_ad(...);              // 加法溢出标志
void set_OF_sb(...);              // 减法溢出标志
```

### 修复：加减乘除操作数未截断
所有 `alu_add/adc/sub/sbb/mul/div` 函数增加 `src &= mask; dest &= mask;`

### 实现：逻辑运算标志位
`alu_and/or/xor`: CF=0, OF=0, PF/ZF/SF 正常设置

---

## 二、FPU (`nemu/src/cpu/fpu.c`)

### 修复：`FLOAT f` 作用域
- `FLOAT f` 声明从函数中部移到函数开头（第15-18行）
- 之前 `exp>=0xff` 和 `exp<0` 分支使用了未声明的 `f`

### 修复：`return f.val` 返回过期值
- `f` 在函数开头初始化后未更新 → 返回值永远是归一化前的旧值
- 修复：在 `return f.val` 前重新 `f.sign/exponent/fraction = ...`

### 修复：`exp==0` denormal 多余 `exp++`
- 循环内已经 `exp++` 到 0，denormal 分支又 `exp++` 到 1（规格化）→ 错误
- 删掉 denormal 分支里的 `exp++`

### 修复：`internal_float_mul` 指数公式
- 之前: `exp_res = 0`, 只在循环里递增
- 修复: `exp_res = fa.exponent + fb.exponent - 127`

### 修复：`internal_float_mul` 有效数缩放
- 之前: `sig_res <<= 3`（左移 3 位）
- 修复: 先右移对齐到 ~27 位（保留 sticky），再 `<<= 3` 留 GRS 空间

### 修复：舍入溢出
- `+0x8` 进位可能触发 `sig_grs >> 27` 设置（有效数 ≥ 2.0）
- 加判断：若溢出则 `sig_grs >>= 1; exp++`

### 实现：左规/右规循环、GRS 舍入（所有 TODO 区域）

---

## 三、opcode 表 (`nemu/src/cpu/decode/opcode.c`)

### 修复：ADD/XOR 位置互换
| 位置 | 之前 | 之后 |
|------|------|------|
| `0x00~05` | 全 `inv` | `add_r2rm_b/v, add_rm2r_b/v, add_i2a_b/v` |
| `0x30~35` | `add_*`（错误位置） | `xor_r2rm_b/v, xor_rm2r_b/v, xor_i2a_b/v` |

### 修复：中文占位符 → 真实函数名
| 位置 | 之前 | 之后 |
|------|------|------|
| `0x68` | `手写PUSH Iv` | `push_i_v` |
| `0x6A` | `手写PUSH Ib` | `push_i_b` |
| `group_3_b/v[4]` | `手写 MUL` | `mul_rm2a_b` / `mul_rm2a_v` |
| `group_3_b/v[5]` | `手写 IMUL` | `imul_rm2a_b` / `imul_rm2a_v` |
| `group_3_b/v[6]` | `手写 DIV` | `div_rm2a_b` / `div_rm2a_v` |
| `group_3_b/v[7]` | `手写 IDIV` | `idiv_rm2a_b` / `idiv_rm2a_v` |

### 补全：已实现但未接入 opcode 表的函数
| 新填入的 opcode | 函数 | 说明 |
|-----------------|------|------|
| `0x48~4F` | `dec_r_v` ×8 | DEC r32（单字节） |
| `0x70~7F` | JCC short 全 16 条 | 条件跳转 short |
| `0x8D` | `lea_rm2r_v` | LEA 取有效地址 |
| `0xA6/A7` | `cmps_b` / `cmps_v` | 串比较 |
| `0xAA/AB` | `stos_b` / `stos_v` | 串存储 |
| `0xC2` | `ret_imm16` | RET imm16 |
| `0xC3` | `ret` | RET |
| `0xE3` | `jecxz_short_` | JECXZ |
| `0xE8` | `call_rel32` | CALL rel32 |
| `0xE9` | `jmp_near` | JMP rel32 |
| `0xEB` | `jmp_short` | JMP rel8 |
| `0x69` | `imul_irm2r_v` | IMUL r32, r/m32, imm32 |
| `0x6B` | `imul_i8rm2r_v` | IMUL r32, r/m32, imm8 |
| `group_5[2]` | `call_rm_v` | CALL r/m32（间接调用） |
| `group_5[4]` | `jmp_rm_v` | JMP r/m32（间接跳转） |

### 补全：两字节 opcode 表 (`0F xx`)
| 区域 | 填写 | 说明 |
|------|------|------|
| `0F 40~4F` | CMOVcc 全 16 条 | 之前只接了 6 条 |
| `0F 80~8F` | JCC near 全 16 条 | 之前全 `inv` |
| `0F 90~9F` | SETcc 全 16 条 | 之前全 `inv` |
| `0F AF` | `imul_rm2r_v` | IMUL r32, r/m32 |

### 函数名对齐（NEMU 命名 = IA-32 别名统一）
| 之前 | 之后 | 原因 |
|------|------|------|
| `jbe_short_` | `jna_short_` | JBE = JNA（Not Above），NEMU 用 jna |
| `jbe_near` | `jna_near` | 同上 |

---

## 四、条件码宏 (`nemu/include/cpu/instr_helper.h`)

### 实现：全部 17 个 `condition_*` 宏

| 宏 | 判断 | 说明 |
|----|------|------|
| `condition_e` | `ZF == 1` | Equal |
| `condition_ne` | `ZF == 0` | Not Equal |
| `condition_b` | `CF == 1` | Below（无符号 <） |
| `condition_ae` | `CF == 0` | Above or Equal（无符号 ≥） |
| `condition_a` | `CF==0 && ZF==0` | Above（无符号 >） |
| `condition_be` | `CF==1 \|\| ZF==1` | Below or Equal（无符号 ≤） |
| `condition_l` | `SF != OF` | Less（有符号 <） |
| `condition_ge` | `SF == OF` | Greater or Equal（有符号 ≥） |
| `condition_g` | `ZF==0 && SF==OF` | Greater（有符号 >） |
| `condition_le` | `ZF==1 \|\| SF!=OF` | Less or Equal（有符号 ≤） |
| `condition_o` / `no` | `OF == 1` / `OF == 0` | Overflow |
| `condition_s` / `ns` | `SF == 1` / `SF == 0` | Sign |
| `condition_p` / `np` | `PF == 1` / `PF == 0` | Parity |
| `condition_c` | `CF == 1` | Carry |
| `condition_ecxz` | `ECX == 0` | ECX Zero |

### 修复：`condition_ne` 标志位错误
- 之前: `SF == 0`（NE = Not Equal 应看 ZF）
- 之后: `ZF == 0`

---

## 五、指令实现文件修复

### `inc.c` / `dec.c`：单操作数宏修复
| 修改 | 之前 | 之后 |
|------|------|------|
| 宏类型 | `make_instr_impl_2op` | `make_instr_impl_1op` |
| 操作数类型 | `r, r, v` / `rm, rm, b` | `r, v` / `rm, b` / `rm, v` |
| 执行函数 | `instr_execute_2op` | `instr_execute_1op` |
| CF 处理 | 无 | 保存 + 恢复旧 CF |

### `not.c`：变量名修复
| 修改 | 之前 | 之后 |
|------|------|------|
| 执行函数名 | `instr_execute_2op` | `instr_execute_1op` |
| 写回变量 | `opr_dest.val = ~opr_dest.val` | `opr_src.val = ~opr_src.val` |

### `neg.c`：函数名修复
| 修改 | 之前 | 之后 |
|------|------|------|
| 执行函数名 | `instr_execute_2op` | `instr_execute_1op` |

### `cmp.c`：补全缺少的操作数组合
| 修改 | 之前 | 之后 |
|------|------|------|
| group_1_bv 条目 | `make_instr_impl_2op(cmp, rm, r, bv)` | `make_instr_impl_2op(cmp, i, rm, bv)` |

### `call.c`：四重修复
| 问题 | 修复 |
|------|------|
| `print_asm_1("call", "", len, ...)` | → `print_asm_0("call", "", len)` |
| `modrm_rm(eip + 1, )` | → `modrm_rm(eip + 1, &rm)` |
| `cpu.eip += rel32` | → `cpu.eip = eip + len + rel32`（rel32 相对下一条） |
| `cpu.eip += rm.val` | → `cpu.eip = rm.val`（间接调用是绝对地址） |

### `pop.c`：三重修复
| 问题 | 修复 |
|------|------|
| `vaddr_read` 多传了第 4 个参数 | 删掉多余的 `opr_src.val` |
| `opr_dest.val` | → `opr_src.val`（单操作数） |

### `jmp.c`：新增间接跳转
```c
make_instr_func(jmp_rm_v) {
    OPERAND rm;
    rm.data_size = data_size;
    int len = 1;
    len += modrm_rm(eip + 1, &rm);
    operand_read(&rm);
    cpu.eip = rm.val;
    print_asm_1("jmp", "", len, &rm);
    return 0;
}
```

### `ret.c`：新增 RET imm16
```c
make_instr_func(ret_imm16) {
    cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
    cpu.esp += 4;
    uint16_t imm = instr_fetch(eip + 1, 2);
    cpu.esp += imm;
    print_asm_0("ret", "", 3);
    return 0;
}
```

### `lea.c`：跳过读内存，直接存地址
```c
r.val = rm.addr;  // 不调 operand_read！直接用地址值本身
```

---

## 六、头文件修复（缺分号）

| 文件 | 修复行 |
|------|--------|
| `call.h` | `call_rel32`/`call_rm_v` 末尾加 `;` |
| `neg.h` | `neg_rm_b`/`neg_rm_v` 末尾加 `;` |
| `pop.h` | `pop_r_v`/`pop_rm_v` 末尾加 `;` |
| `push.h` | 全部声明加 `;`，删除重复的 `push_r_v` |
| `not.h` | `not_rm_b`/`not_rm_v` 末尾加 `;` |
| `ret.h` | `ret` 末尾加 `;`，新增 `ret_imm16` 声明 |
| `jmp.h` | 新增 `jmp_rm_v` 声明 |
| `imul.h` | 全部声明加 `;` |

---

## 七、ELF 装载器 (`kernel/src/elf/elf.c`)

| 修改 | 之前 | 之后 |
|------|------|------|
| `panic("Please implement")` | 直接崩溃 | 实现 `memcpy` + `memset` |
| 段拷贝 | 无 | `memcpy((void*)p_vaddr, (void*)elf+p_offset, p_filesz)` |
| BSS 清零 | 无 | `memset((void*)(p_vaddr+p_filesz), 0, p_memsz-p_filesz)` |

---

## 八、移除干扰文件

| 文件 | 操作 | 原因 |
|------|------|------|
| `src/cpu/alu_cp.c` | 删除 | 与 `alu.c` 重复定义 |
| `src/cpu/cpu_cp.c` | 删除 | 与 `cpu.c` 重复定义 |

---

## 九、已知待修复

| 问题 | 状态 |
|------|------|
| struct/string/hello-str 测试失败 | EAX 被错误符号扩展 (0x80→0xffffff80)，怀疑 MOVZX 或 sign_ext 相关 |
| test-float 测试失败 | FPU 浮点运算待确认（预期会失败） |
| 部分未实现指令 | cli/sti/int_/in/out/iret/lgdt/lidt 不影响当前测试 |

## 十、当前测试通过率

```
PASS: add, mov, mov-cmp, mov-c, mov-jcc, bit, lea,
      fib, fact, bubble-sort, quick-sort, select-sort,
      max, min3, sum, wanshu, prime, gotbaha, leap-year,
      matrix-mul, mul-longlong, sub-longlong, add-longlong,
      movsx, pascal (约 30+ 个)

FAIL: struct, string, hello-str, test-float
```
