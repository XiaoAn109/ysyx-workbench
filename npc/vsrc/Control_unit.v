module Control_unit #(INST_WIDTH = 32) (
	input [INST_WIDTH-1:0] ifu2idu_inst,
	output alu_a_sel,
	output [1:0] alu_b_sel,
	output [3:0] alu_ctrl,
	output sext_32b,
	output rf_wr_en,
	output rf_wr_sel,
	output mem_wr_en,
	output [2:0] mem_wr_sel,
	output [2:0] branch,
	output [3:0] mul_div_rem_sel // for M extension
);
wire [6:0] opcode;
wire [2:0] funct3;
wire [6:0] funct7;

//RV32I
wire is_lui;
wire is_auipc;
wire is_jal;
wire is_jalr;
wire is_beq;
wire is_bne;
wire is_blt;
wire is_bge;
wire is_bltu;
wire is_bgeu;
wire is_lb;
wire is_lh;
wire is_lw;
wire is_lbu;
wire is_lhu;
wire is_sb;
wire is_sh;
wire is_sw;
wire is_addi;
wire is_slti;
wire is_sltiu;
wire is_xori;
wire is_ori;
wire is_andi;
wire is_slli;
wire is_srli;
wire is_srai;
wire is_add;
wire is_sub;
wire is_sll;
wire is_slt;
wire is_sltu;
wire is_xor;
wire is_srl;
wire is_sra;
wire is_or;
wire is_and;

//RV64I
wire is_addiw;
wire is_addw;
wire is_subw;
wire is_slliw;
wire is_sllw;
wire is_srliw;
wire is_srlw;
wire is_sraiw;
wire is_sraw;
wire is_lwu;
wire is_ld;
wire is_sd;

//RV32M
wire is_mul;
wire is_mulh;
wire is_mulhu;
wire is_mulhsu;
wire is_div;
wire is_divu;
wire is_rem;
wire is_remu;

//RV64M
wire is_mulw;
wire is_remw;
wire is_divuw;
wire is_divw;
wire is_remuw;



wire is_u_type;
wire is_j_type;
wire is_b_type;
wire is_r_type;
wire is_i_type;
wire is_s_type;

wire alu_add;
wire alu_sub;
wire alu_shift_left;
wire alu_shift_right_logic;
wire alu_shift_right_arithmetic;
wire alu_less_set_signed;
wire alu_less_set_unsigned;
wire alu_b_out;
wire alu_xor;
wire alu_or;
wire alu_and;
wire alu_mul_div_rem;

assign opcode = ifu2idu_inst[6:0];
assign funct3 = ifu2idu_inst[14:12];
assign funct7 = ifu2idu_inst[31:25];

