module Instruction_decode_unit #(INST_WIDTH = 32 ,DATA_WIDTH = 64, ADDR_WIDTH = 5) (
	input [INST_WIDTH-1:0] ifu2idu_inst,
	output [ADDR_WIDTH-1:0] rf_raddr1,
	output [ADDR_WIDTH-1:0] rf_raddr2,
	output [ADDR_WIDTH-1:0] rf_waddr,
	output [DATA_WIDTH-1:0] imm,
	output alu_a_sel,
	output [1:0] alu_b_sel,
	output [3:0] alu_ctrl,
	output sext_32b,
	output rf_wr_en,
	output rf_wr_sel,
	output mem_wr_en,
	output [2:0] mem_wr_sel,
	output [2:0] branch,
	output [3:0] mul_div_rem_sel
);

Control_unit u_ctrl(
	.ifu2idu_inst(ifu2idu_inst),
	.alu_a_sel(alu_a_sel),
	.alu_b_sel(alu_b_sel),
	.alu_ctrl(alu_ctrl),
	.sext_32b(sext_32b),
	.rf_wr_en(rf_wr_en),
	.rf_wr_sel(rf_wr_sel),
	.mem_wr_en(mem_wr_en),
	.mem_wr_sel(mem_wr_sel),
	.branch(branch),
	.mul_div_rem_sel(mul_div_rem_sel)
);

Immediate_gen u_imm_gen(
	.ifu2idu_inst(ifu2idu_inst),
	.imm(imm)
);

assign rf_raddr1 = ifu2idu_inst[19:15];
assign rf_raddr2 = ifu2idu_inst[24:20];
assign rf_waddr = ifu2idu_inst[11:7];

endmodule
