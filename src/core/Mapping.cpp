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

#include <algorithm>
#include <vector>
#include <memory>

#include <CGRA/Mapping.h>
#include <CGRA/Exception.h>

Mapping::Mapping(std::shared_ptr<CGRA> cgra, int II, std::shared_ptr<OpGraph> opgraph)
{
    this->cgra      = cgra;
    this->II        = II;
    this->opgraph   = opgraph;

    this->mapped    = false;
}

Mapping::~Mapping()
{
}

bool Mapping::isMapped() const
{
    return mapped;
}

void Mapping::setMapped(bool m)
{
    this->mapped = m;
}

CGRA * Mapping::getCGRA() const
{
    return cgra.get();
}

int Mapping::getII() const
{
    return II;
}

void Mapping::mapMRRGNode(OpGraphNode* opnode, MRRGNode* node)
{
    try
    {
        mapping[opnode].push_back(node);
    }
    catch(const std::exception & e)
    {
        throw cgrame_error(std::string("Mapping Exception Thrown by: [") + e.what() + "] at File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__));
    }
}

void Mapping::unmapMRRGNode(OpGraphNode* opnode, MRRGNode* node)
{
    try
    {
        auto iter = find(mapping[opnode].begin(), mapping[opnode].end(), node);
        if(iter != mapping[opnode].end())
        {
            mapping[opnode].erase(iter);
        }
    }
    catch(const std::exception & e)
    {
        throw cgrame_error(std::string("Mapping Exception Thrown by: [") + e.what() + "] at File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__));
    }
}

void Mapping::setMapping(std::map<OpGraphNode*,std::vector<MRRGNode*>> mapping)
{
    this->mapping = mapping;
}


static bool find_sink(const std::vector<MRRGNode*> & val_map, std::map<MRRGNode*,bool> & visited, MRRGNode* src, MRRGNode* sink, int operand)
{
    // mark src visited
    visited[src] = true;

    // check each fanout
    for(auto & fo : src->fanout)
    {
        if(fo == sink && src == sink->fanin[operand])
        {
            return true;
        }

        if(std::find(val_map.begin(), val_map.end(), fo) != val_map.end()) // if the node in the mapping
            if(visited.find(fo) == visited.end()) // and we haven't visited before
            {
                if(find_sink(val_map, visited, fo, sink, operand))
                    return true;
            }
    }
    return false;
}

bool Mapping::verifyOpGraphMappingConnectivity() // Should be const member function, but the [] operator might modify the mapping
{
    bool result = true;
    // 1. check that every op has one MRRG node mapped. also check that the FU can support the OP
    for(auto & op : opgraph->op_nodes)
    {
        if(mapping[op].size() > 1)
        {
            std::cout << "Verify FAILED: Op mapped to more than FU. Op=\'" << *op << "\'" << std::endl;
            return false;
        }
        else if(mapping[op].size() == 0)
        {
            std::cout << "Verify FAILED: Op mapped to no FU. Op=\'" << *op << "\'" << std::endl;
            return false;
        }
        else
        {
            if(!mapping[op][0]->canMapOp(op))
            {
                std::cout << "Verify FAILED: Op mapped to more illegal FU. Op=\'" << *op << "\'" << std::endl;
                return false;
            }
        }
    }
    // 2. Check that there are no shorts between values
    for(auto & val1 : opgraph->val_nodes)
    {
        for(auto & val2 : opgraph->val_nodes)
        {
            if(val1 != val2)
            {
                for(auto & node1 : mapping[val1])
                {
                    for(auto & node2 : mapping[val2])
                    {
                        if(node1 == node2)
                        {
                            std::cout << "Verify FAILED: Different vals mapped to same route. Val1=\'" << *val1 << "\' Val2=\'" << *val2 << "\'" << std::endl;
                            return false;
                        }
                    }
                }
            }
        }
    }
    // 3. for every val, verify source to sink connections for each fanout
    for(auto & val : opgraph->val_nodes)
    {
        std::cout << "Verifying Value: " << *val << std::endl;
        for(int i = 0; i < val->output.size(); i++)
        {
            MRRGNode* srcfu = mapping[val->input][0];
            MRRGNode* sinkfu = mapping[val->output[i]][0];
            std::cout << "Finding sink \'" << *sinkfu << "(operand=" << val->output_operand[i] << ")" << std::endl;
            std::map<MRRGNode*,bool> visited;
            if(!find_sink(mapping[val], visited, srcfu, sinkfu, val->output_operand[i]))
            {
                std::cout << "Verify FAILED: Disconnect between " << *(val->input) << "/" << *srcfu << " -> " << *(val->output[i]) << "/" << *sinkfu << "(operand=" <<val->output_operand[i] << ")" << std::endl;
                std::cout << "Could not find sink \'" << *sinkfu << "(operand=" << val->output_operand[i] << ")" << std::endl;
                result &= false;
            }
            else
            {
                std::cout << "Found sink \'" << *sinkfu << "(operand=" << val->output_operand[i] << ")" << std::endl;
            }
        }
    }
    return result;
}

void Mapping::outputMapping(std::ostream & o) const
{
    o << "Operation Mapping Result:" << std::endl;
    for(auto & op : opgraph->op_nodes)
        o << *op << ": " << *mapping.at(op).front() << std::endl;
    o << std::endl;
    o << "Connection Mapping Result:" << std::endl;
    for(auto & val : opgraph->val_nodes)
    {
        o << *val << ":" << std::endl;
        for(auto & node : mapping.at(val))
            o << "  " << *node << std::endl;
        o << std::endl;
    }
}

void Mapping::outputDetailedMapping(std::ostream & o) const
{
    o << "Operation Mapping Result:" << std::endl;
    for(auto & op : opgraph->op_nodes)
        o << *op << ": " << *mapping.at(op).front() << std::endl;
    o << std::endl;
    o << "Connection Mapping Result:" << std::endl;
    for(auto & val : opgraph->val_nodes)
    {
        for(int fanout_id = 0; fanout_id < val->fanout_result.size(); ++fanout_id)
        {
            o << *val << "->" << *(val->output.at(fanout_id)) << std::endl;
            for(auto & node : val->fanout_result.at(fanout_id))
                o << "  " << *node << std::endl;
        }
        o << std::endl;
    }
}
