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
#include <unordered_set>
#include <unordered_map>

/************ Register **********/
Register::Register(std::string name, unsigned size)
    : Module(name, size)
{
    // Create ports
    addPort("in", PORT_INPUT, "size");
    addPort("out", PORT_OUTPUT_REG, "size");

    // Module type
    mt = MOD_PRIM_REG;
}

// Virtual function that overrides Module::GenericName. Returns generic name of the object
std::string Register::GenericName()
{
    return "register_" + std::to_string(getSize()) + "b";
}

// Virtual function that overrides Module::GetFunctionality. Generates the functionality for register
void Register::GenFunctionality() // Virtual function override
{
    std::cout << SET_INDENT << "always @(posedge CGRA_Clock, posedge CGRA_Reset)\n";
    std::cout << SET_DOUBLE_INDENT << "if (CGRA_Reset == 1)\n";
    std::cout << SET_TRIPLE_INDENT << "out <= 0;\n";
    std::cout << SET_DOUBLE_INDENT << "else\n";
    std::cout << SET_TRIPLE_INDENT << "out <= in;\n";
}

Register::~Register()
{
}

MRRG* Register::createMRRG(unsigned II = 1)
{
    MRRG* result = new MRRG(II);

    for(unsigned i = 0; i < II; i++)
    {
        // create nodes
        MRRGNode* in = new MRRGNode(this, i,"in");
        MRRGNode* reg = new MRRGNode(this, i,"reg");
        reg->latency = 1;
        reg->essential = true;
        MRRGNode* out = new MRRGNode(this, MOD_II(i+1), "out");

        if (II != 1)
        {
            MRRGNode* m_reg = new MRRGNode(this, MOD_II(i+1), "m_reg");
            MRRGNode* m_out = new MRRGNode(this, MOD_II(i+1), "m_out");

            result->nodes[MOD_II(i+1)]["m_reg"] = m_reg;
            result->nodes[MOD_II(i+1)]["m_out"] = m_out;
        }

        // add nodes to MRRG
        result->nodes[i]["in"]= in;
        result->nodes[i]["reg"]= reg;
        result->nodes[MOD_II(i+1)]["out"] = out;
    }
    for(unsigned i = 0; i < II; i++)
    {
        // create nodes
        MRRGNode* in = result->nodes[i]["in"];
        MRRGNode* reg = result->nodes[i]["reg"];
        MRRGNode* out = result->nodes[MOD_II(i+1)]["out"];

        if (II != 1)
        {
            MRRGNode* m_reg = result->nodes[MOD_II(i+1)]["m_reg"];
            MRRGNode* m_out = result->nodes[MOD_II(i+1)]["m_out"];
            MRRGNode* p_out = result->nodes[i]["out"];

            reg->fanout.push_back(m_reg);
            m_reg->fanin.push_back(reg);

            m_reg->fanout.push_back(out);
            out->fanin.push_back(m_reg);

            m_out->fanout.push_back(out);
            out->fanin.push_back(m_out);

            p_out->fanout.push_back(m_out);
            m_out->fanin.push_back(p_out);
        }
        else
        {
            reg->fanout.push_back(out);
            out->fanin.push_back(reg);
        }

        // create node connections
        in->fanout.push_back(reg);
        reg->fanin.push_back(in);
    }
    return result;
}

/************ Multiplexer **********/
Multiplexer::Multiplexer(std::string name, unsigned mux_size, unsigned size)
    : Module(name, size)
{

    assert(mux_size > 1); // A multiplexer must take in at least two inputs

    // Create ports
    for(unsigned i = 0; i < mux_size; i++)
        addPort("in" + std::to_string(i), PORT_INPUT, "size");

    addPort("out", PORT_OUTPUT_REG, "size");
    // select port should have size that is log base 2 of the mux size
    addPort("select", PORT_INPUT, ceil(log2(mux_size)));

    // Updating the mux size
    this->mux_size = mux_size;

    // Module type
    mt = MOD_PRIM_MUX;
}

// Virtual function that overrides Module::GenericName. Returns generic name of the object
std::string Multiplexer::GenericName() // Virtual function override
{
    return "mux_" + std::to_string(mux_size) + "to1_" + std::to_string(getSize()) + "b";
}

// Generates the case statement for the multiplexer
void Multiplexer::GenFunctionality() // Virtual function override
{
    std::cout << SET_INDENT << "always @(*)\n";
    std::cout << SET_DOUBLE_INDENT <<  "case (select)\n";
    for (unsigned i = 0; i < mux_size; ++i)
        std::cout << SET_TRIPLE_INDENT <<  i << ": out = in" << i << ";\n";
    std::cout << SET_TRIPLE_INDENT << "default: out = {size{1'bx}};\n";
    std::cout << SET_DOUBLE_INDENT << "endcase\n";
}

