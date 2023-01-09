module top #(PC_WIDTH = 64, INST_WIDTH = 32, ADDR_WIDTH = 5, DATA_WIDTH = 64) (
	input clk,
	input rst,
	output [PC_WIDTH-1:0] pc,
	output [PC_WIDTH-1:0] next_pc
);
wire [DATA_WIDTH-1:0] data_mem_out; // dmem rdata after byte sel
wire [DATA_WIDTH-1:0] data_mem_in; // dmem wdata
wire [DATA_WIDTH-1:0] data_mem_addr;
wire data_mem_we;
wire [2:0] data_mem_op;

wire [INST_WIDTH-1:0] ifu2idu_inst;
wire [ADDR_WIDTH-1:0] rf_raddr1;
wire [ADDR_WIDTH-1:0] rf_raddr2;
wire [DATA_WIDTH-1:0] rf_rdata1;
wire [DATA_WIDTH-1:0] rf_rdata2;
wire [ADDR_WIDTH-1:0] rf_waddr;
wire [DATA_WIDTH-1:0] rf_wdata;
wire [DATA_WIDTH-1:0] imm;
wire alu_a_sel;
wire [1:0] alu_b_sel;
wire [3:0] alu_ctrl;
wire sext_32b;
wire rf_wr_en;
wire rf_wr_sel;
wire mem_wr_en;
wire [2:0] mem_wr_sel;
wire [2:0] branch;
wire [3:0] mul_div_rem_sel;
wire [DATA_WIDTH-1:0] alu_a;
wire [DATA_WIDTH-1:0] alu_b;
wire [3:0] alu_ctrl;
wire [DATA_WIDTH-1:0] alu_out;
wire less;
wire zero;

wire [DATA_WIDTH-1:0] pc_a;
wire [DATA_WIDTH-1:0] pc_b;

wire pc_a_sel;
wire pc_b_sel;

Instruction_fetch_unit u_IFU(
	.clk(clk),
	.rst(rst),
	.next_pc(next_pc),
	.pc(pc),
	.ifu2idu_inst(ifu2idu_inst)
);

Instruction_decode_unit u_IDU(
	.ifu2idu_inst(ifu2idu_inst),
	.rf_raddr1(rf_raddr1),
	.rf_raddr2(rf_raddr2),
	.rf_waddr(rf_waddr),
	.imm(imm),
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

alu u_alu(
	.alu_a(alu_a),
	.alu_b(alu_b),
	.alu_ctrl(alu_ctrl),
	.sext_32b(sext_32b),
	.mul_div_rem_sel(mul_div_rem_sel),
	.alu_out(alu_out),
	.less(less),
	.zero(zero)
);

RegisterFile u_RegFile(
	.clk(clk),
	.wdata(rf_wdata),
	.waddr(rf_waddr),
	.wen(rf_wr_en),
	.raddr1(rf_raddr1),
	.raddr2(rf_raddr2),
	.rdata1(rf_rdata1),
	.rdata2(rf_rdata2)
);

Branch_condition u_branch(
	.branch(branch),
	.zero(zero),
	.less(less),
	.pc_a_sel(pc_a_sel),
	.pc_b_sel(pc_b_sel)
);

Load_store_unit u_LSU(
	.clk(clk),
	.rst(rst),
	.data_mem_we(data_mem_we),
	.data_mem_op(data_mem_op),
	.data_mem_addr(data_mem_addr),
	.data_mem_in(data_mem_in),
	.data_mem_out(data_mem_out)
);

assign next_pc = pc_a + pc_b;

//Load & Store Unit
assign data_mem_addr = alu_out;
assign data_mem_op = mem_wr_sel;
assign data_mem_we = mem_wr_en;
assign data_mem_in = rf_rdata2;

//alu sel
assign alu_a = alu_a_sel ? pc : rf_rdata1;
MuxKey #(3, 2, DATA_WIDTH) u_alu_b_sel (alu_b, alu_b_sel, {
	2'b00, rf_rdata2,
	2'b01, {{(DATA_WIDTH-3){1'b0}}, 3'h4},
	2'b10, imm
	});

assign rf_wdata = rf_wr_sel ? data_mem_out : alu_out;
assign pc_a = pc_a_sel ? imm : {{(DATA_WIDTH-3){1'b0}}, 3'h4};
assign pc_b = pc_b_sel ? rf_rdata1 : pc;


//check Ebreak
export "DPI-C" function if_ebreak;
function if_ebreak;
	if_ebreak = 1'b0;
	if(ifu2idu_inst == 32'h00100073) begin
		if_ebreak = 1'b1;
		$finish;
		$display("[HIT Ebreak! End Simulation.]");
		$display("[npc: %s at pc = 0x%08x]", (u_RegFile.rf[10] == 0) ? "\033[1;32mHIT GOOD TRAP\033[0m" : "\033[1;31mHIT BAD TRAP\033[0m", pc);
	end
endfunction
//export rf values
import "DPI-C" function void set_gpr_ptr(input logic [63:0] a []);
initial set_gpr_ptr(u_RegFile.rf);

//export inst being executed
import "DPI-C" function void rtl_get_inst(input int inst);
always @(*) begin
	rtl_get_inst(ifu2idu_inst);
end

endmodule
