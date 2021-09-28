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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <iostream>

#include <CGRA/Module.h>
#include <CGRA/ModuleComposites.h>
#include <CGRA/CGRA.h>

#include "pugixml.hpp"

namespace adl1 {

//In the tempalte file, the defines some module templates that will be used later in the architecture
//This struct holds the information for each module. A vector of these structs is declared later in
//the architecture
struct moduleTemplate {
    //Name of the template
    //This will be the base name for the naming convention which I will decide later
    std::string name;

    //Vector of connections
    //Format is to type, to args, from type, from args
    //to types: to, distribute-to
    //from types: from, select from
    //the args represent the target ports
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> connections;

    //Vector of ports for each module
    //Format is name, type, size
    std::map<std::string, std::tuple<port_type, unsigned> > ports;

    //VectorA of primitives to add
    //Format is the type, then the args in the form of a map
    //the key is the attribute, the value is the attribute's value
    std::vector<std::pair<std::string, std::map<std::string, std::string>>> primitives;

    //A map of wires and which ports they correspond to
    //In this context, each wire can be replaced with a port
    std::map<std::string, std::string> wires;

    //A vector containing the submodules and their names
    //The user defines which submodule they want to use and assigns it a name
    //The first element is the name, the second is the string representing the template
    std::vector<std::pair<std::string, std::string>> subModules;

    void addPort(std::string portname, port_type pt, unsigned size)
    {
        if(ports.find(portname) != ports.end()) // if the same port name was repeated
        {
            throw cgrame_adl_error("Multiple ports of the same name: '" + portname + "'");
        }

        ports[portname] =  std::make_tuple(pt, size);
    };

    //This function prints the contents of the module template
    //ToDo: format better and complete it
    void print()
    {
        std::cout << "\n\nPrinting contents of Module Template...\n";
        std::cout << "Name: " << name << std::endl;
        std::cout << "Printing ports:\n";
        for(const auto& elem : ports) {
            //Print the type later
            std::cout << "\nPort name: " << elem.first << "\nPort size: " << std::get<1>(elem.second) << std::endl;
        }
        std::cout << "\n\nPrinting primitives:\n";
        for(const auto elem : primitives) {
            std::cout << "\nPrimitive type: " << elem.first << std::endl;
            std::cout << "Primitive arguments...\n";
            std::map<std::string, std::string> m = elem.second;
            for(auto map : m) {
                std::cout << map.first << "  " << map.second << std::endl;
            }
        }
        std::cout << "\n\nPrinting connections:\n";
        for(const auto &elem : connections) {
            std::cout << std::get<0>(elem) << ": " << std::get<1>(elem) << std::endl << std::get<2>(elem) << ": " << std::get<3>(elem) << "\n\n";
        }
    };
};

//This class holds information on the architectre
//It holds number of rows, cols, the current row, current col, and the dimensions of the current operating pattern
class archData {
    private:
        //The rows and cols in the actual architecture
        int archRows;
        int archCols;

        //The rows and cols of the pattern I am currently parsing
        //This will change often as I examine new pattern tags
        int patternStartRow;
        int patternEndRow;

        int patternStartCol;
        int patternEndCol;

        //The current operating row and col of the CGRA (to be updated every time you operate on a new row    //and col
        int currentRow;
        int currentCol;

    public:
        //Constructor
        archData() {
            archRows = 0;
            archCols = 0;
            patternStartRow = 0;
            patternStartCol = 0;
            patternEndRow = 0;
            patternEndCol = 0;
            currentRow = 0;
            currentCol = 0;
        }

        //Get functions
        int getArchRows() {
            return archRows;
        }

        int getArchCols() {
            return archCols;
        }

        int getPatternStartRow() {
            return patternStartRow;
        }

        int getPatternEndRow() {
            return patternEndRow;
        }

        int getPatternStartCol() {
            return patternStartCol;
        }

        int getPatternEndCol() {
            return patternEndCol;
        }

        int getCurrentRow() {
            return currentRow;
        }

        int getCurrentCol() {
            return currentCol;
        }

        //Set functions
        int setArchRows(int _archRows) {
            archRows = _archRows;
        }

        int setArchCols(int _archCols) {
            archCols = _archCols;
        }

        int setPatternStartRow(int _patternStartRow) {
            patternStartRow = _patternStartRow;
        }

        int setPatternEndRow(int _patternEndRow) {
            patternEndRow = _patternEndRow;
        }

        int setPatternStartCol(int _patternStartCol) {
            patternStartCol = _patternStartCol;
        }

        int setPatternEndCol(int _patternEndCol) {
            patternEndCol = _patternEndCol;
        }

        int setCurrentRow(int _currentRow) {
            currentRow = _currentRow;
        }

        int setCurrentCol(int _currentCol) {
            currentCol = _currentCol;
        }

        //Print function
        void print() {
            std::cout << "Rows in architecture: " << archRows << std::endl;
            std::cout << "Cols in architecture: " << archCols << std::endl;
        }
};

//This struct holds the entire architecture in its members
//It hold the templates described in the template file, all of the submodules that will be added to the final CGRA,
//the definitions, the connections between submodules, the ports (might be no need for this), and the architecture data.
class ADLStructs {
    public:
        //Module template
        //The format is the name of the template, then its contents
        //This vector is filled using the template file
        std::vector<std::pair<std::string, moduleTemplate>> moduleTemplates;

        //The composite modules that are to be added to the top level module
        //The format is the name of the sub module, then its contents
        std::vector<std::pair<std::string, moduleTemplate>> subModules;

        //This will hold any muxes that are created beceause of connections
        std::vector<std::pair<std::string, std::map<std::string, std::string>>> primitives;
        int muxCount = 0;

        //name of definition and its value
        std::vector<std::pair<std::string, int>> definitions;

        //to args, to type, from args, from type
        //same as in module templates
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> connections;

        //port name type, and size
        std::vector<std::tuple<std::string, port_type, unsigned> > ports;

        //Class that holds information on the architecture that the ADLStructs referst to
        archData arch;

        bool add_module_template(pugi::xml_node module);
        bool add_subModule(pugi::xml_node module);
        bool add_definition(pugi::xml_node defintion);
        bool add_connection(pugi::xml_node connection);
        bool add_port(pugi::xml_node port);
        void print();
};

}

