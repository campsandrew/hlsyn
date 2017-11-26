`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
//
//Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
//Date Created: Wed Dec  7 02:56:44 2016
//Assignment: 3
//File: matt_lat2.v
//Description: An HLSM module which represents the C-like behavioral description 
//             into a scheduled high-level statement machine implemented in Verilog.
//
//////////////////////////////////////////////////////////////////////////////////

module HLSM(Clk, Rst, Start, Done, a, b, c, d, e, i);
	input Clk, Rst, Start;
	output reg Done;

	input signed [15:0] a, b, c, d, e;

	output signed [15:0] i;

	reg signed [15:0] f, g, h;

	reg[3:0] state;
	parameter sWait = 0, s2 = 1, s3 = 2, s4 = 3, s5 = 4, sFinal = 5;

	always@(posedge Clk) begin
		if(Rst == 1) begin
			state <= sWait;
			f <= 0;
			g <= 0;
			h <= 0;
			i <= 0;
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
					f <= a * b;
					state <= s3;
				s3: begin
					g <= f * c;
					state <= s4;
				s4: begin
					h <= g * d;
					state <= s5;
				s5: begin
					i <= h * e;
					state <= sFinal;
				sFinal: begin
					Done <= 1;
					state <= sWait;
				end
			endcase
		end
	end

endmodule
