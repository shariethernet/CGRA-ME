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

#ifndef __MODULE__H__
#define __MODULE__H__

#include <unordered_map>
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <CGRA/BitSetting.h>
#include <CGRA/Exception.h>
#include <CGRA/MRRG.h>
#include <CGRA/OpGraph.h>

#define MOD_II(x) ((x) % II)
// Data types
#define STRING_MATRIX std::vector<std::vector<std::string>>
// For indenting the code
#define SET_INDENT "    "
#define SET_DOUBLE_INDENT "        "
#define SET_TRIPLE_INDENT "            "
#define SET_QUAD_INDENT "                "
#define SET_PENTA_INDENT "                    "
// If port is not connected, this is the default print message
#define UNINITIALIZED "/* A PORT IS UNCONNECTED */"
// Debug information
#define ON true
#define OFF false
// Determine if we want to print out debug messages
#define DEBUG_MODE OFF
// DEFAULT DATA SIZE FOR MODULES, can change here
#define DEFAULT_SIZE 32
// A port size of zero in memory indicates that it is parameterized. The actual port size will then be determined by the module size
#define PARAMETERIZED 0
// A very large number to indicate an error
#define ERROR_NUMBER 9999999

class Module;
class PrintList;

// Different types of ports (input, output, output reg, inout, etc)
typedef enum
{
    PORT_INPUT,
    PORT_OUTPUT,
    PORT_OUTPUT_REG,
    PORT_BIDIR,
    PORT_UNSPECIFIED,
} port_type;

std::istream& operator >>(std::istream &is, port_type &porttype);

// Different types of CGRA blocks
typedef enum
{
    STANDARD_NOBYPASS, // CGRA block, NSEW connection, no bypass mux enabled
    STANDARD_BYPASS, // CGRA block, NSEW connection, bypass mux exists
    STANDARD_DIAGONAL, // CGRA blocks that have diagonal as well as NSEW Connections
} CGRABlockType;

std::istream& operator >>(std::istream &is, CGRABlockType &blocktype);

// Port object: contains all information for a given port
typedef struct Port
{
    const std::string& getName() const { return name; }
    Module& getModule() const { return *parent; }

    std::string name; // Name of the port
    std::string parameter; // If the port is parameterized, this is the name of the parameter
    port_type pt; // The type of port
    unsigned size; // Size of the port
    Module* parent; // The module which the port belongs to (its parent)
} Port;

// Connection object: contains all information describing connections between a source port and multiple destination ports
typedef struct Connection
{
    std::string name; // Name of the connection object
    Port* src; // Source port
    std::vector<Port*> dst; // Vector of destination port(s)
} Connection;

// Configuration cell object - this stores information associated with a configuration cell
typedef struct ConfigCell
{
    ConfigCell(std::string name); // Constructor of the configuration cell; initializes the name

    void GenModuleVerilog(); // Generates the verilog for a configuration cell

    const std::string& getName() const { return name; }

    int getStorageSize() const { return cell_size; }

    Port& getSingleConnectedPort() const {
        if (connected_ports.size() != 1) { throw cgrame_error("trying to get single connected port on ConfigCell with multiple (or zero) ports"); }
        return *connected_ports.at(0);
    }

    const std::vector<Port*>& getAllConnectedPorts() const { return connected_ports; }

    // Variables
    std::vector<Port*> connected_ports; // All ports that the config cell is connected to
    unsigned cell_size; // Size (how many bits) does config cell contain
    std::string name; // Name of the cell
} ConfigCell;

// PrintList class: it's a buffer that is used for printing module contents (WireList, SubmodList, and AssignList are of type PrintList)
// This is used because we don't want to print its output immediately after its generated
class PrintList
{
    public:
        void add(std::vector<std::string> Data)
        {
            // Add information into the buffer, taking a vector of strings as input
            std::string ToAdd = "";
            for (unsigned i = 0; i < Data.size(); i++)
                ToAdd.append(Data[i]);
            ToPrint.push_back(ToAdd); // Adding the information into the buffer
        };
        void print() // Printing the buffer
        {
            for (unsigned i = 0; i < ToPrint.size(); i++)
                std::cout << ToPrint[i];
        };
        bool empty() // Check if the buffer is empty
        {
            return ToPrint.empty();
        }
        void pop() // Removes the last thing added into the buffer
        {
            ToPrint.pop_back();
        }
        std::string last()
        {
            return ToPrint[ToPrint.size() - 1];
        }
    private:
        std::vector<std::string> ToPrint; // Vector of strings that stores information in the buffer
};

