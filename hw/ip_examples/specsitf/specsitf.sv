module specsitf #(
    parameter type reg_req_t  = logic,
    parameter type reg_rsp_t  = logic,
    parameter type obi_req_t  = logic,
    parameter type obi_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
);

  logic [31:0] int_reg_1;
  logic [31:0] int_reg_2;

  assign reg_rsp_o.error = 1'b0;
  assign reg_rsp_o.ready = 1'b1;
  assign reg_rsp_o.rdata = int_reg_1;

  import "DPI-C" function void specsitf_comm_init();
  import "DPI-C" function void specsitf_comm_free();
  import "DPI-C" function void specsitf_comm_send(int int_reg_1, int int_reg_2);
  import "DPI-C" function void specsitf_comm_recv(output int int_reg_1, output int int_reg_2);

  initial begin
    specsitf_comm_init();
    int_reg_1 = 0;
    int_reg_2 = 0;
  end

  final begin
    specsitf_comm_free();
  end

  /* send block */
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
    end else begin
      specsitf_comm_send(int_reg_1, int_reg_2);
    end
  end

  /* recv block */
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
    end else begin
      specsitf_comm_recv(int_reg_1, int_reg_2);
    end
  end

endmodule
