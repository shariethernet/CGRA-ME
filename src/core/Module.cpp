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

// Define statements included in .h file
#include <CGRA/Module.h>
#include <ios>
#include <fstream>
#include <ostream>
#include <sstream>
#include <regex>

std::ostream& operator<<(std::ostream& os, const module_type& mtype) {
    switch (mtype) {
        case MOD_INVALID:         return os << "MOD_INVALID";
        case MOD_COMPOSITE:       return os << "MOD_COMPOSITE";
        case MOD_PRIM_FUNC:       return os << "MOD_PRIM_FUNC";
        case MOD_PRIM_REG:        return os << "MOD_PRIM_REG";
        case MOD_PRIM_RF:         return os << "MOD_PRIM_RF";
        case MOD_PRIM_MUX:        return os << "MOD_PRIM_MUX";
        case MOD_PRIM_TRI:        return os << "MOD_PRIM_TRI";
        case MOD_PRIM_IO:         return os << "MOD_PRIM_IO";
        case MOD_PRIM_CUSTOM:     return os << "MOD_PRIM_CUSTOM";
        case MOD_USER:            return os << "MOD_USER";

        default: return os << "FIXME: UNHANDLED module_type PRINT";
    }
}

// Constructor
Module::Module(std::string name, unsigned size)
{
    this->name = name;
    this->templateName = "";
    this->data_size = size;
    this->mt = MOD_COMPOSITE; // Initializing the module type, this is its default setting
    this->parent = NULL; // Initializing its parent to null
}

Module::Module(std::string name, std::string template_name, unsigned size)
{
    this->name = name;
    this->templateName = template_name;
    this->data_size = size;
    this->mt = MOD_COMPOSITE; // Initializing the module type, this is its default setting
    this->parent = NULL; // Initializing its parent to null
}

// Destructor, handling dynamically declared data
Module::~Module()
{
    for(auto it = connections.begin(); it != connections.end(); ++it) // Deleting connections
        delete it->second;
    for(auto it = ports.begin(); it != ports.end(); ++it) // Deleting ports
        delete it->second;
    for(auto it = submodules.begin(); it != submodules.end(); ++it)  // Deleting submodules
        delete it->second;
    for(auto it = configcells.begin(); it != configcells.end(); ++it)  // Deleting configcells
        delete it->second;
}

// Function that returns ALL the modules that have config cells attached to them, and their order
// This is a void function insead of a std::vector<Module*> function because it would otherwise use lots of memory (as it is recursive)
void Module::genConfigOrder(std::vector<ConfigCell*> & ConfigTable) const
{
    for(const auto& name_and_configcell : configcells) {
        ConfigTable.push_back(name_and_configcell.second);
    }

    for(const auto& name_and_submodule : submodules) {
        name_and_submodule.second->genConfigOrder(ConfigTable);
    }
}

// Start of the process that generates verilog code: Sets up data structure and calls function that prints verilog module by module
void Module::genVerilog(std::string dir)
{
    std::set<std::string> PrintedModules; // Data structure to keep track of the modules that were printed
    std::queue<Module*> ToPrint; // Modules to print
    GetModulesToPrint(ToPrint, PrintedModules); // Get the modules that need to be printed (puts them in queue)

    while (!ToPrint.empty()) // Printing out the module prototypes in the queue
    {
        std::string filename = dir + ToPrint.front()->GenericName() + ".v";
        std::cout << "    Generating \"" << filename << "\" ... ";

        std::streambuf* original_sbuf = std::cout.rdbuf();
        std::ofstream hdl_filebuf(filename);
        std::cout.rdbuf(hdl_filebuf.rdbuf()); // Redirects to new file

        // Dumping module Verilog
        ToPrint.front()->GenModuleVerilog(); // Printing the top
        ToPrint.pop(); // Shrinking the queue

        std::cout.rdbuf(original_sbuf); // Restores printout
        std::cout << "Done!" << std::endl;
    }
    if (DetermineConfigCells()) // If there are config cells in the module, print the configcell module prototype
    {
        std::string filename = dir + "ConfigCell.v";
        std::cout << "    Generating \"" << filename << "\" ... ";

        std::streambuf* original_sbuf = std::cout.rdbuf();
        std::ofstream hdl_filebuf(filename);
        std::cout.rdbuf(hdl_filebuf.rdbuf()); // Redirects to new file

        ConfigCell PrintConfig ("ConfigCell"); // Dummy config cell to print
        PrintConfig.GenModuleVerilog(); // Printing out the module declaration for configcell

        std::cout.rdbuf(original_sbuf); // Restores printout
        std::cout << "Done!" << std::endl;
    }

}

// Determine the modules necessary to print out
void Module::GetModulesToPrint(std::queue<Module*> & ToPrint, std::set<std::string> & PrintedModules)
{
    if (mt == MOD_USER) // If it is a user defined module, no need to print it
        return;

    // Checking to see if the module has been processed already
    std::string UniqueModuleName = GenericName(); // Getting unique string to identify the module

    // Check to see if the module was processed
    if (PrintedModules.find(UniqueModuleName) != PrintedModules.end()) // Already set to be printed
        return;

    // Otherwise, the module has not been printed yet, add it to the printed module list and generate verilog code (IMPORTANT TO DO THIS AFTER SUBMODULE CALLS)
    PrintedModules.insert(UniqueModuleName); // Mappping the hash to the name of the function that will be printed
    ToPrint.push(this); // Adding the print job to the queue

    // Recursive calls for submodules - get necessary submodules to print
    for(const auto& name_and_submodule : submodules)
        name_and_submodule.second->GetModulesToPrint(ToPrint, PrintedModules);
}

// This generates a generic name for a module... This can be overriden!
std::string Module::GenericName()
{
    //NOTE: This method used to return the name member after removing
    //all numeric characters within the string, for unclear reasons
    return this->templateName;
}

