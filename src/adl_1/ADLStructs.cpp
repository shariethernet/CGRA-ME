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

#include <vector>
#include <map>
#include <tuple>
#include <iostream>

#include <CGRA/Module.h>
#include <CGRA/ModuleComposites.h>
#include <CGRA/CGRA.h>

#include <pugixml.hpp>

#include "ADLStructs.h"

#define DEBUG_OUT 0

namespace adl1 {

bool ADLStructs::add_module_template(pugi::xml_node module)
{
    if(DEBUG_OUT) std::cout << "\nAdding module: ";

    //Create an instange of a moduleTemplate
    moduleTemplate temp;

    //Look for the module name
    //If there is no name, do something. Otherwise, continue like normal
    auto nameAttr = module.attribute("name");
    if(!nameAttr) {
        if(DEBUG_OUT) std::cout << "One of your modules does not have a name\n";
        return false;
    }
    temp.name = nameAttr.value();
    std::cout << temp.name<< std::endl;


    //Look for the ports in the module
    //These will look like input/output/inout inside the xml
    //First I will scan for inputs
    if(DEBUG_OUT) std::cout << "Scanning for input ports...\n";
    for(auto elem : module.children("input")) {
        if(DEBUG_OUT) std::cout << "Input port found\n";
        //ToDo: Check for errors here
        auto portNameAttr = elem.attribute("name");
        std::string portName = portNameAttr.value();
        auto portSizeAttr = elem.attribute("size");

        //NOTE: Define a default size later
        unsigned size = (portSizeAttr) ? std::stoi(portSizeAttr.value()) : 32;
        temp.addPort(portName, PORT_INPUT, size);
        if(DEBUG_OUT) std::cout << "    Name: " << portName << "\n    Size: " << size << std::endl;
    }


    //Now look for the outputs
    if(DEBUG_OUT) std::cout << "\nScanning for output ports...\n";
    for(auto elem : module.children("output")) {
        if(DEBUG_OUT) std::cout << "Output port found\n";
        //ToDo: Check for errors here
        auto portNameAttr = elem.attribute("name");
        std::string portName = portNameAttr.value();
        auto portSizeAttr = elem.attribute("size");

        //NOTE: Define a default size later
        unsigned size = (portSizeAttr) ? std::stoi(portSizeAttr.value()) : 32;
        temp.addPort(portName, PORT_OUTPUT, size);
        if(DEBUG_OUT) std::cout << "    Name: " << portName << "\n    Size: " << size << std::endl;
    }


    //Now look for the inouts
    if(DEBUG_OUT) std::cout << "\nScanning for inout ports...\n";
    for(auto elem : module.children("inout")) {
        if(DEBUG_OUT) std::cout << "Inout port found\n";
        //ToDo: Check for errors here
        auto portNameAttr = elem.attribute("name");
        std::string portName = portNameAttr.value();
        auto portSizeAttr = elem.attribute("size");

        //NOTE: Define a default size later
        unsigned size = (portSizeAttr) ? std::stoi(portSizeAttr.value()) : 32;
        temp.addPort(portName, PORT_BIDIR, size);
        if(DEBUG_OUT) std::cout << "    Name: " << portName << "\n    Size: " << size << std::endl;
    }


    //Look for wires. Set the second element of the map
    if(DEBUG_OUT) std::cout << "\nScanning for wires...\n";
    for(auto elem : module.children("wire")) {
        if(DEBUG_OUT) std::cout << "Wire found\n";
        std::string wireName = elem.attribute("name").value();
        temp.wires.insert(std::make_pair(wireName, ""));
    }


    //Now scan for primitives
    if(DEBUG_OUT) std::cout << "\nScanning for primitives...\n";
    for(auto elem : module.children("inst")) {
        if(DEBUG_OUT) std::cout << "Inst found. Args are:\n";
        std::map<std::string, std::string> m;
        for(pugi::xml_attribute attr = elem.first_attribute(); attr; attr = attr.next_attribute()) {
            std::cout << "    " << attr.name() << "  " << attr.value() << std::endl;
            m.insert(std::pair<std::string, std::string>(attr.name(), attr.value()));
        }
        std::string type = elem.attribute("module").value();
        temp.primitives.push_back(std::pair<std::string, std::map<std::string, std::string>>(type, m));
    }

    //Now scan for submodules
    if(DEBUG_OUT) std::cout << "\nScanning for sub modules..\n";
    for(auto elem : module.children("sub-module")) {
        if(DEBUG_OUT) std::cout << "Submodule found\n";

        //Get the name of the submodule
        std::string name = elem.attribute("name").value();
        std::string moduleType = elem.attribute("module").value();
        temp.subModules.push_back(std::make_pair(name, moduleType));
    }

    //Now scan for connections
    int muxCount = 0;
    if(DEBUG_OUT) std::cout << "\nScanning for connections...\n";
    for(auto elem : module.children("connection")) {
        if(DEBUG_OUT) std::cout << "Connection found\n";

        //Check to see what type of connections exist here
        auto toAttr = elem.attribute("to");
        auto distributeToAttr = elem.attribute("distribute-to");
        auto fromAttr = elem.attribute("from");
        auto selectFromAttr = elem.attribute("select-from");

        //Variables that hold the information that will be put into the tuple
        std::string toName;
        std::string toType;
        std::string fromName;
        std::string fromType;

        //Fill the names of each
        if(toAttr) {
            toName = toAttr.value();
            toType = "to";
        } else if(distributeToAttr) {
            toName = distributeToAttr.value();
            toType = "distribute-to";
        } else {
            //ToDo: error handling here
        }

        if(fromAttr) {
            fromName = fromAttr.value();
            fromType = "from";
        } else if(selectFromAttr) {
            fromName = selectFromAttr.value();
            fromType = "select-from";
        } else {
            //ToDo: error handing here
        }

        //Add the connection to the vector
        if(fromType == "from" && toType == "to") {

            if(temp.wires.find(toName) != temp.wires.end()) {
                //There is a wire here
                (temp.wires.find(toName))->second = fromName;
            }

            else {
                temp.connections.push_back(std::make_tuple(toType, toName, fromType, fromName));
                std::cout << "    " << toType << ": " << toName << std::endl << fromType << ": " << fromName << std::endl;
            }
        }
        if(fromType == "from" && toType == "distribute-to") {
            //Make a connection to from the from to each of the tos
            std::stringstream ss;
            ss << toName;
            std::string to;
            while(ss>>to) {
                if(temp.wires.find(to) != temp.wires.end()) {
                    (temp.wires.find(to))->second = fromName;
                }
                else {
                    temp.connections.push_back(std::make_tuple("to", to, "from", fromName));
                    std::cout << "    to: " << to << std::endl << fromType << ": " << fromName << std::endl;
                }
            }
        }
        if(fromType == "select-from" && toType == "to") {
            //Determine how many inputs each mux should have
            std::stringstream ss;
            ss << fromName;
            int numInputs = 0;
            int numMuxes = 0;
            std::string tempString;
            while(ss >> tempString) numInputs++;
            std::stringstream aa;
            aa << toName;
            while(aa >> tempString) numMuxes++;

            //Create each mux and add the connections. Naming convention is dumb
            std::stringstream toNames;
            toNames << toName;
            for(int i = 0; i < numMuxes; i++) {
                std::string muxName = temp.name + "_mux" +  std::to_string(muxCount);

                //Create the mux
                std::map<std::string, std::string> m;
                m.insert(std::make_pair("name", muxName));
                m.insert(std::make_pair("size", "32"));
                m.insert(std::make_pair("ninput", std::to_string(numInputs)));
                temp.primitives.push_back(std::make_pair("Multiplexer", m));
                std::stringstream fromNames;
                fromNames << fromName;
                for(int j = 0; j < numInputs; j++) {
                    std::string from, to;
                    to = muxName + ".in" + std::to_string(j);
                    fromNames >> from;
                    temp.connections.push_back(std::make_tuple("to", to, "from", from));
                }
                std::string to;
                toNames >> to;
                if(temp.wires.find(to) != temp.wires.end()) {
                    (temp.wires.find(to))->second = muxName + ".out";
                }
                else temp.connections.push_back(std::make_tuple("to", to, "from", muxName + ".out"));
                muxCount++;
            }
        }
    }

    //Replace the wires with their corresponding ports
    for(auto &elem : temp.connections) {
        if(temp.wires.find(std::get<3>(elem)) != temp.wires.end())
            std::get<3>(elem) = temp.wires.find(std::get<3>(elem))->second;
        if(temp.wires.find(std::get<1>(elem)) != temp.wires.end())
            std::get<1>(elem) = temp.wires.find(std::get<1>(elem))->second;
    }

    //The module template information has been filled
    //Tell the user and add it to the vector of module templates
    if(DEBUG_OUT) std::cout << "\nThe module template information has been added.\n\n";
    moduleTemplates.push_back(std::pair<std::string, moduleTemplate>(temp.name, temp));

    return true;
}

void ADLStructs::print() {

    //Print the templates
    std::cout << "Printing the module templates\n";
    for(auto elem : moduleTemplates) {
        elem.second.print();
    }
    //Call the pring function for the architecture data
    arch.print();
    //ToDo: add other print stuff later
    std::cout << "\n\nPrinting connections:\n";
    for(const auto &elem : connections) {
        std::cout << std::get<0>(elem) << ": " << std::get<1>(elem) << std::endl << std::get<2>(elem) << ": " << std::get<3>(elem) << "\n\n";
    }

    //Submodules
    std::cout << "\n\nPrinting submodules:\n";
    for(auto elem : subModules) {
        std::cout << elem.first << std::endl;
    }
}

}

