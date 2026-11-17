module bist #(
    parameter ADDR_WIDTH,
    parameter DATA_WIDTH
) (
    input                     clk,
    input                     rst,

    input                     bist_start,
    output                    bist_working,
    output                    bist_finish,
    output                    bist_failed,

    output                    bist_we,
    output [ADDR_WIDTH-1 : 0] bist_addr,
    output [DATA_WIDTH-1 : 0] bist_din
);

    wire bist_clk;
    wire bist_reset;

    wire [2 : 0] bist_state;
    wire bist_din_bit;
    
    wire [DATA_WIDTH-1 : 0] sram_dout;

    assign bist_din_bit = bist_din[0];

    assign bist_clk   = (bist_working == 1'b1 || bist_start == 1'b1) ? clk : 1'b0;
    assign bist_reset = (bist_working == 1'b0 && bist_start == 1'b1) ? 1'b1 : 1'b0;

    bist_state_machine #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH)) u_state_machine (
        .rst(rst),

        .bist_clk(bist_clk),
        .bist_reset(bist_reset),

        .bist_we(bist_we),
        .bist_state(bist_state),
        .bist_din(bist_din),
        .bist_addr(bist_addr),

        .bist_working(bist_working),
        .bist_finish(bist_finish)
    );

    bist_check #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH)) u_check (
        .rst(rst),

        .bist_clk(bist_clk),
        .bist_reset(bist_reset),

        .bist_we(bist_we),
        .bist_state(bist_state),
        .bist_din_bit(bist_din_bit),

        .sram_dout(sram_dout),
        .bist_failed(bist_failed)
    );

endmodule