// Generates the Verilog code for a module
void Module::GenModuleVerilog()
{
    // Determining if there are config cells in the module
    bool HasConfig = DetermineConfigCells();
    // Determining if there are any registers in the module
    bool HasRegs = DetermineRegs();

    // Printing out the module header
    GenModuleHeader(HasConfig, HasRegs);

    // Generating the parameters of the module
    GenParameters();

    // Printing out the port specifications (input or output)
    GenPortSpecs(HasConfig, HasRegs);

    // Printing out verilog for connections and submodules
    GenConnections();

    // Then add the module's functionality
    GenFunctionality();

    // Printing out the end of the module
    std::cout << "endmodule\n\n";
}

// Finds out if a module has config cells inside it, or in one of its submodule. Returns true if either case is correct.
bool Module::DetermineConfigCells()
{
    if (!configcells.empty())
        return true; // Config cell found in module
    // Otherwise, look into the submoduels
    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it) // For all submodules
    {
        if(it->second->DetermineConfigCells()) // Make recursive calls
            return true; // Config cell found in submodule means config cell exists in module
    }
    return false; // No config cells found in submodules
}

// Finds out if a module has registers in it. Works very similarly to the DetermineconfigCells() function
bool Module::DetermineRegs()
{
    if (mt == MOD_PRIM_REG || mt == MOD_PRIM_RF)
        return true; // Module is a register
    // Otherwise, look in submodules for a register
    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it)
    {
        if(it->second->DetermineRegs())
            return true; // Register found in submodule
    }
    return false; // No registers found in submodules
}

// Generating the module header in Verilog code
void Module::GenModuleHeader(bool HasConfig, bool HasRegisters)
{
    std::vector<std::string> PortList; // Buffer that contains ports that we want to print out
    std::cout << "module " << GenericName() << "("; // Printing module header and name, e.g. "module MyModuleName ( "
    if (HasConfig) // If there are config cells inside, we need to add some extra ports
    {
        PortList.push_back("Config_Clock");
        PortList.push_back("Config_Reset");
        PortList.push_back("ConfigIn");
        PortList.push_back("ConfigOut");
    }
    if (HasRegisters) // If there are registers inside, we want to add some other extra ports
    {
        PortList.push_back("CGRA_Clock");
        PortList.push_back("CGRA_Reset");
    }
    // Getting all the ports that the module has (port specifications such as input, output, inout come later)
    for(const auto& name_and_port : ports)
        PortList.push_back(name_and_port.second->name);

    for (unsigned i = 0; i < PortList.size(); i++) // For all ports
    {
        std::cout << PortList[i]; // Print it out
        if (i != PortList.size() - 1) // Space out the ports with commas
            std::cout << ", ";
    }
    std::cout << ");\n";  // Ending the module header
}

// Printing out the parameters in the verilog module
void Module::GenParameters()
{
    for(std::map<std::string,unsigned>::iterator it = parameterlist.begin(); it != parameterlist.end(); ++it) // For all parameters
    {
        std::cout << SET_INDENT << "parameter " << it->first << " = " << it->second << ";\n"; // Print it along with its default size
    }
}

// Printing out all the ports, and their specifications (input or output)
void Module::GenPortSpecs(bool HasConfig, bool HasRegisters)
{
    std::cout << SET_INDENT << "// Specifying the ports\n"; // Port generation comment
    if (HasConfig) // If there are config cells, declare the following port specifications
    {
        std::cout << SET_INDENT << "input Config_Clock, Config_Reset, ConfigIn;\n";
        std::cout << SET_INDENT << "output ConfigOut;\n";
    }
    if (HasRegisters)
        std::cout << SET_INDENT << "input CGRA_Clock, CGRA_Reset;\n";
    // Now, go through all the other ports and print their specifications
    for(std::map<std::string,Port*>::iterator it = ports.begin(); it != ports.end(); ++it)
    {
        Port* curPort = it->second;
        if (curPort->pt == PORT_INPUT) // If port is input
            std::cout << SET_INDENT << "input ";
        else if  (curPort->pt == PORT_OUTPUT) // If the port is output
            std::cout << SET_INDENT << "output ";
        else if (curPort->pt == PORT_OUTPUT_REG) // If the port is output reg
            std::cout << SET_INDENT << "output reg ";
        else if (curPort->pt == PORT_BIDIR) // If the port is a bidirectional
            std::cout << SET_INDENT << "inout ";
        else // Otherwise, print out an error message
            std::cout << "<ERROR: DID NOT DECLARE PORT SPECIFICATION!>";

        // Printing the size of the port
        if (curPort->size == PARAMETERIZED) // If the port size is parameterized, include the parameter name in there
            std::cout << "[" << curPort->parameter << "-1:0] ";
        if (curPort->size > 1) // If the port size is greater than one, use array notation
            std::cout << "[" << curPort->size-1 << ":0] ";
        // Printing out the port name
        std::cout << curPort->name << ";\n";
    }
}

// Generates the verilog for connections
void Module::GenConnections()
{
    STRING_MATRIX Matrix; // Declaring a matrix to hold connection information
    PrintList WireList, SubmodList, AssignList; // Buffers that will contain output... wirelist->wires, submodulelist->submodules and configcells, assignlist->assign statements
    GenerateMatrix(Matrix); // Generating the matrix

    // Deals with how to generate verilog code to describe all the connections between the module, submodule, and config cells. Also generates necessary wires
    DetermineConnections(Matrix, WireList, SubmodList, AssignList); // The verilog information will be stored in the three buffers (WireList, SubmodList, and AssignList)

    // Printing out those buffers (that contain the actual verilog code to be printed)
    WireList.print();
    SubmodList.print();
    AssignList.print();
}

// This function exists to be overridden
void Module::GenFunctionality()
{
    // Currently set to do nothing
}

