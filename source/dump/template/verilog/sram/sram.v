module sram #(
    parameter ADDR_WIDTH,
    parameter DATA_WIDTH
) (
    input                     clk,
    input                     mbist,

    input                     csb,
    input                     we,
    input [ADDR_WIDTH-1 : 0]  addr,
    input [DATA_WIDTH-1 : 0]  din,

    input                     bist_csb,
    input                     bist_we,
    input [ADDR_WIDTH-1 : 0]  bist_addr,
    input [DATA_WIDTH-1 : 0]  bist_din,

    output [DATA_WIDTH-1 : 0] dout
);

    // ------------------------ Memory ------------------------ //
    reg [DATA_WIDTH-1 : 0] memory [2**(ADDR_WIDTH)-1 : 0];

    // ------------------------ Register ------------------------ //
    reg                    csb_reg;
    reg                    we_reg;
    reg [ADDR_WIDTH-1 : 0] addr_reg;
    reg [DATA_WIDTH-1 : 0] din_reg;
    reg [DATA_WIDTH-1 : 0] dout_reg;


    // ------------------------ Register ------------------------ //
    wire                    sram_csb;
    wire                    sram_we;
    wire [ADDR_WIDTH-1 : 0] sram_addr;
    wire [DATA_WIDTH-1 : 0] sram_din;

    assign sram_csb  = (mbist == 1'b0) ? csb   : bist_csb;
    assign sram_we   = (mbist == 1'b0) ? we    : bist_we;
    assign sram_addr = (mbist == 1'b0) ? addr  : bist_addr;
    assign sram_din  = (mbist == 1'b0) ? din : bist_din;

    always @(posedge clk) begin
        csb_reg  <= sram_csb;
        we_reg   <= sram_we;
        addr_reg <= sram_addr;
        din_reg  <= sram_din;
    end

    // ------------------------ Operation ----------------------- //
    always @(negedge clk) begin : read_operation
        if (csb_reg == 1'b0 && we_reg == 1'b0) begin
            dout_reg = memory[addr_reg];
        end
    end
    assign dout = dout_reg;
    
    always @(negedge clk) begin : write_operation
        if (csb_reg == 1'b0 && we_reg == 1'b1) begin
            memory[addr_reg] <= din_reg;
        end
    end

endmodule