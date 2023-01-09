module Load_store_unit #(DATA_WIDTH = 64) (
	input clk,
	input rst,
	input data_mem_we,
	input [2:0] data_mem_op,
	input [DATA_WIDTH-1:0] data_mem_addr,
	input [DATA_WIDTH-1:0] data_mem_in,
	output [DATA_WIDTH-1:0] data_mem_out
);
wire [3:0] data_mem_ctrl = {data_mem_we, data_mem_op};
wire is_lb;
wire is_lh;
wire is_lw;
wire is_ld;
wire is_lbu;
wire is_lhu;
wire is_lwu;
wire is_sb;
wire is_sh;
wire is_sw;
wire is_sd;
wire is_other;

wire [DATA_WIDTH-1:0] raddr;
wire [DATA_WIDTH-1:0] rdata;
wire ren;

wire [7:0] mask;
wire [7:0] wmask;
wire [DATA_WIDTH-1:0] waddr;
wire [DATA_WIDTH-1:0] wdata;
wire wen;

assign is_lb  = (data_mem_ctrl == 4'b0000);
assign is_lh  = (data_mem_ctrl == 4'b0001);
assign is_lw  = (data_mem_ctrl == 4'b0010);
assign is_ld  = (data_mem_ctrl == 4'b0011);
assign is_lbu = (data_mem_ctrl == 4'b0100);
assign is_lhu = (data_mem_ctrl == 4'b0101);
assign is_lwu = (data_mem_ctrl == 4'b0110);
assign is_sb  = (data_mem_ctrl == 4'b1000);
assign is_sh  = (data_mem_ctrl == 4'b1001);
assign is_sw  = (data_mem_ctrl == 4'b1010);
assign is_sd  = (data_mem_ctrl == 4'b1011);
assign is_other = (data_mem_op == 3'b111);

assign raddr = data_mem_addr;
assign ren = is_lb | is_lh | is_lw | is_ld | is_lbu | is_lhu | is_lwu;
assign data_mem_out = is_lb  ? {{(DATA_WIDTH-8){rdata[7]}}, rdata[7:0]} :
											is_lh  ? {{(DATA_WIDTH-16){rdata[15]}}, rdata[15:0]} :
											is_lw  ? {{(DATA_WIDTH-32){rdata[31]}}, rdata[31:0]} :
											is_ld  ? rdata :
											is_lbu ? {{(DATA_WIDTH-8){1'b0}}, rdata[7:0]} :
											is_lhu ? {{(DATA_WIDTH-16){1'b0}}, rdata[15:0]} :
											is_lwu ? {{(DATA_WIDTH-32){1'b0}}, rdata[31:0]} : {(DATA_WIDTH){1'b0}};

assign mask = is_sb ? 8'b0000_0001 :
							 is_sh ? 8'b0000_0011 :
							 is_sw ? 8'b0000_1111 :
							 is_sd ? 8'b1111_1111 : 8'b0;

assign wen = is_sb | is_sh | is_sw | is_sd;

Reg #(2*DATA_WIDTH+8, 0) u_write_reg (clk, rst, {data_mem_addr, data_mem_in, mask}, {waddr, wdata, wmask}, 1'b1);


import "DPI-C" function void rtl_pmem_read(input longint raddr, output longint rdata, input bit ren);
import "DPI-C" function void rtl_pmem_write(input longint waddr, input longint wdata, input byte wmask, input bit wen);

always @(*) begin
  rtl_pmem_read(raddr, rdata, ren);
  rtl_pmem_write(waddr, wdata, wmask, wen);
end

endmodule