// Generating the matrix that will hold all the connections between submodules
void Module::GenerateMatrix(STRING_MATRIX & Matrix)
{
    // Intialization of matrix size (this does not change later)
    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it)
    {
        std::vector<std::string> StringVector;
        // XXX: This leaves dangling connections being hard-wired to 0s
        for (auto& port : it->second->ports)
        {
            StringVector.push_back("." + port.first + "(0) " + UNINITIALIZED); // overwritten by valid connections afterwards
        }
        Matrix.push_back(StringVector); // Adding vector to matrix
    }
}

// Deals with how to generate verilog describing the connections between the module, submodule, and config cells. Also generates necessary wires
void Module::DetermineConnections(STRING_MATRIX & Matrix, PrintList & WireList, PrintList & SubmodList, PrintList & AssignList)
{
    std::stringstream buffer; // Buffers to append strings
    std::string Signal; // Holds the wire name
    std::string PreviousWire, ConfigInput, ConfigOutput; // Stores the names of verilog ports/wires
    int SubmodIndex, PortIndex; // Variables used to keep track of position in matrix

    PreviousWire = "ConfigIn";

    // ************** CONFIG CELL WIRING IN MODULE ************** //

    if (!configcells.empty()) // If there are config cells, add comments
    {
        WireList.add({"\n", SET_INDENT, "// Wires for the the config cells\n"});
        SubmodList.add({"\n", SET_INDENT, "// Declaring the config cells\n"});
    }
    for(std::map<std::string, ConfigCell*>::iterator it = configcells.begin(); it != configcells.end(); ++it) // For all the config cells in the module
    {
        buffer << it->second->name << "_sig"; // Naming the wire (that will connect to the submodule)
        Signal = buffer.str();
        buffer.str(std::string()); // Clearing the buffer

        // Declaring the verilog wire for connecting the config cell to submodule
        if (it->second->cell_size > 1) // If size is greater than one,
            WireList.add({SET_INDENT, "wire [", std::to_string(it->second->cell_size - 1), ":0] ", Signal, ";\n"});
        else // Otherwise, if the size is 1
            WireList.add({SET_INDENT, "wire ", Signal, ";\n"});

        ConfigInput = PreviousWire; // Otherwise, wire the previous wire in to ConfigIn. This is to make a chain of config cells

        // Declaring the verilog wire necessary to connect the config cell chain
        buffer << it->second->name << "_config";
        ConfigOutput = buffer.str();
        buffer.str(std::string()); // Clearing the buffer
        WireList.add({SET_INDENT, "wire ", ConfigOutput, ";\n"});
        PreviousWire = ConfigOutput; // Setting the previous wire (necessary for making the chain)

        // Declaring the config cell
        SubmodList.add({SET_INDENT , "ConfigCell #(", std::to_string(it->second->cell_size), ") ", it->second->name, " (\n",
                SET_DOUBLE_INDENT, ".Config_Clock(Config_Clock),\n", SET_DOUBLE_INDENT, ".Config_Reset(Config_Reset),\n",
                SET_DOUBLE_INDENT, ".ConfigIn(", ConfigInput, "),\n", SET_DOUBLE_INDENT, ".ConfigOut(" , ConfigOutput , "),\n",
                SET_DOUBLE_INDENT, ".select(", Signal, "));\n"});

        // Modify the matrix to show the connection between config cell and submodule
        for (auto* port : it->second->getAllConnectedPorts())
        {
            SubmodIndex = FindSubmoduleIndex(port->parent->name); // Find submodule index
            if (SubmodIndex < 0) continue; // Case: connecting to port instead of submodule
            PortIndex = port->parent->FindPortIndex(port->name); // Find port index that it is connected to
            Matrix[SubmodIndex][PortIndex] = "." + port->name  + "(" + Signal + ")"; // Update that entry in the matrix
        }
    }

    // ************** END OF CONFIG CELL WIRING IN MODULE ************** //

    // ************** WIRING OTHER CONNECTIONS WITHIN MODULE (between module and submodules, configcells excluded) ************** //

    if (!connections.empty()) // If there are connections, write a comment
        WireList.add({"\n",SET_INDENT, "// Wires connecting the main module and submodules\n"});

    // For all connection objects, between modules/submodules (config cells not included)
    for(std::map<Port*,Connection*>::iterator it = connections.begin(); it != connections.end(); ++it)
    {
        if (it->second->src->parent == this) // If the source port is one of the main module's inputs
        {
            for(unsigned j = 0; j < it->second->dst.size(); j++) // For all destination ports
            {
                if (it->second->dst[j]->parent == this) // If the destination port is not in a submodule, connect directly to output
                {
                    AssignList.add({SET_INDENT, "assign ", it->second->dst[j]->name, " = ", Signal, ";\n"});
                    continue;
                }
                // Otherwise it is a submodule, and assign its connection appropriately
                SubmodIndex = FindSubmoduleIndex(it->second->dst[j]->parent->name); // Finding the submodule that the destination is connected
                PortIndex = it->second->dst[j]->parent->FindPortIndex(it->second->dst[j]->name); // Go to that submodule, and then find the port index of that port name
                buffer << "." << it->second->dst[j]->name << "(" << it->second->src->name << ")";
                Matrix[SubmodIndex][PortIndex] = buffer.str();
                buffer.str(std::string()); // Clearing the buffer
            }
        }
        else // If the source port is in one of the submodules
        {
            if (it->second->dst.size() == 1 && it->second->dst[0]->parent == this) // If we only have one port connection and its connected to the main module's port, we do not want a wire
            {
                int SubmodIndex = FindSubmoduleIndex(it->second->src->parent->name); // Finding the submodule that the destination is connected
                int PortIndex = it->second->src->parent->FindPortIndex(it->second->src->name); // Go to that submodule, and then find the port index of that port name
                buffer << "." << it->second->src->name << "(" << it->second->dst[0]->name << ")";
                Matrix[SubmodIndex][PortIndex] = buffer.str();
                buffer.str(std::string()); // Clearing the buffer
                continue;
            }
            // Naming and declaring the verilog wire necessary
            buffer << it->second->src->parent->name << "_" << it->second->src->name << "_sig"; // Naming the wire
            Signal = buffer.str(); // Assigning the wire the name
            buffer.str(std::string()); // Clearing the buffer

            // Declaring the verilog wire
            if (it->second->src->size == PARAMETERIZED && it->second->src->parameter == "size")
            {
                WireList.add({SET_INDENT , "wire [" , std::to_string(it->second->src->parent->getSize() - 1) , ":0] " , Signal , ";\n"});
            }
            else if (it->second->src->size > 1) // If the size is greater than one
                WireList.add({SET_INDENT , "wire [" , std::to_string(it->second->src->size - 1) , ":0] " , Signal , ";\n"});
            else // Otherwise, if the size is 1
                WireList.add({SET_INDENT , "wire " , Signal , ";\n"});

            // Assigning that wire to the corresponding input
            int SubmodIndex = FindSubmoduleIndex(it->second->src->parent->name); // Finding the submodule that the destination is connected
            int PortIndex = it->second->src->parent->FindPortIndex(it->second->src->name); // Go to that submodule, and then find the port index of that port name
            buffer << "." << it->second->src->name << "(" << Signal << ")";
            Matrix[SubmodIndex][PortIndex] = buffer.str();
            buffer.str(std::string()); // Clearing the buffer

            // Now, send this wire to all the destination ports
            for(unsigned j = 0; j < it->second->dst.size(); j++) // For all destination ports
            {
                if (it->second->dst[j]->parent == this) // If the destination port is not in a submodule, connect directly to output
                {
                    AssignList.add({SET_INDENT, "assign ", it->second->dst[j]->name, " = ", Signal, ";\n"});
                    continue;
                }

                // Otherwise it is a submodule, and assign its connection appropriately
                SubmodIndex = FindSubmoduleIndex(it->second->dst[j]->parent->name); // Finding the submodule that the destination is connected
                PortIndex = it->second->dst[j]->parent->FindPortIndex(it->second->dst[j]->name); // Go to that submodule, and then find the port index of that port name
                buffer << "." << it->second->dst[j]->name << "(" << Signal << ")";
                Matrix[SubmodIndex][PortIndex] = buffer.str();
                buffer.str(std::string()); // Clearing the buffer
            }
        }
    }

    // ************** END OF WIRING OTHER CONNECTIONS WITHIN MODULE ************** //


    // ************** DEAL WITH DECLARATION OF SUBMODULES (Wiring of them to create necessary config cell chain also handled!) ************** //

    // std::string PreviousWire, ConfigInput, ConfigOutput has been previously declared in above section!
    buffer.str(std::string()); // Clearing the previously declared buffer
    int i = 0; // Loop variable

    if (!submodules.empty()) // If there are submodules, add a comment
        SubmodList.add({"\n", SET_INDENT, "// Declaring the submodules\n"});

    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it) // For each submodule, print it and its connections
    {
        SubmodList.add({SET_INDENT, it->second->GenericName(), " ", it->second->PrintParameters(), it->second->name, "(\n"}); // Printing beginning of submodule
        if (it->second->DetermineConfigCells()) // If the module has config cells somewhere inside it
        {
            ConfigInput = PreviousWire; // Otherwise, wire the previous module into ConfigIn (to create a chain)
            // Declaring the verilog wire for the config cell chain, and wiring it in
            buffer << it->second->name << "_config";
            ConfigOutput = buffer.str();
            buffer.str(std::string()); // Clearing the buffer
            PreviousWire = ConfigOutput;
            WireList.add({SET_INDENT, "wire ", PreviousWire, ";\n"});

            // Adding the config cell ports in (required if there are config cells)
            SubmodList.add({SET_DOUBLE_INDENT, ".Config_Clock(Config_Clock),\n", SET_DOUBLE_INDENT, ".Config_Reset(Config_Reset),\n", SET_DOUBLE_INDENT, ".ConfigIn(", ConfigInput, "),\n",
                    SET_DOUBLE_INDENT, ".ConfigOut(", ConfigOutput, ")"});
            SubmodList.add({",\n"}); // This needs to be on a seperate line, to indicate a comma was added last (this information is used below when printing the other ports)
        }
        if (it->second->DetermineRegs())
        {
            SubmodList.add({SET_DOUBLE_INDENT, ".CGRA_Clock(CGRA_Clock),\n", SET_DOUBLE_INDENT, ".CGRA_Reset(CGRA_Reset)"});
            SubmodList.add({",\n"}); // This needs to be on a seperate line, to indicate a comma was added last (this information is used below when printing the other ports)
        }
        // Now, print out the remaining ports that the module has
        for(unsigned j = 0; j < Matrix[i].size(); j++)
        {
            SubmodList.add({SET_DOUBLE_INDENT, Matrix[i][j]}); // Add the connection element
            // If the port currently being printed is not the last one...
            if (j+1 != Matrix[i].size())
                SubmodList.add({",\n"}); // Add a comma
        }
        SubmodList.add({");\n"}); // Printing end of submodule
        // Ending the module header
        i++; // Incrementing the index
    }
    // If there were submodules with config cells, we need to assign ConfigOut to complete the chain
    if (PreviousWire != "ConfigIn")
        AssignList.add({SET_INDENT, "assign ConfigOut = ", PreviousWire, ";\n"});

    // ************** END OF DEAL WITH DECLARATION OF SUBMODULES ************** //
}

