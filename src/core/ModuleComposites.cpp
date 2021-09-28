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

#include <CGRA/ModuleComposites.h>
#include <ios>

MemPort::MemPort(std::string name, int num_connections, unsigned size)
    : Module(name, size)
{
    mt = MOD_COMPOSITE;

    assert(num_connections >= 2);
    this->num_connections = num_connections;

    // add constant ports (always will have these ports)
    addPort("out", PORT_OUTPUT);

    // make all input ports
    for(int i = 0; i < num_connections; i++)
    {
        addPort("in" + std::to_string(i), PORT_INPUT);
    };

    // add FU for loads/stores
    addSubModule(new MEMUnit("mem_unit"));

    // add input muxs
    addSubModule(new Multiplexer("mux_addr", num_connections));
    addSubModule(new Multiplexer("mux_data", num_connections));
    addConfig(new ConfigCell("MuxAddr"), {"mux_addr.select"});
    addConfig(new ConfigCell("MuxData"), {"mux_data.select"});

    // make all mux input connections
    for(int i = 0; i < num_connections; i++)
    {
        addConnection("this.in" + std::to_string(i), "mux_data.in" + std::to_string(i));
        addConnection("this.in" + std::to_string(i), "mux_addr.in" + std::to_string(i));
    }

    // connections to the mem unit
    addConnection("mux_addr.out", "mem_unit.addr");
    addConnection("mux_data.out", "mem_unit.data_in");
    addConnection("mem_unit.data_out", "this.out");
    addConfig(new ConfigCell("WriteRq"), {"mem_unit.w_rq"});
}

std::string MemPort::GenericName()
{
    return "memoryPort_" + std::to_string(num_connections) + "connect_" + std::to_string(getSize()) + "b";
}

MemPort::~MemPort()
{
}

IOPort::IOPort(std::string name, int num_inputs, unsigned size)
    : Module(name, size)
{
    mt = MOD_COMPOSITE;

    assert(num_inputs >= 1);
    this->num_inputs = num_inputs;

    // add constant ports (always will have these ports)
    addPort("out", PORT_OUTPUT, size);
    for(int i = 0; i < num_inputs; i++)
    {
        addPort("in" + std::to_string(i), PORT_INPUT, size);
    };
    addPort("bidir", PORT_BIDIR, size);

    addSubModule(new IO("io_unit", size));
    addConnection("io_unit.bidir", "this.bidir");

    // add connection(s) + mux to io unit
    if(num_inputs == 1)
    {
        addConnection("this.in0", "io_unit.in");
        addConnection("io_unit.out", "this.out");
    }
    else
    {
        addSubModule(new Multiplexer("mux_in", num_inputs, size));

        for(int i = 0; i < num_inputs; i++)
        {
            addConnection("this.in" + std::to_string(i), "mux_in.in" + std::to_string(i));
        }

        addConnection("mux_in.out", "io_unit.in");
        addConnection("io_unit.out", "this.out");

        addConfig(new ConfigCell("MuxConfig"), {"mux_in.select"});
    }
}

std::string IOPort::GenericName()
{
    return "ioPort_" + std::to_string(num_inputs) + "in";
}

IOPort::~IOPort()
{
}


static const std::map<std::string, CGRABlockType> block_type_map =
{
    {"no-bypass", STANDARD_NOBYPASS},
    {"bypass", STANDARD_BYPASS},
    {"diagonal", STANDARD_DIAGONAL},
};

std::istream& operator >>(std::istream &is, CGRABlockType &type)
{
    std::string str;
    is >> str;
    if (block_type_map.count(str) > 0)
        type = block_type_map.at(str);
    else
        is.setstate(std::ios_base::failbit);

    return is;
}

