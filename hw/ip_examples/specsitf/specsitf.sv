module specsitf #(
    parameter type reg_req_t  = logic,
    parameter type reg_rsp_t  = logic,
    parameter type obi_req_t  = logic,
    parameter type obi_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,
    input reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
);

  logic [31:0] reg1, reg2, reg3;

  logic [31:0] ctrl = '0;
  logic [31:0] st = '0;

  import "DPI-C" function void specsitf_comm_init();
  import "DPI-C" function void specsitf_comm_free();
  import "DPI-C" function void specsitf_comm_send(
    int reg1,
    int reg2,
    int ctrl
  );
  import "DPI-C" function void specsitf_comm_recv(
    output int reg3,
    output int st
  );

  initial specsitf_comm_init();
  final specsitf_comm_free();

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      reg1 <= '0;
      reg2 <= '0;
      ctrl <= '0;
    end else if (reg_req_i.valid && reg_req_i.write) begin
      case (reg_req_i.addr[7:2])
        0: reg1 <= reg_req_i.wdata;
        1: reg2 <= reg_req_i.wdata;
        3: ctrl <= reg_req_i.wdata;
        default: ;
      endcase
    end
  end

  always_ff @(posedge clk_i) begin
    int tmp_reg3, tmp_st;
    specsitf_comm_send(reg1, reg2, ctrl);
    specsitf_comm_recv(tmp_reg3, tmp_st);
    reg3 <= tmp_reg3;
    st   <= tmp_st;
  end

  always_comb begin
    reg_rsp_o = '{ready: 1'b1, error: 1'b0, rdata: '0};
    if (reg_req_i.valid && !reg_req_i.write) begin
      case (reg_req_i.addr[7:2])
        0: reg_rsp_o.rdata = reg1;
        1: reg_rsp_o.rdata = reg2;
        2: reg_rsp_o.rdata = reg3;
        3: reg_rsp_o.rdata = st;
        4: reg_rsp_o.rdata = ctrl;
        default: reg_rsp_o.error = 1'b1;
      endcase
    end
  end
endmodule
