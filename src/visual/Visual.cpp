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

#include <string>
#include <set>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

#include <CGRA/Exception.h>
#include <CGRA/Visual.h>

#define NO_PREVIOUS_CYCLE_INPUT

void genCGRAVisual(std::string exe_path, std::shared_ptr<CGRA> cgra, int II)
{
    std::ifstream par_js(exe_path + "/../../src/visual/visual.partial.js");
    if(par_js.fail())
        throw cgrame_visual_error("Failed Creating ifstream visual.partial.js");
    std::ofstream context_f(exe_path + "/../../output/context-div.js", std::ios::trunc);
    if(context_f.fail())
        throw cgrame_visual_error("Failed Creating ofstream on context-div.js");
    std::ofstream f(exe_path + "/../../output/CGRA.js", std::ios::trunc);
    if(f.fail())
        throw cgrame_visual_error("Failed Creating ofstream on CGRA.js");

    MRRG * mrrg = cgra->getMRRG(II).get();
    if(!mrrg->nodes.size())
        throw cgrame_visual_error("MRRG Is Empty, Unable to Produce Visualization");

    // Fill context-div.js
    context_f << "document.write(\"\\" << std::endl;
    context_f << "    <div class=\\\"tab\\\">\\" << std::endl;
    context_f << "        <button class=\\\"tablinks\\\" onclick=\\\"showContext(event, 'context_0')\\\" id=\\\"defaultOpen\\\">Context 0</button>\\" << std::endl;
    for(int i = 1; i < mrrg->nodes.size(); ++i)
        context_f << "        <button class=\\\"tablinks\\\" onclick=\\\"showContext(event, 'context_" << i << "')\\\">Context " << i << "</button>\\" << std::endl;
    context_f << "    </div>\\" << std::endl;
    for(int i = 0; i < mrrg->nodes.size(); ++i)
        context_f << "    <div id=\\\"context_" << i << "\\\" class=\\\"tabcontent\\\"></div>\\" << std::endl;
    context_f << "\");" << std::endl;

    // Create top level function call
    f << "function draw() {" << std::endl;
    for(int i = 0; i < mrrg->nodes.size(); ++i)
        f << "    draw_helper(" << i << ");" << std::endl;
    f << "}" << std::endl;

    std::vector<std::set<std::pair<int, std::string>, std::greater<std::pair<int, std::string>>>> cluster_type;

    int max_module_depth = 0;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        cluster_type.push_back(std::set<std::pair<int, std::string>, std::greater<std::pair<int, std::string>>>());
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); n++)
        {
            std::string s = n->second->getFullName();
            int count = 0;
            std::string::size_type pre_pos = 0;
            while(true)
            {
                auto pos = s.find('.', pre_pos);
                pre_pos = pos + 1;
                if(pos == std::string::npos)
                    break;
                else
                    cluster_type[i].insert(std::make_pair(++count, s.substr(0, pos)));
            }
            if(count > max_module_depth)
                max_module_depth = count;
        }
    }

    //if(max_module_depth <= 4)
    f << "var clusterColor = [ '#6698FF', '#27CEC8', '#30FF89', '#EEFF99' ];" << std::endl;
    //else
    //throw cgrame_visual_error("CGRA Architecture Is Too Complicated");

    // Create cluster types
    f << "var clusterTypes = [];" << std::endl;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        f << "clusterTypes.push([" << std::endl;
        for(auto & s : cluster_type[i])
        {
            f << "[\"" << s.second << "\", " << s.first << "]," << std::endl;
        }
        f << "]);" << std::endl;
    }

#ifndef NO_PREVIOUS_CYCLE_INPUT
    std::multimap<MRRGNode*, MRRGNode*> input_nodes;
