) (
    input                     clk,
    input                     rst,
    input                     csb,
    input                     we,
    input [ADDR_WIDTH-1 : 0]  addr,
    input [DATA_WIDTH-1 : 0]  din,
    output [DATA_WIDTH-1 : 0] dout,

    input                     bist_start,
    output                    bist_working,
    output                    bist_finish,
    output                    bist_failed
);

    // --------------------- BIST Signal --------------------- //
    wire bist_we;
    wire [ADDR_WIDTH-1 : 0] bist_addr;
    wire [DATA_WIDTH-1 : 0] bist_din;

    // --------------------- SRAM init --------------------- //
    sram #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH)) u_sram (
        .clk(clk),
        .mbist(bist_working),
        
        .csb(csb),
        .we(we),
        .addr(addr),
        .din(din),
        
        .bist_csb(1'b0),
        .bist_we(bist_we),
        .bist_addr(bist_addr),
        .bist_din(bist_din),
        
        .dout(dout)
    );

    bist #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH)) u_bist (
        .clk(clk),
        .rst(rst),

        .bist_start(bist_start),
        .bist_working(bist_working),
        .bist_finish(bist_finish),
        .bist_failed(bist_failed),

        .bist_we(bist_we),
        .bist_addr(bist_addr),
        .bist_din(bist_din)
    );

endmodule