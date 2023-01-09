module alu #(DATA_WIDTH = 64) (
	input [DATA_WIDTH-1:0] alu_a,
	input [DATA_WIDTH-1:0] alu_b,
	input [3:0] alu_ctrl,
	input sext_32b,
	input [3:0] mul_div_rem_sel,
	output [DATA_WIDTH-1:0] alu_out,
	output less,
	output zero
);

wire arithmetic_logic;
wire left_right;
wire unsigned_signed;
wire sub_add;
wire [DATA_WIDTH-1:0] result;
wire carry_flag;
wire carry;
wire overflow;
wire [DATA_WIDTH-1:0] shift;
wire [DATA_WIDTH-1:0] and_ab;
wire [DATA_WIDTH-1:0] or_ab;
wire [DATA_WIDTH-1:0] xor_ab;
wire [DATA_WIDTH-1:0] alu_b_sub;
wire [DATA_WIDTH-1:0] shift_left;
wire [DATA_WIDTH-1:0] shift_right_logic_64b;
wire [DATA_WIDTH-1:0] shift_right_arithmetic_64b;
wire [31:0] shift_right_logic_32b;
wire [31:0] shift_right_arithmetic_32b;
wire [DATA_WIDTH-1:0] mul_ab;
wire [2*DATA_WIDTH-1:0] mulh_ab;
wire [2*DATA_WIDTH-1:0] mulhu_ab;
wire [2*DATA_WIDTH-1:0] mulhsu_ab;
wire [DATA_WIDTH-1:0] div_ab;
wire [DATA_WIDTH-1:0] divu_ab;
wire [31:0] div_ab_32b;
wire [31:0] divu_ab_32b;
wire [DATA_WIDTH-1:0] rem_ab;
wire [DATA_WIDTH-1:0] remu_ab;
wire [31:0] rem_ab_32b;
wire [31:0] remu_ab_32b;
wire [DATA_WIDTH-1:0] mul_div_rem_ab;