#endif

    // Create nodes
    f << "var nodes = [];" << std::endl;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        std::set<std::string> nodes;
        f << "nodes.push(new vis.DataSet([" << std::endl;
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); n++)
        {
            if(n->second->type == MRRG_NODE_FUNCTION)
                f << "{ id: '" << n->second->getFullName() << "', label: '" << n->second->getFullName() <<"', shape: 'box'}," << std::endl;
            else
                f << "{ id: '" << n->second->getFullName() << "', label: '" << n->second->getFullName() <<"'}," << std::endl;
            nodes.insert(n->second->getFullName());
        }
        // Find nodes not belong in this cycle
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); ++n)
        {
            for(auto fanout = n->second->fanout.begin(); fanout != n->second->fanout.end(); fanout++)
            {
#ifndef NO_PREVIOUS_CYCLE_INPUT
                if((*fanout)->cycle != n->second->cycle) // Input node from previous cycle to the node found
                    input_nodes.insert(std::make_pair(*fanout, n->second));
#endif
                if(nodes.find((*fanout)->getFullName()) == nodes.end())
                {
                    f << "{ id: '" << **fanout << "' , label: '" << **fanout << "', color: '#B464FF'}," << std::endl;
                    nodes.insert((*fanout)->getFullName());
                }
            }
        }
        f << "]));" << std::endl;
    }

    // Create edges
    f << "var edges = [];" << std::endl;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        f << "edges.push(new vis.DataSet([" << std::endl;
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); ++n)
        {
            for(auto fanout = n->second->fanout.begin(); fanout != n->second->fanout.end(); fanout++)
            {
                f << "{ from: '" << (*(n->second)) << "', to: '" << (**fanout) << "', arrows: 'to' }," << std::endl;
            }
        }
        f << "]));" << std::endl;
    }

#ifndef NO_PREVIOUS_CYCLE_INPUT
    for(const auto & input_node : input_nodes)
    {
        f << "nodes[" << input_node.first->cycle << "].update([" << "{ id: '" << input_node.second->getFullName() << "', label: '" << input_node.second->getFullName() << "', color: 'grey'}]);" << std::endl;
        f << "edges[" << input_node.first->cycle << "].add([" << "{ from: '" << input_node.second->getFullName() << "', to: '" << input_node.first->getFullName() << "', arrows: 'to' }]);" << std::endl;
    }
#endif

    f << par_js.rdbuf();
}

void genMappingVisual(std::string exe_path, const Mapping & mapping)
{
    std::ifstream par_js(exe_path + "/../../src/visual/visual.partial.js");
    if(par_js.fail())
        throw cgrame_visual_error("Failed Creating ifstream visual.partial.js");
    std::ofstream context_f(exe_path + "/../../output/context-div.js", std::ios::trunc);
    if(context_f.fail())
        throw cgrame_visual_error("Failed Ccreating ofstream on context-div.js");
    std::ofstream f(exe_path + "/../../output/CGRA.js", std::ios::trunc);
    if(f.fail())
        throw cgrame_visual_error("Failed Creating ofstream on CGRA.js");

    // Get MRRG
    auto mrrg = mapping.getCGRA()->getMRRG(mapping.getII()).get();
    if(!mrrg->nodes.size())
        throw cgrame_visual_error("MRRG Is Empty, Unable to Produce Visualization");

    // Fill context-div.js
    context_f << "document.write(\"\\" << std::endl;
    context_f << "    <div class=\\\"tab\\\">\\" << std::endl;
    context_f << "        <button class=\\\"tablinks\\\" onclick=\\\"showContext(event, 'context_0')\\\" id=\\\"defaultOpen\\\">Context 0</button>\\" << std::endl;
    for(int i = 1; i < mrrg->nodes.size(); ++i)
        context_f << "        <button class=\\\"tablinks\\\" onclick=\\\"showContext(event, 'context_" << i << "')\\\">Context " << i << "</button>\\" << std::endl;
    context_f << "    </div>\\" << std::endl;
    for(int i = 0; i < mrrg->nodes.size(); ++i)
        context_f << "    <div id=\\\"context_" << i << "\\\" class=\\\"tabcontent\\\"></div>\\" << std::endl;
    context_f << "\");" << std::endl;

    // Create top level function call
    f << "function draw() {" << std::endl;
    for(int i = 0; i < mrrg->nodes.size(); ++i)
        f << "    draw_helper(" << i << ");" << std::endl;
    f << "}" << std::endl;

    std::vector<std::set<std::pair<int, std::string>, std::greater<std::pair<int, std::string>>>> cluster_type;

    int max_module_depth = 0;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        cluster_type.push_back(std::set<std::pair<int, std::string>, std::greater<std::pair<int, std::string>>>());
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); ++n)
        {
            std::string s = n->second->getFullName();
            int count = 0;
            std::string::size_type pre_pos = 0;
            while(true)
            {
                auto pos = s.find('.', pre_pos);
                pre_pos = pos + 1;
                if(pos == std::string::npos)
                    break;
                else
                    cluster_type[i].insert(std::make_pair(++count, s.substr(0, pos)));
            }
            if(count > max_module_depth)
                max_module_depth = count;
        }
    }

    //if(max_module_depth <= 4)
    f << "var clusterColor = [ '#6698FF', '#27CEC8', '#30FF89', '#EEFF99' ];" << std::endl;
    //else
    //throw cgrame_visual_error("CGRA's Architecture Is Too Complicated");

    // Create cluster types
    f << "var clusterTypes = [];" << std::endl;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        f << "clusterTypes.push([" << std::endl;
        for(auto & s : cluster_type[i])
        {
            f << "[\"" << s.second << "\", " << s.first << "]," << std::endl;
        }
        f << "]);" << std::endl;
    }

