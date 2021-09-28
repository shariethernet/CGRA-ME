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

#include <CGRA/Module.h>

#include <regex>

/************ FuncUnit **********/
// Map that stores all possible modes of a FuncUnit
std::map<OpGraphOpCode, LLVMMode> FuncUnit::all_modes =
{
    // Adding functionality to the funcunit
    // You can add a mode to the FuncUnit by specifying all_modes.push_back ({"module name", {"functionality"}, "wire name that feeds into the multiplexer"});
    // Functionality can be multiple lines, hence the reason why it is a vector. Each string represents one line, hence the reason it is a vector
    {OPGRAPH_OP_ADD,    {"op_add",      "add",         {"assign c = a + b;"},      "add_sel"}},
    {OPGRAPH_OP_SUB,    {"op_sub",      "sub",         {"assign c = a - b;"},      "sub_sel"}},
    {OPGRAPH_OP_MUL,    {"op_multiply", "multiply",    {"assign c = a * b;"},      "mul_sel"}},
    {OPGRAPH_OP_DIV,    {"op_divide",   "divide",      {"assign c = a / b;"},      "div_sel"}},
    {OPGRAPH_OP_AND,    {"op_and",      "and",         {"assign c = a & b;"},      "and_sel"}},
    {OPGRAPH_OP_OR,     {"op_or",       "or",          {"assign c = a | b;"},      "or_sel"}},
    {OPGRAPH_OP_XOR,    {"op_xor",      "xor",         {"assign c = a ^ b;"},      "xor_sel"}},
    {OPGRAPH_OP_SHL,    {"op_shl",      "shl",         {"assign c = a << b;"},     "shl_sel"}},
    {OPGRAPH_OP_SHRL,   {"op_lshr",     "lshr",        {"assign c = a >> b;"},     "lshr_sel"}},
    {OPGRAPH_OP_SHRA,   {"op_ashr",     "ashr",        {"assign c = a >>> b;"},    "ashr_sel"}},
    {OPGRAPH_OP_CONST,  {"op_const",    "const",       {"//const;"},               "const_sel"}},
    {OPGRAPH_OP_LOAD,   {"op_load",     "load",        {"//load;"},                "load_sel"}},
    {OPGRAPH_OP_STORE,  {"op_store",    "store",       {"//store;"},               "store_sel"}},
};

// Returns a unique name for a funcunit
std::string FuncUnit::GenericName()
{
    std::string NameToReturn = "func_" + std::to_string(getSize()) + "b"; // Base name is "func"
    for (unsigned i = 0; i < supported_modes.size(); i++)
    {
        // Add a "_(module_name_inside)" to the name
        NameToReturn.append("_");
        NameToReturn.append(all_modes[supported_modes[i]].OpName);
    }
    return NameToReturn;
}

// FuncUnit constructor
FuncUnit::FuncUnit(std::string name, std::vector<OpGraphOpCode> supported_modes, unsigned size, int II, int latency)
    : Module(name, size)
{
    // Latency and II will be set here when the mapper supports different latency for funcunits

    // Module type
    mt = MOD_PRIM_FUNC;
    // Create the ports
    addPort("in_a", PORT_INPUT, "size");
    addPort("in_b", PORT_INPUT, "size");
    if (supported_modes.size() > 1) // If there is more than one mode, we need to add a select line, we will also have a mux, so make port "out" a reg
    {
        addPort("select", PORT_INPUT, ceil(log2(supported_modes.size())));
        addPort("out", PORT_OUTPUT_REG, "size");
    }
    else // Otherwise, "out" can just be a wire
    {
        addPort("out", PORT_OUTPUT, "size");
    }

    this->supported_modes = supported_modes;

    for (unsigned i = 0; i < supported_modes.size(); i++)
    {
        std::string width_suffix = "_" + std::to_string((int)size) + "b";
        addSubModule(new CustomModule(
            all_modes[supported_modes[i]].ModuleName + width_suffix,
            all_modes[supported_modes[i]].Functionality,
            size));
    }
}

