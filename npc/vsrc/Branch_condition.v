module Branch_condition(
	input [2:0] branch,
	input zero,
	input less,
	output pc_a_sel,
	output pc_b_sel
);

wire branch_eq;
wire branch_neq;
wire branch_less;
wire branch_greater;
wire no_jump;
wire jump_pc;
wire jump_reg;

assign branch_eq			= (branch == 3'b100);
assign branch_neq			= (branch == 3'b101);
assign branch_less		= (branch == 3'b110);
assign branch_greater = (branch == 3'b111);
assign no_jump				= (branch == 3'b000);
assign jump_pc 				= (branch == 3'b001);
assign jump_reg				= (branch == 3'b010);
//0 for 4, 1 for imm
assign pc_a_sel = jump_pc | jump_reg | (branch_eq & zero) | (branch_neq & ~zero) | (branch_less & less) | (branch_greater & ~less);
// 0 for pc, 1 for rs1
assign pc_b_sel = jump_reg;


endmodule