SimpleFU::SimpleFU(std::string name, CGRABlockType BlockType)
    : Module(name)
{
    this->BlockType = BlockType;
    mt = MOD_COMPOSITE;

    if (BlockType == STANDARD_NOBYPASS)
    {
        // add input muxs
        addSubModule(new Multiplexer("mux_a", 5));
        addSubModule(new Multiplexer("mux_b", 5));
        // add FU
        addSubModule(new FuncUnit("func"));
        // add reg
        addSubModule(new Register("register"));
        // add output mux
        addSubModule(new Multiplexer("mux_out", 2));
        // config cells
        addConfig(new ConfigCell("MuxAConfig"), {"mux_a.select"});
        addConfig(new ConfigCell("MuxBConfig"), {"mux_b.select"});
        addConfig(new ConfigCell("MuxOutConfig"), {"mux_out.select"});
        addConfig(new ConfigCell("FuncConfig"), {"func.select"});
        // add all ports
        addPort("in0", PORT_INPUT);
        addPort("in1", PORT_INPUT);
        addPort("in2", PORT_INPUT);
        addPort("in3", PORT_INPUT);
        addPort("out", PORT_OUTPUT);
        // to mux_a
        addConnection("this.in0", "mux_a.in0");
        addConnection("this.in1", "mux_a.in1");
        addConnection("this.in2", "mux_a.in2");
        addConnection("this.in3", "mux_a.in3");
        addConnection("mux_out.out", "mux_a.in4");
        // to mux_b
        addConnection("this.in0", "mux_b.in0");
        addConnection("this.in1", "mux_b.in1");
        addConnection("this.in2", "mux_b.in2");
        addConnection("this.in3", "mux_b.in3");
        addConnection("mux_out.out", "mux_b.in4");
        // to funcunit
        addConnection("mux_a.out", "func.in_a");
        addConnection("mux_b.out", "func.in_b");
        // to reg
        addConnection("func.out", "register.in");
        // to output mux
        addConnection("func.out", "mux_out.in0");
        addConnection("register.out", "mux_out.in1");
        // to output
        addConnection("mux_out.out", "this.out");
    }
    else if (BlockType == STANDARD_BYPASS)
    {
        // add input muxs
        addSubModule(new Multiplexer("mux_a", 5));
        addSubModule(new Multiplexer("mux_b", 5));
        // add FU
        addSubModule(new FuncUnit("func"));
        // add bypass mux
        addSubModule(new Multiplexer("mux_bypass", 3));
        // add reg
        addSubModule(new Register("register"));
        // add output mux
        addSubModule(new Multiplexer("mux_out", 2));
        // config cells
        addConfig(new ConfigCell("MuxAConfig"), {"mux_a.select"});
        addConfig(new ConfigCell("MuxBConfig"), {"mux_b.select"});
        addConfig(new ConfigCell("MuxBypassConfig"), {"mux_bypass.select"});
        addConfig(new ConfigCell("MuxOutConfig"), {"mux_out.select"});
        addConfig(new ConfigCell("FuncConfig"), {"func.select"});
        // add all ports
        addPort("in0", PORT_INPUT);
        addPort("in1", PORT_INPUT);
        addPort("in2", PORT_INPUT);
        addPort("in3", PORT_INPUT);
        addPort("out", PORT_OUTPUT);
        // to mux_a
        addConnection("this.in0", "mux_a.in0");
        addConnection("this.in1", "mux_a.in1");
        addConnection("this.in2", "mux_a.in2");
        addConnection("this.in3", "mux_a.in3");
        addConnection("register.out", "mux_a.in4");
        // to mux_b
        addConnection("this.in0", "mux_b.in0");
        addConnection("this.in1", "mux_b.in1");
        addConnection("this.in2", "mux_b.in2");
        addConnection("this.in3", "mux_b.in3");
        addConnection("register.out", "mux_b.in4");
        // to funcunit
        addConnection("mux_a.out", "func.in_a");
        addConnection("mux_b.out", "func.in_b");
        // to bypass
        addConnection("mux_a.out", "mux_bypass.in0");
        addConnection("mux_b.out", "mux_bypass.in1");
        addConnection("func.out", "mux_bypass.in2");
        // to reg
        addConnection("mux_bypass.out", "register.in");
        // to output mux
        addConnection("mux_bypass.out", "mux_out.in0");
        addConnection("register.out", "mux_out.in1");
        // to output
        addConnection("mux_out.out", "this.out");
    }
    else if (BlockType == STANDARD_DIAGONAL)
    {
        // add input muxs
        addSubModule(new Multiplexer("mux_a", 9));
        addSubModule(new Multiplexer("mux_b", 9));
        // add FU
        addSubModule(new FuncUnit("func"));
        // add bypass mux
        addSubModule(new Multiplexer("mux_bypass", 3));
        // add reg
        addSubModule(new Register("register"));
        // add output mux
        addSubModule(new Multiplexer("mux_out", 2));
        // config cells
        addConfig(new ConfigCell("MuxAConfig"), {"mux_a.select"});
        addConfig(new ConfigCell("MuxBConfig"), {"mux_b.select"});
        addConfig(new ConfigCell("MuxBypassConfig"), {"mux_bypass.select"});
        addConfig(new ConfigCell("MuxOutConfig"), {"mux_out.select"});
        addConfig(new ConfigCell("FuncConfig"), {"func.select"});
        // add all ports
        addPort("in0", PORT_INPUT);
        addPort("in1", PORT_INPUT);
        addPort("in2", PORT_INPUT);
        addPort("in3", PORT_INPUT);
        addPort("in4", PORT_INPUT);
        addPort("in5", PORT_INPUT);
        addPort("in6", PORT_INPUT);
        addPort("in7", PORT_INPUT);
        addPort("out", PORT_OUTPUT);
        // to mux_a
        addConnection("this.in0", "mux_a.in0");
        addConnection("this.in1", "mux_a.in1");
        addConnection("this.in2", "mux_a.in2");
        addConnection("this.in3", "mux_a.in3");
        addConnection("this.in4", "mux_a.in4");
        addConnection("this.in5", "mux_a.in5");
        addConnection("this.in6", "mux_a.in6");
        addConnection("this.in7", "mux_a.in7");
        addConnection("register.out", "mux_a.in8");
        // to mux_b
        addConnection("this.in0", "mux_b.in0");
        addConnection("this.in1", "mux_b.in1");
        addConnection("this.in2", "mux_b.in2");
        addConnection("this.in3", "mux_b.in3");
        addConnection("this.in4", "mux_b.in4");
        addConnection("this.in5", "mux_b.in5");
        addConnection("this.in6", "mux_b.in6");
        addConnection("this.in7", "mux_b.in7");
        addConnection("register.out", "mux_b.in8");
        // to funcunit
        addConnection("mux_a.out", "func.in_a");
        addConnection("mux_b.out", "func.in_b");
        // to bypass
        addConnection("mux_a.out", "mux_bypass.in0");
        addConnection("mux_b.out", "mux_bypass.in1");
        addConnection("func.out", "mux_bypass.in2");
        // to reg
        addConnection("mux_bypass.out", "register.in");
        // to output mux
        addConnection("mux_bypass.out", "mux_out.in0");
        addConnection("register.out", "mux_out.in1");
        // to output
        addConnection("mux_out.out", "this.out");
    }
    else
    {
        std::cout << "ERROR, unintended mode\n";
    }
}

std::string SimpleFU::GenericName()
{
    return "cgrablock_bt" + std::to_string((int)(BlockType));
}

SimpleFU::~SimpleFU()
{
}

