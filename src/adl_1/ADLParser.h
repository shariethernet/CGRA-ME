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
#include <iostream>
#include <sstream>

#include <CGRA/Module.h>
#include <CGRA/CGRA.h>

#include <pugixml.hpp>

#include "ADLStructs.h"

namespace adl1 {

//This is the main parser function. Pass it the template file name, the architecture file name, and it will return
//a CGRA pointer filled with data as described in the architecture
std::shared_ptr<CGRA> parseADL(std::string templateFileName, std::string archFileName);

//This function fills the xmlData with the information found in the template document
bool fill_module_templates(pugi::xml_node templateDoc, ADLStructs &xmlData);

//This function fills the xmlData with the information found in the architecture document
bool fill_arch_data(pugi::xml_node archDoc, ADLStructs &xmlData);

//This function fills the xmlData with the information declared in each pattern within the architecture document
bool declare_pattern(pugi::xml_node pattern, ADLStructs &xmlData);

//This function takes in a module template and creates a pointer to a module filled with its contents
Module* create_module(std::string name, moduleTemplate type, ADLStructs &xmlData);

//This function adds primitives to a module
bool add_primitive_to_module(Module* &m, std::string type, std::map<std::string, std::string> &args);

//This function adds connections to a module
bool add_connection_to_module(Module* &m, std::string toType, std::string toArgs, std::string fromType, std::string fromArgs);

//This function will take in the name of a block in the form of (rel m n) and
//expand it to the form block_x_y where x and y are the correct values referred to by the rel name
std::string expand_rel_name(std::string rel, ADLStructs &xmlData);

//This function takes in a string 'block' in the form block.port and removes the port
//The block string changes to just 'block' and the port string is filled with 'port'
bool separate_block_and_port(std::string connection, std::string &block, std::string &port);

//This function fills the CGRA pointer using the xmlData
std::shared_ptr<CGRA> API_caller(ADLStructs &xmlData);

//This function adds connections between modules within the CGRA
bool add_connection_to_CGRA(std::shared_ptr<CGRA> &m, std::string toType, std::string toArgs, std::string fromType, std::string fromArgs);

//This funciton expands the name of any string containing the counter to reflect the counters actual value
std::string expand_counter_name(std::string name, std::string counter, int value);

//This function expands the name of a relative block to reflect the proper value when wrap around is turned on
std::string expand_wrap_name(std::string rel, ADLStructs &xmlData, std::vector<std::pair<std::string, int>> counters);

//This takes in integers n and m and returns the formatted block name block_n_m
//This made my life so much eaiser
std::string create_xml_block(int row, int col);

bool expand_all_definitions(ADLStructs &xmlData);

std::vector<std::string> multiple_rel_expand(std::string rels);

std::string math_expand(std::string before);

bool is_number(std::string& a);

//This function takes in a list delimited by whitespace and returns it in the form
//of a vector of type T
    template<typename T>
static std::vector<T> tokenize_list_as(const std::string &str)
{
    std::stringstream ss(str);
    std::vector<T> args;

    T x;
    while(ss >> x) {
        args.push_back(x);
    }
    return args;
};

    template<typename T>
static T read(const std::string &str)
{
    std::istringstream is(str);
    T x;
    if(is >> x) return x;
    else {
        std::cout << "[ERROR] Could not convert type\n";
    }
};

}

