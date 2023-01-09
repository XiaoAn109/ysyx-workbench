#include "include/common.h"
#include "include/basic_func.h"
#include "include/memory.h"
#include "include/sdb.h"

int main(int argc, char *argv[]) {
	sim_init(argc, argv);
	resetn(5);
	svSetScope(svGetScopeFromName("TOP.top"));
	sdb_mainloop();
	return sim_exit();
}
