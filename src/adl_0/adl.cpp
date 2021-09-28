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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(XML_VALIDATION)
#include <libxml++/libxml++.h>
#endif

#include <tuple>
#include <vector>
#include <iostream>

#include <pugixml.hpp>

#include "adl.h"
#include "template_xml.h"

namespace adl0 {

static const int default_col = 1;
static const int default_row = 1;

template<typename T>
static bool empty(const T a)
{
    return std::begin(a) == std::end(a);
}

static int uniqueId(void)
{
    static int x = 0;
    return x++;
}

static std::string block_name(ADL::grid_position pos) {
    std::stringstream ss;
    ss << "block_" << pos.row << "_" << pos.col << "_";
    return ss.str();
}

static bool is_valid_identifier(const std::string &ident, const std::string &orig) {
    if (ident.size() == 0) {
        std::cerr << "error: empty identifier after expansion on pattern '" << orig << "'" << std::endl;
        return false;
    } else if (ident.size() > 1) {
        if (!std::isalpha(ident[0])) {
            std::cerr << "error: identifier '" << ident << "' suffixed other than alphabet.";
            if (!orig.empty())
                std::cerr << "(after expansion on pattern '" << orig << "')";
            std::cerr << std::endl;
            return false;
        } else if (ident.find_first_of("\t\n ") != std::string::npos) {
            std::cerr << "error: identifier '" << ident << "' contains white spaces.";
            if (!orig.empty())
                std::cerr << "(after expansion on pattern '" << orig << "')";
            std::cerr << std::endl;
            return false;
        }
    }
    // ToDo: other cases
    return true;
}

std::map<std::string, std::string> attribute_map(const pugi::xml_node node)
{
    std::map<std::string, std::string>m;
    for (auto && attr : node.attributes()) {
        m.insert(m.begin(), std::make_pair(attr.name(), attr.value()));
    }
    return m;
}

std::map<std::string, std::string> attribute_map(const pugi::xml_node node, ADL::environment env)
{
    std::map<std::string, std::string>m;
    for (auto && attr : node.attributes()) {
        m.insert(m.begin(), std::make_pair(attr.name(), ADL::expand_name(attr.value(), env)));
    }
    return m;
}

void  ADL::debug_print_node(const std::string name, const pugi::xml_node node)
{
    debug_out << name << ":";
    for (pugi::xml_attribute attr: node.attributes())
    {
        debug_out << " " << attr.name() << "=" << attr.value();
    }

    for (pugi::xml_node child: node.children())
    {
        debug_out << ", child " << child.name();
    }
    debug_out << ";" << std::endl;
}

void debug_print_node(const std::string name, const pugi::xml_node node, const ADL::environment & env)
{
    auto os = env.debug_out();
    if (os == nullptr)
        return;

    auto & debug_out = *os;

    debug_out << name << ":";
    for (pugi::xml_attribute attr: node.attributes())
    {
        debug_out << " " << attr.name() << "=" << attr.value();
    }

    for (pugi::xml_node child: node.children())
    {
        debug_out << ", child " << child.name();
    }
    debug_out << ";" << std::endl;
}

void  debug_print_all_nodes(const pugi::xml_node node, const ADL::environment & env, int indents)
{
    auto os = env.debug_out();
    if (os == nullptr)
        return;

    for (int i = 0; i < indents; ++i)
        *os << "    ";

    debug_print_node(node.name(), node, env);
    for (auto && child : node.children()) {
        debug_print_all_nodes(child, env, indents + 1);
    }
}

bool  ADL::conditional(ADL::environment env, const pugi::xml_node node)
{
    auto ifAttr = node.attribute("if");

    if (!ifAttr)
        return true;

    debug_out << "condition {" << std::endl;
    std::string condition(ifAttr.value());
    bool b = expand_as<bool>(condition, env);
    debug_out << "}" << std::endl;

    return b;
}

void ADL::macro_definition(environment &env, const pugi::xml_node definitionNode)
{
    debug_print_node("definitionNode", definitionNode);

    debug_out << "definition {" << std::endl;
    auto varAttr = definitionNode.attribute("variable");
    auto funcAttr = definitionNode.attribute("function");

    if (!varAttr && !funcAttr) {
        std::cerr << "Definition requires a name" << std::endl;
        throw std::runtime_error("no name definition");
    }

    if (varAttr && funcAttr) {
        std::cerr << "You cannot define variable and function at the same time." << std::endl;
        throw std::runtime_error("invalid attribute");
    }

    auto defAttr = definitionNode.attribute("be");
    std::string def(defAttr.value());

    if (varAttr) {
        std::string var(varAttr.value());
        env.function(var, const_function(var, def));
    } else if (funcAttr) {
        std::string func(funcAttr.value());
        env.function(func, user_function(func, def));
    }
    debug_out << "}" << std::endl;
}

void  ADL::connection_declaration(ADL::environment env, const pugi::xml_node connectionNode)
{
    debug_print_node("connection", connectionNode);

    if (!conditional(env, connectionNode))
        return;

    auto nameAttr = connectionNode.attribute("name");
    auto fromAttr = connectionNode.attribute("from");
    auto selectFromAttr = connectionNode.attribute("select-from");
    auto toAttr = connectionNode.attribute("to");
    auto distributeToAttr = connectionNode.attribute("distribute-to");
    std::string nameTemplate(nameAttr.value());
    auto name = expand_name(nameTemplate, env);

    if ((!fromAttr && !selectFromAttr) || (fromAttr && selectFromAttr)) {
        std::cerr << "error: Connection must have an attribute named either 'from' or 'select-from'." << std::endl;
        debug_out << "error: Connection must have an attribute named either 'from' or 'select-from'." << std::endl;
        return;
    }

    if ((!toAttr && !distributeToAttr) || (toAttr && distributeToAttr)) {
        std::cerr << "error: Connection must have an attribute named either 'to' or 'distribute-to'." << std::endl;
        debug_out << "error: Connection must have an attribute named either 'to' or 'distribute-to'." << std::endl;
        return;
    }

    debug_out << "connection {" << std::endl;
    if (!nameAttr) {
        debug_out << "Connection has no name attribute" << std::endl;
        name = "con" + std::to_string(uniqueId());
    }

    if (is_valid_identifier(name, nameTemplate)) {
        debug_out << "Given name is invalid. Create new one." << std::endl;
        name = "con" + std::to_string(uniqueId());
    }

    debug_out << "name: " << name << std::endl;

    std::vector<std::string> inputPorts, multiplexers, outputPorts;

    auto fromPrefix = expand_name(connectionNode.attribute("from-prefix").value(), env);
    auto toPrefix = expand_name(connectionNode.attribute("to-prefix").value(), env);

    // Expand names for input ports.
    if (selectFromAttr) {
        std::string selectFromPortsTemplate(selectFromAttr.value());
        auto selectFromPorts = tokenize_list(expand_name(selectFromPortsTemplate, env));
        for (auto && port : selectFromPorts) {
            inputPorts.push_back(fromPrefix + port);
        }
    } else {
        std::string fromPortsTemplate(fromAttr.value());
        auto fromPorts = tokenize_list(expand_name(fromPortsTemplate, env));
        for (auto && port : fromPorts) {
            inputPorts.push_back(fromPrefix + port);
        }
    }

    if (inputPorts.size() == 0) {
        debug_out << "No input ports." << std::endl;
        return;
    }

    // Expand names for from ports.
    if (distributeToAttr) {
        std::string distributeToPortTemplate(distributeToAttr.value());
        auto distributeToPorts = tokenize_list(expand_name(distributeToPortTemplate, env));
        for (auto && port : distributeToPorts) {
            outputPorts.push_back(toPrefix + port);
        }
        multiplexers.push_back(name + "_mux");
    } else {
        std::string toPortsTemplate(toAttr.value());
        auto toPorts = tokenize_list(expand_name(toPortsTemplate, env));
        for (auto && port : toPorts) {
            outputPorts.push_back(toPrefix + port);
        }

        for (int i = 0; i < outputPorts.size(); ++i) {
            multiplexers.push_back(name + "_mux" + std::to_string(i));
        }
    }

    if (distributeToAttr && fromAttr && inputPorts.size() > 1) {
        std::cerr << "error: You cannot distribute from more than one input port. You perhaps want to use 'select-from' instead." << std::endl;
        debug_out << "error: You cannot distribute from more than one input port. You perhaps want to use 'select-from' instead." << std::endl;
        return;
    }

    if (toAttr && fromAttr && inputPorts.size() != outputPorts.size()) {
        std::cerr << "error: Number of the input ports and that of the output ports must be equal." << std::endl;
        debug_out << "error: Number of the input ports and that of the output ports must be equal." << std::endl;
        return;
    }

    // No need to instantiate MUXes for a single input.
    if (inputPorts.size() <= 1) {
        multiplexers.clear();
    }

    // Instantiate MUXes.
    for (auto && muxName : multiplexers) {
        env.module().addPrimitive(muxName, "mux", {std::make_pair("inputs", std::to_string(inputPorts.size()))});
        env.module().addConfig((muxName + "Config"), {muxName + ".select"});
        int i = 0;
        for (auto && fromPort : inputPorts) {
            env.module().addConnection(fromPort, muxName + ".in" + std::to_string(i++));
        }
    }

    for (int i = 0; i < outputPorts.size(); ++i) {
        auto outputPort = outputPorts.at(i);
        std::string fromPort{};
        if (fromAttr && toAttr) {
            fromPort = inputPorts.at(i);
        } else if (multiplexers.size() == 0) {
            fromPort = inputPorts.front();
        } else if (multiplexers.size() == 1) {
            fromPort = multiplexers.front() + ".out";
        } else {
            fromPort = multiplexers.at(i) + ".out";
        }
        env.module().addConnection(fromPort, outputPort);
    }

    debug_out << "}" << std::endl;
}

void  ADL::port_declaration(ADL::environment env, const pugi::xml_node portNode)
{
    debug_print_node("port", portNode);

    if (!conditional(env, portNode))
        return;

    debug_out << "port {" << std::endl;

    auto nameAttr = portNode.attribute("name");
    std::string nameTemplate(nameAttr.value());
    auto name = expand_name(nameTemplate, env);

    if (!nameAttr) {
        debug_out << "Port has no name attribute" << std::endl;
    }

    if (!is_valid_identifier(name, nameTemplate)) {
        debug_out << "Invalid port name" << std::endl;
    }

    std::string portType = portNode.name();
    port_type pt;
    if (!portType.compare("input")) {
        pt = PORT_INPUT;
    } else if (!portType.compare("output")) {
        pt = PORT_OUTPUT;
    } else if (!portType.compare("inout")) {
        pt = PORT_BIDIR;
    } else {
        debug_out << "Invalid port type:" << portType << std::endl;
        return;
    }

    auto sizeAttr = portNode.attribute("size");
    auto size = sizeAttr ? expand_as<unsigned int>(sizeAttr.value(), env) : DEFAULT_SIZE;

    env.module().addPort(name, pt, size);
    debug_out << "}" << std::endl;
}

void  ADL::wire_declaration(ADL::environment env, const pugi::xml_node wireNode)
{
    debug_print_node("wire", wireNode);

    if (!conditional(env, wireNode))
        return;

    debug_out << "wire {" << std::endl;

    auto nameAttr = wireNode.attribute("name");
    std::string nameTemplate(nameAttr.value());
    auto name = expand_name(nameTemplate, env);

    if (!nameAttr) {
        debug_out << "wire has no name attribute" << std::endl;
    }

    if (!is_valid_identifier(name, nameTemplate)) {
        debug_out << "Invalid wire name" << std::endl;
    }

    auto sizeAttr = wireNode.attribute("size");
    auto size = sizeAttr ? expand_as<unsigned int>(sizeAttr.value(), env) : DEFAULT_SIZE;

    env.module().addWire(name);
    debug_out << "}" << std::endl;
}