//RV32I
assign is_lui    = (opcode == 7'h37);
assign is_auipc  = (opcode == 7'h17);
assign is_jal    = (opcode == 7'h6f);
assign is_jalr   = (opcode == 7'h67) & (funct3 == 3'h0);
assign is_beq    = (opcode == 7'h63) & (funct3 == 3'h0);
assign is_bne    = (opcode == 7'h63) & (funct3 == 3'h1);
assign is_blt    = (opcode == 7'h63) & (funct3 == 3'h4);
assign is_bge    = (opcode == 7'h63) & (funct3 == 3'h5);
assign is_bltu   = (opcode == 7'h63) & (funct3 == 3'h6);
assign is_bgeu   = (opcode == 7'h63) & (funct3 == 3'h7);
assign is_lb     = (opcode == 7'h03) & (funct3 == 3'h0);
assign is_lh     = (opcode == 7'h03) & (funct3 == 3'h1);
assign is_lw     = (opcode == 7'h03) & (funct3 == 3'h2);
assign is_lbu    = (opcode == 7'h03) & (funct3 == 3'h4);
assign is_lhu    = (opcode == 7'h03) & (funct3 == 3'h5);
assign is_sb     = (opcode == 7'h23) & (funct3 == 3'h0);
assign is_sh     = (opcode == 7'h23) & (funct3 == 3'h1);
assign is_sw     = (opcode == 7'h23) & (funct3 == 3'h2);
assign is_addi   = (opcode == 7'h13) & (funct3 == 3'h0);
assign is_slti   = (opcode == 7'h13) & (funct3 == 3'h2);
assign is_sltiu  = (opcode == 7'h13) & (funct3 == 3'h3);
assign is_xori   = (opcode == 7'h13) & (funct3 == 3'h4);
assign is_ori    = (opcode == 7'h13) & (funct3 == 3'h6);
assign is_andi   = (opcode == 7'h13) & (funct3 == 3'h7);
assign is_slli   = (opcode == 7'h13) & (funct3 == 3'h1) & (funct7[6:1] == 6'h00);
assign is_srli   = (opcode == 7'h13) & (funct3 == 3'h5) & (funct7[6:1] == 6'h00);
assign is_srai   = (opcode == 7'h13) & (funct3 == 3'h5) & (funct7[6:1] == 6'h10);
assign is_add    = (opcode == 7'h33) & (funct3 == 3'h0) & (funct7 == 7'h00);
assign is_sub    = (opcode == 7'h33) & (funct3 == 3'h0) & (funct7 == 7'h20);
assign is_sll    = (opcode == 7'h33) & (funct3 == 3'h1) & (funct7 == 7'h00);
assign is_slt    = (opcode == 7'h33) & (funct3 == 3'h2) & (funct7 == 7'h00);
assign is_sltu   = (opcode == 7'h33) & (funct3 == 3'h3) & (funct7 == 7'h00);
assign is_xor    = (opcode == 7'h33) & (funct3 == 3'h4) & (funct7 == 7'h00);
assign is_srl    = (opcode == 7'h33) & (funct3 == 3'h5) & (funct7 == 7'h00);
assign is_sra    = (opcode == 7'h33) & (funct3 == 3'h5) & (funct7 == 7'h20);
assign is_or     = (opcode == 7'h33) & (funct3 == 3'h6) & (funct7 == 7'h00);
assign is_and    = (opcode == 7'h33) & (funct3 == 3'h7) & (funct7 == 7'h00);

//RV64I
assign is_addiw  = (opcode == 7'h1b) & (funct3 == 3'h0); 
assign is_addw   = (opcode == 7'h3b) & (funct3 == 3'h0) & (funct7 == 7'h00);
assign is_subw   = (opcode == 7'h3b) & (funct3 == 3'h0) & (funct7 == 7'h20);
assign is_slliw  = (opcode == 7'h1b) & (funct3 == 3'h1) & (funct7 == 7'h00);
assign is_sllw   = (opcode == 7'h3b) & (funct3 == 3'h1) & (funct7 == 7'h00);
assign is_srliw  = (opcode == 7'h1b) & (funct3 == 3'h5) & (funct7 == 7'h00);
assign is_srlw   = (opcode == 7'h3b) & (funct3 == 3'h5) & (funct7 == 7'h00);
assign is_sraiw  = (opcode == 7'h1b) & (funct3 == 3'h5) & (funct7 == 7'h20);
assign is_sraw   = (opcode == 7'h3b) & (funct3 == 3'h5) & (funct7 == 7'h20);
assign is_lwu    = (opcode == 7'h03) & (funct3 == 3'h6);
assign is_ld     = (opcode == 7'h03) & (funct3 == 3'h3);
assign is_sd     = (opcode == 7'h23) & (funct3 == 3'h3);

//RV32M
assign is_mul    = (opcode == 7'h33) & (funct3 == 3'h0) & (funct7 == 7'h01);
assign is_mulh   = (opcode == 7'h33) & (funct3 == 3'h1) & (funct7 == 7'h01);
assign is_mulhu  = (opcode == 7'h33) & (funct3 == 3'h3) & (funct7 == 7'h01);
assign is_mulhsu = (opcode == 7'h33) & (funct3 == 3'h2) & (funct7 == 7'h01);
assign is_div    = (opcode == 7'h33) & (funct3 == 3'h4) & (funct7 == 7'h01);
assign is_divu   = (opcode == 7'h33) & (funct3 == 3'h5) & (funct7 == 7'h01);
assign is_rem    = (opcode == 7'h33) & (funct3 == 3'h6) & (funct7 == 7'h01);
assign is_remu   = (opcode == 7'h33) & (funct3 == 3'h7) & (funct7 == 7'h01);

//RV64M
assign is_mulw   = (opcode == 7'h3b) & (funct3 == 3'h0) & (funct7 == 7'h01);
assign is_remw   = (opcode == 7'h3b) & (funct3 == 3'h6) & (funct7 == 7'h01);
assign is_divuw  = (opcode == 7'h3b) & (funct3 == 3'h5) & (funct7 == 7'h01);
assign is_divw   = (opcode == 7'h3b) & (funct3 == 3'h4) & (funct7 == 7'h01);
assign is_remuw  = (opcode == 7'h3b) & (funct3 == 3'h7) & (funct7 == 7'h01);

assign is_u_type = is_lui | is_auipc ;
assign is_j_type = is_jal ;
assign is_b_type = is_beq | is_bne | is_blt | is_bge | is_bltu | is_bgeu ;
assign is_r_type = is_add | is_sub | is_sll | is_slt | is_sltu | is_xor 
                   | is_srl | is_sra | is_or | is_and | is_addw | is_subw
									 | is_sllw | is_srlw | is_sraw | is_mul | is_mulh | is_mulhu | is_mulhsu | is_mulw
									 | is_div | is_divu | is_divw | is_divuw | is_rem | is_remu | is_remw | is_remuw;
assign is_i_type = is_jalr | is_lb | is_lh | is_lw | is_lbu | is_lhu 
                   | is_addi | is_slti | is_sltiu | is_xori | is_ori | is_andi
                   | is_slli | is_srli | is_srai | is_addiw | is_slliw | is_srliw | is_sraiw
									 | is_ld | is_lwu;
assign is_s_type = is_sb | is_sh | is_sw | is_sd;

assign alu_add                    = is_auipc | is_addi | is_add | is_jal | is_jalr | is_lb | is_lh | is_lw
								                    | is_lbu | is_lhu | is_ld | is_lwu | is_addiw | is_addw | is_s_type;
assign alu_sub										= is_sub | is_subw;
assign alu_shift_left							= is_slli | is_sll | is_slliw | is_sllw;
assign alu_shift_right_logic      = is_srli | is_srl | is_srliw | is_srlw;
assign alu_shift_right_arithmetic = is_srai | is_sra | is_sraiw | is_sraw;
assign alu_less_set_signed        = is_slti | is_slt | is_beq | is_bne | is_blt | is_bge;
assign alu_less_set_unsigned      = is_sltiu | is_sltu | is_bltu | is_bgeu;
assign alu_b_out                  = is_lui;
assign alu_xor										= is_xori | is_xor;
assign alu_or                     = is_ori | is_or;
assign alu_and										= is_andi | is_and;
assign alu_mul_div_rem						= is_mul | is_mulh | is_mulhu | is_mulhsu | is_mulw
																		| is_div | is_divu | is_divw | is_divuw
																		| is_rem | is_remu | is_remw | is_remuw;
																				

//1 for rs1, 0 for PC
assign alu_a_sel = is_auipc | is_jal | is_jalr;
//10 for imm, 00 for rs2, 01 for 4
assign alu_b_sel = is_r_type | is_b_type ? 2'b00 :
							 is_jal | is_jalr ? 2'b01 : 2'b10;
assign alu_ctrl = alu_add										 ? 4'b0000 :
									alu_sub										 ? 4'b1000 :
									alu_shift_left						 ? 4'b0001 :
									alu_less_set_signed				 ? 4'b0010 :
									alu_less_set_unsigned 		 ? 4'b1010 :
									alu_b_out									 ? 4'b0011 :
									alu_xor										 ? 4'b0100 :
									alu_shift_right_logic 		 ? 4'b0101 :
									alu_shift_right_arithmetic ? 4'b1101 :
									alu_or									   ? 4'b0110 :
									alu_and                    ? 4'b0111 :
									alu_mul_div_rem						 ? 4'b1001 : 4'b1011; //1011 for invalid alu ctrl

assign sext_32b = is_addiw | is_slliw | is_srliw | is_sraiw | is_addw | is_subw | is_sllw | is_srlw | is_sraw;
assign rf_wr_en = ~(is_b_type | is_s_type);
assign rf_wr_sel = is_lb | is_lh | is_lw | is_lbu | is_lhu | is_ld | is_lwu; //whether load mem to rf 
assign mem_wr_en = is_s_type;
assign mem_wr_sel = is_lb  ? 3'b000 :
										is_lh  ? 3'b001 :
										is_lw  ? 3'b010 :
										is_ld  ? 3'b011 :
										is_lbu ? 3'b100 :
										is_lhu ? 3'b101 :
										is_lwu ? 3'b110 :
										is_sb  ? 3'b000 :
										is_sh  ? 3'b001 :
										is_sw  ? 3'b010 :
										is_sd  ? 3'b011 : 3'b111; //3'b111 for no load store
assign branch = is_jal  ? 3'b001 :
								is_jalr ? 3'b010 :
								is_beq  ? 3'b100 :
								is_bne  ? 3'b101 :
								is_blt  ? 3'b110 :
								is_bge  ? 3'b111 :
								is_bltu ? 3'b110 :
								is_bgeu ? 3'b111 : 3'b000;

assign mul_div_rem_sel = is_mul ? 4'b0000 :
												 is_mulh ? 4'b0001 :
												 is_mulhu ? 4'b0010 :
												 is_mulhsu ? 4'b0011 :
												 is_mulw ? 4'b0100 :
												 is_div ? 4'b0101 :
												 is_divu ? 4'b0110 :
												 is_divw ? 4'b0111 :
												 is_divuw ? 4'b1000 :
												 is_rem ? 4'b1001 :
												 is_remu ? 4'b1010 :
												 is_remw ? 4'b1011 :
												 is_remuw ? 4'b1100 : 4'b1101; //1101 for no M extension
endmodule