// Types of modules
typedef enum module_type
{
    MOD_INVALID, // Invalid module; used for error checking
    MOD_COMPOSITE, // Module that contains other submodules (e.g. SimpleFU)
    MOD_PRIM_FUNC, // FuncUnit
    MOD_PRIM_REG, // Register
    MOD_PRIM_RF, // Register File
    MOD_PRIM_MUX, // Multiplexer
    MOD_PRIM_TRI, // Tristate buffer
    MOD_PRIM_IO, // IO device
    MOD_PRIM_CUSTOM, // Custom module used to implement 3-operand instructions (e.g. LLVM add)
    MOD_USER, // User defined module: currently unused
} module_type;

std::ostream& operator<<(std::ostream& os, const module_type& mtype);

class Module
{
    public:
        Module(std::string name, unsigned size = DEFAULT_SIZE);
        Module(std::string name, std::string template_name, unsigned size = DEFAULT_SIZE);
        virtual ~Module();

        // Other functions for debugging purposes
        void print();
        void print_dot();
        void print_ports();
        void print_connections();
        void print_submodules();
        void print_configcells();


        //Functions to add area and power estimates of a given module
        float power();
        float area();

        // Functions to modify a module
        bool addConfig(ConfigCell* c, std::vector<std::string> ConnectTo); // add a config cell (configcell, {ports to connect to})
        bool addSubModule(Module* m); // add a submodule (with pointer to module we are adding)
        // add connection ("submodulename.portname", "submodulename.portname") Also assumes that port widths are the same size, returns true if sucessful
        bool addConnection(std::string src, std::string dst);
        // adds a port ("portname", port type, size = DEFAULT_SIZE);
        bool addPort(std::string portname, port_type pt, unsigned size = DEFAULT_SIZE);
        // adds a parameterized port ("portname", port type, parameter name, parameter size), parameter size is used to specify default size for a parameter
        bool addPort(std::string portname, port_type pt, std::string ParameterName, unsigned size = DEFAULT_SIZE);


        // Function that returns ALL the modules that have config cells attached to them, and their order
        void genConfigOrder(std::vector<ConfigCell*> & ConfigTable) const;
        // Function that generates the verilog code
        void genVerilog(std::string dir);

        Module* getSubModule(std::string);

        // VARIABLES
        // internal parameters, ports, connections and modules, and configuration cells
        // submodules variable MUST be public, as used in the CGRA.cpp file

        std::map<std::string, unsigned> parameterlist;
        std::map<std::string, Port*>    ports;
        std::map<Port*, Connection*>    connections;
        std::map<std::string, Module*>  submodules;
        std::map<std::string, ConfigCell*>  configcells; // configcellname -> configcell

        Module* parent; // Stores parent module

        // Accessor functions
        unsigned getSize() const // Returns size of module
        {
            if(this != NULL)
                return data_size;
            else
                return 0;
        }
        std::string getName() const // Returns module name
        {
            if(this != NULL)
                return name;
            else
                return "";
        }
        module_type getModuleType() const // Returns module type
        {
            if(this != NULL)
                return mt;
            else
                return MOD_USER;
        }
        std::string ReturnPath() const // Returns the full path from the CGRA to a module (useful for CAD tools to simulate CGRA)
        {
            if (parent->name == "CGRA")
                return name;
            return parent->ReturnPath() + "/" + name;
        }

        bool hasConfigCells() const { return not configcells.empty(); }

        virtual std::vector<BitSetting> getBitConfig(
            const ConfigCell& ccell,
            const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
            const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
        ) const {
            std::cout << getName() << '\n';
            for (const auto& op_and_nodes : mrrg_nodes_from_op_node) {
                std::cout << '\t' << *op_and_nodes.first << '\n';
                for (const auto& mrrg_node : op_and_nodes.second) {
                    std::cout << "\t\t" << *mrrg_node << '\n';
                }
            }
            for (const auto& val_and_nodes : mrrg_nodes_from_val_node) {
                std::cout << '\t' << *val_and_nodes.first << '\n';
                for (const auto& mrrg_node : val_and_nodes.second) {
                    std::cout << "\t\t" << *mrrg_node << '\n';
                }
            }
            (void)mrrg_nodes_from_val_node; throw cgrame_error("getBitConfig: uninplemented for " + getName());
        }
        virtual std::string GenericName(); // Generates a generic name for the module, can be overridden by inherited virtual functions.