std::vector<BitSetting> Multiplexer::getBitConfig(
    const ConfigCell& ccell,
    const std::map<OpGraphOp*,std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
    const std::map<OpGraphVal*,std::set<MRRGNode*>>& mrrg_nodes_from_val_node
) const {
    const std::vector<std::regex> required_node_regexes {
        std::regex("\\.mux$"), std::regex("\\.out$"),
    };
    const std::regex in_regex("\\.in([0-9]+)$");

    std::unordered_set<int> inputs_used;

    for (const auto& val_node_and_mrrg_nodes : mrrg_nodes_from_val_node) {
        std::vector<bool> required_node_types_found;
        for (const auto& req_node_regex : required_node_regexes) {
            required_node_types_found.push_back(false);
            for (const auto& mrrg_node : val_node_and_mrrg_nodes.second) {
                std::smatch match_results;
                if (std::regex_search(mrrg_node->getHierarchyQualifiedName(), match_results, req_node_regex)) {
                    if (required_node_types_found.back()) {
                        throw cgrame_error("found a node that matched two required node regexes");
                    } else {
                        required_node_types_found.back() = true;
                    }
                }
            }
        }

        if (std::all_of(begin(required_node_types_found), end(required_node_types_found), [&](auto&& v) { return v; })) {
            for (const auto& mrrg_node : val_node_and_mrrg_nodes.second) {
                std::smatch match_results;
                std::regex_search(mrrg_node->getHierarchyQualifiedName(), match_results, in_regex);
                if (match_results.size() == 2) {
                    inputs_used.insert(stoi(match_results[1].str()));
                }
            }
        } else {
            // ignore the val
        }
    }

    if (inputs_used.empty()) {
        return {(size_t)std::lround(ceil(log2(mux_size))), BitSetting::DONT_CARE_PREFER_LOW};
    } else {
        if (inputs_used.size() != 1) {
            throw cgrame_error("Multiplexer must only have one input");
        } else {
            return bitsettings_from_int(*begin(inputs_used), (int)std::lround(ceil(log2(mux_size))));
        }
    }
}

MRRG* Multiplexer::createMRRG(unsigned II = 1)
{
    MRRG* result = new MRRG(II);

    for(unsigned i = 0; i < II; i++)
    {
        // create nodes
        MRRGNode* out = new MRRGNode(this, i, "out");
        out->pt = MUX_OUT;
        out->essential = true;

        result->nodes[i]["out"] = out;

        MRRGNode* mux = new MRRGNode(this, i, "mux");
        mux->essential = true;
        result->nodes[i]["mux"] = mux;

        mux->fanout.push_back(out);
        out->fanin.push_back(mux);

        for(unsigned j = 0; j < mux_size; j++)
        {
            MRRGNode* in = new MRRGNode(this, i, "in" + std::to_string(j));
            in->pt = MUX_IN;
            in->essential = true;
            result->nodes[i]["in" + std::to_string(j)] = in;

            // create node connections
            in->fanout.push_back(mux);
            mux->fanin.push_back(in);
        }

    }

    return result;
}

Multiplexer::~Multiplexer()
{
}

/************ RegisterFile **********/
RegisterFile::RegisterFile(std::string name, unsigned NumInputPorts, unsigned NumOutputPorts, unsigned Log2Registers, unsigned size)
    : Module(name, size)
{
    assert(Log2Registers > 0); // We assume a register file must have at least two registers

    // Copying over parameters
    this->NumInputPorts = NumInputPorts;
    this->NumOutputPorts = NumOutputPorts;
    this->Log2Registers = Log2Registers;

    // Generating input ports
    for (unsigned i = 0; i < NumInputPorts; i++)
    {
        addPort("in" + std::to_string(i), PORT_INPUT, "size", size); // input data
        addPort("address_in" + std::to_string(i), PORT_INPUT, "log2regs", Log2Registers); // input addresses
        addPort("WE" + std::to_string(i), PORT_INPUT, 1); // write enable
    }

    // Generating output ports
    for (unsigned i = 0; i < NumOutputPorts; i++)
    {
        addPort("out" + std::to_string(i), PORT_OUTPUT_REG, "size", size); // output ports/registers
        addPort("address_out" + std::to_string(i), PORT_INPUT, "log2regs", Log2Registers); // addresses for output
    }

    // module type: Register File
    mt = MOD_PRIM_RF;
}

// Generates a unique name for a register file,based on number of input ports and output ports
std::string RegisterFile::GenericName()
{
    return "registerFile_" + std::to_string(NumInputPorts) + "in_" + std::to_string(NumOutputPorts) + "out_" + std::to_string(getSize()) + "b";
}

// Virtual function override for printing parameters
std::string RegisterFile::PrintParameters()
{
    return "#(" + std::to_string(Log2Registers) + ", " + std::to_string(data_size) + ") ";
}

