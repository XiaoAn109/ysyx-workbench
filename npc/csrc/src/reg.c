#include "../include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};
extern CPU_state cpu;
uint64_t *cpu_gpr = NULL;
extern "C" void set_gpr_ptr(const svOpenArrayHandle r) {
  cpu_gpr = (uint64_t *)(((VerilatedDpiOpenVar*)r)->datap());
}
extern Vtop *top;
// 一个输出RTL中通用寄存器的值的示例
void dump_gpr() {
  printf("32 General Registers:\n");
  for(int i = 0; i < 32; i++) {
    printf("\033[1;32m%-3s:\033[1;35m  " FMT_WORD " | \033[0m", regs[i], cpu_gpr[i]);
    if(i%4 == 3) {
      printf("\n");
    }
  }
  printf("Program Counter:\n");
  printf("\033[1;31m%-3s:\033[1;35m  " FMT_WORD "\n\033[0m", "$pc", top->pc);
}

word_t reg_str2val(const char *s, bool *success) {
  char tmp[3] = {s[1], s[2]};
  for (int i = 0; i < 32; i++) {
    if(!strcmp(tmp, regs[i])) {
      *success = true;
      return cpu_gpr[i];
    }
  }
  if(!strcmp(tmp, "pc")) {
    *success = true;
    return top->pc;
  }
  //Error("Register not found!");
  return 0;
}

void set_regfile() {
	for (int i=0; i<32; i++) {
		cpu.gpr[i] = cpu_gpr[i];
	}
}


