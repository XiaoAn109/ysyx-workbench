`define RESET_PC 64'h0000000080000000
module Instruction_fetch_unit #(INST_WIDTH = 32, PC_WIDTH = 64) (
	input clk,
	input rst,
	input [PC_WIDTH-1:0] next_pc,
	output [PC_WIDTH-1:0] pc,
	output [INST_WIDTH-1:0] ifu2idu_inst
);

	wire [PC_WIDTH-1:0] imem2ifu_inst;
	//instruction fetch
	import "DPI-C" function void rtl_pmem_read(input longint raddr, output longint rdata, input bit ren);
	always @(negedge clk) begin
			rtl_pmem_read(pc, imem2ifu_inst, ~rst);
	end
	assign ifu2idu_inst = imem2ifu_inst[INST_WIDTH-1:0];

	Reg #(PC_WIDTH, `RESET_PC) u_PC_Reg(clk, rst, next_pc, pc, 1'b1);
endmodule
