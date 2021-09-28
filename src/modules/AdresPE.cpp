/*******************************************************************************
 * CGRA-ME Software End-User License Agreement
 *
 * The software programs comprising "CGRA-ME" and the documentation provided
 * with them are copyright by its authors S. Chin, K. Niu, N. Sakamoto, J. Zhao,
 * A. Rui, S. Yin, A. Mertens, J. Anderson, and the University of Toronto. Users
 * agree to not redistribute the software, in source or binary form, to other
 * persons or other institutions. Users may modify or use the source code for
 * other non-commercial, not-for-profit research endeavours, provided that all
 * copyright attribution on the source code is retained, and the original or
 * modified source code is not redistributed, in whole or in part, or included
 * in or with any commercial product, except by written agreement with the
 * authors, and full and complete attribution for use of the code is given in
 * any resulting publications.
 *
 * Only non-commercial, not-for-profit use of this software is permitted. No
 * part of this software may be incorporated into a commercial product without
 * the written consent of the authors. The software may not be used for the
 * design of a commercial electronic product without the written consent of the
 * authors. The use of this software to assist in the development of new
 * commercial CGRA architectures or commercial soft processor architectures is
 * also prohibited without the written consent of the authors.
 *
 * This software is provided "as is" with no warranties or guarantees of
 * support.
 *
 * This Agreement shall be governed by the laws of Province of Ontario, Canada.
 *
 * Please contact Prof. Anderson if you are interested in commercial use of the
 * CGRA-ME framework.
 ******************************************************************************/

#include <CGRA/user-inc/UserModules.h>

std::string AdresPE::GenericName()
{
    return "adres_" + std::to_string(num_inputs) + "in_" + fu_type;
}

AdresPE::AdresPE(std::string name, int num_inputs, std::string fu_type)
    : Module(name),
    fu_type(fu_type) // specifies type of FU
{
    this->num_inputs = num_inputs;

    // define op lists for types for functional units
    using OpGraphVec = std::vector<OpGraphOpCode>;
    std::map<std::string, OpGraphVec> fu_oplist;
    fu_oplist["vliw"] = {
      OPGRAPH_OP_ADD,
      OPGRAPH_OP_MUL,
      OPGRAPH_OP_SUB,
      OPGRAPH_OP_DIV,
      OPGRAPH_OP_AND,
      OPGRAPH_OP_OR,
      OPGRAPH_OP_XOR,
      OPGRAPH_OP_SHL,
      OPGRAPH_OP_SHRA,
      OPGRAPH_OP_SHRL};
    fu_oplist["cga"] = {
      OPGRAPH_OP_ADD,
      OPGRAPH_OP_SUB};

    mt = MOD_COMPOSITE;

    // Instantiate submodules
    addSubModule(new Multiplexer("mux_a", num_inputs + 2));  // mux for FU input "A" (+2 bc of const, rf)
    addSubModule(new Multiplexer("mux_b", num_inputs + 2));  // mux for FU input "B" (+2 bc of const, mux_out)
    addSubModule(new Multiplexer("mux_bypass", num_inputs)); // bypass mux for all inputs
    addSubModule(new ConstUnit("const"));                    // const unit
    addSubModule(new FuncUnit("func",fu_oplist[fu_type]));   // choose supported ops according to 'fu_type'
    addSubModule(new Multiplexer("mux_out", 2)); // mux for output (choose bypass or fu)

    // Instantiate configuration cells
    addConfig(new ConfigCell("MuxAConfig"), {"mux_a.select"});
    addConfig(new ConfigCell("MuxBConfig"), {"mux_b.select"});
    addConfig(new ConfigCell("MuxBypassConfig"), {"mux_bypass.select"});
    addConfig(new ConfigCell("MuxOutConfig"), {"mux_out.select"});
    addConfig(new ConfigCell("FuncConfig"), {"func.select"});

    // Instantiate input & output ports
    for(int i = 0; i < num_inputs; i++)
        addPort("in" + std::to_string(i), PORT_INPUT);
    addPort("out", PORT_OUTPUT);

    // Register file input & output ports
    addPort("rf_to_muxa", PORT_INPUT); // input port for external RF into MUXA
    addPort("rf_to_muxout", PORT_INPUT); // input port for external RF into MUX_OUT
    addPort("fu_to_rf", PORT_OUTPUT); // output port for external RF from FU

    // Mux A connections
    for(int i = 0; i < num_inputs; i++)
        addConnection("this.in" + std::to_string(i), "mux_a.in" + std::to_string(i));

    addConnection("const.out", "mux_a.in" + std::to_string(num_inputs));
    addConnection("this.rf_to_muxa", "mux_a.in" + std::to_string(num_inputs + 1)); // rf connected to one input
    addConnection("mux_a.out", "func.in_a");

    // Mux B connections
    for(int i = 0; i < num_inputs; i++)
        addConnection("this.in" + std::to_string(i), "mux_b.in" + std::to_string(i));

    addConnection("const.out", "mux_b.in" + std::to_string(num_inputs));
    addConnection("mux_out.out", "mux_b.in" + std::to_string(num_inputs + 1));
    addConnection("mux_b.out", "func.in_b");

    // BYPASS MUX CONNECTIONS
    for(int i = 0; i < num_inputs; i++)
        addConnection("this.in" + std::to_string(i), "mux_bypass.in" + std::to_string(i));
    addConnection("mux_bypass.out", "mux_out.in1");

    // output muxing
    addConnection("this.rf_to_muxout", "mux_out.in0");
    addConnection("func.out", "this.fu_to_rf"); // connect registered output to rf
    addConnection("mux_out.out", "this.out");
}