// Prints the register file and always block
// begin block for CGRA Reset MUST be named
// We must use either entirely blocking or nonblocking assignment statements in this always block, no mixes allowed
void RegisterFile::GenFunctionality()
{
    std::cout << "\n" << SET_INDENT << "// Setting the always blocks and inside registers\n";
    std::cout << SET_INDENT << "reg [size-1:0] register[2**log2regs-1:0];\n";
    std::cout << SET_INDENT << "always@(posedge CGRA_Clock, posedge CGRA_Reset)\n";
    std::cout << SET_DOUBLE_INDENT << "if(CGRA_Reset)\n";
    std::cout << SET_TRIPLE_INDENT << "begin : RESET\n";
    std::cout << SET_QUAD_INDENT << "integer i;\n";
    std::cout << SET_QUAD_INDENT << "for (i = 0; i < 2**log2regs; i = i+1)\n";
    std::cout << SET_PENTA_INDENT << "register[i] = 0;\n";
    std::cout << SET_TRIPLE_INDENT << "end\n";
    std::cout << SET_DOUBLE_INDENT << "else\n";
    std::cout << SET_TRIPLE_INDENT << "begin\n";
    for (unsigned i = 0; i < NumOutputPorts; i++)
        std::cout << SET_QUAD_INDENT << "out" << i << " = " << "register[address_out" << i << "];\n";
    for (unsigned i = 0; i < NumInputPorts; i++)
    {
        std::cout << SET_QUAD_INDENT << "if(WE" << i << ")\n";
        std::cout << SET_PENTA_INDENT << "register[address_in" << i << "] = in" << i << ";\n";
    }
    std::cout << SET_TRIPLE_INDENT << "end\n";
}

std::vector<BitSetting> RegisterFile::getBitConfig(
    const ConfigCell& ccell,
    const std::map<OpGraphOp*,std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
    const std::map<OpGraphVal*,std::set<MRRGNode*>>& mrrg_nodes_from_val_node
) const {
    const std::regex reg_regex("\\.reg([0-9]+)$");
    const std::regex in_regex("\\.in([0-9]+)$");
    const std::regex out_regex("\\.out([0-9]+)$");

    struct AssociatedPorts {
        std::unordered_set<int> inputs = {};
        std::unordered_set<int> outputs = {};

        AssociatedPorts& operator+=(const AssociatedPorts& rhs) {
            inputs.insert(begin(rhs.inputs), end(rhs.inputs));
            outputs.insert(begin(rhs.outputs), end(rhs.outputs));
            return *this;
        }
    };

    // std::unordered_map<int, AssociatedPorts> reg_to_port;

    std::unordered_map<int, int> in_to_reg;
    std::unordered_map<int, int> out_to_reg;
    std::unordered_set<int> iports_used;

    for (const auto& val_node_and_mrrg_nodes : mrrg_nodes_from_val_node) {
        AssociatedPorts ports;
        int reg_num = -1;
        for (const auto& mrrg_node : val_node_and_mrrg_nodes.second) {
            std::smatch match_results;
            std::regex_search(mrrg_node->getHierarchyQualifiedName(), match_results, reg_regex);
            if (match_results.size() == 2) {
                reg_num = stoi(match_results[1].str());
            } else {
                std::regex_search(mrrg_node->getHierarchyQualifiedName(), match_results, in_regex);
                if (match_results.size() == 2) {
                    ports.inputs.insert(stoi(match_results[1].str()));
                } else {
                    std::regex_search(mrrg_node->getHierarchyQualifiedName(), match_results, out_regex);
                    if (match_results.size() == 2) {
                        ports.outputs.insert(stoi(match_results[1].str()));
                    }
                }
            }
        }

        if (reg_num < 0) {
            throw cgrame_error("didn't find a regester MRRG node!");
        } else {
            // const auto insert_results = reg_to_port.insert(reg_num, ports);
            // if (not insert_results.second) {
            //     auto& their_ports = insert_results.first->second;
            //     their_ports += ports;
            // }
            for (const auto& iport : ports.inputs) {
                iports_used.insert(iport);
                if (not in_to_reg.emplace(iport, reg_num).second) {
                    throw cgrame_error("multiple fanin to output!");
                }
            }

            for (const auto& oport : ports.outputs) {
                if (not out_to_reg.emplace(oport, reg_num).second) {
                    throw cgrame_error("multiple fanin to output!");
                }
            }

        }
    }

    const std::regex addrIn_port_regex("^address_in([0-9]+)$");
    const std::regex writeEn_port_regex("^WE([0-9]+)$");
    const std::regex addrOut_port_regex("^address_out([0-9]+)$");


    {const std::string& port_name = ccell.getSingleConnectedPort().getName();
    std::smatch match_results;
    std::regex_search(port_name, match_results, writeEn_port_regex);
    if (match_results.size() == 2) {
        if (iports_used.find(stoi(match_results[1].str())) == end(iports_used)) {
            return { BitSetting::LOW };
        } else {
            return { BitSetting::HIGH };
        }
    } else {
        std::regex_search(port_name, match_results, addrIn_port_regex);
        if (match_results.size() == 2) {
            const auto& find_results = in_to_reg.find((stoi(match_results[1].str())));
            if (find_results == end(in_to_reg)) {
                return { Log2Registers, BitSetting::DONT_CARE_PREFER_LOW };
            } else {
                return bitsettings_from_int(find_results->second, Log2Registers);
            }
        } else {
            std::regex_search(port_name, match_results, addrOut_port_regex);
            if (match_results.size() == 2) {
                const auto& find_results = out_to_reg.find(stoi(match_results[1].str()));
                if (find_results == end(out_to_reg)) {
                    return { Log2Registers, BitSetting::DONT_CARE_PREFER_LOW };
                } else {
                    return bitsettings_from_int(find_results->second, Log2Registers);
                }
            } else {
                throw cgrame_error("unable to determine correct config value for " + port_name);
            }
        }
    }}
}

