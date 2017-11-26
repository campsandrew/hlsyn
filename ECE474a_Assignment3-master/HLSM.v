`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
//Name: Brett Bushnell
//Date: 11/30/2015
//Lab: Tuesday 2-5
//project: Lab Practical 4
//Description: HLSM module that behaviorally captures the assignment requirement
//////////////////////////////////////////////////////////////////////////////////


//NOTE: Matt, I copied this here to give us an example of how an HLSM looks from 274a, should help when generating the output file

module HLSM(A, go, Clk, Rst, i, busy, dfreq);
    input [7:0]A;
    input go, Clk, Rst;
    output reg [3:0]dfreq;
    output reg busy;
    output reg [3:0]i;
    reg [3:0]dcnt;
    reg [7:0]now, last;
    reg [8:0]signedDiff;
    reg [7:0]absDiff;
    
    reg[3:0] State, NextState;
    parameter Init = 0, s2 = 1, s3 = 2, s4 = 3, s5 = 4;
    
    //commands for states
    always@(A, go, State)begin
        case(State)
            Init: begin
                busy <= 0;
                i <= 0;
                dcnt <= 0;
                if (go == 1)
                    NextState <= s2;
                else
                    NextState <= Init;
            end
            s2: begin
                busy <= 1;
                now <= A;
                dfreq <= 4'd0;
                NextState <= s3;
           end
           s3: begin
               i <= i + 1;
               last <= now;
               now <= A;
               signedDiff <= last - now;
               if(signedDiff[8] == 1)begin
                  assign absDiff = signedDiff;
                  absDiff <= absDiff + 1;
               end
               else
                  absDiff <= last - now;
               if (i < 15 && absDiff >= 5 )
                NextState <= s3;
               else if (i < 15 && absDiff < 5)
                NextState <= s5;
               else if (i >= 15)
                NextState <= s4;
           end
           s4: begin
               assign dfreq = dcnt;
               NextState <= Init;
           end
           s5: begin
               dcnt <= dcnt + 1;
           end
        endcase     
    end
    
    always@(posedge Clk)begin
        if (Rst == 1)
            State <= Init;
        else
            State <= NextState;
    end
endmodule
