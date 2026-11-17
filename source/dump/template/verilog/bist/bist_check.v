module bist_check #(
    parameter ADDR_WIDTH,
    parameter DATA_WIDTH
) (
    input                     rst,

    input                     bist_clk,
    input                     bist_reset,

    input [2 : 0]             bist_state,
    input                     bist_din_bit,
    input                     bist_we,

    input [ADDR_WIDTH-1 : 0]  sram_dout,

    output                    bist_failed
);
    // --------------------- Parameter --------------------- //
    localparam MARCH_0 = 3'b000;   // Write 0 from low address
    localparam MARCH_1 = 3'b001;   // Read 0 and then write 1 from low address
    localparam MARCH_2 = 3'b010;   // Read 1 and then write 0 from high address
    localparam MARCH_3 = 3'b011;   // Read 0 and then write 1 from high address 
    localparam MARCH_4 = 3'b100;   // Read 1 and then write 0 from low address  
    localparam MARCH_FINISH = 3'b101;

    localparam ZERO_WDATA = {ADDR_WIDTH{1'b0}};
    localparam ONE_WDATA  = {ADDR_WIDTH{1'b1}};
    
    // --------------------- BIST Signal --------------------- //
    reg bist_failed_reg;

    always @(posedge bist_clk or negedge rst) begin
        if (rst == 1'b0 || bist_reset == 1'b1) begin
            bist_failed_reg <= 1'b0;
        end else begin
            // No need to check
            if (bist_state == MARCH_FINISH || bist_state == MARCH_0 || bist_we == 1'b0)
                bist_failed_reg <= bist_failed_reg;
            else begin
                // Check 0
                if (bist_din_bit == 1'b1) begin
                    if (sram_dout == ZERO_WDATA) bist_failed_reg <= bist_failed_reg & 1'b0;
                    else                          bist_failed_reg <= bist_failed_reg & 1'b1;
                end
                // Check 1
                else if (bist_din_bit == 1'b0) begin
                    if (sram_dout == ONE_WDATA)  bist_failed_reg <= bist_failed_reg & 1'b0;
                    else                          bist_failed_reg <= bist_failed_reg & 1'b1;
                end
            end
        end
    end

    assign bist_failed = bist_failed_reg;

endmodule