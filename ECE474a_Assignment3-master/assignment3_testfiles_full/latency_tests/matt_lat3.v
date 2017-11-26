`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
//
//Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
//Date Created: Wed Dec  7 03:05:23 2016
//Assignment: 3
//File: matt_lat3.v
//Description: An HLSM module which represents the C-like behavioral description 
//             into a scheduled high-level statement machine implemented in Verilog.
//
//////////////////////////////////////////////////////////////////////////////////

module HLSM(Clk, Rst, Start, Done, a, b, c, d, e, f, g, h, i, j, k, l);
	input Clk, Rst, Start;
	output reg Done;

	input signed [15:0] a, b, c, d, e, f, g, h;

	output signed [15:0] i, j, k, l;


	reg[3:0] state;
	parameter sWait = 0, s2 = 1, s3 = 2, s4 = 3, s5 = 4, sFinal = 5;

	always@(posedge Clk) begin
		if(Rst == 1) begin
			state <= sWait;
			i <= 0;
			j <= 0;
			k <= 0;
			l <= 0;
			Done <= 0;
		end
		else begin
			case(state)
				sWait: begin
					Done <= 0;
					if (Start == 1)
						state <= s2;
					else
						state <= sWait;
				end
				s2: begin
					i <= a * b;
					state <= s3;
				s3: begin
					l <= g * h;
					state <= s4;
				s4: begin
					k <= e * f;
					state <= s5;
				s5: begin
					j <= c * d;
					state <= sFinal;
				sFinal: begin
					Done <= 1;
					state <= sWait;
				end
			endcase
		end
	end

endmodule