#ifndef NO_PREVIOUS_CYCLE_INPUT
    std::multimap<MRRGNode*, MRRGNode*> input_nodes;
#endif

    // Create nodes
    f << "var nodes = [];" << std::endl;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        std::set<std::string> nodes;
        f << "nodes.push(new vis.DataSet([" << std::endl;
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); ++n)
        {
            if(n->second->type == MRRG_NODE_FUNCTION)
                f << "{ id: '" << n->second->getFullName() << "', label: '" << n->second->getFullName() <<"', shape: 'box'}," << std::endl;
            else
                f << "{ id: '" << n->second->getFullName() << "', label: '" << n->second->getFullName() <<"'}," << std::endl;
            nodes.insert(n->second->getFullName());
        }
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); ++n)
        {
            for(auto fanout = n->second->fanout.begin(); fanout != n->second->fanout.end(); fanout++)
            {
#ifndef NO_PREVIOUS_CYCLE_INPUT
                if((*fanout)->cycle != n->second->cycle) // Input node from previous cycle to the node found
                    input_nodes.insert(std::make_pair(*fanout, n->second));
#endif
                if(nodes.find((*fanout)->getFullName()) == nodes.end())
                    f << "{ id: '" << **fanout << "' , label: '" << **fanout << "', color: '#B464FF'}," << std::endl;
            }
        }
        f << "]));" << std::endl;
    }

    // Create edges
    f << "var edges = [];" << std::endl;
    for(unsigned int i = 0; i < mrrg->nodes.size(); ++i)
    {
        f << "edges.push(new vis.DataSet([" << std::endl;
        for(auto n = mrrg->nodes[i].begin(); n != mrrg->nodes[i].end(); ++n)
        {
            for(auto fanout = n->second->fanout.begin(); fanout != n->second->fanout.end(); fanout++)
            {
                f << "{ from: '" << (*(n->second)) << "', to: '" << (**fanout) << "', arrows: 'to' }," << std::endl;
            }
        }
        f << "]));" << std::endl;
    }

#ifndef NO_PREVIOUS_CYCLE_INPUT
    for(const auto & input_node : input_nodes)
    {
        f << "nodes[" << input_node.first->cycle << "].update([" << "{ id: '" << input_node.second->getFullName() << "', label: '" << input_node.second->getFullName() << "', color: 'grey'}]);" << std::endl;
        f << "edges[" << input_node.first->cycle << "].add([" << "{ from: '" << input_node.second->getFullName() << "', to: '" << input_node.first->getFullName() << "', arrows: 'to' }]);" << std::endl;
    }
#endif

    f << "var nodeMapped = []; while(nodeMapped.push([]) < " << mrrg->nodes.size() << ");" << std::endl;
    for(const auto & map : mapping.getMapping())
    {
        for(const auto & mrrg_node : map.second)
        {
            f << "nodeMapped[" << mrrg_node->cycle << "].push({ id: '" << mrrg_node->getFullName() << "', color: 'red', title: '" << map.first->name << "' });" << std::endl;
        }
    }

    f << par_js.rdbuf();
}

