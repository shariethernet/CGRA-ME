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

SimpleArchFU::SimpleArchFU(std::string name, int num_inputs, bool full_fu)
    : Module(name)
{
    mt = MOD_COMPOSITE;

    // add input muxs
    addSubModule(new Multiplexer("mux_a", num_inputs + 2)); // plus 2 for feedback and constant
    addSubModule(new Multiplexer("mux_b", num_inputs + 2));

    // for passthrough
    addSubModule(new Multiplexer("mux_bypass", num_inputs)); // for all eight inputs

    // add const unit
    addSubModule(new ConstUnit("const"));

    // add FU
    if(full_fu)
    {
        addSubModule(new FuncUnit(
              "func",
              {
              OPGRAPH_OP_ADD,
              OPGRAPH_OP_MUL,
              OPGRAPH_OP_SUB,
              OPGRAPH_OP_DIV,
              OPGRAPH_OP_AND,
              OPGRAPH_OP_OR,
              OPGRAPH_OP_XOR,
              OPGRAPH_OP_SHL,
              OPGRAPH_OP_SHRA,
              OPGRAPH_OP_SHRL}));
    }
    else
    {
        addSubModule(new FuncUnit(
              "func",
              {
              OPGRAPH_OP_ADD,
              OPGRAPH_OP_SUB,
              OPGRAPH_OP_AND,
              OPGRAPH_OP_OR,
              OPGRAPH_OP_XOR,
              OPGRAPH_OP_SHL,
              OPGRAPH_OP_SHRA,
              OPGRAPH_OP_SHRL}));
    }

    // add reg
    addSubModule(new Register("register"));

    // add fu output mux
    addSubModule(new Multiplexer("mux_out", 2)); // 2, 1 for bypass and one from the register output
    // config cells
    addConfig(new ConfigCell("MuxAConfig"), {"mux_a.select"});
    addConfig(new ConfigCell("MuxBConfig"), {"mux_b.select"});
    addConfig(new ConfigCell("MuxBypassConfig"), {"mux_bypass.select"});
    addConfig(new ConfigCell("MuxOutConfig"), {"mux_out.select"});
    addConfig(new ConfigCell("FuncConfig"), {"func.select"});

    // unit input ports
    for(int i = 0; i < num_inputs; i++)
    {
        addPort("in" + std::to_string(i), PORT_INPUT);
    }
    // output port
    addPort("out", PORT_OUTPUT);

    // to mux_a
    for(int i = 0; i < num_inputs; i++)
    {
        addConnection("this.in" + std::to_string(i), "mux_a.in" + std::to_string(i));
    }
    addConnection("mux_out.out", "mux_a.in" + std::to_string(num_inputs));
    addConnection("const.out", "mux_a.in" + std::to_string(num_inputs + 1));

    // to mux_b
    for(int i = 0; i < num_inputs; i++)
    {
        addConnection("this.in" + std::to_string(i), "mux_b.in" + std::to_string(i));
    }
    addConnection("mux_out.out", "mux_b.in" + std::to_string(num_inputs));
    addConnection("const.out", "mux_b.in" + std::to_string(num_inputs + 1));

    // to mux_bypass
    for(int i = 0; i < num_inputs; i++)
    {
        addConnection("this.in" + std::to_string(i), "mux_bypass.in" + std::to_string(i));
    }

    // to funcunit
    addConnection("mux_a.out", "func.in_a");
    addConnection("mux_b.out", "func.in_b");

    // to reg
    addConnection("func.out", "register.in");

    // to output mux
    addConnection("register.out", "mux_out.in0");
    addConnection("mux_bypass.out", "mux_out.in1");

    // to output
    addConnection("mux_out.out", "this.out");

}

