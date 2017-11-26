`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
//
//Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
//Date Created: Wed Dec  7 03:45:17 2016
//Assignment: 3
//File: matt_test2.v
//Description: An HLSM module which represents the C-like behavioral description 
//             into a scheduled high-level statement machine implemented in Verilog.
//
//////////////////////////////////////////////////////////////////////////////////

module HLSM(Clk, Rst, Start, Done, a, b, c, z, x);
	input Clk, Rst, Start;
	output reg Done;

	input [15:0] a, b, c;

	output reg [7:0] z;
	output reg [15:0] x;

	reg [7:0] d, e, f, g;

	reg[3:0] state;
	parameter sWait = 0, s2 = 1, s3 = 2, s4 = 3, s5 = 4, s6 = 5, sFinal = 6;

	always@(posedge Clk) begin
		if(Rst == 1) begin
			state <= sWait;
			d <= 0;
			e <= 0;
			f <= 0;
			g <= 0;
			z <= 0;
			x <= 0;
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
					d <= a + b;
					state <= s3;
				end
				s3: begin
					e <= a + c;
					state <= s4;
				end
				s4: begin
					f <= a * c;
					state <= s5;
				end
				s5: begin
					g <= d > e;
					state <= s6;
				end
				s6: begin
					z <= g ? d : e;
					x <= f - d;
					state <= sFinal;
				end
				sFinal: begin
					Done <= 1;
					state <= sWait;
				end
			endcase
		end
	end

endmodule