// Virtual function that overrides Module::GetConnections. Generates the wires and submodules for FuncUnit
void FuncUnit::GenConnections() // Virtual function override
{
    PrintList WireList, SubmodList; // Buffers that will contain output
    WireList.add({"\n", SET_INDENT, "// Declaring wires to direct module output into multiplexer\n"}); // Wire comment
    SubmodList.add({"\n", SET_INDENT, "// Declaring the submodules\n"}); // Submodule list comment
    for (unsigned i = 0; i < supported_modes.size(); i++)
    {
        // Adding the required wire for the mode
        WireList.add({SET_INDENT, "wire [size-1:0] ", all_modes[supported_modes[i]].WireName, ";\n"});
        // Adding the required submodule for the mode
        SubmodList.add({
            SET_INDENT,
            all_modes[supported_modes[i]].ModuleName,
            " #(size) ",
            all_modes[supported_modes[i]].ModuleName,
            "(.a(in_a), .b(in_b), .c(",
            all_modes[supported_modes[i]].WireName, "));\n"});
    }
    // Printing out the buffers (wires and submodules)
    WireList.print();
    SubmodList.print();
}

std::vector<BitSetting> FuncUnit::getBitConfig(
    const ConfigCell& ccell,
    const std::map<OpGraphOp*,std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
    const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
) const {
    const auto bits_needed = std::lround(ceil(log2(supported_modes.size())));
    if (mrrg_nodes_from_op_node.empty()) {
        return { (size_t)bits_needed, BitSetting::DONT_CARE_PREFER_LOW };

    } else if (mrrg_nodes_from_op_node.size() == 1) {
        const auto find_result = std::find(begin(supported_modes), end(supported_modes), begin(mrrg_nodes_from_op_node)->first->opcode);
        if (find_result == end(supported_modes)) {
            throw cgrame_error("couldn't find op in supported modes list");
        } else {
            return bitsettings_from_int(std::distance(begin(supported_modes), find_result), bits_needed);
        }

    } else {
        throw cgrame_error("expect either 0 or 1 op nodes");
    }
}

// Virtual function that overrides Module::GetFunctionality. Generates the case statement for FuncUnit
void FuncUnit::GenFunctionality()
{
    if (supported_modes.size() > 1)
    {
        std::cout << "\n" << SET_INDENT << "always @*\n";
        std::cout << SET_DOUBLE_INDENT << "case (select)\n";
        for (unsigned i = 0; i < supported_modes.size(); i++)
            std::cout << SET_TRIPLE_INDENT << i << ": out = " << all_modes[supported_modes[i]].WireName << ";\n";
        std::cout << SET_TRIPLE_INDENT << "default: out = {size{1'bx}};\n";
        std::cout << SET_DOUBLE_INDENT << "endcase\n";
    }
    else if (supported_modes.size() == 1) // We do not want a case statement, because the select variable does not exist
    {
        std::cout << SET_INDENT << "assign out = " << all_modes[supported_modes[0]].WireName << ";\n";
    }
    else
    {
        std::cout << SET_INDENT << "// WARNING: func does not contain any submodules to do operations\n";
        std::cout << SET_INDENT << "assign out = {size{1'bx}};\n";
    }
}

FuncUnit::~FuncUnit()
{
}

