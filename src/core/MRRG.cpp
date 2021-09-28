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

#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>

#include <assert.h>

#include <CGRA/MRRG.h>

#define FORALL(a, b) for(auto (a) = (b).begin(); (a) != (b).end(); (a)++)

// TODO: actuall write this function....
MRRG::~MRRG()
{
}

void MRRG::print_dot()
{

    std::cout << "digraph {\n";
    for(unsigned int i = 0; i < nodes.size(); i++)
    {
        for(auto n = nodes[i].begin(); n != nodes[i].end(); n++)
        {
            for(auto fanout = n->second->fanout.begin(); fanout != n->second->fanout.end(); fanout++)
            {
                std::cout << "\"" << *(n->second) << "\"->\"" << **fanout << "\";\n";
            }
        }
    }
    std::cout << "}\n";
}

static std::string find_cluster_name(std::string s1, std::string s2)
{
    unsigned int last_dot = 0;
    for(unsigned int i = 0; i < std::min(s1.size(), s2.size()); i++)
    {
        if(s1[i] == s2[i])
        {
            if(s1[i] == '.' || s1[i] == ':')
                last_dot = i;
        }
        else
            break;
    }

    std::cout << "last dot of (" << s1 << "," << s2 << ") is " << last_dot << "\n";
    return s1.substr(0, last_dot);
}

static bool isDirectSubCluster(std::string c, std::string sub)
{
    if(c == sub || c.size() >= sub.size())
        return false;

    bool found_dot = false;
    unsigned int i;
    for(i = 0; i < c.size(); i++)
    {
        if(c[i] == sub[i])
        {
            if(sub[i] == '.')
                found_dot = true;
        }
        else
        {
            if(found_dot)
                break;
            else
                return false;
        }
    }
    for(; i < c.size(); i++)
    {
        if(sub[i] == '.')
            return false;
    }
    return true;
}

// returns a vector of all fu's that can be routed to from the src_fu
static std::vector<std::pair<MRRGNode*, int> > findNeighbourFUs(MRRGNode* src_fu)
{
    std::vector<std::pair<MRRGNode*, int> > result;

    // do BFS to find all nodes

    std::map<MRRGNode*, bool> visited;

    std::queue<std::pair<MRRGNode*, int> > to_visit;

    to_visit.push({src_fu, 0});

    while(!to_visit.empty())
    {
        auto n = to_visit.front();
        to_visit.pop();

        int dist = n.second;

        for(auto const &next : n.first->fanout)
        {
            if(visited.find(next) == visited.end())
            {
                if(next->type == MRRG_NODE_FUNCTION)
                {
                    result.push_back({next,dist});
                }
                else
                {
                    to_visit.push({next, dist + 1});
                }
                visited[next] = true;
            }
        }
    }
    return result;
}

void MRRG::finalize()
{
    // populate vectors
    FORALL(it1, nodes)
    {
        FORALL(it2, (*it1))
        {
            if(it2->second->type == MRRG_NODE_FUNCTION)
                function_nodes.push_back(it2->second);
            else if(it2->second->type == MRRG_NODE_ROUTING)
                routing_nodes.push_back(it2->second);
            else
                assert(0);
        }
    }

    // For all Function unit nodes, find neighbours nodes
    for(auto &f : function_nodes)
    {
        f->neighbourFUs = findNeighbourFUs(f);
    }
}

static void print_subcluster(std::map<std::string, std::string> & clusters, std::string current_cluster)
{
    std::string s = current_cluster;
    if (current_cluster == "")
        return;

    std::string contents = clusters[current_cluster];

    std::cout << "subgraph \"cluster_" << s << "\"{\n";
    for(auto c = clusters.begin(); c != clusters.end(); ++c)
    {
        if(isDirectSubCluster(s, c->first))
        {
            print_subcluster(clusters, c->first);
        }
    }
    clusters[current_cluster] = "";
    std::cout << contents << "\nlabel = \"" << s << "\";\n}\n";
}

void MRRG::print_dot_clustered()
{
    std::map<std::string, std::string> clusters;

    clusters[""] = "";

    for(unsigned int i = 0; i < this->II; i++)
    {
        clusters[std::to_string(i)] = "";
    }

    for(unsigned int i = 0; i < nodes.size(); i++) // nodes.begin(); it != nodes.end(); ++it)
    {
        for(auto n = nodes[i].begin(); n != nodes[i].end(); n++)
        {
            for(auto fanout = n->second->fanout.begin(); fanout != n->second->fanout.end(); fanout++)
            {
                std::string srcname = n->second->getFullName();
                std::string dstname = (*fanout)->getFullName();

                std::string subgraph = find_cluster_name(srcname, dstname);

                clusters[subgraph] += "\"" + srcname + "\"->\"" + dstname +  "\";\n";
            }
        }
    }

    std::cout << "digraph {\n";
    for(unsigned int i = 0; i < this->II; i++)
    {
        print_subcluster(clusters, std::to_string(i));
    }

    std::cout << "}\n";
}

