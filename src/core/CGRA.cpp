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

#include <CGRA/CGRA.h>
#include <CGRA/ModuleComposites.h>
#include <assert.h>
#include <algorithm>
#include <fstream>


CGRA::CGRA()
    : Module("CGRA")
{
    maxII = -1;     // For sanity... the creator of the CGRA object should set this to the correct value for the modelled CGRA
    templateName = "cgra";
}

CGRA::~CGRA()
{
    // TODO: This may be better suited to be implemented in the MRRG class while also providing correct copy constructors
    for(int j = 0; j < mrrgs.size(); j++)
    {
        if(mrrgs[j])
        {
            for(int i = 0; i <  mrrgs[j]->II; i++)
            {
                for(auto& it : mrrgs[j]->nodes[i]) // Deleting connections
                {
                    delete it.second;
                }
            }
        }
    }
}

// TODO: this function does not account for maxII
std::shared_ptr<MRRG> CGRA::getMRRG(int II)
{
    if(mrrgs.size() < II)
    {
        mrrgs.resize(II, std::shared_ptr<MRRG>(NULL));
    }

    if(!mrrgs[II-1].get())
    {
        mrrgs[II-1] = std::shared_ptr<MRRG>(createMRRG(II));
        mrrgs[II-1]->finalize();
    }

    return mrrgs[II-1];
}


// Function that generates the bitstream
BitStream CGRA::genBitStream(const Mapping& mapping)
{
    std::vector<ConfigCell*> config_table; // Table that holds modules that need to be configured
    genConfigOrder(config_table); // Initializing the table
    BitStream bitstream;

    std::cout << "config table: \n\t";
    for (auto* p : config_table) {
        std::cout << p << ' ';
    }
    std::cout << '\n';

    std::map<const Module*, std::map<OpGraphOp*, std::set<MRRGNode*>>> mrrgnodes_for_op_node_for_module;
    for(auto* op : mapping.getOpGraph().op_nodes) // For all OPNodes, we want to get bits for IO devices and FuncUnits
    {
        for (auto* n : mapping.getMappingList(op)) {
            mrrgnodes_for_op_node_for_module[n->parent][op].insert(n);
        }
    }

    std::cout << "Value mappings: (val node: {mrrg node })\n";
    for(auto* val : mapping.getOpGraph().val_nodes) {
        std::cout << val << ":";
        for (auto* nptr : mapping.getMappingList(val)) {
            const auto& mrrg_node = *nptr;
            std::cout << mrrg_node << " ";
        }
        std::cout << '\n';
    }

    std::map<const Module*, std::map<OpGraphVal*, std::set<MRRGNode*>>> mrrgnodes_for_val_node_for_module;
    for(auto* val : mapping.getOpGraph().val_nodes) // For all val nodes, we examine them to determine how to configure the multiplexers
    {
        for(auto* n : mapping.getMappingList(val)) // For all mapped nodes
        {
            mrrgnodes_for_val_node_for_module[n->parent][val].insert(n);
        }
    }

    if (mrrgnodes_for_val_node_for_module.find(nullptr) != end(mrrgnodes_for_val_node_for_module)) {
        std::cout << "detected mrrg node(s) with unset parent (no containing Module)!:\n";
        // for (const auto& val_and_nodes : module_and_mrrg_nodes_for_val_node.second) {
        //     std::cout << '\t' << *val_and_nodes.first << '\n';
        //     for (const auto& mrrg_node : val_and_nodes.second) {
        //         std::cout << "\t\t" << *mrrg_node << '\n';
        //     }
        // }
    }

    for (const auto* ccell : config_table) {
        const auto* module = &ccell->getSingleConnectedPort().getModule();

        const auto module_and_mrrg_nodes_for_val_node = mrrgnodes_for_val_node_for_module.find(module);
        const auto module_and_mrrg_nodes_for_op_node = mrrgnodes_for_op_node_for_module.find(module);
        const bool val_nodes_available = module_and_mrrg_nodes_for_val_node != end(mrrgnodes_for_val_node_for_module);
        const bool op_nodes_available = module_and_mrrg_nodes_for_op_node != end(mrrgnodes_for_op_node_for_module);
        const auto value_for_bitstream = [&](){
            try {
                if (val_nodes_available) {
                    if (op_nodes_available) {
                        return module->getBitConfig(*ccell, module_and_mrrg_nodes_for_op_node->second, module_and_mrrg_nodes_for_val_node->second);
                    } else {
                        return module->getBitConfig(*ccell, {}, module_and_mrrg_nodes_for_val_node->second);
                    }
                } else {
                    if (op_nodes_available) {
                        return module->getBitConfig(*ccell, module_and_mrrg_nodes_for_op_node->second, {});
                    } else {
                        return module->getBitConfig(*ccell, {}, {});
                    }
                }
            } catch (...) {
                std::cout << "Exception thrown when getting bitconfig from " << module->getName() << " with the following input:" << std::endl;
                std::cout << "ConfigCell name: " << ccell->getName() << std::endl;
                std::cout << "Op Nodes:" << std::endl;
                if (op_nodes_available) {
                    for (const auto& op_and_nodes : module_and_mrrg_nodes_for_op_node->second) {
                        std::cout << '\t' << *op_and_nodes.first << std::endl;
                        for (const auto& mrrg_node : op_and_nodes.second) {
                            std::cout << "\t\t" << *mrrg_node << std::endl;
                        }
                    }
                } else {
                    std::cout << "\t[ none ]" << std::endl;
                }
                std::cout << "Value Nodes:" << std::endl;
                if (val_nodes_available) {
                    for (const auto& val_and_nodes : module_and_mrrg_nodes_for_val_node->second) {
                        std::cout << '\t' << *val_and_nodes.first << std::endl;
                        for (const auto& mrrg_node : val_and_nodes.second) {
                            std::cout << "\t\t" << *mrrg_node << std::endl;
                        }
                    }
                } else {
                    std::cout << "\t[ none ]" << std::endl;
                }
                std::cout << "End listing" << std::endl;
                throw;
            }
        }();

        if (value_for_bitstream.size() != ccell->getStorageSize()) {
            std::cout << ("wrong number of bits returned for " + module->getName() + "::" + ccell->getName()) << ": " << value_for_bitstream << ". Expected " << ccell->getStorageSize() << " bits\n";
            throw cgrame_error("wrong number of bits returned for " + module->getName() + "::" + ccell->getName());
        } else {
            bitstream.append(ccell, value_for_bitstream);
        }
    }

    bitstream.print_debug(); // Prints bitstream in debug mode (with spaces seperating bits for different config cells)

    // for (auto* mptr : config_table) {
    //     if (mptr) {
    //         std::cout << mptr->getName();
    //     }
    //     std::cout << '\n';
    // }

    return bitstream;
}