// Printing the parameters of a module (when we instantiate an instance of it).
// IMPORTANT: Right now, it only handles a single parameter, which handles the bitwidth of a module. If we want to handle more parameters, the user will have to specify how they are used
// This would be done through a virtual override of this function
std::string Module::PrintParameters()
{
    if (parameterlist.size() == 1) // If there is a parameter, we assume that there is a parameter named for the module data size
    {
        if (parent != NULL && parent->parameterlist.size() == 1 && parameterlist.begin()->first == parent->parameterlist.begin()->first)
        {
            // If the parent module has the same parameter name, pass it through
            std::string Parameter = "#(" + parameterlist.begin()->first + ") "; // Pass through the parameter (instead of passing a number)
            return Parameter;
        }
        else // Otherwise, just pass the data size into the parameter
        {
            std::string Parameter = "#(" + std::to_string(data_size) + ") "; // The default parameter is the data size here
            return Parameter;
        }
    }
    return ""; // If there are no parameters, return nothing
}

// Finds the index in the module for a given port name, returns -1 if string is not found (0 is first index)
int Module::FindPortIndex(std::string PortName)
{
    int index = 0;
    for(std::map<std::string,Port*>::iterator it = ports.begin(); it != ports.end(); ++it)
    {
        if (it->second->name == PortName) // Compare the port name with the specified one
            return index; // If port name is found, return index location
        index++;
    }
    return -1; // If the string is not found, return -1
}