MRRG* RegisterFile::createMRRG(unsigned int II)
{
    MRRG* result = new MRRG(II);

    // Create all nodes
    for(unsigned i = 0; i < II; i++)
    {
        for(unsigned j = 0; j < NumInputPorts; j++)
        {
            MRRGNode* in = new MRRGNode(this, i, "in" + std::to_string(j));
            result->nodes[i]["in" + std::to_string(j)] = in;

            for(unsigned k = 0; k < (1 << Log2Registers); k++)
            {
                MRRGNode* regm = new MRRGNode(this, i, "reg" + std::to_string(k) + "_m" + std::to_string(j));
                result->nodes[i]["reg" + std::to_string(k) + "_m" + std::to_string(j)] = regm;

                in->fanout.push_back(regm);
                regm->fanin.push_back(in);
            }
        }

        for(unsigned j = 0; j < (1 << Log2Registers); j++)
        {
            MRRGNode* regfb = new MRRGNode(this, i, "reg" + std::to_string(j) + "_fb");
            result->nodes[i]["reg" + std::to_string(j) + "_fb"] = regfb;

            MRRGNode* reg = new MRRGNode(this, i, "reg" + std::to_string(j));
            reg->latency = 1;
            result->nodes[i]["reg" + std::to_string(j)] = reg;

            regfb->fanout.push_back(reg);
            reg->fanin.push_back(regfb);
        }

        for(unsigned j = 0; j < NumOutputPorts; j++)
        {
            MRRGNode* out = new MRRGNode(this, i, "out" + std::to_string(j));
            result->nodes[i]["out" + std::to_string(j)] = out;
            for(unsigned k = 0; k < (1 << Log2Registers); k++)
            {
                MRRGNode* outm = new MRRGNode(this, i, "out" + std::to_string(j) + "_m" + std::to_string(k));
                result->nodes[i]["out" + std::to_string(j) + "_m" + std::to_string(k)] = outm;

                outm->fanout.push_back(out);
                out->fanin.push_back(outm);
            }
        }
    }

    // Create all connections
    for(unsigned i = 0; i < II; i++)
    {
        // make all input connections
        // input port to reg connections
        for(unsigned j = 0; j < NumInputPorts; j++)
        {
            for(unsigned k = 0; k < (1 << Log2Registers); k++)
            {
                MRRGNode* reg =  result->nodes[i]["reg" + std::to_string(k)];
                MRRGNode* regm = result->nodes[i]["reg" + std::to_string(k) + "_m" + std::to_string(j)];

                regm->fanout.push_back(reg);
                reg->fanin.push_back(regm);
            }
        }

        // reg to reg connections
        for(unsigned k = 0; k < (1 << Log2Registers); k++)
        {
            MRRGNode* reg0 =  result->nodes[i]["reg" + std::to_string(k)];
            MRRGNode* reg1 =  result->nodes[MOD_II(i+1)]["reg" + std::to_string(k) + "_fb"];

            reg0->fanout.push_back(reg1);
            reg1->fanin.push_back(reg0);
        }

        // reg to outputs
        // all output ports will be on the next cycle
        for(unsigned j = 0; j < NumOutputPorts; j++)
        {
            for(unsigned k = 0; k < (1 << Log2Registers); k++)
            {
                MRRGNode* out = result->nodes[MOD_II(i+1)]["out" + std::to_string(j) + "_m" + std::to_string(k)];
                MRRGNode* reg = result->nodes[i]["reg" + std::to_string(k)];

                reg->fanout.push_back(out);
                out->fanin.push_back(reg);
            }
        }
    }

    return result;
}

// Destructor
RegisterFile::~RegisterFile()
{
}