/*
// Generates timing constraints for CGRA using an opgraph that is already mapped
void CGRA::genTimingConstraints(OpGraph * opgraph)
{
    // the Tickle script we will be writing to
    std::ofstream Timing;
    Timing.open("primetime.tcl");

    // Similar to the bitstream generator, we want all modules that we need to configure
    std::vector<Module*> ConfigTable; // Table that holds modules that need to be configured
    genConfigOrder(ConfigTable); // Initializing the table

    // Disabling all IO's, Muxes, and FuncUnits
    Timing << "# disabling required timing paths\n";
    for (unsigned i = 0; i < ConfigTable.size(); i++)
    {
        if (ConfigTable[i]->getModuleType() == MOD_PRIM_MUX) // If it's a configurable mux, disable it (no need to disable output port if all inputs are disabled
        {
            Timing << "set_disable_timing " << ConfigTable[i]->ReturnPath() << "/in*\n"; // "in" is hardcoded for mux inputs"
        }
        else if (ConfigTable[i]->getModuleType() == MOD_PRIM_FUNC)
        {
            for (auto it = ConfigTable[i]->submodules.begin(); it != ConfigTable[i]->submodules.end(); it++) // If it's a FuncUnit, disable all submodules
            {
                Timing << "set_disable_timing " << it->second->ReturnPath() << "\n";
            }
        }
        else if (ConfigTable[i]->parent->getModuleType() == MOD_PRIM_IO) // If its parent is an IO device
        {
            Timing << "set_disable_timing " << ConfigTable[i]->parent->ReturnPath() << "\n";
        }
        else if (ConfigTable[i]->getModuleType() == MOD_PRIM_TRI) // If it's just a Tristate, and it's parent is not an IO device
        {
            Timing << "set_disable_timing " << ConfigTable[i]->ReturnPath() << "\n";
        }
    }

    // Re-enabling the required paths
    Timing << "# now re-enabling required timing paths\n";
    for(auto & op: opgraph->op_nodes) // For all OPNodes, we want to get bits for IO devices and FuncUnits
    {
        if (op->opcode == OPGRAPH_OP_INPUT) // If it's an input node -> IO
        {
            assert(op->mapped_nodes[0]->parent->getModuleType() == MOD_PRIM_IO); // We assume we are dealing with an IO
            Timing << "remove_disable_timing " << op->mapped_nodes[0]->parent->ReturnPath() << "\n";
            Timing << "set_disable_timing " << op->mapped_nodes[0]->parent->ReturnPath() << "/OE\n";
        }
        else if (op->opcode == OPGRAPH_OP_OUTPUT) // If it's an output node -> IO
        {
            assert(op->mapped_nodes[0]->parent->getModuleType() == MOD_PRIM_IO); // We assume we are dealing with an IO
            Timing << "remove_disable_timing " << op->mapped_nodes[0]->parent->ReturnPath() << "\n";
        }
        else // If it's a function node->FuncUnit
        {
            assert(op->mapped_nodes[0]->parent->getModuleType() == MOD_PRIM_FUNC); // We assume we are dealing with an FuncUnit
            Timing << "remove_disable_timing " << op->mapped_nodes[0]->parent->ReturnPath()
                << "/" << ((FuncUnit*)op->mapped_nodes[0]->parent)->all_modes[op->opcode].ModuleName << "\n";
        }
    }
    for(auto & val: opgraph->val_nodes) // Dealing with the Multiplexers
    {
        for(auto & n: val->mapped_nodes)
        {
            if (n->pt == MUX_IN) // If it's a mux in node
            {
                Timing << "remove_disable_timing " << n->parent->ReturnPath() << "/in";
                Timing << getNumAfterString(n->name, ".in") << "\n";
            }
        }
    }
    Timing.close();
}
*/