MRRG* FuncUnit::createMRRG(unsigned II = 1)
{
    MRRG* result = new MRRG(II);

    assert(getII() > 0);
    for(unsigned i = 0; i < II; i+= getII())
    {
        // create nodes
        MRRGNode* in_a = new MRRGNode(this, i, "in_a");
        MRRGNode* in_b = new MRRGNode(this, i, "in_b");
        MRRGNode* fu  = new MRRGNode(this, i, "fu", MRRG_NODE_FUNCTION);
        fu->operand[0] = in_a;
        fu->operand[1] = in_b;
        for (unsigned i = 0; i < supported_modes.size(); i++)
        {
            fu->supported_ops.push_back(supported_modes[i]);
        }
        MRRGNode* m_in_a = new MRRGNode(this, i, "m_in_a");
        MRRGNode* m_in_b = new MRRGNode(this, i, "m_in_b");
        MRRGNode* m_out  = new MRRGNode(this, i, "m_out");
        MRRGNode* out  = new MRRGNode(this, i, "out");

        // add nodes to MRRG
        result->nodes[i]["in_a"] = in_a;
        result->nodes[i]["in_b"] = in_b;
        result->nodes[i]["fu"] = fu;
        result->nodes[i]["m_in_a"] = m_in_a;
        result->nodes[i]["m_in_b"] = m_in_b;
        result->nodes[i]["m_out"] = m_out;
        result->nodes[i]["out"] = out;
    }

    for(unsigned i = 0; i < II; i+= getII())
    {
        MRRGNode* in_a      = result->nodes[i]["in_a"];
        MRRGNode* in_b      = result->nodes[i]["in_b"];
        MRRGNode* fu        = result->nodes[i]["fu"];
        MRRGNode* m_in_a    = result->nodes[i]["m_in_a"];
        MRRGNode* m_in_b    = result->nodes[i]["m_in_b"];
        MRRGNode* out       = result->nodes[i]["out"];

        MRRGNode* m_out_next    = result->nodes[MOD_II(i + getLatency())]["m_out"];
        MRRGNode* out_next      = result->nodes[MOD_II(i + getLatency())]["out"];
#define connect(a,b) (a)->fanout.push_back(b); (b)->fanin.push_back(a);
        connect(in_a, fu);
        connect(in_b, fu);
        connect(fu, m_out_next);
        connect(m_out_next, out_next);

        connect(in_a, m_in_a);
        connect(in_b, m_in_b);

        connect(m_in_a, out);
        connect(m_in_b, out);
#undef connect
    }

    return result;
}

MEMUnit::MEMUnit(std::string name, unsigned size)
    : Module(name, size)
{
    // Create the ports
    addPort("addr", PORT_INPUT, "size"); //FIXME: size shouldn't be left as default
    addPort("data_in", PORT_INPUT, "size", size);
    addPort("data_out", PORT_OUTPUT, "size", size);
    addPort("w_rq", PORT_INPUT, 1);
}

std::string MEMUnit::GenericName()
{
    return "memUnit_" + std::to_string(getSize()) + "b";
}

void MEMUnit::GenFunctionality()
{
    std::cout << std::endl;
    std::cout << SET_INDENT << "// ALERT: This module is an unimplemented place holder.\n";
}

std::vector<BitSetting> MEMUnit::getBitConfig(
    const ConfigCell& ccell,
    const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
    const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
) const {
    static const std::vector<std::pair<OpGraphOpCode,std::regex>> node_name_checks {
        {OPGRAPH_OP_LOAD,  std::regex("\\.data_out$")},
        {OPGRAPH_OP_STORE, std::regex("\\.data_in$")},
    };

    if (mrrg_nodes_from_op_node.empty()) {
        return { BitSetting::DONT_CARE_PREFER_LOW };

    } else {
        const OpGraphOp& op = [&]() -> decltype(auto) {
            if (mrrg_nodes_from_op_node.size() == 1) {
                return *(begin(mrrg_nodes_from_op_node)->first);
            } else {
                throw cgrame_error("expected exactly one op node");
            }
        }();

        bool found_good_node = false;
        for (const auto& val_and_nodes : mrrg_nodes_from_val_node) {
            if (val_and_nodes.second.size() == 1) {
                for (const auto& opcode_and_regex : node_name_checks) {
                    std::smatch match_results;
                    const auto& node_name = (**begin(val_and_nodes.second)).getHierarchyQualifiedName();
                    if (std::regex_search(node_name, match_results, opcode_and_regex.second)) {
                        if (opcode_and_regex.first == op.opcode) {
                            found_good_node = true;
                        } else {
                            cgrame_error("found unexpected node: " + node_name);
                        }
                    }
                }
            } else {
                throw cgrame_error("expected exactly one mrrg node for the val");
            }
        }

        if (found_good_node) {
            return { from_bool<BitSetting>(op.opcode == OPGRAPH_OP_STORE) };
        } else {
            throw cgrame_error("didn't find expcted mrrg node in any val");
        }
    }
}