// Finds the index in the module for a given submodule name, returns -1 if string is not found (0 is first index)
int Module::FindSubmoduleIndex(std::string SubmoduleName)
{
    int index = 0;
    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it)
    {
        if (it->second->name == SubmoduleName) // Compare the port name with the specified one
            return index; // If port name is found, return index location
        index++;
    }
    return -1; // If the string is not found, return -1
}

// Debug information: Printing out module information
void Module::print()
{
    std::cout << this->name << ":\n";
    std::cout << "ports:\n";
    print_ports();
    std::cout << "connections:\n";
    print_connections();
    std::cout << "submodules:\n";
    print_submodules();
}


void Module::print_dot()
{
    // print node info
    std::cout << this->name << ";\n";
    // for all src ports
    for(std::map<Port*,Connection*>::iterator it = connections.begin(); it != connections.end(); ++it)
        // print all edges
        for(unsigned j = 0; j < it->second->dst.size(); j++)
        {
            std::cout << it->first->parent->name << "." << it->first->name << "->" << it->second->dst[j]->parent->name << "." << it->second->dst[j]->name << ";\n";
        }

    // print for all composite submodules
    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it)
    {
        if(it->second->getModuleType() == MOD_COMPOSITE)
            it->second->print_dot();
    }
}

void Module::print_ports()
{
    for(std::map<std::string,Port*>::iterator it = ports.begin(); it != ports.end(); ++it)
        std::cout << it->second->name << "\n";

}

void Module::print_connections()
{
    for(std::map<Port*,Connection*>::iterator it = connections.begin(); it != connections.end(); ++it)
    {
        // print all edges
        for(unsigned j = 0; j < it->second->dst.size(); j++)
        {
            std::cout << this->name << "<-" << it->second->dst[j]->parent->name << ";\n";
        }
    }
}

void Module::print_submodules()
{
    // print for all composite submodules
    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it)
    {
        std::cout << it->second->name << "\n";
    }
}

void Module::print_configcells()
{
    // print for all composite submodules
    for(std::map<std::string,ConfigCell*>::iterator it = configcells.begin(); it != configcells.end(); ++it)
    {
        std::cout << it->second->name << "\n";
    }
}

//Dummy function that prints the power consumed by a given module
//The calculation done for the power is on the rhs of the thisPower equality
float Module::power()
{
    if(submodules.size() <= 0) {
        float thisPower = ports.size() + connections.size() + configcells.size();
        return(thisPower);
    }
    else {
        float power = 0;
        for(auto elem : submodules) {
            power += (elem.second)->power();
        }
        float thisPower = ports.size() + connections.size() + configcells.size();
        return(thisPower + power);
    }
}

//Dummy function that prints the area of a given module
//The calculation can be seen in the thisArea equality
//Currently its just the number of submodules
float Module::area()
{
    if(submodules.size() <= 0) {
        float thisPower = 1;
        return(thisPower);
    }
    else {
        float power = 0;
        for(auto elem : submodules) {
            power += (elem.second)->area();
        }
        float thisPower = 1;
        return(thisPower + power);
    }
}


// Function that gives the module and port names, given a string
static bool getmoduleport(std::string name, std::string* module, std::string* port)
{
    std::size_t dot_loc;
    if(std::string::npos != (dot_loc = name.find_first_of(".")))
    {
        *module = name.substr(0,dot_loc);
    }
    *port = name.substr(dot_loc + 1);

    return (std::string::npos == port->find_first_of("."));
}

// Function that adds a config cell to a module
// Takes config cell, and vector of ports (via string name) to connect to
bool Module::addConfig(ConfigCell* c, std::vector<std::string> ConnectTo)
{

    if(configcells[c->name]) // If the config cell name already exists
    {
        delete c; // Delete the dynamically allocated memory
        return false; // Return we did not add config cell
        // This is done for every error statement
    }

    // Now... find if the ports specified exists
    for (unsigned i = 0; i < ConnectTo.size(); i++) // For all the ports specified
    {
        std::string modulename, portname;
        if (!getmoduleport(ConnectTo[i], &modulename, &portname)) // Error with finding the module and port names given, output error message and quit
        {
            std::cout << "Connection ERROR!(" << this->name << "): module.port error\n";
            delete c;
            return false;
        }

        // Find the addresses of the modules and then port
        Module* module = (modulename == "this")?this:submodules[modulename];
        if(!module) // Error with finding module
        {
            std::cout << "Connection ERROR!(" << this->name << "): module not found ("<< modulename << ")\n";
            delete c;
            return false;
        }
        Port* port = module->ports[portname];
        if(!port) // Error with finding port
        {
            std::cout << "Connection ERROR!(" << this->name << "): port not found ("<< portname << ")\n";
            delete c;
            return false;
        }

        // Adding the port to list of connections for config cells
        c->connected_ports.push_back(port);


        // We need to make sure that the config cell size is at least the size of the port it connects to.
        // This updates the config cell size to do that
        if (port->size == PARAMETERIZED)
        {
            if (port->parent->parameterlist[port->parameter] > c->cell_size)
            {
                c->cell_size = port->parent->parameterlist[port->parameter];
            }
        }
        else if (port->size > c->cell_size)
            c->cell_size = port->size;
    }

    configcells[c->name] = c; // Add config cell to the list
    return true;
}

