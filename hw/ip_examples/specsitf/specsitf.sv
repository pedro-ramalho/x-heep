module specsitf #(
    parameter type reg_req_t  = logic,
    parameter type reg_rsp_t  = logic,
    parameter type obi_req_t  = logic,
    parameter type obi_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  masters_req_o,
    input  obi_resp_t masters_resp_i,

    input  obi_req_t  slave_req_i,
    output obi_resp_t slave_resp_o
);

  assign reg_rsp_o.rdata = '0;
  assign reg_rsp_o.error = 1'b0;
  assign reg_rsp_o.ready = 1'b1;
  assign reg_rsp_o.rdata = 32'b0;

  assign masters_req_o.req   = '0;
  assign masters_req_o.addr  = '0;
  assign masters_req_o.we    = '0;
  assign masters_req_o.be    = '0;
  assign masters_req_o.wdata = '0;
  assign slave_resp_o.gnt    = '0;
  ;
  assign slave_resp_o.rdata = '0;
  ;
  assign slave_resp_o.rvalid = '0;
  ;

  import "DPI-C" function void test_specsitf(int data);

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (rst_ni) begin
      if (reg_req_i.valid && reg_req_i.write) begin
        test_specsitf(reg_req_i.wdata);
      end
    end
  end

endmodule