MRRG* MEMUnit::createMRRG(unsigned int II)
{
    MRRG* result = new MRRG(II);

    for(unsigned i = 0; i < II; i++)
    {
        // create FU node
        MRRGNode* fu  = new MRRGNode(this, i, "mem", MRRG_NODE_FUNCTION);
        fu->supported_ops.push_back(OPGRAPH_OP_LOAD);
        fu->supported_ops.push_back(OPGRAPH_OP_STORE);
        result->nodes[i]["mem"] = fu;

        // create input nodes and connections
        MRRGNode* addr = new MRRGNode(this, i, "addr");
        result->nodes[i]["addr"] = addr;
        fu->operand[0] = addr;

        MRRGNode* data_in = new MRRGNode(this, i, "data_in");
        result->nodes[i]["data_in"] = data_in;
        fu->operand[1] = data_in;

        addr->fanout.push_back(fu);
        fu->fanin.push_back(addr);
        data_in->fanout.push_back(fu);
        fu->fanin.push_back(data_in);

        // create output nodes and connections
        MRRGNode* data_out = new MRRGNode(this, MOD_II(i + 1), "data_out");
        result->nodes[MOD_II(i + 1)]["data_out"] = data_out;
        data_out->fanin.push_back(fu);
        fu->fanout.push_back(data_out);
    }

    return result;
}

MEMUnit::~MEMUnit()
{
}

ConstUnit::ConstUnit(std::string name, int size)
    : Module(name, size)
{
    addPort("out", PORT_OUTPUT, "size", size);

    // FIXME:
    // The second argument in the ConfigCell constructor below actually does
    // not make a Verilog connection to the port correctly. But this should
    // only affect Verilog generation flow. The actual Verilog connection
    // assignment is hardcoded in ConstUnit::GenFunctionality().
    addConfig(new ConfigCell("ConstVal"), {"this.out"});
}

std::string ConstUnit::GenericName()
{
    return "const_" + std::to_string(getSize()) + "b";
}

void ConstUnit::GenFunctionality()
{
    // FIXME:
    // Once Module::addConfig() is fixed to accept ports as connection, this
    // method should be removed.
    std::cout << std::endl;
    std::cout << SET_INDENT << "assign out = ConstVal_sig;";
    std::cout << std::endl;
}

std::vector<BitSetting> ConstUnit::getBitConfig(
    const ConfigCell& ccell,
    const std::map<OpGraphOp*,std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
    const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
) const {
    if (mrrg_nodes_from_val_node.empty()) {
        return {data_size, BitSetting::DONT_CARE_PREFER_LOW};
    } else {
        std::vector<BitSetting> result(data_size, BitSetting::LOW);
        result.front() = BitSetting::HIGH;
        result.back() = BitSetting::HIGH;
        return result;
    }
}

MRRG* ConstUnit::createMRRG(unsigned int II)
{
    MRRG* result = new MRRG(II);

    for(unsigned i = 0; i < II; i++)
    {
        // create FU node
        MRRGNode* fu  = new MRRGNode(this, i, "const", MRRG_NODE_FUNCTION);
        fu->supported_ops.push_back(OPGRAPH_OP_CONST);
        result->nodes[i]["const"] = fu;

        // create output nodes and connections
        MRRGNode* data_out = new MRRGNode(this, i, "out");
        result->nodes[i]["out"] = data_out;
        data_out->fanin.push_back(fu);
        fu->fanout.push_back(data_out);
    }

    return result;
}

ConstUnit::~ConstUnit()
{
}