// Function that adds a submodule to the module
bool Module::addSubModule(Module* m)
{
    // If the submodule name exists, we do not add it again
    if(submodules[m->name])
    {
        std::stringstream msg;
        msg << "Submodule with same name: " << m->name << " already exists\n";
        delete m;
        throw cgrame_model_error(msg.str());
    }
    // Otherwise, add it
    m->parent = this; // Setting the parent
    submodules[m->name] = m; // Adding the module to list of submodules
    return true;
}

// Returns a pointer to a submodule
Module* Module::getSubModule(std::string m)
{
    // If the submodule name exists, we do not add it again
    if(!submodules[m])
    {
        std::stringstream msg;
        msg << "Could not find submodule: '" << m << "'\n";
        throw cgrame_model_error(msg.str());
    }
    return submodules[m];
}

// name1 is src, name2 is dst
bool Module::addConnection(std::string name1, std::string name2)
{
    std::string module1_name;
    std::string module2_name;

    std::string port1_name;
    std::string port2_name;


    // split names
    if(!getmoduleport(name1, &module1_name, &port1_name) || !getmoduleport(name2, &module2_name, &port2_name))
    {
        std::cout << "Connection ERROR! Could not make connection \"" << name1 << "\" to \""<< name2 << "\" within module \"" << this->name  << "\" \n";
        std::cout << "Connection ERROR!(" << this->name << "): module.port error\n";
        abort();
        return false;
    }

    // find module 1 and port
    Module* module1;
    if(module1_name == "this")
        module1 = this;
    else
        module1 = submodules[module1_name];
    if(!module1) // Error with finding module
    {
        std::cout << "Connection ERROR! Could not make connection \"" << name1 << "\" to \""<< name2 << "\" within module \"" << this->name  << "\" \n";
        std::cout << "Connection ERROR!(" << this->name << "): module1 not found ("<< module1_name << ")\n";
        abort();
        return false;
    }
    Port*   port1   = module1->ports[port1_name];
    if(!port1) // Error with finding port
    {
        std::cout << "Connection ERROR! Could not make connection \"" << name1 << "\" to \""<< name2 << "\" within module \"" << this->name  << "\" \n";
        std::cout << "Connection ERROR!(" << this->name << "): port1 not found ("<< port1_name << ")\n";
        abort();
        return false;
    }

    // find module 2 and port
    Module* module2;
    if(module2_name == "this")
        module2 = this;
    else
        module2 = submodules[module2_name];
    if(!module2) // Error with finding module
    {
        std::cout << "Connection ERROR! Could not make connection \"" << name1 << "\" to \""<< name2 << "\" within module \"" << this->name  << "\" \n";
        std::cout << "Connection ERROR!(" << this->name << "): module2 not found ("<< module2_name << ")\n";
        abort();
        return false;
    }
    Port*   port2   = module2->ports[port2_name];
    if(!port2) // Error with finding port
    {
        std::cout << "Connection ERROR! Could not make connection \"" << name1 << "\" to \""<< name2 << "\" within module \"" << this->name  << "\" \n";
        std::cout << "Connection ERROR!(" << this->name << "): port2 not found ("<< port2_name << ")\n";
        abort();
        return false;
    }

    Port* srcport = port1; // Source port
    Port* dstport = port2; // Destination port

    // Make sure that the destination port is not already connected to anything
    for(auto & c : connections)
    {
        for(auto & d : c.second->dst)
        {
            if(d == dstport)
            {
                std::cout << "Connection ERROR! Could not make connection \"" << name1 << "\" to \""<< name2 << "\" within module \"" << this->name  << "\" \n";
                std::cout << "Connection ERROR! In module (" << this->name << "): destination(" << name2 << ") already connected to (" << c.second->src->parent->name << "." << c.second->src->name <<")\n" ;
                abort();
                return false;
            }
        }
    }

    if(!connections[srcport]) // If there is no connection object for the given source port, make one
    {
        connections[srcport] = new Connection();
        connections[srcport]->src = srcport;
    }
    else
    {
        // if we already have a srcport entry, check that we are not adding a previous dst
        for(auto & d: connections[srcport]->dst)
        {
            if(d == dstport)
                return false;
        }
    }

    // Add the connection
    connections[srcport]->dst.push_back(dstport);

    // If debug mode is on, print out connection information
    if (DEBUG_MODE)
        std::cout << "New Connection:" << connections[srcport]->src->parent->name << "."
            << connections[srcport]->src->name << "->"
            << connections[srcport]->dst.back()->parent->name << "."
            << connections[srcport]->dst.back()->name << "\n";
    return true;
}

bool Module::addPort(std::string portname, port_type pt, unsigned size)
{
    if(ports[portname]) // If the port already exists
    {
        std::stringstream msg;
        msg << "Port " << portname << " already exists\n";
        throw cgrame_model_error(msg.str());
    }

    // Assigning port parameters
    Port* p = new Port(); // Create port
    p->name = portname; // Assign name
    p->pt = pt; // Assign port type
    p->size = size; // Assign port size
    p->parent = this; // Connect port to its parent
    ports[portname] = p; // Adding port to list

    if (DEBUG_MODE) // Debug information
        std::cout << this->name << ": added port - " << p->name << "\n";
    return true;
}

