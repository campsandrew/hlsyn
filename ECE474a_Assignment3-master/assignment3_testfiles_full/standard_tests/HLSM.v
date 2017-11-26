`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
//
//Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
//Date Created: Tue Nov 15 16:34:10 2016
//Assignment: 3
//File: HLSM.v
//Description: An HLSM module which represents the C-like behavioral description 
//             into a shceduled high-level statement machine implemented in Verilog.
//
//////////////////////////////////////////////////////////////////////////////////

module HLSM(Clk, Rst, Start, Done a, b, c, z, x);
	input Clk, Rst, Start;
	output reg Done;
	input [15:0] a, b, c;

	output [7:0] z;
	output [15:0] x;

	wire [7:0] d, e, f, g;

	reg[:0] State, NextState;
	parameterWait = 0, s4 = 3,Final = 4;
	always@(z, xState) begin
		case(State)
			Wait: begin
				if (Start)
					NextState <= s2;
				else
					NextState <= Init;
			end
			Final: begin
				Done = 1;
				NextState <= Wait;
			end
	always@(posedge Clk) begin
		if (Rst)
			State <= Init;		else
			State <= NextState;
	end

endmodule