    template<typename T>
static std::vector<T> read_array(std::string str)
{
    std::vector<T> v;
    std::stringstream ss(str);

    T i;

    while (ss >> i)
    {
        v.push_back(i);
    }

    return v;
}

static std::pair<int,int> get_range(std::string axis, std::pair<int,int> defaults, const pugi::xml_node &patternNode, ADL::environment env)
{
    auto fromName = axis + "-from";
    auto toName = axis + "-to";
    auto rangeName = axis + "-range";
    auto fromAttr = patternNode.attribute(fromName.c_str());
    auto toAttr = patternNode.attribute(toName.c_str());
    auto rangeAttr = patternNode.attribute(rangeName.c_str());

    if ((fromAttr || toAttr) && rangeAttr) {
        std::cerr << "You cannot use " << axis << "-from and " << axis << "-to with " << axis << "-range at the same time" << std::endl;
        throw std::runtime_error("invalid attribute");
    }

    auto range = read_array<int>(ADL::expand_name(rangeAttr.value(), env));

    if (!(fromAttr || toAttr) && rangeAttr && range.size() > 2) {
        std::cerr << "Ill-formatted " << axis << "-range: '" << rangeAttr.value() << "'" << std::endl;
        throw std::runtime_error("invalid attribute");
    }

    int from = fromAttr ? ADL::expand_as<int>(fromAttr.value(), env)
        : range.size() >= 1 ? range[0]
        : defaults.first;
    int to = toAttr ? ADL::expand_as<int>(toAttr.value(), env)
        : range.size() >= 2 ? range[1]
        : range.size() >= 1 ? range[0]
        : defaults.second;

    return std::make_pair(from, to);
}

static ADL::grid_area target_area(ADL::environment env, const pugi::xml_node &patternNode, std::ostream & debug_out)
{
    auto rowRange = get_range("row", std::make_pair(0, env.areaSize().row - 1), patternNode, env);
    auto colRange = get_range("col", std::make_pair(0, env.areaSize().col - 1), patternNode, env);

    debug_out << "row-range: " << rowRange.first << "-" << rowRange.second << std::endl;
    debug_out << "col-range: " << colRange.first << "-" << colRange.second << std::endl;

    if (auto onlyEdgeAttr = patternNode.attribute("only-edge")) {
        auto edgeNames = read_array<std::string>(ADL::expand_name(onlyEdgeAttr.value(), env));
        std::vector<ADL::division> edges;
        for (auto && edge : edgeNames) {
            debug_out << "edge name: " << edge << std::endl;
            edges.push_back(ADL::read_division(edge));
        }

        ADL::grid_area area(std::make_pair(rowRange.first, colRange.first), std::make_pair(rowRange.second, colRange.second), std::move(edges));
        return area;
    } else {
        ADL::grid_area area(std::make_pair(rowRange.first, colRange.first), std::make_pair(rowRange.second, colRange.second));
        return area;
    }
}

void  ADL::pattern_declaration(ADL::environment env, const pugi::xml_node patternNode)
{
    debug_print_node("pattern", patternNode);

    if (!conditional(env, patternNode))
        return;

    debug_out << "pattern {" << std::endl;

    std::vector<pugi::xml_node> blocks;
    for (auto && block: patternNode.children("block")) {
        blocks.push_back(block);
    }

    // If a pattern element has no block element,
    // it is assumed that there is a implicit block element which contains
    // the children of the pattern element.
    if (blocks.empty()) {
        blocks.push_back(patternNode);
    }

    pugi::xml_attribute rowAttr = patternNode.attribute("row");
    pugi::xml_attribute colAttr = patternNode.attribute("col");
    debug_out << "rowAttr: " << rowAttr.value() << ", colAttr: " << colAttr.value() << std::endl;
    auto rowNum = expand_as<int>(rowAttr.value(), env);
    auto colNum = expand_as<int>(colAttr.value(), env);
    debug_out << "rowNum: " << rowNum << ", colNum: " << colNum << std::endl;
    int rows = rowAttr ? rowNum
        : colAttr ? blocks.size() / colNum
        : default_row;
    int cols = colAttr ? colNum
        : rowAttr ? blocks.size() / rowNum
        : default_col;
    pattern_size patsize(rows, cols);

    if (rows * cols != blocks.size()) {
        std::cerr << "Pattern is not rectangle: rows=" << rows << ", cols=" << cols << ", blocks=" << blocks.size() << std::endl;
        return;
    }

    auto counterAttr = patternNode.attribute("counter");
    auto rowSkipAttr = patternNode.attribute("row-skip");
    std::string rowSkipVal(expand_name(rowSkipAttr.value(), env));
    int rowSkipNum = rowSkipAttr ? atoi(rowSkipVal.c_str()) : 0;
    std::string counterName(expand_name(counterAttr.value(), env));
    auto colSkipAttr = patternNode.attribute("col-skip");
    std::string colSkipVal(expand_name(colSkipAttr.value(), env));
    int colSkipNum = colSkipAttr ? atoi(colSkipVal.c_str()) : 0;
    auto wrapAroundAttr = patternNode.attribute("wrap-around");
    std::string wrapAroundVal(expand_name(wrapAroundAttr.value(), env));
    wrapAroundVal = wrapAroundAttr ? wrapAroundVal : "off";
    bool wrapAround;
    if(wrapAroundVal == "on") wrapAround = true;
    if(wrapAroundVal == "off") wrapAround = false;

    std::cout << "row-skip: " << rowSkipNum << "\ncol-skip: " << colSkipNum << "\n";
    auto area = target_area(env, patternNode, debug_out);

    // evaulate macro definitions _after_ all attributes are expanded.
    for (auto && def : patternNode.children("definition")) {
        macro_definition(env, def);
    }

    //row and col counter
    auto rowCounterAttr = patternNode.attribute("row-counter");
    std::string rowCounterName(expand_name(rowCounterAttr.value(), env));
    auto colCounterAttr = patternNode.attribute("col-counter");
    std::string colCounterName(expand_name(colCounterAttr.value(), env));
    int i = 0;
    auto prevRow = (*(area.begin())).row - area.left();

    //This is a small loop to find the number of rows and cols in the area since it is not done properly above
    //I want to do this separately from the above code because the above code works with the code below
    //and messing with the above code will likely cause problems
    int numberOfRows = 0;
    int numberOfCols = 0;
    for(auto it = area.begin(); it != area.end(); ++it) {
        const auto pos = *it;
        const auto row = pos.row - area.left();
        const auto col = pos.col - area.top();
        if(row > numberOfRows)
            numberOfRows = row;
        if(col > numberOfCols)
            numberOfCols = col;
    }
    //Increment them by 1 since counting starts at 0 above
    numberOfRows++;
    numberOfCols++;
    std::cout << "Number of rows: " << numberOfRows << "\nNumber of cols: " << numberOfCols << "\n";

    bool keepGoing = true;
    for (auto it = area.begin(); (it != area.end() && keepGoing); ++it) {
        env.position(it);
        std::vector<std::pair<std::string, int>> counters;

        if (counterAttr) {
            debug_out << "counter " << counterName << ": " << i << std::endl;
            env.function(counterName, const_function(counterName, std::to_string(i)));
            counters.push_back(std::make_pair(counterName, i));
        }
        const auto pos = *it;
        const auto row = pos.row - area.left();
        const auto col = pos.col - area.top();
        const auto patno = (row % rows) * cols + (col % cols);
        if(rowCounterAttr) {
            debug_out << "row-counter " << rowCounterName << ": " << row << std::endl;
            env.function(rowCounterName, const_function(counterName, std::to_string(row)));
            counters.push_back(std::make_pair(rowCounterName, row));
        }
        if(colCounterAttr) {
            debug_out << "col-counter " << colCounterName << ": " << col << std::endl;
            env.function(colCounterName, const_function(counterName, std::to_string(col)));
            counters.push_back(std::make_pair(colCounterName, row));
        }
        debug_out << "row: " << row << "(" << rows << "), col: " << col << "(" << cols << "), patno: "<< patno << std::endl;

        //In this loop I will parse the connection to account for the wrap around functionality
        //I decide not to add this to other functions because then I will have to change their parameters
        //And that will become very messy
        if(wrapAround) {
            for(auto &&elem : blocks[patno].children("connection")) {
                std::string oldFrom = elem.attribute("from").value();
                std::string oldTo = elem.attribute("to").value();

                //Check that the user is referring to relative blocks here
                bool isRel = false;
                if(oldTo.substr(0, 4) == "(rel") isRel = true;

                if(isRel) {
                    //Note: user should only use a single character for their counter name (for now)
                    elem.remove_attribute("to");
                    elem.append_attribute("to") = (parse_wrap_rel(oldTo, row, numberOfRows, col, numberOfRows, counters)).c_str();
                }
            }
        }

        block_declaration(env, blocks[patno]);

        // do the col skips
        // if the next skip results on putting on a new row, stop skipping
        bool keepColSkip = true;
        int colTemp = colSkipNum;
        while(colTemp > 0 && keepColSkip) {
            auto nextIt = it;
            ++nextIt;
            if((*(nextIt)).row - area.left() != row)
                keepColSkip = false;
            if(keepColSkip)
                ++it;
            colTemp--;
            if(it == area.end())
                keepGoing = false;
        }
        i += 1 + colSkipNum;

        //If the next position on the grid brings us to a new row, jump rows
        auto nextIt = it;
        ++nextIt;
        const auto newPos = *nextIt;
        const auto newRow = newPos.row - area.left();
        if(row != newRow) {
            int rowsToSkip = rowSkipNum;
            while(rowsToSkip > 0) {
                int remainingCols = numberOfCols;
                while(remainingCols > 0) {
                    ++it;
                    if(it == area.end())
                        keepGoing = false;
                    remainingCols--;
                }
                i+=cols;
                rowsToSkip--;
            }
        }
    }
    debug_out << "}" << std::endl;
}

void template_subcommand(ADL::environment &env, const pugi::xml_node &command, pugi::xml_node &target, const pugi::xml_node &copyFrom)
{
    std::string commandName(command.name());
    if (commandName == "attribute") {
        auto overwrite = command.attribute("overwrite").as_bool(false);
        auto name = command.attribute("name").value();
        auto value = command.attribute("value").value();
        if (overwrite || copyFrom.attribute(name).empty()) {
            target.remove_attribute(name);
            target.append_attribute(name) = value;
        }
    } else if (commandName == "children") {
        auto insert = std::string(command.attribute("insert").value());
        for (auto && child : command) {
            if (insert == "" || insert == "tail")
                target.append_copy(child);
            else if (insert == "head")
                target.prepend_copy(child);
            else {
                std::cerr << "ERROR: Unknown insert position: " << insert << std::endl;
            }
        }
    }
}

std::string ADL::parse_wrap_rel(std::string rel, int currentRow, int maxRow, int currentCol, int maxCol, std::vector<std::pair<std::string, int>> counters)
{
    //this function is only called if the user is using the 'rel' functionality in the XML
    std::cout << "The user is referring to a relative block... Check for wrap around...\n";
    std::cout << "Looking for the first number in rel...\n";
    bool counter1Found = false;
    bool number1Found = false;
    bool foundAllNumbers = false;
    size_t range;
    std::string firstNum, secondNum;

    for(int i = 5; !counter1Found && !number1Found; i++) {
        if(rel[i] == ' ') number1Found = true;
        if(rel[i] == '(') counter1Found = true;

        //expand the counter here and replace it with the number
        if(counter1Found) {
            std::cout << "The first number is a counter... expanding counter...\n";
            //Find out which counter it is
            std::string counterName = rel.substr(i+1, 1);
            std::cout << counterName << std::endl;
            int newNumber;
            for(auto elem : counters) {
                if(counterName == elem.first) newNumber = elem.second;
            }
            std::string toReplace = "(" + counterName + ")";
            rel.replace((rel.find(toReplace)), toReplace.length(), std::to_string(newNumber));
            std::cout << "The string has been changed to: " << rel << std::endl;
            number1Found = true;
            while(rel[i] != ' ') i++;
            //abort();
        }

        if(number1Found) {
            std::cout << "The first number was found\n";
            int tempFirstNumber = std::stoi(rel.substr(5, i-5));
            std::cout << "Number: " << tempFirstNumber << "\nChanging number...\n";
            if(tempFirstNumber + currentRow >= maxRow) tempFirstNumber = tempFirstNumber%maxRow;
            if(tempFirstNumber + currentRow < 0) tempFirstNumber = (tempFirstNumber%maxRow + maxRow)%maxRow;
            bool number2Found = false;
            bool counter2Found = false;
            firstNum = std::to_string(tempFirstNumber);
            for(int j = i+1; !counter2Found && !number2Found; j++) {
                if(rel[j] == ')') number2Found = true;
                if(rel[j] == '(') counter2Found = true;

                if(counter2Found) {
                    std::cout << "The second number is a counter... expanding counter...\n";
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
                    std::cout << "The second number was found\n";
                    int tempSecondNumber = std::stoi(rel.substr(i+1, j-(i+1)));
                    std::cout << "Number: " << tempSecondNumber << "\nChanging number...\n";
                    if(tempSecondNumber + currentCol >= maxCol) tempSecondNumber = tempSecondNumber%maxCol;
                    if(tempSecondNumber + currentCol < 0) tempSecondNumber = (tempSecondNumber%maxCol + maxCol)%maxCol;

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
        std::cout << "The string is " << reconstructed << "\n";
        return reconstructed;
    }

    return rel;
}

void ADL::template_interpreter(environment env, pugi::xml_node target, const pugi::xml_node templ)
{
    debug_print_node("template target", target);
    debug_print_node("template", templ);

    if (!conditional(env, target))
        return;

    debug_out << "template {" << std::endl;

    std::vector<std::string> attributeNames = tokenize_list(expand_name(templ.attribute("attributes").value(), env));

    std::map<std::string, std::string> attributes;
    for (auto && attr : attributeNames) {
        attributes[attr] = target.attribute(attr.c_str()).value();
    }

    env.function("attribute", [&attributes](environment env, std::vector<std::string> args){
            if (args.size() != 2)
            std::cerr << "Invalid number of arguments for attribute()" << std::endl;
            return attributes[args.at(1)];
            });

    for (auto && node : templ.children()) {
        std::string nodeName(node.name());
        if (nodeName == "append") {
            auto elem = target.append_child(node.attribute("element").value());
            debug_print_node("append to", elem);
            for (auto && command : node) {
                template_subcommand(env, command, elem, elem);
            }
            debug_print_node("appended: ", elem);
        } else if (nodeName == "copy") {
            auto pat = node.attribute("pattern").value();
            for (auto && t : target.children(pat)) {
                debug_print_node("copy from", t);
                auto elem = target.append_copy(t);
                elem.set_name(node.attribute("element").value());
                for (auto && command : node) {
                    template_subcommand(env, command, elem, t);
                }
                debug_print_node("copy to", elem);
            }
        } else if (nodeName == "remove") {
            auto pat = node.attribute("pattern").value();
            debug_out << "remove element " << pat << std::endl;
            target.remove_child(pat);
        } else {
            debug_out << "ERROR: unknown template command " << nodeName << std::endl;
            debug_print_node("template element: ", node);
        }
    }

    for (auto && pat : target.children("temp-pattern"))
    {
        pattern_declaration(env, pat);
    }

    target.remove_child("temp-pattern");
}

void ADL::user_declaration(environment env, pugi::xml_node node)
{
    std::string name(node.name());
    debug_print_node(name, node);

    debug_out << "syntactic sugar {" << std::endl;

    if (syntaxSugarTemplates.count(name) == 0) {
        std::cerr << "Element `" << name << "' is not registered as a syntax sugar." << std::endl;
        throw std::runtime_error("template not found");
    }
    template_interpreter(env, node, syntaxSugarTemplates.at(name));

    debug_out << "}" << std::endl;
}

std::string ADL::module_registration(const std::string & name, const std::string & mode)
{

    if (moduleTemplates.count(name) == 0) {
        std::cerr << "ERROR: module " << name << " is not found!" << std::endl;
        throw std::runtime_error("module not found");
    }

    auto p = moduleTemplates.at(name);
    auto env = p.first;
    auto moduleNode = p.second;

    debug_print_node("module registration", moduleNode);

    debug_out << "module {" << std::endl;

    // ToDo: To prevent collision, it is better to use base64 or other encodings instead of hash.
    auto mangledName = name + "_" + (mode == "" ? "" : std::to_string(std::hash<std::string>()(mode)));

    if (modules.count(mangledName) == 1) {
        debug_out << "module " << mangledName << " found." << std::endl;
        debug_out << "}" << std::endl;
        return mangledName;
    }

    env.function("mode", const_function("mode", mode));

    // evaulate macro definitions _after_ all attributes are expanded.
    for (auto && def : moduleNode.children("definition")) {
        macro_definition(env, def);
    }

    debug_out << "Module registration" << std::endl;

    auto mod = std::make_shared<ADLModuleWrapper>(mangledName);
    env.module(mod);

    try {
        pugi::xpath_node_set ports = moduleNode.select_nodes("(input|output|inout)");
        for (pugi::xpath_node port: ports)
        {
            port_declaration(env, port.node());
        }
    } catch(const pugi::xpath_exception& ex)
    {
        debug_out << "Exception caught: " << ex.what() << std::endl;
        debug_out << "I think there's no port" << std::endl;
    }
    for (auto && pat : moduleNode.children("wire"))
    {
        wire_declaration(env, pat);
    }
    for (auto && pat : moduleNode.children("inst"))
    {
        module_instantiation(env, pat);
    }
    for (auto && pat : moduleNode.children("connection"))
    {
        connection_declaration(env, pat);
    }

    modules[mangledName] = mod;
    debug_out << "module " << mangledName << " registered." << std::endl;

    debug_out << "}" << std::endl;

    return mangledName;
}

std::string ADL::module_instantiation(ADL::environment env, const pugi::xml_node moduleNode)
{
    debug_print_node("module instantiation", moduleNode);

    if (!conditional(env, moduleNode))
        return "";

    pugi::xml_attribute classAttr = moduleNode.attribute("module");
    pugi::xml_attribute nameAttr = moduleNode.attribute("name");
    pugi::xml_attribute modeAttr = moduleNode.attribute("mode");


    debug_out << "module {" << std::endl;
    std::string name(expand_name(nameAttr.value(), env));
    std::string mode(expand_name(modeAttr.value(), env));

    if (!nameAttr) {
        debug_out << "Module has no name attribute" << std::endl;
    }

    if (!nameAttr || name.empty()) {
        debug_out << "Invalid module name" << std::endl;
    }

    // Module instantiation by reference
    std::string cls = classAttr.value();
    debug_out << "class: " << cls << std::endl;
    if (moduleTemplates.count(cls) == 1){
        debug_out << "user defined: " << cls << std::endl;
        auto module = module_registration(cls, mode);
        env.module().addSubModule(name, modules.at(module));
    } else {
        debug_out << "primitive" << std::endl;
        env.module().addPrimitive(name, cls, attribute_map(moduleNode, env));
    }
    debug_out << std::endl;;

    debug_out << "}" << std::endl;

    return name;
}
std::string ADL::module_declaration(ADL::environment env, const pugi::xml_node moduleNode)
{
    debug_print_node("module declaration", moduleNode);

    if (!conditional(env, moduleNode))
        return "";

    pugi::xml_attribute classAttr = moduleNode.attribute("class");
    pugi::xml_attribute nameAttr = moduleNode.attribute("name");


    debug_out << "module {" << std::endl;
    std::string name(nameAttr.value());

    if (!nameAttr) {
        debug_out << "Module has no name attribute" << std::endl;
    }

    if (!nameAttr || name.empty()) {
        debug_out << "Invalid module name" << std::endl;
    }

    if (!classAttr && empty(moduleNode.children())) {
        debug_out << "No content in the module" << std::endl;
    }

    if (classAttr && !empty(moduleNode.children())) {
        debug_out << "Currently no support for module inheritance" << std::endl;
    }

    // evaulate macro definitions _after_ all attributes are expanded.
    for (auto && def : moduleNode.children("definition")) {
        macro_definition(env, def);
    }

    if (moduleTemplates.count(name) == 1) {
        debug_out << "ERROR: module " << name << " is already declared." << std::endl;
        debug_out << "}" << std::endl;
        return name;
    }

    moduleTemplates[name] = std::make_pair(env, moduleNode);
    debug_out << "module " << name << " is declared." << std::endl;

    debug_out << "}" << std::endl;

    return name;
}

void  ADL::block_declaration(ADL::environment env, pugi::xml_node block)
{
    debug_print_node("block", block);

    if (!conditional(env, block))
        return;

    debug_out << "block " << "(" << env.position().row << "," << env.position().col <<") {" << std::endl;

    // evaulate macro definitions _after_ all attributes are expanded.
    for (auto && def : block.children("definition")) {
        macro_definition(env, def);
    }

    try {
        pugi::xpath_node_set ports = block.select_nodes("(input|output|inout)");
        for (pugi::xpath_node port: ports)
        {
            port_declaration(env, port.node());
        }
    } catch(const pugi::xpath_exception& ex)
    {
        debug_out << "Exception caught: " << ex.what() << std::endl;
        debug_out << "I think there's no port" << std::endl;
    }
    for (auto && pat : block.children("wire"))
    {
        wire_declaration(env, pat);
    }
    std::vector<pugi::xml_node> modules;
    auto moduleAttr = block.attribute("module");
    if (moduleAttr) {
        auto m = block.append_child("inst");
        for (auto && attr : block.attributes()) {
            m.append_copy(attr);
        }
        block.remove_attribute(moduleAttr);
    }

    for (auto && m : block.children("inst")) {
        modules.push_back(m);
    }

    if (modules.size() > 1) {
        debug_out << "More than one module in the block" << std::endl;
    }
    for (auto && m : modules)
    {
        if (m.attribute("name")) {
            debug_out << "Module already has a name." << std::endl;
        } else {
            debug_out << "Name of the module which a block only has is set with default value." << std::endl;
            m.remove_attribute("name");
            m.append_attribute("name") = "(rel 0 0)";
        }
        module_instantiation(env, m);
    }
    for (auto && c : block.children("connection"))
    {
        connection_declaration(env, c);
    }
    debug_out << "}" << std::endl;
}

// from http://stackoverflow.com/questions/2482716/function-in-c-for-finding-if-a-word-is-prefix
    template<class C, class T, class A>
bool starts_with(std::basic_string<C,T,A> const& haystack,
        std::basic_string<C,T,A> const& needle)
{
    return needle.length() <= haystack.length() &&
        std::equal(needle.begin(), needle.end(), haystack.begin());
}

void  ADL::architecture_declaration(ADL::environment env, const pugi::xml_node arch)
{
    debug_print_node("architecture", arch);

    if (!conditional(env, arch))
        return;

    pugi::xml_attribute rowAttr = arch.attribute("row");
    pugi::xml_attribute colAttr = arch.attribute("col");
    int row = rowAttr ? rowAttr.as_int() : default_row;
    int col = colAttr ? colAttr.as_int() : default_col;
    ADL::area_size areasize(row, col);
    env.areaSize(row, col);
    env.function("rows", const_function("rows", std::to_string(row)));
    env.function("cols", const_function("cols", std::to_string(col)));

    std::map<std::string, std::string> configs;
    for (auto && attr : arch.attributes()) {
        if (starts_with(std::string(attr.name()), ADL::TopCGRAModule::config_prefix))
            configs[attr.name()] = attr.value();
    }
    env.module().setConfiguration(configs);

    for (auto && def : arch.children("definition")) {
        macro_definition(env, def);
    }

    debug_out << "pattern {" << std::endl;
    try {
        pugi::xpath_node_set ports = arch.select_nodes("(input|output|inout)");
        for (pugi::xpath_node port: ports)
        {
            port_declaration(env, port.node());
        }
    } catch(const pugi::xpath_exception& ex)
    {
        debug_out << "Exception caught: " << ex.what() << std::endl;
        debug_out << "I think there's no port" << std::endl;
    }
    for (auto && pat : arch.children("wire"))
    {
        wire_declaration(env, pat);
    }
    try {
        pugi::xpath_node_set ports = arch.select_nodes("(pattern|mesh|diagonal)");

        for (pugi::xpath_node pat: ports)
        {
            std::string name = pat.node().name();
            if (name == "pattern") {
                pattern_declaration(env, pat.node());
            } else {
                user_declaration(env, pat.node());
            }
        }
    } catch(const pugi::xpath_exception& ex)
    {
        debug_out << "Exception caught: " << ex.what() << std::endl;
        debug_out << "I think there's no pattern/mesh" << std::endl;
    }
    for (auto && pat : arch.children("connection"))
    {
        connection_declaration(env, pat);
    }
    debug_out << "}" << std::endl;
}

void  ADL::cgra_declaration(environment env, const pugi::xml_node doc)
{
    auto cgra = doc.child("cgra");
    debug_out << "cgra {" << std::endl;

    for (auto && def : cgra.children("definition")) {
        macro_definition(env, def);
    }

    debug_out << "arch:" << std::endl;
    for (pugi::xml_node mod: cgra.children("module"))
    {
        module_declaration(env, mod);
    }
    architecture_declaration(env, cgra.child("architecture"));
    debug_out << "}" << std::endl;
}

bool load_template_xml(pugi::xml_document &doc)
{
    pugi::xml_parse_result result = doc.load_string(template_xml);

    return doc;
}

std::unique_ptr<CGRA> ADL::parse(const pugi::xml_node doc)
{
    // auto cgra = std::make_unique<CGRA>();
    std::shared_ptr<CGRA> cgra(new CGRA());
    auto cgramod = std::make_shared<TopCGRAModule>(*cgra);
    parse_result = std::make_shared<environment>(cgramod);
    auto & env = *parse_result;
    env.debug_out(debug_out);

    pugi::xml_document templs;
    if (!load_template_xml(templs)) {
        std::cerr << "Failed to read the template file." << std::endl;
        throw std::runtime_error("failed to read the template file");
    }

    for (auto && templ : templs.first_child().children("template")) {
        std::string pat(templ.attribute("pattern").value());
        if (pat.empty()) {
            std::cerr << "Template's target element name should not be empty. " << std::endl;
            throw std::runtime_error("empty name as template element");
        }
        if (syntaxSugarTemplates.count(pat) != 0) {
            std::cerr << "Duplicate elements in template: " << pat << std::endl;
            throw std::runtime_error("duplicate template element");
        }
        syntaxSugarTemplates[std::move(pat)] = templ;
    }

    env.function("rel", [](environment env, std::vector<std::string> args){
            if (args.size() != 3)
            std::cerr << "Invalid number of arguments for rel()" << std::endl;
            auto row = std::stoi(args[1], nullptr, 0) + env.position().row;
            auto col = std::stoi(args[2], nullptr, 0) + env.position().col;
            return block_name(grid_position(row, col));
            });

    env.function("block", [](environment env, std::vector<std::string> args){
            if (args.size() != 3)
            std::cerr << "Invalid number of arguments for block()" << std::endl;
            auto row = std::stoi(args[1], nullptr, 0);
            auto col = std::stoi(args[2], nullptr, 0);
            return block_name(grid_position(row, col));
            });

    env.function("on-edge", [](environment env, std::vector<std::string> args){
            if (args.size() != 2)
            std::cerr << "Invalid number of arguments for on-edge()" << std::endl;
            auto edge = ADL::read_division(args.at(1));
            bool onEgde = env.in_division(edge);
            return std::to_string(onEgde);
            });

    env.function("first", [](environment env, std::vector<std::string> args){
            auto it = args.begin();
            ++it; // skip the function name

            if (it != args.end()) {
            return *it;
            } else {
            return std::string();
            }
            });

    env.function("empty", [](environment env, std::vector<std::string> args){
            auto it = args.begin();
            ++it; // skip the function name

            if (it != args.end()) {
            return std::to_string(false);
            } else {
            return std::to_string(true);
            }
            });

    env.function("elem", [](environment env, std::vector<std::string> args){
            if (args.size() < 2) {
            std::cerr << "Invalid number of arguments for elem()" << std::endl;
            return std::to_string(false);
            } else {
            auto it = args.begin();
            ++it; // skip the function name

            auto elem = *it;
            ++it;

            for (; it != args.end(); ++it) {
            auto arg = *it;
            if (arg == elem)
            return std::to_string(true);
            }
            return std::to_string(false);
            }
            });

    env.function("eq", [](environment env, std::vector<std::string> args){
            if (args.size() >= 2)
            std::cerr << "Invalid number of arguments for string-=()" << std::endl;
            auto it = args.begin();
            ++it; // skip the function name

            auto elem = *it;
            ++it;

            for (; it != args.end(); ++it) {
            auto arg = *it;
            if (arg != elem)
            return std::to_string(false);
            }
            return std::to_string(true);
            });

    env.function("suffix", [](environment env, std::vector<std::string> args){
            if (args.size() >= 2)
            std::cerr << "Invalid number of arguments for prefix()" << std::endl;
            auto it = args.begin();
            ++it; // skip the function name

            auto prefix = *it;
            ++it;

            std::ostringstream ostr;
            if (it != args.end())
            ostr << *it << prefix;
            ++it;

            for (; it != args.end(); ++it) {
            ostr << " " << *it << prefix;
            }
            return ostr.str();
            });

    env.function("prefix", [](environment env, std::vector<std::string> args){
            if (args.size() >= 2)
            std::cerr << "Invalid number of arguments for prefix()" << std::endl;
            auto it = args.begin();
            ++it; // skip the function name

            auto prefix = *it;
            ++it;

            std::ostringstream ostr;
            if (it != args.end())
            ostr << prefix << *it;
            ++it;

            for (; it != args.end(); ++it) {
            ostr << " " << prefix << *it;
            }
            return ostr.str();
            });

    env.function("edge", [](environment env, std::vector<std::string> args){
            if (args.size() != 1)
            std::cerr << "Invalid number of arguments for on-edge()" << std::endl;
            if (env.in_division(division::NW))
            return "northwest";
            else if (env.in_division(division::N))
            return "north";
            else if (env.in_division(division::NE))
            return "northeast";
            else if (env.in_division(division::W))
            return "west";
            else if (env.in_division(division::C))
            return "center";
            else if (env.in_division(division::E))
            return "east";
            else if (env.in_division(division::SW))
            return "southwest";
            else if (env.in_division(division::S))
            return "south";
            else if (env.in_division(division::SE))
            return "southeast";
            else return "invalid";
    });

    env.function("not", [](environment env, std::vector<std::string> args){
            if (args.size() != 2)
            std::cerr << "Invalid number of arguments for not()" << std::endl;
            std::istringstream iss(args.at(1));
            bool b;
            iss >> b;
            return std::to_string(!b);
            });

    env.function("and", accum_function<bool, bool>("and", true,
                [](int a, int b) {
                if (b) {
                return std::make_pair(true, true);
                } else {
                return std::make_pair(false, false);
                }
                }));

    env.function("or", accum_function<bool, bool>("or", false,
                [](int a, int b) {
                if (b) {
                return std::make_pair(true, false);
                } else {
                return std::make_pair(false, true);
                }
                }));

    env.function("+", accum_function<int, int>("+", 0,
                [](int a, int b) {
                return std::make_pair(a + b, true);
                }));

    env.function("-", accum_function<int, int>("-", 0,
                [](int a, int b) {
                return std::make_pair((-a) - b, true);
                }));

    env.function("*", accum_function<int, int>("*", 1,
                [](int a, int b) {
                return std::make_pair(a * b, true);
                }));

    env.function("begin", const_function("begin", "("));
    env.function("end", const_function("end", ")"));
    env.function("space", const_function("space", " "));
    env.function("$", function_apply);

    cgra_declaration(env, doc);

    // return dynamic_cast<std::unique_ptr<CGRA>>(env.module().instantiate("cgra"));
    return std::unique_ptr<CGRA>(dynamic_cast<CGRA *>(env.module().instantiate("cgra").release()));
    // return std::unique_ptr<CGRA>(env.module().instantiate("cgra"));
}

}

// vim:et sw=4:
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(XML_VALIDATION)
#include <libxml++/libxml++.h>
#endif

#include "pugixml.hpp"