bool Module::addPort(std::string portname, port_type pt, std::string ParameterName, unsigned size)
{
    if(ports[portname]) // If the port already exists
    {
        std::stringstream msg;
        msg << "Port " << portname << " already exists\n";
        throw cgrame_model_error(msg.str());
    }

    // Assigning port parameters
    Port* p = new Port(); // Create port
    p->name = portname; // Assign name
    p->pt = pt; // Assign port type
    p->size = PARAMETERIZED; // PARAMETERIZED = 0: it's a flag to say that the port is parameterized
    p->parameter = ParameterName; // Set the parameter name
    p->parent = this; // Connect port to parent
    ports[portname] = p; // Add port to list
    parameterlist[ParameterName] = size; // Setting the default size of the parameter

    if (DEBUG_MODE) // Debug information
        std::cout << this->name << ": added port - " << p->name << "\n";

    return true;
}
MRRG* Module::createMRRG(unsigned II)
{
    if(mt != MOD_COMPOSITE)
        return NULL;

    std::map<std::string, MRRG*> subMRRGs;
    // create MRRG for all sub modules
    for(std::map<std::string,Module*>::iterator it = submodules.begin(); it != submodules.end(); ++it)
    {
        MRRG* temp = it->second->createMRRG(II);
        assert(temp);
        subMRRGs[it->first] = temp;
    }

    // create current MRRG with port nodes
    MRRG* result = new MRRG(II);
    // for all ports, create and add node to MRRG
    for(std::map<std::string,Port*>::iterator it = ports.begin(); it != ports.end(); ++it)
    {
        for(unsigned i = 0; i < II; i++)
        {
            result->nodes[i][it->first] = new MRRGNode(this, i, it->first);
        }
    }
    // also add all subMRRG nodes to this graph
    for(auto submrrg = subMRRGs.begin(); submrrg != subMRRGs.end(); ++submrrg)
    {
        unsigned i = 0;
        for(auto nodes = submrrg->second->nodes.begin(); nodes != submrrg->second->nodes.end(); ++nodes)
        {
            for(auto thenode = nodes->begin(); thenode != nodes->end(); ++thenode)
            {
                result->nodes[i][submrrg->first + "." + thenode->first] = thenode->second;
                // update node name
                thenode->second->name = submrrg->first + "." + thenode->first;
            }

            i++;
        }
    }

    subMRRGs[this->name] = result;

    // make all connections
    // for all src ports
    for(std::map<Port*,Connection*>::iterator it = connections.begin(); it != connections.end(); ++it)
    {
        // TODO: this is hacky .... need to look at verilog generation to simplify the number of port types we have
        if(it->first->pt != PORT_INPUT && it->first->pt != PORT_OUTPUT && it->first->pt != PORT_OUTPUT_REG)
            continue;

        std::string src_module_name = it->first->parent->name;
        std::string src_module_port_name = it->first->name;

        // connect to all destinations
        for(unsigned j = 0; j < it->second->dst.size(); j++)
        {
            std::string dst_module_name = it->second->dst[j]->parent->name;
            std::string dst_module_port_name = it->second->dst[j]->name;

            for(unsigned k = 0; k < II; k++)
            {
                // create link for each cycle
                MRRGNode* src_node = subMRRGs[src_module_name]->nodes[k][src_module_port_name];
                MRRGNode* dst_node = subMRRGs[dst_module_name]->nodes[k][dst_module_port_name];

                src_node->fanout.push_back(dst_node);
                dst_node->fanin.push_back(src_node);
            }
        }
    }

    //Delete all subMRRGS
    for(auto submrrg = subMRRGs.begin(); submrrg != subMRRGs.end(); ++submrrg)
    {
        if(submrrg->second != result) // dont delete the result!
            delete submrrg->second;
    }

    return result;
}

/************ TriState **********/
TriState::TriState(Mode mode, std::string name, unsigned size)
    : Module(name, size)
    , mode(mode)
{

    // adding ports
    addPort("enable", PORT_INPUT, 1);
    addPort("in", PORT_INPUT, "size");
    addPort("out", PORT_OUTPUT, "size");

    // module type
    mt = MOD_PRIM_TRI;
}

void TriState::GenFunctionality()
{
    std::cout << SET_INDENT << "assign out = enable ? in : {size{1'bZ}};\n";
}

std::vector<BitSetting> TriState::getBitConfig(
    const ConfigCell& ccell,
    const std::map<OpGraphOp*,std::set<MRRGNode*>>& mrrg_nodes_from_op_node,
    const std::map<OpGraphVal*, std::set<MRRGNode*>>& mrrg_nodes_from_val_node
) const {
    const std::regex in_regex("\\.in$");
    const std::regex out_regex("\\.out$");

    if (mrrg_nodes_from_val_node.empty()) {
        return {BitSetting::DONT_CARE_PREFER_LOW};
    } else if (mrrg_nodes_from_val_node.size() != 1) {
        throw cgrame_error("expect one val node");
    } else {
        const auto& val_node = begin(mrrg_nodes_from_val_node)->first;
        const auto& mrrg_nodes = begin(mrrg_nodes_from_val_node)->second;
        if (mrrg_nodes.size() != 1) {
            throw cgrame_error("expect one MRRG node");
        } else {
            const auto& mrrg_node = **begin(mrrg_nodes);
            const auto& name = mrrg_node.getHierarchyQualifiedName();
            if (regex_search(name, in_regex)) {
                return {BitSetting::HIGH};
            } else if (regex_search(name, out_regex)) {
                return {BitSetting::LOW};
            } else {
                throw cgrame_error("unexpected MRRG node");
            }
        }
    }
}