MRRGNode::MRRGNode(Module* parent, unsigned int cycle, std::string name, MRRGNode_Type type, bool essential)
{
    this->cycle = cycle;
    this->name = name;
    this->type = type;

/*
    if(type == MRRG_NODE_FUNCTION)
        this->base_cost = 2.0;
    else
        this->base_cost = 1.0;
*/
    this->capacity = 1;
    //this->occupancy = 0;
    this->parent = parent;
    this->pt = UNSPECIFIED;
    this->prev = NULL;
    this->min_latency = 0;
    this->max_latency = 0;
    this->latency = 0;
    this->delay = 0;

    if(type == MRRG_NODE_FUNCTION)
        this->essential = true;
};

std::string MRRGNode::getFullName()
{
    return std::to_string(cycle) + ":" + name;
}

/*
float MRRGNode::getCost(float penalty_factor)
{
    return base_cost * occupancy + (occupancy <= capacity ? 0.0 : (occupancy - capacity) * penalty_factor);
}

*/
bool MRRGNode::canMapOp(OpGraphOp const * op)
{
    FORALL(f, supported_ops)
    {
        if(op->opcode == *f)
            return true;
    }
    return false;
}


std::ostream& operator<<(std::ostream& os, const NodePortType& npt) {
    switch (npt) {
        case UNSPECIFIED:      return os << "UNSPECIFIED";
        case MUX_OUT:          return os << "MUX_OUT";
        case MUX_IN:           return os << "MUX_IN";

        default: return os << "FIXME: UNHANDLED NodePortType PRINT";
    }
}

std::ostream& operator<< (std::ostream& out, const MRRGNode& node)
{
    return out << node.cycle << ":" << node.name;
};



static inline void link(MRRGNode* a, MRRGNode* b)
{
    a->fanout.push_back(b);
    b->fanin.push_back(a);
}

static inline void unlink(MRRGNode* a, MRRGNode* b)
{
    auto elem = std::find(a->fanout.begin(), a->fanout.end(), b);
    assert(elem != a->fanout.end());
    a->fanout.erase(elem);


    auto elem1 = std::find(b->fanin.begin(), b->fanin.end(), a);
    assert(elem1 != b->fanin.end());
    b->fanin.erase(elem1);
}

// This function removes redundant MRRG nodes in the graph
void MRRG::reduce()
{
    std::vector<MRRGNode*> to_remove;
    // remove routing nodes that have one fan in
    for(auto & r : routing_nodes)
    {
        if(r->essential)
        {
            continue;
        }

        if(r->fanin.size() == 1)
        {
            MRRGNode* p = r->fanin[0];
            // remove link to current node
            unlink(p, r);

            // link p to each of r's fanouts
            // unlink r to each fanout
            auto fanout = r->fanout; // must copy so that we can modify r->fanout
            for(auto & fo : fanout)
            {
                // if(fo == p) // if there is a back edge to the predecessor abort
                // abort();

                link(p, fo);
                unlink(r, fo);
            }

            to_remove.push_back(r);

        }
    }

    std::cout << "MRRG Reduce removing: " << to_remove.size() << " nodes.\n";

    for(auto & r : to_remove)
    {
        auto elem = std::find(routing_nodes.begin(), routing_nodes.end(), r);
        assert(elem != routing_nodes.end());
        routing_nodes.erase(elem);

        std::cout << "Removing: " << *r << "\n";
        delete r;
        assert(verify());
    }

}

    template <typename T>
static bool check_unique(std::vector<T> v)
{
    if(v.size() < 2)
        return true;

    for(int i = 0; i < v.size() - 1; i++)
    {
        for(int j = i + 1; j < v.size(); j++)
        {
            if(v[i] == v[j])
                return false;
        }
    }

    return true;
}

bool MRRG::verify()
{

    bool result = true;
    // Check every routing node in the MRRG
    for(auto & r : routing_nodes)
    {
        // check that fanins and fanouts are unique (i.e. no duplicate pointers)
        if(!check_unique(r->fanin))
        {
            std::cout << "Fanins not unique for node: " << *r << "\n";
            result &= false;
        }
        if(!check_unique(r->fanout))
        {
            std::cout << "Fanouts not unique for node: " << *r << "\n";
            result &= false;
        }
        // check that for every fanout, there is a fanin
        for(auto & fo : r->fanout)
        {

            bool found_r = false;
            for(auto & fi : fo->fanin)
            {
                if(fi == r)
                {
                    found_r = true;
                    break;
                }
            }

            if(!found_r)
            {
                std::cout << "Missing back link: " << *r << " <- " << *fo << "\n";
                result &= false;
            }


        }
    }

    for(auto & r : function_nodes)
    {
        // check that fanins and fanouts are unique (i.e. no duplicate pointers)
        if(!check_unique(r->fanin))
        {
            std::cout << "Fanins not unique for node: " << *r << "\n";
            result &= false;
        }

        if(!check_unique(r->fanout))
        {
            std::cout << "Fanouts not unique for node: " << *r << "\n";
            result &= false;
        }

        // check that for every fanout, there is a fanin
        for(auto & fo : r->fanout)
        {

            bool found_r = false;
            for(auto & fi : fo->fanin)
            {
                if(fi == r)
                {
                    found_r = true;
                    break;
                }
            }

            if(!found_r) {
                std::cout << "Missing back link: " << *r << " <- " << *fo << "\n";
                result &= false;
            }
        }
    }
    return result;
}

