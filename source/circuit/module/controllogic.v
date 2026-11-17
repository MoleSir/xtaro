module control_logic (
    input  clk,
    input  csb,
    input  we,
    input  rbl,
    output wl_en,
    output p_en_bar,
    output sa_en,
    output we_en
);

    wire clk_bar;
    wire we_bar;
    wire csb_bar;
    wire rbl_bar;
    wire sa_net1;
    wire sa_net2;
    wire wl_net1;
    wire wl_net2;

    assign clk_bar = ~clk;
    assign we_bar = ~we;
    assign csb_bar = ~csb;
    assign rbl_bar = ~rbl;

    assign p_en_bar = ~(csb_bar & we_bar & clk);

    assign we_en = csb_bar & we;
    
    assign sa_net1 = csb_bar & we_bar;
    assign sa_net2 = rbl_bar & clk_bar;
    assign sa_en = sa_net1 & sa_net2;

    assign wl_net1 = csb_bar & we;
    assign wl_net2 = csb_bar & clk_bar & rbl;
    assign wl_en = wl_net1 | wl_net2;

    
endmodule