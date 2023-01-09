#include <dlfcn.h>

#include "../include/difftest-def.h"
#include "../include/memory.h"

void dump_gpr();

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;

extern NPC_state npc_state;
extern CPU_state cpu;


void init_difftest(char *ref_so_file, long img_size, int port) {
	assert(ref_so_file != NULL);

	void *handle;
	handle = dlopen(ref_so_file, RTLD_LAZY);
	assert(handle);

	ref_difftest_memcpy = (void (*)(paddr_t, void *, size_t, bool))dlsym(handle, "difftest_memcpy");
	assert(ref_difftest_memcpy);

	ref_difftest_regcpy = (void (*)(void *, bool))dlsym(handle, "difftest_regcpy");
	assert(ref_difftest_regcpy);

	ref_difftest_exec = (void (*)(uint64_t))dlsym(handle, "difftest_exec");
	assert(ref_difftest_exec);

	void (*ref_difftest_init)(int) = (void (*)(int))dlsym(handle, "difftest_init");
	assert(ref_difftest_init);

	Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
	Log("The result of every instruction will be compared with %s. "
		"This will help you a lot for debugging, but also significantly reduce the performance. "
		"If it is not necessary, you can turn it off.", ref_so_file);

	ref_difftest_init(port);
	ref_difftest_memcpy(BASE_ADDR, guest_to_host(BASE_ADDR), img_size, DIFFTEST_TO_REF);
	ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
}
static inline int check_reg_idx(int idx) {
	assert(idx >= 0 && idx < 32);
	return idx;
}

static inline const char *reg_name(int idx, int width) {
	extern const char *regs[];
	return regs[check_reg_idx(idx)];
}

static inline bool difftest_check_reg(const char *name, vaddr_t pc, word_t ref, word_t dut) {
	if(ref != dut) {
    Log("%s is different after executing instruction at pc = " FMT_WORD
        ", right = " FMT_WORD ", wrong = " FMT_WORD ", diff = " FMT_WORD,
        name, pc, ref, dut, ref ^ dut);
    return false;
  }
  return true;
}

static bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
	bool ret = true;
	for (int i=0; i<32; i++) {
		ret = difftest_check_reg(reg_name(i, 64), pc, ref_r->gpr[i], cpu.gpr[i]);
		if(!ret) return ret;
	}
	if(ref_r->pc != cpu.pc) {
		Log("NPC Program Counter " FMT_WORD " differs from NEMU! The REF PC is " FMT_WORD, cpu.pc, ref_r->pc);
		return false;
	}
	return ret;
}
static void checkregs(CPU_state *ref, vaddr_t pc) {
	if(!isa_difftest_checkregs(ref, pc)) {
		npc_state.state = NPC_ABORT;
		npc_state.halt_pc = pc;
		dump_gpr();
	}
}

void difftest_step(vaddr_t pc) {
	CPU_state ref_r;
	ref_difftest_exec(1);
	ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);

	checkregs(&ref_r, pc);
}
