module bist_state_machine #(
    parameter ADDR_WIDTH = 4,
    parameter DATA_WIDTH = 4
) (
    input                     rst,

    input                     bist_clk,
    input                     bist_reset,

    output                    bist_we,
    output [2 : 0]            bist_state,
    output [DATA_WIDTH-1 : 0] bist_wdata,
    output [ADDR_WIDTH-1 : 0] bist_addr,

    output                    bist_finish,
    output                    bist_working
);

    // --------------------- Parameter --------------------- //
    localparam MARCH_0 = 3'b000;   // Write 0 from low address
    localparam MARCH_1 = 3'b001;   // Read 0 and then write 1 from low address
    localparam MARCH_2 = 3'b010;   // Read 1 and then write 0 from low address
    localparam MARCH_3 = 3'b011;   // Read 0 and then write 1 from high address
    localparam MARCH_4 = 3'b100;   // Read 1 and then write 0 from high address 
    localparam MARCH_5 = 3'b101;   // Read 0 from low address
    localparam MARCH_FINISH = 3'b101;

    localparam ZERO_WDATA = {DATA_WIDTH{1'b0}};
    localparam ONE_WDATA  = {DATA_WIDTH{1'b1}};

    // --------------------- BIST Signal --------------------- //
    reg  bist_we_reg;
    reg  bist_we_next;

    reg  [ADDR_WIDTH-1 : 0] bist_addr_reg;
    reg  [ADDR_WIDTH-1 : 0] bist_addr_next;

    reg  [DATA_WIDTH-1 : 0] bist_wdata_reg;
    reg  [DATA_WIDTH-1 : 0] bist_wdata_next;
    
    reg  [2 : 0] bist_state_reg;
    reg  [2 : 0] bist_state_next;

    reg bist_finish_reg;
    reg bist_finish_next;

    reg bist_working_reg;
    reg bist_working_next;

    always @(posedge bist_clk or negedge rst) begin
        if (rst == 1'b0) begin
            bist_working_reg <= 1'b0;
            bist_finish_reg  <= 1'b0;
            bist_state_reg   <= MARCH_FINISH;

            bist_addr_reg    <= {ADDR_WIDTH{1'b0}};
            bist_wdata_reg   <= ZERO_WDATA;
            bist_we_reg      <= 1'b0;     
        end else if (bist_reset) begin
            // Signal reset
            bist_working_reg <= 1'b1;
            bist_finish_reg  <= 1'b0;
            bist_state_reg   <= MARCH_0;

            bist_addr_reg    <= {ADDR_WIDTH{1'b0}};
            bist_wdata_reg   <= ZERO_WDATA;
            bist_we_reg      <= 1'b1;
        end else begin
            // Update 
            bist_working_reg <= bist_working_next;
            bist_finish_reg  <= bist_finish_next;
            bist_state_reg  <= bist_state_next;

            bist_addr_reg   <= bist_addr_next;
            bist_wdata_reg  <= bist_wdata_next;
            bist_we_reg     <= bist_we_next;
        end
    end

    always @* begin : next_state_logic
        case (bist_state_reg)
        MARCH_0 : begin
            // Write '0' from low address
            if (bist_addr_reg == {ADDR_WIDTH{1'b1}}) begin
                // Finish MARCH 0, switch to MARCH 1
                bist_state_next   = MARCH_1;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = {ADDR_WIDTH{1'b0}};
                bist_wdata_next   = ONE_WDATA;
                bist_we_next      = 1'b0;
            end else begin
                // Continue write 0
                bist_state_next   = MARCH_0;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next   = bist_addr_reg + 1'b1;
                bist_wdata_next  = ZERO_WDATA;
                bist_we_next     = 1'b1;
            end
        end

        MARCH_1 : begin
            // Read 0 and then write 1 from low address
            if (bist_addr_reg == {ADDR_WIDTH{1'b1}} && bist_we_reg == 1'b1) begin
                // Finish MARCH 1, switch to MARCH 2
                bist_state_next   = MARCH_2;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = {ADDR_WIDTH{1'b0}};
                bist_wdata_next   = ZERO_WDATA;
                bist_we_next      = 1'b0;
            end else if (bist_we_reg == 1'b0) begin
                // Read a '0', now write this address to '1'
                bist_state_next   = MARCH_1;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = bist_addr_reg;
                bist_wdata_next   = ONE_WDATA;
                bist_we_next      = 1'b1;
            end else begin
                // Write a '1', now read next address
                bist_state_next   = MARCH_1;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = bist_addr_reg + 1'b1;
                bist_wdata_next   = ONE_WDATA;
                bist_we_next      = 1'b0;
            end
        end

        MARCH_2 : begin
            // Read 1 and then write 0 from low address
            if (bist_addr_reg == {ADDR_WIDTH{1'b1}} && bist_we_reg == 1'b1) begin
                // Finish MARCH 2, switch to MARCH 3
                bist_state_next   = MARCH_3;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = {ADDR_WIDTH{1'b1}};
                bist_wdata_next   = ONE_WDATA;
                bist_we_next      = 1'b0;
            end else if (bist_we_reg == 1'b0) begin
                // Read a '1', now write this address to '0'
                bist_state_next   = MARCH_2;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = bist_addr_reg;
                bist_wdata_next   = ZERO_WDATA;
                bist_we_next      = 1'b1;
            end else begin
                // Write a '0', now read next address
                bist_state_next   = MARCH_2;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = bist_addr_reg + 1'b1;
                bist_wdata_next   = ZERO_WDATA;
                bist_we_next      = 1'b0;
            end
        end

        MARCH_3 : begin
            // Read 0 and then write 1 from high address
            if (bist_addr_reg == {ADDR_WIDTH{1'b0}} && bist_we_reg == 1'b1) begin
                // Finish MARCH 2, switch to MARCH 3
                bist_state_next   = MARCH_4;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = {ADDR_WIDTH{1'b1}};
                bist_wdata_next   = ZERO_WDATA;
                bist_we_next      = 1'b0;
            end else if (bist_we_reg == 1'b0) begin
                // Read a '0', now write this address to '1'
                bist_state_next   = MARCH_3;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next   = bist_addr_reg;
                bist_wdata_next  = ONE_WDATA;
                bist_we_next     = 1'b1;
            end else begin
                // Write a '0', now read next address
                bist_state_next   = MARCH_3;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = bist_addr_reg - 1'b1;
                bist_wdata_next   = ONE_WDATA;
                bist_we_next      = 1'b0;
            end
        end
        
        MARCH_4 : begin
            // Read 1 and then write 0 from high address 
            if (bist_addr_reg == {ADDR_WIDTH{1'b0}} && bist_we_reg == 1'b1) begin
                // Finish MARCH 2, switch to MARCH 3
                bist_state_next   = MARCH_5;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = {ADDR_WIDTH{1'b0}};
                bist_wdata_next   = ZERO_WDATA;
                bist_we_next      = 1'b0;
            end else if (bist_we_reg == 1'b0) begin
                // Read a '1', now write this address to '0'
                bist_state_next   = MARCH_4;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next    = bist_addr_reg;
                bist_wdata_next   = ZERO_WDATA;
                bist_we_next      = 1'b1;
            end else begin
                // Write a '0', now read next address
                bist_state_next  = MARCH_4;
                bist_finish_next = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next   = bist_addr_reg - 1'b1;
                bist_wdata_next  = ZERO_WDATA;
                bist_we_next     = 1'b0;
            end
        end

        MARCH_5 : begin
            // Read '0' from low address
            if (bist_addr_reg == {ADDR_WIDTH{1'b1}}) begin
                // Finish MARCH 5
                bist_state_next   = MARCH_FINISH;
                bist_finish_next  = 1'b1;
                bist_working_next = 1'b0;
    
                bist_addr_next    = {ADDR_WIDTH{1'b0}};
                bist_wdata_next   = ONE_WDATA;
                bist_we_next      = 1'b0;
            end else begin
                // Continue read 0
                bist_state_next   = MARCH_5;
                bist_finish_next  = 1'b0;
                bist_working_next = 1'b1;
    
                bist_addr_next   = bist_addr_reg + 1'b1;
                bist_wdata_next  = ZERO_WDATA;
                bist_we_next     = 1'b0;
            end
        end
        
        MARCH_FINISH : begin
            bist_state_next   = MARCH_FINISH;
            bist_finish_next  = 1'b1;
            bist_working_next = 1'b0;

            bist_addr_next    = {ADDR_WIDTH{1'b0}};
            bist_wdata_next   = ZERO_WDATA;
            bist_we_next      = 1'b0;
        end

        endcase
    end

    assign bist_we = bist_we_reg;
    assign bist_state = bist_state_reg;
    assign bist_wdata = bist_wdata_reg;
    assign bist_addr = bist_addr_reg;
    assign bist_finish  = bist_finish_reg;
    assign bist_working = bist_working_reg;

endmodule