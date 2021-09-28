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

#ifndef MRRG__H_
#define MRRG__H_

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <CGRA/OpGraph.h>

typedef enum
{
    MRRG_NODE_ROUTING,
    MRRG_NODE_FUNCTION,
    MRRG_NODE_IO,
    MRRG_NODE_STORAGE
} MRRGNode_Type;

typedef enum
{
    UNSPECIFIED,
    MUX_OUT,
    MUX_IN,
} NodePortType;

std::ostream& operator<<(std::ostream& os, const NodePortType& npt);

class OpGraph;
class OpGraphOp;
class Module;

class MRRGNode
{
    public:
        MRRGNode(Module* parent, unsigned int cycle, std::string name, MRRGNode_Type type = MRRG_NODE_ROUTING, bool essential = false);


        // Node properties
        std::string name;
        MRRGNode_Type type;
        NodePortType pt;

        bool canMapOp(OpGraphOp const * op);
        std::vector<OpGraphOpCode> supported_ops;

        std::vector<MRRGNode*>  fanout;
        std::vector<MRRGNode*>  fanin;
        std::map<int, MRRGNode*> operand;
        std::vector<std::pair<MRRGNode*, int> > neighbourFUs;

        Module* parent;

        std::string getFullName();
        const std::string& getHierarchyQualifiedName() const { return name; }

        unsigned int cycle;
        bool essential; // if true, the node will never be removed by MRRG::reduce()
        unsigned int     delay; // Delay in picoseconds

        // Latency in cycles through the node min, max if latency is programmable
        unsigned int     min_latency;
        unsigned int     max_latency;
        unsigned int     latency;
        
        // the capacity of the node, how many things can be mapped to it
        int     capacity;

        // variable
        MRRGNode* prev;
/*
        // fixed mapper data
        float   base_cost;
        int     occupancy;

        float getCost(float penalty_factor = 1);


        //find which op node that the MRRG node corresponds to
        OpGraphOp* getOpNodePtr (const OpGraph& graph);
*/        

    private:

        friend std::ostream& operator<< (std::ostream& out, const MRRGNode& node);
};

class MRRG
{
    public:
        MRRG(int II)
        {
            this->II = II;
            nodes.resize(II);
        };
        ~MRRG();

        // this function sets up all the datastructures that will be used for mapping and may perform optimization on the MRRG
        void finalize();
        // Removes unnecessary nodes
        void reduce();
        // Checks MRRG properties, links etc
        bool verify();

        unsigned int II;
        void print_dot();
        void print_dot_clustered();

        std::vector<std::map<std::string, MRRGNode*>> nodes;

        std::vector<MRRGNode*> function_nodes;
        std::vector<MRRGNode*> routing_nodes;
};

#endif

