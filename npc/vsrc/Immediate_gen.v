module Immediate_gen #(INST_WIDTH = 32, DATA_WIDTH = 64) (
	input [INST_WIDTH-1:0] ifu2idu_inst,
	output [DATA_WIDTH-1:0] imm
);

	wire [6:0] opcode;
	wire [DATA_WIDTH-1:0] immI;
	wire [DATA_WIDTH-1:0] immU;
	wire [DATA_WIDTH-1:0] immS;
	wire [DATA_WIDTH-1:0] immB;
	wire [DATA_WIDTH-1:0] immJ;

	assign opcode = ifu2idu_inst[6:0];

	assign immI = {{(DATA_WIDTH-12){ifu2idu_inst[31]}}, ifu2idu_inst[31:20]};
	assign immU = {{(DATA_WIDTH-32){ifu2idu_inst[31]}}, ifu2idu_inst[31:12], 12'b0};
	assign immS = {{(DATA_WIDTH-12){ifu2idu_inst[31]}}, ifu2idu_inst[31:25], ifu2idu_inst[11:7]};
	assign immB = {{(DATA_WIDTH-12){ifu2idu_inst[31]}}, ifu2idu_inst[7], ifu2idu_inst[30:25], ifu2idu_inst[11:8], 1'b0};
	assign immJ = {{(DATA_WIDTH-20){ifu2idu_inst[31]}}, ifu2idu_inst[19:12], ifu2idu_inst[20], ifu2idu_inst[30:21], 1'b0};
	
	MuxKey #(9, 7, DATA_WIDTH) u_imm_type (imm, opcode, {
		7'b0010111, immU,
    7'b0110111, immU,
    7'b1100011, immB,
		7'b1101111, immJ,
		7'b1100111, immI,
		7'b0000011, immI,
		7'b0100011, immS,
		7'b0010011, immI,
		7'b0011011, immI
		});

endmodule