assign sub_add = (alu_ctrl == 4'b1000 | alu_ctrl == 4'b0010 | alu_ctrl == 4'b1010);
assign arithmetic_logic = (alu_ctrl == 4'b1101);
assign left_right = (alu_ctrl == 4'b0001);
assign unsigned_signed = (alu_ctrl == 4'b1010);
assign alu_b_sub = ~alu_b+1;

//adder
/* verilator lint_off WIDTH */
assign {carry_flag, result} = sub_add ? alu_a+~alu_b+1 : alu_a+alu_b;
assign carry = sub_add^carry_flag;
assign overflow = sub_add ? ((alu_a[DATA_WIDTH-1] == alu_b_sub[DATA_WIDTH-1]) && (result[DATA_WIDTH-1] != alu_a[DATA_WIDTH-1])) : ((alu_a[DATA_WIDTH-1] == alu_b[DATA_WIDTH-1]) && (result[DATA_WIDTH-1] != alu_a[DATA_WIDTH-1]));
assign zero = ~(| result);

//shift
assign shift_left = alu_a<<alu_b[5:0];
assign shift_right_logic_64b = alu_a>>alu_b[5:0];
assign shift_right_arithmetic_64b = $signed(alu_a)>>>alu_b[5:0];
assign shift_right_logic_32b = alu_a[31:0]>>alu_b[5:0];
assign shift_right_arithmetic_32b = $signed(alu_a[31:0])>>>alu_b[5:0];

MuxKey #(4, 2, DATA_WIDTH) u_shifter (shift, {left_right, arithmetic_logic}, {
	2'b00, sext_32b ? {{(DATA_WIDTH-32){shift_right_logic_32b[31]}}, shift_right_logic_32b[31:0]} : shift_right_logic_64b,
	2'b01, sext_32b ? {{(DATA_WIDTH-32){shift_right_arithmetic_32b[31]}}, shift_right_arithmetic_32b[31:0]} : shift_right_arithmetic_64b,
	2'b10, sext_32b ? {{(DATA_WIDTH-32){shift_left[31]}}, shift_left[31:0]} : shift_left,
	2'b11, sext_32b ? {{(DATA_WIDTH-32){shift_left[31]}}, shift_left[31:0]} : shift_left
	});

//logic
assign and_ab = alu_a & alu_b;
assign or_ab = alu_a | alu_b;
assign xor_ab = alu_a ^ alu_b;

//compare
assign less = unsigned_signed ? sub_add^carry : result[DATA_WIDTH-1]^overflow; 

//mul div rem
assign mul_ab        = alu_a*alu_b;
assign mulh_ab       = {{(DATA_WIDTH){alu_a[DATA_WIDTH-1]}}, alu_a}*{{(DATA_WIDTH){alu_b[DATA_WIDTH-1]}}, alu_b};
assign mulhu_ab      = {{(DATA_WIDTH){1'b0}}, alu_a}*{{(DATA_WIDTH){1'b0}}, alu_b};
assign mulhsu_ab     = {{(DATA_WIDTH){alu_a[DATA_WIDTH-1]}}, alu_a}*{{(DATA_WIDTH){1'b0}}, alu_b};
assign div_ab		     = $signed(alu_a)/$signed(alu_b);
assign divu_ab       = alu_a/alu_b;
assign div_ab_32b		 = $signed(alu_a[31:0])/$signed(alu_b[31:0]);
assign divu_ab_32b   = alu_a[31:0]/alu_b[31:0];
assign rem_ab		     = $signed(alu_a)%$signed(alu_b);
assign remu_ab       = alu_a%alu_b;
assign rem_ab_32b		 = $signed(alu_a[31:0])%$signed(alu_b[31:0]);
assign remu_ab_32b   = alu_a[31:0]%alu_b[31:0];
MuxKey #(13, 4, DATA_WIDTH) u_mul_div_rem_sel (mul_div_rem_ab, mul_div_rem_sel, {
	4'b0000, mul_ab,
	4'b0001, mulh_ab[2*DATA_WIDTH-1:DATA_WIDTH],
	4'b0010, mulhu_ab[2*DATA_WIDTH-1:DATA_WIDTH],
	4'b0011, mulhsu_ab[2*DATA_WIDTH-1:DATA_WIDTH],
	4'b0100, {{(DATA_WIDTH-32){mul_ab[31]}}, mul_ab[31:0]},
	4'b0101, div_ab,
	4'b0110, divu_ab,
	4'b0111, {{(DATA_WIDTH-32){div_ab_32b[31]}}, div_ab_32b},
	4'b1000, {{(DATA_WIDTH-32){divu_ab_32b[31]}}, divu_ab_32b},
	4'b1001, rem_ab,
	4'b1010, remu_ab,
	4'b1011, {{(DATA_WIDTH-32){rem_ab_32b[31]}}, rem_ab_32b},
	4'b1100, {{(DATA_WIDTH-32){remu_ab_32b[31]}}, remu_ab_32b}
	});
	
//out_sel
MuxKey #(12, 4, DATA_WIDTH) u_out_sel(alu_out, alu_ctrl, {
	4'b0000, sext_32b ? {{(DATA_WIDTH-32){result[31]}}, result[31:0]} : result,
	4'b1000, sext_32b ? {{(DATA_WIDTH-32){result[31]}}, result[31:0]} : result,
	4'b0001, shift,
	4'b0101, shift,
	4'b1101, shift,
	4'b0010, {{(DATA_WIDTH-1){1'b0}}, less},
	4'b1010, {{(DATA_WIDTH-1){1'b0}}, less},
	4'b0011, alu_b,
	4'b0100, xor_ab,
	4'b0110, or_ab,
	4'b0111, and_ab,
	4'b1001, mul_div_rem_ab
	});



endmodule
