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

//This is the new XML language parser. It performs the same functions as the old one created by Noriaki, but does so in a simpler manner. His problem solving methods were used.

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <sstream>

#include <ctype.h>

#include <CGRA/user-inc/UserFunction.h>

#include <pugixml.hpp>

#include "ADLParser.h"
#include "ADLStructs.h"
#include "SyntacticSugar.h"

#define DEBUG_OUT 0

namespace adl1 {

std::shared_ptr<CGRA> parseADL(std::string templateFileName, std::string archFileName)
{
    //Parse the file that has the module templates inside of it
    pugi::xml_document templateDoc;
    auto templateSource = templateFileName.c_str();

    pugi::xml_parse_result templateResult = templateDoc.load_file(templateSource);

    //Create an instance of the class that holds the info for the API caller
    ADLStructs xmlData;

    //Fill the module templates of the xmlData by passing it by reference to this function that will
    //return a bool indicating whether or not it was successful in filling the structs
    bool templatesFilled = fill_module_templates(templateDoc, xmlData);

    //If it was unsuccessful, abort
    if(!templatesFilled) {
        std::cout << "[ERROR] There was an error in parsing the module templates file... aborting\n";
        return NULL;
    }

    if(DEBUG_OUT) std::cout << "Templates file [" << templateSource << "] parsed without errors.\nBegin parsing architecture file\n";

    //Parse the file that has the architecture inside of it
    pugi::xml_document archDoc;
    auto archSource = archFileName.c_str();
    pugi::xml_parse_result archResult = archDoc.load_file(archSource);

    //Fill the architecture portion of the structures by passing xmlData by reference to this function
    bool archFilled = fill_arch_data(archDoc, xmlData);

    //If unsuccessful, abort
    if(!archFilled) {
        std::cout << "[ERROR] There was an error in parsing the architecture file... aborting\n";
        return NULL;
    }

    //Call the API to create a new CGRA
    std::shared_ptr<CGRA> result = API_caller(xmlData);
    return result;
};

bool fill_module_templates(pugi::xml_node templateDoc, ADLStructs &xmlData)
{
    //The template document begins with <templates>
    //Grab this child of the template document
    auto templates = templateDoc.child("templates");

    if(DEBUG_OUT) std::cout << "Begin finding templates:\n";

    //Go through each module tag here
    for(pugi::xml_node mod : templates.children("module")) {
        if(DEBUG_OUT) std::cout << "Module tag found\n";
        xmlData.add_module_template(mod);
    }
    return true;
};

bool fill_arch_data(pugi::xml_node archDoc, ADLStructs &xmlData)
{
    //std::cout << "Begin filling architecture\n";
    //The architecture document begins with <architecture>
    //Grab this child of the arch document
    auto architecture = archDoc.child("architecture");

    //Set the rows and cols of the CGRA
    auto archRowAttr = architecture.attribute("row");
    auto archColAttr = architecture.attribute("col");
    std::string archRowName = archRowAttr.value();
    std::string archColName = archColAttr.value();
    xmlData.arch.setArchRows(atoi(archRowName.c_str()));
    xmlData.arch.setArchCols(atoi(archColName.c_str()));

    //Look for all of the patterns and act on them
    if(DEBUG_OUT) std::cout << "\nBegin scanning for patterns...\n";
    for(auto pattern : architecture.children("pattern")) {
        //extract the start and the end of the ranges into the xmlData
        std::stringstream colRange, rowRange;
        std::string colRangeStr;
        int patternStartCol, patternEndCol;

        //ToDo: include error checking here for 2 arguments of col-range
        colRangeStr = pattern.attribute("col-range").value();
        colRange << colRangeStr;
        colRange >> patternStartCol >> patternEndCol;
        xmlData.arch.setPatternStartCol(patternStartCol);
        xmlData.arch.setPatternEndCol(patternEndCol);

        std::string rowRangeStr;
        int patternStartRow, patternEndRow;

        rowRangeStr = pattern.attribute("row-range").value();
        rowRange << rowRangeStr;
        rowRange >> patternStartRow >> patternEndRow;
        xmlData.arch.setPatternStartRow(patternStartRow);
        xmlData.arch.setPatternEndRow(patternEndRow);
        //Do what the pattern says
        if(DEBUG_OUT) std::cout << "\nPattern found\nRow range: " << rowRangeStr << "\nCol Range: " << colRangeStr << std::endl;
        declare_pattern(pattern, xmlData);
    }

    for(auto mesh : architecture.children("mesh")) {
        create_orthogonal_mesh(xmlData, mesh);
    }

    for(auto diagonal : architecture.children("diagonal")) {
        create_diag_mesh(xmlData, diagonal);
    }

    return true;
}

bool declare_pattern(pugi::xml_node pattern, ADLStructs &xmlData)
{
std::cout << "Looking for patterns";
    //The 'row' and 'col' attributes determine how to increment the for loop
    //I must get these attributes, and if they are not there, they default to 1
    auto colAttr = pattern.attribute("cols");
    auto rowAttr = pattern.attribute("rows");
    std::string colAttrValue = colAttr.value();
    std::string rowAttrValue = rowAttr.value();
    int colInc = (colAttr) ? atoi(colAttrValue.c_str()) : 1;
    int rowInc = (rowAttr) ? atoi(rowAttrValue.c_str()) : 1;
    auto rowSkipAttr = pattern.attribute("row-skip");
    auto colSkipAttr = pattern.attribute("col-skip");
    std::string rowSkipAttrVal = rowSkipAttr.value();
    std::string colSkipAttrVal = colSkipAttr.value();
    int rowSkip = (rowSkipAttr) ? atoi(rowSkipAttrVal.c_str()) : 0;
    int colSkip = (colSkipAttr) ? atoi(colSkipAttrVal.c_str()) : 0;
    colInc += colSkip;
    rowInc += rowSkip;

    //Here I will do the counter things
    //I will have a vector that keeps track of the counter names and their values
    auto counterAttr = pattern.attribute("counter");
    auto rowCounterAttr = pattern.attribute("row-counter");
    auto colCounterAttr = pattern.attribute("col-counter");
    auto wrapAttr = pattern.attribute("wrap-around");

    std::string counterName = counterAttr.value();
    std::string rowCounterName = rowCounterAttr.value();
    std::string colCounterName = colCounterAttr.value();
    std::string wrapVal = wrapAttr.value();

    bool wrap = wrapVal == "on" ? true : false;

    //Execute the pattern by going through each column in a row before moving to the next row
    //ToDo: error checking with bounds in the block declaration
    for(int row = xmlData.arch.getPatternStartRow(); row <= xmlData.arch.getPatternEndRow(); row += rowInc) {
        for(int col = xmlData.arch.getPatternStartCol(); col <= xmlData.arch.getPatternEndCol(); col += colInc) {
            xmlData.arch.setCurrentRow(row);
            xmlData.arch.setCurrentCol(col);

            //Look for blocks in patterns and declare the blocks
            //First check that the number of blocks declared in the pattern is less than the subpattern rows and cols
            if(pattern.select_nodes("block").size() > (colInc*rowInc)) {
                if(DEBUG_OUT) std::cout << "Too many blocks declared in the pattern.\n";
                return false;
            }
            if(pattern.select_nodes("connection").size() > (colInc*rowInc)) {
                if(DEBUG_OUT) std::cout << "Too many connections declared in the pattern.\n";
            }

            //Declare each block. The naming convention is here also
            for(auto elem : pattern.children("block")) {
                std::string identifier = elem.attribute("module").value();
                if(DEBUG_OUT) std::cout << "\nBlock found. Creating instance of module " << identifier << std::endl;
                std::string name = "block_" + std::to_string(xmlData.arch.getCurrentRow()) + "_" + std::to_string(xmlData.arch.getCurrentCol());

                if(counterAttr) {
                    int value = (xmlData.arch.getCurrentRow() - xmlData.arch.getPatternStartRow()) * (1 + xmlData.arch.getPatternEndCol() - xmlData.arch.getPatternStartCol()) + (xmlData.arch.getCurrentCol() - xmlData.arch.getPatternStartCol());
                    identifier = expand_counter_name(identifier, counterName, value);
                    // std::cout << "Expanded name to " << identifier << std::endl;
                }

                if(rowCounterAttr) {
                    int value = xmlData.arch.getCurrentRow() - xmlData.arch.getPatternStartRow();
                    identifier = expand_counter_name(identifier, rowCounterName, value);
                    // std::cout << "Expanded name to " << identifier << std::endl;
                }

                if(colCounterAttr) {
                    int value = xmlData.arch.getCurrentCol() - xmlData.arch.getPatternStartCol();
                    identifier = expand_counter_name(identifier, colCounterName, value);
                    // std::cout << "Expanded name to " << identifier << std::endl;
                }

                if(DEBUG_OUT) std::cout << "Creating " << name << std::endl;
                moduleTemplate temp;
                for(auto elem : xmlData.moduleTemplates) {
                    if(elem.first == identifier) {
                        temp = elem.second;
                        if(DEBUG_OUT) std::cout << "Module " << identifier << " found within templates\n";
                    }
                }
                xmlData.subModules.push_back(std::pair<std::string, moduleTemplate>(name, temp));

                //Move over to the next operating block if I need to
                xmlData.arch.setCurrentCol(xmlData.arch.getCurrentCol()+1);
                if(xmlData.arch.getCurrentCol() > xmlData.arch.getPatternEndCol()) {
                    xmlData.arch.setCurrentCol(col);
                    xmlData.arch.setCurrentRow(xmlData.arch.getCurrentRow() + rowInc);
                }
            }

            //Now I must look for ports that are added to the top level module
            //Now that I'm done iterating through the block creations, I'll iterate through the connection creations
            //But first I must reset my current operating row and col
            xmlData.arch.setCurrentRow(row);
            xmlData.arch.setCurrentCol(col);

            for(auto elem : pattern.children("inout")) {
                //Keep track of counters for expanding names
                std::vector<std::pair<std::string, int>> counters;

                auto portNameAttr = elem.attribute("name");
                std::string portName = portNameAttr.value();
                if(counterAttr) {
                    int value = (xmlData.arch.getCurrentRow() - xmlData.arch.getPatternStartRow()) * (1 + xmlData.arch.getPatternEndCol() - xmlData.arch.getPatternStartCol()) + (xmlData.arch.getCurrentCol() - xmlData.arch.getPatternStartCol());
                    portName = expand_counter_name(portName, counterName, value);
                    counters.push_back(std::make_pair(counterName, value));
                }

                if(rowCounterAttr) {
                    int value = xmlData.arch.getCurrentRow() - xmlData.arch.getPatternStartRow();
                    portName = expand_counter_name(portName, rowCounterName, value);
                    counters.push_back(std::make_pair(rowCounterName, value));
                }

                if(colCounterAttr) {
                    int value = xmlData.arch.getCurrentCol() - xmlData.arch.getPatternStartCol();
                    portName = expand_counter_name(portName, colCounterName, value);
                    counters.push_back(std::make_pair(colCounterName, value));
                }
                auto portSizeAttr = elem.attribute("size");
                unsigned size = (portSizeAttr)? std::stoi(portSizeAttr.value()) : 32;
                xmlData.ports.push_back(std::make_tuple(portName, PORT_BIDIR, size));

            }

            for(auto elem : pattern.children("connection")) {
                //First, we must check to see if the user is refering to a relative block, or an absolute block
                //Check to see what type of connections exist here
                auto toAttr = elem.attribute("to");
                auto distributeToAttr = elem.attribute("distribute-to");
                auto fromAttr = elem.attribute("from");
                auto selectFromAttr = elem.attribute("select-from");
                auto busWidthAttr = elem.attribute("size");

                //Variables that hold the information that will be put into the tuple
                std::string toName;
                std::string toType;
                std::string fromName;
                std::string fromType;
                std::string block, port;

                //Keep track of counters for expanding names
                std::vector<std::pair<std::string, int>> counters;

                //Fill the names of each
                if(toAttr && distributeToAttr) {
                    throw cgrame_error("[ADL1] only one of \"to\" and \"distribute-to\" is allowed");
                } else if(toAttr) {
                    toName = toAttr.value();
                    toType = "to";
                } else if(distributeToAttr) {
                    toName = distributeToAttr.value();
                    toType = "distribute-to";
                } else {
                    throw cgrame_error("[ADL1] \"to\"/\"distribute-to\" attribute must be specified");
                }

                if(fromAttr && selectFromAttr) {
                    throw cgrame_error("[ADL1] only one of \"from\" and \"select-from\" is allowed");
                } else if(fromAttr) {
                    fromName = fromAttr.value();
                    fromType = "from";
                } else if(selectFromAttr) {
                    fromName = selectFromAttr.value();
                    fromType = "select-from";
                } else {
                    throw cgrame_error("[ADL1] \"from\"/\"select-from\" attribute must be specified");
                }

                //Do counter things here
                if(counterAttr) {
                    int value = (xmlData.arch.getCurrentRow() - xmlData.arch.getPatternStartRow()) * (1 + xmlData.arch.getPatternEndCol() - xmlData.arch.getPatternStartCol()) + (xmlData.arch.getCurrentCol() - xmlData.arch.getPatternStartCol());
                    toName = expand_counter_name(toName, counterName, value);
                    fromName = expand_counter_name(fromName, counterName, value);
                    counters.push_back(std::make_pair(counterName, value));
                    //if(DEBUG_OUT) std::cout << "Expanded name to " << identifier << std::endl;
                }

                if(rowCounterAttr) {
                    int value = xmlData.arch.getCurrentRow() - xmlData.arch.getPatternStartRow();
                    toName = expand_counter_name(toName, rowCounterName, value);
                    fromName = expand_counter_name(fromName, rowCounterName, value);
                    counters.push_back(std::make_pair(rowCounterName, value));
                    //if(DEBUG_OUT) std::cout << "Expanded name to " << identifier << std::endl;
                }

                if(colCounterAttr) {
                    int value = xmlData.arch.getCurrentCol() - xmlData.arch.getPatternStartCol();
                    toName = expand_counter_name(toName, colCounterName, value);
                    fromName = expand_counter_name(fromName, colCounterName, value);
                    counters.push_back(std::make_pair(colCounterName, value));
                    //if(DEBUG_OUT) std::cout << "Expanded name to " << identifier << std::endl;
                }

                //Expand the vector of names here
                std::vector<std::string> oldTos = multiple_rel_expand(toName);

                std::vector<std::string> tos;
                for(auto elem : oldTos) {
                    //Check for relative blocks here
                    if(elem.substr(0, 4) == "(rel") {
                        std::cout << "User is referring to relative block\n";
                        separate_block_and_port(elem, block, port);
                        if(wrap) block = expand_wrap_name(block, xmlData, counters);
                        block = expand_rel_name(block, xmlData);
                        elem = block + "." + port;
                    }
                    elem = math_expand(elem);
                    tos.push_back(elem);
                }


                std::vector<std::string> oldFroms = multiple_rel_expand(fromName);
                std::vector<std::string> froms;
                for(auto elem : oldFroms) {
                    if(elem.substr(0, 4) == "(rel") {
                        if(DEBUG_OUT) std::cout << "User is referring to relative block\n";
                        separate_block_and_port(elem, block, port);
                        if(wrap) block = expand_wrap_name(block, xmlData, counters);
                        block = expand_rel_name(block, xmlData);
                        elem = block + "." + port;
                    }
                    elem = math_expand(elem);
                    froms.push_back(elem);
                }


                if(fromType == "from" && toType == "to") {
                    std::string from = froms.back();
                    std::string to = tos.back();
                    froms.pop_back();
                    tos.pop_back();
                    if(DEBUG_OUT) std::cout << "Connection from " << from << " to " << to << std::endl;
                    xmlData.connections.push_back(std::make_tuple(toType, to, fromType, from));
                }

                if(fromType == "from" && toType == "distribute-to") {
                    std::string from = froms.back();
                    froms.pop_back();
                    while(tos.size() > 0) {
                        std::string to = tos.back();
                        tos.pop_back();
                        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
                        if(DEBUG_OUT) std::cout << "    to: " << to << std::endl << "from" << ": " << from << std::endl;
                    }
                }
                if(fromType == "select-from" && toType == "to") {
                    int numInputs = froms.size();
                    int numMuxes = tos.size();
                    for(int i = 0; i < numMuxes; i++) {
                        std::string muxName = "top_mux" + std::to_string(xmlData.muxCount);
                        std::map<std::string, std::string> m;
                        std::string busWidth = "32";
                        if (busWidthAttr)
                            busWidth = busWidthAttr.value();
                        m.insert(std::make_pair("name", muxName));
                        m.insert(std::make_pair("size", busWidth));
                        m.insert(std::make_pair("ninput", std::to_string(numInputs)));
                        xmlData.primitives.push_back(std::make_pair("Multiplexer", m));
                        for(int j = 0; j < numInputs; j++) {
                            std::string from, to;
                            to = muxName + ".in" + std::to_string(j);
                            from = froms[j];
                            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
                        }
                        std::string to = tos[i];
                        xmlData.connections.push_back(std::make_tuple("to", to, "from", muxName + ".out"));
                        xmlData.muxCount++;
                    }
                }
            }
        }
    }
}

Module* create_module(std::string name, moduleTemplate type, ADLStructs &xmlData)
{
    //Now create a module and fill it with things from temp
    Module* m(new Module(name, type.name));

    //Add the ports to the module
    for(auto &elem : type.ports) {
        m->addPort(elem.first, std::get<0>(elem.second), std::get<1>(elem.second));
    }

    //Add the primitives
    for(auto elem : type.primitives) {
        //Find out what type the primitive is then add it to the module with the correct parameters
        add_primitive_to_module(m, elem.first, elem.second);
    }

    //Add the sub modules
    for(auto elem : type.subModules) {
        //Check if the submodules is declared in templates
        bool found = false;
        moduleTemplate mod;
        for(auto sub : xmlData.moduleTemplates) {
            if(sub.first == elem.second){
                found = true;
                mod = sub.second;
            }
        }
        if(found) {
            m->addSubModule(create_module(elem.first, mod, xmlData));
        }
    }

    //Add the connections
    for(auto &elem : type.connections) {
        std::string toArgs, toType, fromArgs, fromType;
        toArgs = std::get<1>(elem);
        toType = std::get<0>(elem);
        fromArgs = std::get<3>(elem);
        fromType = std::get<2>(elem);
        add_connection_to_module(m, toType, toArgs, fromType, fromArgs);
    }

    return m;
}

bool add_primitive_to_module(Module* &m, std::string type, std::map<std::string, std::string> &args)
{
    if(type == "FuncUnit") {
        std::string funcUnitName = args.at("name");
        unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        auto op = args.count("op") == 0 ? std::vector<OpGraphOpCode>{OPGRAPH_OP_ADD, OPGRAPH_OP_MUL} : tokenize_list_as<OpGraphOpCode>(args.at("op"));
        m->addSubModule(new FuncUnit(funcUnitName, op, size));
        m->addConfig(new ConfigCell(funcUnitName + "_Config"), {funcUnitName + ".select"});
    } else if(type == "IO") {
        std::string IOName = args.at("name");
        unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        m->addSubModule( new IO(IOName, size));
    } else if(type == "Multiplexer") {
        std::string muxName = args.at("name");
        unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        unsigned muxSize = args.count("ninput") == 0 ? 32 : std::stoi(args.at("ninput"));
        m->addSubModule(new Multiplexer(muxName, muxSize, size));
        // XXX: enforcing config cell on mux does not conform with API usage
        m->addConfig(new ConfigCell(muxName + "_Config"), {muxName + ".select"});
    } else if(type == "Register") {
        std::string regName = args.at("name");
        unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        m->addSubModule(new Register(regName, size));
    } else if(type == "Tristate") {
        std::string triName = args.at("name");
        unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        m->addSubModule(new TriState(TriState::Mode::PLAIN, triName, size));
    } else if(type == "RegisterFile") {
        std::string regName = args.at("name");
        auto ninput = args.count("ninput") == 0 ? 1 : std::stoi(args.at("ninput"));
        auto noutput = args.count("noutput") == 0 ? 1 : std::stoi(args.at("noutput"));
        auto log2nregister = args.count("log2-nregister") == 0 ? : std::stoi(args.at("log2-nregister"));
        auto size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        m->addSubModule(new RegisterFile(regName, ninput, noutput, log2nregister, size));
        // XXX: enforcing config cell on RF addres_inX/outX does not conform with API usage
        for (unsigned int i = 0; i < ninput; i++) {
            std::string i_str = std::to_string(i);
            m->addConfig(
                new ConfigCell(regName + "_addr_i" + i_str),
                {regName + ".address_in" + i_str});
            m->addConfig(
                new ConfigCell(regName + "_WE" + i_str),
                {regName + ".WE" + i_str});
        }
        for (unsigned int i = 0; i < noutput; i++) {
            std::string i_str = std::to_string(i);
            m->addConfig(
                new ConfigCell(regName + "_addr_o" + i_str),
                {regName + ".address_out" + i_str});
        }
    } else if(type == "MEMUnit") {
        std::string memName = args.at("name");
        m->addSubModule(new MEMUnit(memName));
    } else if(type == "MemPort") {
        std::string memPortName = args.at("name");
        auto ninput = args.count("ninput") == 0 ? 1 : std::stoi(args.at("ninput"));
        m->addSubModule(new MemPort(memPortName, ninput));
    } else if(type == "SimpleFU") {
        auto simpName = args.at("name");
        auto blockType = args.count("type") == 0 ? STANDARD_NOBYPASS : read<CGRABlockType>(args.at("type"));
        m->addSubModule(new SimpleFU(simpName, blockType));
    } else if(type == "ConstUnit") {
        auto constName = args.at("name");
        unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        m->addSubModule(new ConstUnit(constName, size));
    } else {
        unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
        std::string customName = args.at("name");
        userFunctions temp;
        int i = 0;
        int funcNum = -1;
        for(auto elem : temp.functionNames) {
            if(elem == type) funcNum = i;
            i++;
        }
        if(funcNum != -1) temp.functionVector[funcNum](customName, size, args, m);
    }
    return true;
}

//double check that this is correct
bool add_connection_to_module(Module* &m, std::string toType, std::string toArgs, std::string fromType, std::string fromArgs)
{
    //Check the type of 'to' and 'from' connection and act accordingly
    //There aren't that many permutations of types, so I will handle each one individually
    if(toType == "to" && fromType == "from") {
        if(m->addConnection(fromArgs, toArgs))
            if(DEBUG_OUT) std::cout << "Adding connection from: " << fromArgs << " to: " << toArgs << std::endl;
    }
    return true;
}

//This should be improved
//ToDo: allow this to handle numbers greater than 9
std::string expand_rel_name(std::string rel, ADLStructs &xmlData)
{
    if(DEBUG_OUT) std::cout << "Expanding: " << rel << std::endl;
    std::stringstream ss(rel);
    int relRow, relCol;
    bool rowFilled = false;
    //True is positive, false is negative
    bool sign;

    //The first number I come across is the row
    int temp;
    std::string x;
    while(ss>>x) {
        //Check for a negative sign
        sign = !(x[0] == '-');
        if(!sign) x.erase(x.begin());
        //Make sure size of x is 1
        while(x.size() > 1) x.pop_back();
        if(isdigit(x[0])) {
            if(!rowFilled) {
                rowFilled = true;
                relRow = atoi(x.c_str());
                if(!sign) relRow = -relRow;
            } else {
                relCol = atoi(x.c_str());
                if(!sign) relCol = -relCol;
            }
        }
    }
    int row = xmlData.arch.getCurrentRow() + relRow;
    int col = xmlData.arch.getCurrentCol() + relCol;
    return ("block_" + std::to_string(row) + "_" + std::to_string(col));
}

bool separate_block_and_port(std::string connection, std::string &block, std::string &port)
{
    size_t pos = connection.find(".");
    int x = pos;
    block = connection.substr(0, x);
    port = connection.substr(x+1);
    return true;
}

std::shared_ptr<CGRA> API_caller(ADLStructs &xmlData)
{
    //Create the result that is to be returned
    std::shared_ptr<CGRA> result(new CGRA());

    //First, I should create and add each composite module
    //Do this by iterating through the subModules member of the xmlData and adding the result to the CGRA
    if(DEBUG_OUT) std::cout << "Looking for submodules to add...\n";
    for(auto elem : xmlData.subModules) {
        std::string subModuleName = elem.first;
        moduleTemplate subModuleType = elem.second;
        if(DEBUG_OUT) std::cout << "    Adding " << subModuleName << std::endl;
        Module* m = create_module(subModuleName, subModuleType, xmlData);

        result->addSubModule(m);
    }

    //Add the ports if there are any
    if(DEBUG_OUT) std::cout << "Looking for ports to add...\n";
    for(auto &elem : xmlData.ports) {
        std::string portName = std::get<0>(elem);
        port_type portType = std::get<1>(elem);
        unsigned portSize = std::get<2>(elem);
        result->addPort(portName, portType, portSize);
    }

    //Looking for muxes to add if there are any
    if(DEBUG_OUT) std::cout << "Looking for muxes to add...\n";
    for(auto elem : xmlData.primitives) {
        if(elem.first == "Multiplexer") {
            std::map<std::string, std::string> args = elem.second;
            auto muxSize = args.count("ninput") == 0 ? 2 : std::stoi(args.at("ninput"));
            std::string muxName = args.at("name");
            unsigned size = args.count("size") == 0 ? 32 : std::stoi(args.at("size"));
            result->addSubModule(new Multiplexer(muxName, muxSize, size));
            // XXX: enforcing config cell on mux does not conform with API usage
            result->addConfig(new ConfigCell(muxName + "_Config"), {muxName + ".select"});
        }
    }

    //Now add the connections if there are any
    if(DEBUG_OUT) std::cout << "Looking for connections to add... " << xmlData.connections.size() << "\n";
    for(auto elem : xmlData.connections) {
        std::string toArgs, toType, fromArgs, fromType;
        toType = std::get<0>(elem);
        toArgs = std::get<1>(elem);
        fromType = std::get<2>(elem);
        fromArgs = std::get<3>(elem);
        add_connection_to_CGRA(result, toType, toArgs, fromType, fromArgs);
    }

    return result;
}

bool add_connection_to_CGRA(std::shared_ptr<CGRA> &m, std::string toType, std::string toArgs, std::string fromType, std::string fromArgs)
{
    //Check the type of 'to' and 'from' connection and act accordingly
    //There aren't that many permutations of types, so I will handle each one individually
    if(toType == "to" && fromType == "from") {
        if(m->addConnection(fromArgs, toArgs))
            if(DEBUG_OUT) std::cout << "Adding connection from: " << fromArgs << " to: " << toArgs << std::endl;
    }

    return true;
}

std::string expand_counter_name(std::string name, std::string counter, int value)
{
    std::string counterString = "(" + counter + ")";
    std::size_t pos = name.find(counterString);
    while(pos!=std::string::npos) {
        name.replace(pos, counterString.length(), std::to_string(value));
        pos = name.find(counterString);
    }

    return name;
}

std::string expand_wrap_name(std::string rel, ADLStructs &xmlData, std::vector<std::pair<std::string, int>> counters)
{
    //this function is only called if the user is using the 'rel' functionality in the XML
    if(DEBUG_OUT) {
        std::cout << "The user is referring to a relative block... Check for wrap around...\n";
        std::cout << "Looking for the first number in rel...\n";
    }

    bool counter1Found = false;
    bool number1Found = false;
    bool foundAllNumbers = false;
    size_t range;
    std::string firstNum, secondNum;

    int currentCol = xmlData.arch.getCurrentCol() - xmlData.arch.getPatternStartCol();
    int currentRow = xmlData.arch.getCurrentRow() - xmlData.arch.getPatternStartRow();
    int maxRow = xmlData.arch.getPatternEndRow() - xmlData.arch.getPatternStartRow();
    int maxCol = xmlData.arch.getPatternEndCol() - xmlData.arch.getPatternStartCol();

    for(int i = 5; !counter1Found && !number1Found; i++) {
        if(rel[i] == ' ') number1Found = true;
        if(rel[i] == '(') counter1Found = true;

        //expand the counter here and replace it with the number
        if(counter1Found) {
            if(DEBUG_OUT) std::cout << "The first number is a counter... expanding counter...\n";
            //Find out which counter it is
            std::string counterName = rel.substr(i+1, 1);
            if(DEBUG_OUT) std::cout << counterName << std::endl;
            int newNumber;
            for(auto elem : counters) {
                if(counterName == elem.first) newNumber = elem.second;
            }
            std::string toReplace = "(" + counterName + ")";
            rel.replace((rel.find(toReplace)), toReplace.length(), std::to_string(newNumber));
            if(DEBUG_OUT) std::cout << "The string has been changed to: " << rel << std::endl;
            number1Found = true;
            while(rel[i] != ' ') i++;
            //abort();
        }

        if(number1Found) {
            if(DEBUG_OUT) std::cout << "The first number was found\n";
            int tempFirstNumber = std::stoi(rel.substr(5, i-5));
            if(DEBUG_OUT) std::cout << "Number: " << tempFirstNumber << "\nChanging number...\n";
            if(tempFirstNumber + currentRow >= maxRow) tempFirstNumber = tempFirstNumber%(maxRow+1) - (maxRow+1);
            if(tempFirstNumber + currentRow < 0) tempFirstNumber = (tempFirstNumber%(maxRow+1) + maxRow+1)%(maxRow+1);
            bool number2Found = false;
            bool counter2Found = false;
            firstNum = std::to_string(tempFirstNumber);
            for(int j = i+1; !counter2Found && !number2Found; j++) {
                if(rel[j] == ')') number2Found = true;
                if(rel[j] == '(') counter2Found = true;

                if(counter2Found) {
                    if(DEBUG_OUT) std::cout << "The second number is a counter... expanding counter...\n";
                    std::string counterName = rel.substr(j+1, 1);
                    int newNumber;
                    for(auto elem : counters) {
                        if(counterName == elem.first) newNumber = elem.second;
                    }
                    std::string toReplace = "(" + counterName + ")";
                    rel.replace((rel.find(toReplace)), toReplace.length(), std::to_string(newNumber));
                    number2Found = true;
                    while(rel[j] != ')') j++;
                }
                if(number2Found) {
                    if(DEBUG_OUT) std::cout << "The second number was found\n";
                    int tempSecondNumber = std::stoi(rel.substr(i+1, j-(i+1)));
                    if(DEBUG_OUT) std::cout << "Number: " << tempSecondNumber << "\nChanging number...\n";
                    if(tempSecondNumber + currentCol >= maxCol) tempSecondNumber = tempSecondNumber%(maxCol+1) - (maxCol + 1);
                    if(tempSecondNumber + currentCol < 0) tempSecondNumber = (tempSecondNumber%(maxCol+1) + maxCol + 1)%(maxCol+1);

                    secondNum = std::to_string(tempSecondNumber);
                    foundAllNumbers = true;
                    range = j - 5;
                }
            }
        }
    }

    if(foundAllNumbers) {
        std::string reconstructed = rel;
        std::string replace = firstNum + " " + secondNum;
        reconstructed.replace(5, range, replace);
        if(DEBUG_OUT) std::cout << "The string is " << reconstructed << "\n";
        return reconstructed;
    }

    return rel;
}

std::string create_xml_block(int row, int col)
{
    return "block_" + std::to_string(row) + "_" + std::to_string(col);
}

bool expand_all_definitions(ADLStructs &xmlData)
{
    //create parser function
}

std::vector<std::string> multiple_rel_expand(std::string rels)
{
    int size = rels.size();
    std::vector<std::string> result;
    std::string rel = "";
    bool finish = false;
    bool start = true;
    int openCount = 0;
    int closeCount = 0;
    for(int i = 0; i < size; i++) {
        if(rels[i] == '(') {
            start = true;
            if(closeCount == openCount && openCount != 0) {
                rel.pop_back();
                while(rel.back() == ' ') rel.pop_back();
                //std::cout << rel << std::endl;
                result.push_back(rel);
                rel = "";
            }
            openCount++;
        }

        if(start) rel += rels[i];

        if(rels[i] == ')') {
            closeCount++;
        }

        if(i == size-1) result.push_back(rel);
    }

    return result;
}

// FIXME: This method is inflexible and constrained usecase to be `num0 * num1 + num2`
std::string math_expand(std::string before)
{
    std::size_t pos = before.find("*");
    std::string num1, num2;
    int numCheck = pos;

    // FIXME: only check 1 digit for now
    num1 += before[numCheck - 1];
    num2 += before[numCheck + 1];

    //check if both are numbers
    if(is_number(num1) && is_number(num2)) {
        int arg1 = std::stoi(num1);
        int arg2 = std::stoi(num2);
        int result = arg1 * arg2;
        std::string toReplace = num1 + "*" + num2;
        std::string resultString = std::to_string(result);
        before.replace(before.find(toReplace), toReplace.length(), resultString);
    }

    pos = before.find("+");
    numCheck = pos;
    num1 = "";
    num1 += before[numCheck - 1];
    num2 = "";
    num2 += before[numCheck + 1];

    if(is_number(num1) && is_number(num2)) {
        int arg1 = std::stoi(num1);
        int arg2 = std::stoi(num2);
        int result = arg1 + arg2;
        std::string toReplace = num1 + "+" + num2;
        std::string resultString = std::to_string(result);
        before.replace(before.find(toReplace), toReplace.length(), resultString);
    }

std::cout << before << std::endl;
//abort();
    return before;
}

bool is_number(std::string &a)
{
    std::string::const_iterator it = a.begin();
    while(it != a.end() && std::isdigit(*it)) it++;
    return !a.empty() && it == a.end();
};

}