MRRG* TriState::createMRRG(unsigned II = 1)
{
    MRRG* result = new MRRG(II);

    for(unsigned i = 0; i < II; i++)
    {
        // create nodes
        MRRGNode* io = [&]() {
            if (mode == Mode::PROVIDES_IO_OP) {
                return new MRRGNode(this, i, "io", MRRG_NODE_FUNCTION);
            } else {
                return new MRRGNode(this, i, "io");
            }
        }();
        MRRGNode* out = new MRRGNode(this, i, "out");
        MRRGNode* in = new MRRGNode(this, i, "in");

        io->operand[0] = in;

        in->fanout.push_back(io); // in is an input into the io block, which will be output from the CGRA
        io->fanin.push_back(in);

        io->fanout.push_back(out);
        out->fanin.push_back(io); // out is an output from the io block, which will be an input to the CGRA

        result->nodes[i]["out"] = out;
        result->nodes[i]["in"] = in;
        result->nodes[i]["io"] = io;


        // functionality
        io->supported_ops.push_back(OPGRAPH_OP_INPUT);
        io->supported_ops.push_back(OPGRAPH_OP_OUTPUT);
        //        result->outputs["out"].push_back(out);
        //        result->inputs["in"].push_back(in);

    }

    return result;
}

std::string TriState::GenericName()
{
    return "tristate_" + std::to_string(getSize()) + "b";
}

TriState::~TriState()
{
}


/************ IO **********/
IO::IO(std::string name, unsigned size)
: Module(name, size)
{
    // adding ports
    addPort("in", PORT_INPUT, "size");
    addPort("out", PORT_OUTPUT, "size");
    addPort("bidir", PORT_BIDIR, "size");

    addSubModule(new Register("reg_in", size));
    addSubModule(new Register("reg_out", size));
    addSubModule(new TriState(TriState::Mode::PROVIDES_IO_OP, "OE", size));
    addConfig(new ConfigCell("OEConfig"), {"OE.enable"});

    addConnection("this.in", "reg_in.in");
    addConnection("reg_in.out", "OE.in");
    addConnection("this.bidir", "reg_out.in");
    addConnection("OE.out", "this.bidir");
    addConnection("reg_out.out", "this.out");

    // module type
    mt = MOD_COMPOSITE;
}

std::string IO::GenericName()
{
    return "io_" + std::to_string(getSize()) + "b";
}

IO::~IO()
{
}

/************ CustomModule **********/
CustomModule::CustomModule(std::string name, std::vector<std::string> Function, unsigned size)
    : Module(name, size)
{
    this->templateName = name;

    // Create ports
    addPort("a", PORT_INPUT, "size");
    addPort("b", PORT_INPUT, "size");
    addPort("c", PORT_OUTPUT, "size");

    // Module type
    mt = MOD_PRIM_CUSTOM;

    // Getting the functionality
    this->Function = Function;
}

// Virtual function that overrides Module::GetFunctionality. Generates the functionality for CustomModule
void CustomModule::GenFunctionality()
{
    for (unsigned i = 0; i < Function.size(); i++)
        std::cout << SET_INDENT << Function[i] << "\n";
}

std::string CustomModule::GenericName()
{
    return name;
}

CustomModule::~CustomModule()
{
}


/************ UserModule **********/
UserModule::UserModule(std::string prototype, std::string name, std::vector<std::string> Ports)
    : Module (name)
{
    // assigning prototype name
    this->prototype = prototype;

    // adding ports
    for (unsigned i = 0; i < Ports.size(); i++)
    {
        addPort(Ports[i], PORT_UNSPECIFIED); // We do not care if the port is input or output
    }
    // module type
    mt = MOD_USER;

}

// Virtual function that overrides Module::GenericName. Returns generic name of the object
std::string UserModule::GenericName() // Virtual function override
{
    return prototype;
}

UserModule::~UserModule()
{
}

/************ ConfigCell **********/
ConfigCell::ConfigCell(std::string name)
{
    this->name = name;
    cell_size = 1; // Default size
}

// Prints out the Verilog code for the config cell
void ConfigCell::GenModuleVerilog()
{
    // Module header
    std::cout << "module ConfigCell(ConfigIn, ConfigOut, Config_Clock, Config_Reset, select);\n";
    // Parameter specification
    std::cout << SET_INDENT << "parameter size = 1;\n";
    // Port specifications
    std::cout << SET_INDENT << "input Config_Clock, Config_Reset, ConfigIn;\n";
    std::cout << SET_INDENT << "output ConfigOut;\n";
    std::cout << SET_INDENT << "output [size-1:0] select;\n";
    // Functionality
    std::cout << SET_INDENT << "reg [size-1:0] temp;\n\n";
    std::cout << SET_INDENT << "always @(posedge Config_Clock, posedge Config_Reset)\n";
    std::cout << SET_DOUBLE_INDENT << "if (Config_Reset)\n";
    std::cout << SET_TRIPLE_INDENT << "temp = 0;\n";
    std::cout << SET_DOUBLE_INDENT << "else\n";
    std::cout << SET_TRIPLE_INDENT << "begin\n";
    std::cout << SET_QUAD_INDENT << "temp = temp >> 1;\n";
    std::cout << SET_QUAD_INDENT << "temp[size-1] = ConfigIn;\n";
    std::cout << SET_TRIPLE_INDENT << "end\n";
    std::cout << SET_INDENT << "assign select = temp;\n";
    std::cout << SET_INDENT << "assign ConfigOut = temp[0];\n";
    std::cout << "endmodule\n";
}

/************ Extractor ***********/
static const std::map<std::string, port_type> port_type_map =
{
    {"input", PORT_INPUT},
    {"output", PORT_OUTPUT},
    {"output-reg", PORT_OUTPUT_REG},
    {"bidir", PORT_BIDIR},
    {"unspecified", PORT_UNSPECIFIED},
};

std::istream& operator >>(std::istream &is, port_type &type)
{
    std::string str;
    is >> str;
    if (port_type_map.count(str) > 0)
        type = port_type_map.at(str);
    else
        is.setstate(std::ios_base::failbit);

    return is;
}