    protected:
        // Recursively create the MRRG for a given II
        virtual MRRG* createMRRG(unsigned II);

        // Direct helper functions for GenVerilog()
        void GetModulesToPrint(std::queue<Module*> & ToPrint, std::set<std::string> & PrintedModMap); // Function that returns what modules need to be printed in Verilog (avoiding duplicate printing)
        void GenModuleVerilog(); // Function that generates verilog for a module (submodules not included)
        // END OF HELPER FUNCTIONS

        // Helper functions used in function GenModuleVerilog
        bool DetermineConfigCells(); // Determines if configuration cells exist within a module
        bool DetermineRegs(); // Determines if registers exist within a module
        void GenModuleHeader(bool HasConfig, bool HasRegisters); // Generates module header
        void GenParameters(); // Prints out module parameters
        void GenPortSpecs(bool HasConfig, bool HasRegisters); // Prints out the ports, and their specification
        virtual void GenConnections(); // Prints out submodules and their connectivity, can be overriden (by FuncUnit)
        virtual void GenFunctionality(); // Generates the functionality for the module, e.g. always block, can be overriden by inherited virtual functions
        // END OF HELPER FUNCTIONS

        // Helper functions used in function GenConnections
        void GenerateMatrix(STRING_MATRIX & Matrix); // Generates a matrix to store connections between submodules
        void DetermineConnections(STRING_MATRIX & Matrix, PrintList & WireList, PrintList & SubmodList, PrintList & AssignList); // Print out the connections between submodules in Verilog
        // END OF HELPER FUNCTIONS

        virtual std::string PrintParameters(); // Function to override parameters within a module during Verilog declaration
        // Helper functions used in DetermineConnections, used to help insert connection information into a matrix
        int FindPortIndex(std::string PortName);
        int FindSubmoduleIndex(std::string SubmoduleName);
        // END OF HELPER FUNCTIONS

        unsigned data_size; // Bitwidth of the data the module is handling
        module_type mt; // Module type

        std::string         templateName;           // Module name
        std::string name; // Module name
};

typedef struct
{
    int II;
    int Latency;
} FuncUnitMode;


// Struct to hold information of one possible mode of a FuncUnit (e.g. add)
typedef struct
{
    std::string ModuleName; // Name of the module to achieve that function
    std::string OpName; // Name of the operation
    std::vector<std::string> Functionality; // Verilog code necessary to achieve that function
    std::string WireName; // Name of the wire we will use when connecting that module to others
} LLVMMode;


// Below there is a class for every type of supported module
// Functional Unit, does LLVM computations
class FuncUnit : public Module
{
    public:
        // Constructor takes in a name, the operations that the unit supports, and the size of the module
        FuncUnit(std::string name, std::vector<OpGraphOpCode> supported_modes = {OPGRAPH_OP_ADD, OPGRAPH_OP_MUL}, unsigned size = DEFAULT_SIZE, int II = 1, int latency = 0);
        virtual void GenFunctionality();
        virtual void GenConnections();
        virtual std::vector<BitSetting> getBitConfig(
            const ConfigCell& ccell,
            const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
            const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
        ) const override;
        virtual std::string GenericName(); // Generates a generic name for the module
        virtual ~FuncUnit();
        MRRG* createMRRG(unsigned II);

        int getII() const
        {
            return 1; //TODO: NEED TO SORT out II and LATENCY if different operations on the same func unit have different II/Latency
            //    return selected_mode.II;
        };
        int getLatency() const
        {
            return 0; //TODO: NEED TO SORT out II and LATENCY if different operations on the same func unit have different II/Latency
            //    return selected_mode.Latency;
        };
        // For the bitstream generation. Maps an operation to a set of bits
        FuncUnitMode selected_mode; // Currently selected Mode
        static std::map<OpGraphOpCode, LLVMMode> all_modes;
        std::vector<OpGraphOpCode> supported_modes;
};

