module Reg #(WIDTH = 1, RESET_VAL = 0) (
	input clk,
	input rst,
	input [WIDTH-1:0] din,
	output reg [WIDTH-1:0] dout,
	input wen
);
	always @(posedge clk) begin
		if(rst) dout <= RESET_VAL;
		else if (wen) dout <= din;
	end

endmodule

