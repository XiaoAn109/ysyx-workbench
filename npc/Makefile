TOPNAME = top
WAVENAME ?= dump
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --x-initial fast --noassert \
				--trace
INC_PATH ?= $(NPC_HOME)/csrc/include
CFLAGS += -CFLAGS "$(shell llvm-config --cxxflags) -g -I$(INC_PATH)"
LDFLAGS += -LDFLAGS "$(shell llvm-config --libs) -lreadline"

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

ifndef DEBUG
	NPC_FLAGS = --batch
else
	NPC_FLAGS =
endif

ifndef TRACE_OFF
	NPC_FLAGS += --log=$(abspath npc-log.txt)
endif

ifndef ITRACE_OFF
	NPC_FLAGS += --itrace
endif

ifdef DIFFTEST_ON
	NPC_FLAGS += --diff=$(NEMU_HOME)/build/riscv64-nemu-interpreter-so
endif

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))


# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

$(BIN): $(VSRCS) $(CSRCS)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^\
		$(CFLAGS) $(LDFLAGS) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

all: default

run: $(BIN)
	@rm -rf *.txt
	@$^ $(NPC_FLAGS) $(IMG)

gdb: $(BIN)
	gdb -s $^ --args $^ $(NPC_FLAGS) $(IMG)

wave: $(BIN) $(WAVENAME).vcd
	@gtkwave $(WAVENAME).vcd &

clean:
	rm -rf $(OBJ_DIR) $(BIN) *.vcd *.txt

.PHONY: default all clean run