class MEMUnit : public Module {
    public:
        // Constructor takes in a name, the operations that the unit supports, and the size of the module
        MEMUnit(std::string name, unsigned size = DEFAULT_SIZE);
        //        virtual void GenFunctionality();
        //        virtual void GenConnections();
        virtual ~MEMUnit();
        virtual std::string GenericName(); // Generates a generic name for the module
        virtual void GenFunctionality();
        MRRG* createMRRG(unsigned II);

        // For the bitstream generation. Maps an operation to a set of bits
        virtual std::vector<BitSetting> getBitConfig(
            const ConfigCell& ccell,
            const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
            const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
        ) const override;
};

class ConstUnit : public Module {
    public:
        // Constructor takes in a name, the operations that the unit supports, and the size of the module
        ConstUnit(std::string name, int size = DEFAULT_SIZE);
        virtual ~ConstUnit();
        virtual std::string GenericName(); // Generates a generic name for the module
        virtual void GenFunctionality();
        MRRG* createMRRG(unsigned II);

        virtual std::vector<BitSetting> getBitConfig(
            const ConfigCell& ccell,
            const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
            const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
        ) const override;
};
class Register : public Module {
    public:
        Register(std::string, unsigned size = DEFAULT_SIZE); // ("name", size)
        virtual ~Register();
        virtual std::string GenericName(); // Generates a generic name for the module
        virtual void GenFunctionality();
        MRRG* createMRRG(unsigned II);
};

class Multiplexer : public Module {
    public:
        Multiplexer(std::string, unsigned mux_size, unsigned size = DEFAULT_SIZE); // ("name", mux size, size)
        virtual ~Multiplexer();
        virtual std::string GenericName(); // Generates a generic name for the module
        virtual void GenFunctionality();
        virtual std::vector<BitSetting> getBitConfig(
            const ConfigCell& ccell,
            const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
            const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
        ) const override;
        unsigned getMuxSize() { return mux_size; };
        MRRG* createMRRG(unsigned II);
    private:
        unsigned mux_size; // size of multiplexer (e.g. how many inputs)
};

class RegisterFile : public Module {
    public:
        RegisterFile(std::string name, unsigned NumInputPorts, unsigned NumOutputPorts, unsigned Log2Registers, unsigned size = DEFAULT_SIZE); // ("name", {"functionality"}, size)
        virtual ~RegisterFile();
        virtual void GenFunctionality();
        virtual std::vector<BitSetting> getBitConfig(
            const ConfigCell& ccell,
            const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
            const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
        ) const override;

        virtual std::string GenericName();
        virtual std::string PrintParameters(); // Needed because Register File has two parameters
        MRRG* createMRRG(unsigned II);
    private:
        unsigned NumInputPorts, NumOutputPorts, Log2Registers;
};

class TriState : public Module {
    public:
        enum class Mode : char {
            PROVIDES_IO_OP,
            PLAIN
        };

        TriState(Mode mode, std::string, unsigned size = DEFAULT_SIZE);
        virtual std::string GenericName();
        virtual ~TriState();
        virtual std::vector<BitSetting> getBitConfig(
            const ConfigCell& ccell,
            const std::map<OpGraphOp*, std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
            const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
        ) const override;
        virtual MRRG* createMRRG(unsigned II);
        virtual void GenFunctionality();
    private:
        Mode mode;
};

class IO : public Module {
    public:
        IO(std::string, unsigned size = DEFAULT_SIZE); // ("name", size)
        Module* getTriState(); // This is used for the bitstream generation
        virtual std::string GenericName();
        virtual ~IO();
};

class CustomModule : public Module {
    public:
        CustomModule(std::string name, std::vector<std::string> Function, unsigned size = DEFAULT_SIZE); // ("name", {"functionality"}, size)
        virtual ~CustomModule();
        virtual void GenFunctionality();
        virtual std::string GenericName();
    private:
        std::vector<std::string> Function; // functionality lines of code (this will be printed in GenFunctionality()
};

class UserModule : public Module {
    public:
        UserModule(std::string prototype, std::string name, std::vector<std::string> Ports); // ("prototype name", "module name", {"list of all ports"})
        virtual std::string GenericName(); // Generates a generic name for the module
        virtual ~UserModule();
    private:
        std::string prototype; // prototype name of module
};

#endif