// Floorplans CGRABlocks on ASIC chip
void CGRA::genFloorPlan()
{
    // the Tickle script we will be writing to
    std::ofstream Floorplan;
    Floorplan.open("encounter.tcl");
    // Setting up the variables in the tcl script
    Floorplan << "variable blockarea\n"; // Area of a CGRA block
    Floorplan << "variable blocklength [expr sqrt($blockarea) / 0.85]\n"; // How long a block will be
    Floorplan << "variable N " << ROWS << "\n"; // # of rows in CGRA
    Floorplan << "variable M " << COLS << "\n"; // # of cols in CGRA
    Floorplan << "variable cgrawidth [expr $N*$blocklength]\n"; // CGRA width
    Floorplan << "variable cgraheight [expr $M*$blocklength]\n"; // CGRA height
    Floorplan << "variable border 20\n"; // Width of the border used on ASIC chip
    // Loading the libraries required
    Floorplan << "# loading libraries\n";
    Floorplan << "loadConfig encounter.conf 0\n";
    Floorplan << "setUIVar rda_Input ui_netlist CGRA_comp.v\n";
    Floorplan << "setUIVar rda_Input ui_topcell CGRA\n";
    Floorplan << "commitConfig\n";
    // Setting up the chip
    Floorplan << "# set up chip\n";
    Floorplan << "floorPlan -site core -s $cgrawidth $cgraheight $border $border $border $border\n";

    // Floorplanning the CGRABlocks
    Floorplan << "# floorplan blocks\n";

    unsigned CurrentROW = 0, CurrentCOL = 0;
    for (auto it = submodules.begin(); it != submodules.end(); it++) // Iterate through all the non-IO modules in CGRA
    {
        if (it->second->getModuleType() != MOD_PRIM_IO) // If it's not an IO device, floorplan it
        {
            Floorplan << "setObjFPlanBox Module " << it->second->getName();
            // First argument
            Floorplan << " [expr $border+" << CurrentCOL << "*$blocklength] ";
            // Second argument
            Floorplan << "[expr $cgraheight+$border-" << (CurrentROW+1) << "*$blocklength] ";
            // Third argument
            Floorplan << "[expr $border+" << (CurrentCOL+1) << "*$blocklength] ";
            // Fourth argument
            Floorplan << "[expr $cgraheight+$border-" << CurrentROW << "*$blocklength]\n";
            CurrentCOL++; // Move to next position in CGRA
            if (CurrentCOL == COLS) // Change to a new row if we reached the end of it
            {
                CurrentCOL = 0;
                CurrentROW++;
            }
        }
    }

    // Placing the design
    Floorplan << "# placing design\n";
    Floorplan << "getMultiCpuUsage -localCpu\n";
    Floorplan << "setPlaceMode -fp false\n";
    Floorplan << "placeDesign -prePlaceOpt\n";

    // Routing the design
    Floorplan << "# routing design\n";
    Floorplan << "setNanoRouteMode -quiet -drouteStartIteration default\n";
    Floorplan << "setNanoRouteMode -quiet -routeTopRoutingLayer default\n";
    Floorplan << "setNanoRouteMode -quiet -routeBottomRoutingLayer default\n";
    Floorplan << "setNanoRouteMode -quiet -drouteEndIteration default\n";
    Floorplan << "setNanoRouteMode -quiet -routeWithTimingDriven false\n";
    Floorplan << "setNanoRouteMode -quiet -routeWithSiDriven false\n";
    Floorplan << "routeDesign -globalDetail\n";

    // Saving the design
    Floorplan << "saveDesign CGRA.enc\n";
    // Getting spef file for Primetime
    Floorplan << "rcOut -spef CGRA.spef\n";
    // Getting netlist for Primetime
    Floorplan << "saveNetlist CGRA_Golden.v\n";

    Floorplan.close();
}

