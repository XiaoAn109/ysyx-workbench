#include "../include/basic_func.h"

#define MAX_PRINT_NUM 10

VerilatedContext *contextp = NULL;
VerilatedVcdC *tfp = NULL;
Vtop *top = NULL;
extern uint8_t *inst_mem;
extern NPC_state npc_state;
extern CPU_state cpu;

FILE *log_fp = NULL;
static char *log_file = NULL;
static char *elf_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;
static char logbuf[128];

static bool itrace_on = false;

void sdb_set_batch_mode();
void init_isa();
void init_difftest(char *ref_so_file, long img_size, int port);
void difftest_step(vaddr_t pc);
void init_disasm(const char *triple);
void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
void set_regfile();

static uint32_t rtl_inst;
extern "C" void rtl_get_inst(int inst) {
	rtl_inst = inst;
}

void step_and_dump_wave() {
	top->eval();
	contextp->timeInc(1);
	tfp->dump(contextp->time());
}
void single_cycle() {
	top->clk = !top->clk;
	step_and_dump_wave();
	top->clk = !top->clk;
	step_and_dump_wave();
}

void resetn(int n) {
	top->rst = 1;
	for(int i = 0; i < n; i ++) {
		single_cycle();
		Log("reset for 1 cycle!");
	}
	Log("reset released!");
	top->rst = 0;
}

int sim_exit() {
	step_and_dump_wave();
	tfp->close();
	delete top;
	delete contextp;
	free(inst_mem);
	int good = (npc_state.state == NPC_END && npc_state.halt_ret == 0) ||
		(npc_state.state == NPC_QUIT);
	return !good;
}
static long load_img() {
	if(img_file == NULL) {
		Log("No image is given. Use the default build-in image.");
		return 4096; //built-in image size
	}

	FILE *fp = fopen(img_file, "rb");
	Assert(fp, "Can not open '%s'", img_file);

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);

	Log("The image is %s, size = %ld", img_file, size);
	
	fseek(fp, 0, SEEK_SET);
	int ret = fread(guest_to_host(BASE_ADDR), size, 1, fp);
	assert(ret == 1);

	fclose(fp);
	return size;
}

static void init_log(const char *log_file) {
	log_fp = stdout;
	if(log_file != NULL) {
		FILE *fp = fopen(log_file, "w");
		Assert(fp, "Can not open '%s'", log_file);
		log_fp = fp;
	}
	Log("Log is written to %s", log_file ? log_file : "stdout");
}

static void init_rand() {
	srand(time(0));
}

static void init_sdb() {
	init_regex();
	init_wp_pool();
}

static int parse_args(int argc, char *argv[]) {
	const struct option table[] = {
		{"batch", no_argument, NULL, 'b'},
		{"itrace", no_argument, NULL, 'i'},
		{"log", required_argument, NULL, 'l'},
		{"diff", required_argument, NULL, 'd'},
		{"port", required_argument, NULL, 'p'},
		{"elf", required_argument, NULL, 'e'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, NULL, 0},
	};
	int o;
	while ((o = getopt_long(argc, argv, "-bhl:e:d:p:", table, NULL)) != -1) {
		switch (o) {
			case 'b':
				sdb_set_batch_mode();
				break;
			case 'i':
				itrace_on = true;
				break;
			case 'p':
				sscanf(optarg, "%d", &difftest_port);
				break;
			case 'l':
				log_file = optarg;
				break;
			case 'd':
				diff_so_file = optarg;
				break;
			case 'e':
				elf_file = optarg;
				break;
			case 1:
				img_file = optarg;
				return 0;
			default:
				printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
				printf("\t-b,--batch              run with batch mode\n");
				printf("\t-i,--itrace             itrace on\n");
				printf("\t-l,--log=FILE           output log to FILE\n");
				printf("\t-e,--elf=FILE           read an elf FILE\n");
				printf(
						"\t-d,--diff=REF_SO        run DiffTest with reference "
						"REF_SO\n");
				printf(
						"\t-p,--port=PORT          run DiffTest with port PORT\n");
				printf("\n");
				exit(0);
		}
	}
	return 0;
}
		
void sim_init(int argc, char *argv[]) {
	contextp = new VerilatedContext;
	tfp = new VerilatedVcdC;
	top = new Vtop{contextp};
	contextp->traceEverOn(true);
	top->trace(tfp, 0);
	tfp->open("dump.vcd");
	
	top->clk = 1;
	top->rst = 0;
	step_and_dump_wave();
	parse_args(argc, argv);
	init_rand();
	init_log(log_file);
	init_mem();
	init_isa();
	long img_size = load_img();
	if(diff_so_file != NULL) init_difftest(diff_so_file, img_size, difftest_port);
	init_sdb();
	if(itrace_on) init_disasm("riscv64-pc-linux-gnu");
}


void sim_exec(uint64_t n) {
	switch(npc_state.state) {
		case NPC_END:
		case NPC_ABORT:
			printf("Program execution has ended. To restart the program, exit NPC and run again.\n");
			return;
		default:
			npc_state.state = NPC_RUNNING;
	}
	bool print_step = false;
	if(n < MAX_PRINT_NUM) print_step = true;
	while(n-- && !contextp->gotFinish()) { 
		top->clk = !top->clk;
		step_and_dump_wave();
		//itrace evaluate
		if(log_fp != stdout && itrace_on) {
			char *p = logbuf;
			p += snprintf(p, sizeof(logbuf), FMT_WORD ":", top->pc);
			int i;
			uint8_t *inst = (uint8_t *)&rtl_inst;
			for (i = 3; i>=0; i--) {
				p += snprintf(p, 4, " %02x", inst[i]);
			}
			memset(p, ' ', 1);
			p += 1;
			disassemble(p, logbuf+sizeof(logbuf)-p, top->pc, (uint8_t *)&rtl_inst, 4);
			if(print_step) puts(logbuf);
			log_write("%s\n", logbuf);
		}
		//check if instruction is ebreak
		if(if_ebreak())
		 	npc_state.state = NPC_END;
		// pc and next_pc is right at this time
		vaddr_t pc = top->pc;
		cpu.pc = top->next_pc;
		//printf("pc is 0x%016lx, next_pc is 0x%016lx\n", top->pc, top->next_pc);
		top->clk = !top->clk;
		step_and_dump_wave();
		//load the npc gpr to sim structure cpu.gpr
		set_regfile();
		//difftest step
		if(diff_so_file != NULL) difftest_step(pc);
		//check if watchpoint is triggered
		if(check_wp())
		 	npc_state.state = NPC_STOP;
		//check npc state
		if(npc_state.state != NPC_RUNNING)
			break;
	}
	switch(npc_state.state) {
		case NPC_RUNNING:
			npc_state.state = NPC_STOP;
			break;
		case NPC_END:
		case NPC_ABORT:
			Log("NPC is END/ABORT");
		case NPC_QUIT:
			Log("Wait to exit");
	}
}
