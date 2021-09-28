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

#include "pugixml.hpp"

#include <tuple>
#include <vector>
#include <iostream>
#include "CGRA/Module.h"
#include "CGRA/ModuleComposites.h"
#include "CGRA/CGRA.h"
#include "adl.h"
#include <CGRA/user-inc/UserModules.h>

namespace adl0 {

const std::string ADL::TopCGRAModule::config_prefix = "cgra-";
const std::string ADL::TopCGRAModule::config_rows = ADL::TopCGRAModule::config_prefix + "rows";
const std::string ADL::TopCGRAModule::config_cols = ADL::TopCGRAModule::config_prefix + "cols";

static auto & dout = std::cout;

// http://stackoverflow.com/questions/2333728/stdmap-default-value
    template <template<class,class,class...> class C, typename K, typename V, typename... Args>
V at_default(const C<K,V,Args...>& m, K const& key, const V & defval)
{
    typename C<K,V,Args...>::const_iterator it = m.find( key );
    if (it == m.end())
        return defval;
    return it->second;
}

    template <typename T>
static T read(const std::string & str)
{
    std::istringstream is(str);
    T x;
    is >> x;
    return x;
}

void ADL::WrapperModule::addConfig(Module &m, std::string name, std::vector<std::string> ConnectTo)
{
    dout << "addConfig: " << name << std::endl;
    m.addConfig(new ConfigCell(name), ConnectTo);
}
void ADL::WrapperModule::addSubModule(Module &m, std::string name, WrapperModule &mod)
{
    dout << "addSubmodule: " << name << std::endl;
    m.addSubModule(mod.instantiate(name).release());
}
void ADL::WrapperModule::addPrimitive(Module &m, std::string name, std::string type, const std::map<std::string, std::string> &args)
{
    dout << "addPrimitive: " << name << " : " << type << std::endl;
    if (type == "mux") {
        int size = std::stoi(args.at("inputs")); // ToDo: catch std::invalid_argument and std::out_of_range.

        m.addSubModule(new Multiplexer(name, size));
    } else if (type == "io") {
        m.addSubModule(new IO(name));
    } else if (type == "FuncUnit") {
        auto op = args.count("op") == 0 ? std::vector<OpGraphOpCode>{OPGRAPH_OP_ADD, OPGRAPH_OP_MUL } : tokenize_list_as<OpGraphOpCode>(args.at("op"));
        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        auto latency = args.count("latency") == 0 ? 0 : std::stoi(args.at("latency"));
        auto II = args.count("II") == 0 ? 1 : std::stoi(args.at("latency"));
        m.addSubModule(new FuncUnit(name, op, size, II, latency));
        if (op.size() == 0) {
            std::cerr << "No operations!" << std::endl;
        } else if (op.size() > 1) {
            m.addConfig(new ConfigCell(name + "_Config"), {name + ".select"});
        }
    } else if (type == "PHIUnit") {
        /*        auto op = args.count("op") == 0 ? std::vector<OpGraphOpCode>{OPGRAPH_OP_ADD, OPGRAPH_OP_MUL } : tokenize_list_as<OpGraphOpCode>(args.at("op"));
                  auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
                  m.addSubModule(new FuncUnit(name, op, size));
                  if (op.size() == 0) {
                  std::cerr << "No operations!" << std::endl;
                  } else if (op.size() > 1) {
                  m.addConfig(new ConfigCell(name + "_Config"), {name + ".select"});
                  }
         */
    }
    else if (type == "MEMUnit") {
        m.addSubModule(new MEMUnit(name));
    }
#if 0
    else if (type == "GEPUnit") {

    }
#endif
    else if (type == "Register") {
        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        m.addSubModule(new Register(name, size));
    } else if (type == "Multiplexer") {
        auto muxSize = args.count("ninput") == 0 ? 0 : std::stoi(args.at("ninput"));
        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        m.addSubModule(new Multiplexer(name, muxSize, size));
    } else if (type == "Tristate") {
        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        m.addSubModule(new TriState(TriState::Mode::PLAIN, name, size));
    } else if (type == "IO") {
        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        m.addSubModule(new IO(name, size));
    } else if (type == "CustomModule") {
        auto functionalities = tokenize_list(args.count("functionalities") == 0 ? "" : args.at("functionalities"));
        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        m.addSubModule(new CustomModule(name, functionalities, size));
    } else if (type == "RegisterFile") {
        auto ninput = args.count("ninput") == 0 ? 0 : std::stoi(args.at("ninput"));
        auto noutput = args.count("noutput") == 0 ? 0 : std::stoi(args.at("noutput"));
        auto log2nregister = args.count("log2-nregister") == 0 ? 0 : std::stoi(args.at("log2-nregister"));
        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        m.addSubModule(new RegisterFile(name, ninput, noutput, log2nregister, size));
    } else if (type == "UserModule") {
        auto prototype = args.count("prototype") == 0 ? "" : args.at("prototype");
        auto ports = tokenize_list(args.count("ports") == 0 ? "" : args.at("ports"));
        m.addSubModule(new UserModule(prototype, name, ports));
    } else if (type == "SimpleFU") {
        auto blockType = args.count("type") == 0 ? STANDARD_NOBYPASS : read<CGRABlockType>(args.at("type"));
        m.addSubModule(new SimpleFU(name, blockType));
    } else if (type == "AdresPE") {
        for(auto elem : args)
        {
            std::cout << elem.first << "     " << elem.second << "\n";
        }
        auto ninput = args.count("ninput") == 0 ? 0 : std::stoi(args.at("ninput"));
        auto fu_type = args.count("op") == 0 ? "cga" : args.at("op");

        auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
        std::cout << "Adding new AdresPE...\n";
        m.addSubModule(new AdresPE(name, ninput, fu_type));
        std::cout << "AdresPE added with name: " << name << " inputs: " << ninput << " mode: " << args.at("op");
    }  else {
        /*
           auto size = args.count("size") == 0 ? DEFAULT_SIZE : std::stoi(args.at("size"));
           userFunctions test;
           int i = 0;
           int funcNum = -1;
           for(auto elem : test.functionNames)
           {
           if(elem == type) funcNum = i;
           i++;
           }
           Module* a = &m;
           if(funcNum != -1) test.functionVector[funcNum](name, size, args, a);
           m.addConfig(new ConfigCell(name + "_Config"), {name + ".select"});
         */
    }
}

void ADL::WrapperModule::addWire(std::string wire)
{
    dout << "addWire: " << wire << std::endl;
    addAlias(wire);
}
void ADL::WrapperModule::addPort(Module &m, std::string portname, port_type pt, unsigned size)
{
    dout << "addPort: " << portname << " : " << pt << std::endl;
    m.addPort(portname, pt, size);
}
void ADL::WrapperModule::addConnection(Module &m, std::string from, std::string to)
{
    dout << "addConnection: " << from << " -> " << to << std::endl;
    m.addConnection(from, to);
}

bool ADL::WrapperModule::isAlias(const std::string & alias) {
    return aliases_.count(alias) != 0;
}
void ADL::WrapperModule::addAlias(const std::string & alias) {
    aliases_.insert(alias);
}
void ADL::WrapperModule::addAliasSubstantia(const std::string & alias, const std::string & subst) {
    correspondences_[alias] = subst;
}
std::string ADL::WrapperModule::substantia(std::string alias) {
    std::set<std::string> visited;
    while (correspondences_.count(alias) != 0) {
        if (visited.count(alias) != 0) {
            std::cerr << "Cyclar dependency is found during resolving alias " << alias << std::endl;
            throw std::runtime_error("Cyclar denepdency");
        } else {
            visited.insert(alias);
            alias = correspondences_.at(alias);
        }
    }
    if (isAlias(alias)) {
        std::cerr << "No substantia is registered as alias " << alias << std::endl;
        throw std::runtime_error("No substantia");
    }

    return alias;
}

void ADL::ADLModuleWrapper::addSubModule(std::string name, std::shared_ptr<WrapperModule> mod) {
    submodules.push_back(std::make_pair(name, std::move(mod)));
}
void ADL::ADLModuleWrapper::addPrimitive(std::string name, std::string mod, const std::map<std::string, std::string> &args) {
    primitives.push_back(std::make_tuple(name, mod, args));
}
void ADL::ADLModuleWrapper::addPort(std::string portname, port_type pt, unsigned size) {
    ports.push_back(std::make_tuple(portname, pt, size));
}
void ADL::ADLModuleWrapper::addConfig(std::string name, std::vector<std::string> ConnectTo) {
    configs.push_back(std::make_pair(name, ConnectTo));
}
void ADL::ADLModuleWrapper::addConnection(std::string from, std::string to) {
    if (isAlias(to)) {
        std::cout << "alias: " << to << " = " << from << std::endl;
        addAliasSubstantia(to, from);
    } else {
        connections.push_back(std::make_tuple(from, to));
    }
}
std::unique_ptr<Module> ADL::ADLModuleWrapper::instantiate(std::string name) {
    // auto m = std::make_shared<Module>(name);
    std::unique_ptr<Module> m (new ADLModule(name, cls_));
    return instantiate(std::move(m));
}
std::unique_ptr<Module> ADL::ADLModuleWrapper::instantiate(std::unique_ptr<Module> m) {
    // auto m = std::make_shared<Module>(name);
    for (auto && port : ports) {
        WrapperModule::addPort(*m, std::get<0>(port), std::get<1>(port), std::get<2>(port));
    }
    for (auto && module : primitives) {
        WrapperModule::addPrimitive(*m, std::get<0>(module), std::get<1>(module), std::get<2>(module));
    }
    for (auto && submodule : submodules) {
        // m->addSubModule(submodule.second->instantiate(submodule.first).get());
        WrapperModule::addSubModule(*m, submodule.first, *submodule.second);
    }
    for (auto && config : configs) {
        WrapperModule::addConfig(*m, config.first, config.second);
    }
    for (auto && connection : connections) {
        auto from = substantia(std::get<0>(connection));
        auto to = substantia(std::get<1>(connection));
        WrapperModule::addConnection(*m, from, to);
    }
    return std::move(m);
}

void ADL::ADLModuleWrapper::setConfiguration(const std::map<std::string, std::string> config)
{
    for (auto && c : config) {
        std::cerr << "unknown configuration for ADLModuleWrapper: " << c.first << "=" << c.second << std::endl;
    }
}

void ADL::TopCGRAModule::setConfiguration(const std::map<std::string, std::string> config)
{
    for (auto && c : config) {
        if (c.first == config_rows) {
            cgra.ROWS = std::stoi(c.second);
        } else if (c.first == config_cols) {
            cgra.COLS = std::stoi(c.second);
        } else {
            std::cerr << "unknown configuration for TopCGRAModule: " << c.first << "=" << c.second << std::endl;
        }
    }
}

// vim:et sw=4:
}

