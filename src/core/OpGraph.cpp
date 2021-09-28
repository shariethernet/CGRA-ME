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
#include <sstream>
#include <cmath>
#include <algorithm>
#include <assert.h>

#include <CGRA/OpGraph.h>

#define FORALL(a, b) for(auto (a) = (b).begin(); (a) != (b).end(); (a)++)

using namespace std;

map<string, OpGraphOpCode> OpGraphOp::opcode_map =
{
    {"nop", OPGRAPH_OP_NOP},
    {"input", OPGRAPH_OP_INPUT},
    {"output", OPGRAPH_OP_OUTPUT},
    {"const", OPGRAPH_OP_CONST},
    {"trunc", OPGRAPH_OP_TRUNC},
    {"sext", OPGRAPH_OP_SEXT},
    {"zext", OPGRAPH_OP_ZEXT},
    {"phi", OPGRAPH_OP_PHI},
    {"add", OPGRAPH_OP_ADD},
    {"sub", OPGRAPH_OP_SUB},
    {"mul", OPGRAPH_OP_MUL},
    {"div", OPGRAPH_OP_DIV},
    {"and", OPGRAPH_OP_AND},
    {"or", OPGRAPH_OP_OR},
    {"xor", OPGRAPH_OP_XOR},
    {"shl", OPGRAPH_OP_SHL},
    {"shra", OPGRAPH_OP_SHRA},
    {"shrl", OPGRAPH_OP_SHRL},
    {"gep", OPGRAPH_OP_GEP},
    {"icmp", OPGRAPH_OP_ICMP},
    {"load", OPGRAPH_OP_LOAD},
    {"store", OPGRAPH_OP_STORE},
    {"shr", OPGRAPH_OP_SHR},
};

// ToDo: Make faster later.
ostream& operator <<(ostream &os, const OpGraphOpCode &opcode)
{

    for (auto && p : OpGraphOp::opcode_map)
    {
        if (p.second == opcode)
        {
            os << p.first;
            return os;
        }
    }

    os << "UNKNOWN: " << (unsigned int) opcode;
    abort();
    return os;
}

istream& operator >>(istream &is, OpGraphOpCode &opcode)
{
    std::string str;
    is >> str;
    if (OpGraphOp::opcode_map.count(str) > 0)
        opcode = OpGraphOp::opcode_map.at(str);
    else
    {
        if(str.size() > 0)
        {
            std::cout << "Unsupported command: " << str << std::endl;
            std::cout << "Aborting..." << std::endl;
            abort();
        }
        is.setstate(std::ios_base::failbit);
    }

    return is;
}

OpGraphNode::~OpGraphNode()
{
}



OpGraphOp::OpGraphOp(string name, OpGraphOpCode code)
    : OpGraphNode(name)
{
    this->opcode = code;
    this->output = NULL;
}

OpGraphOp::OpGraphOp(string name)
    : OpGraphNode(name)
{
    this->opcode = OPGRAPH_OP_NOP;
    this->output = NULL;
}

bool OpGraphOp::setOperand(int num, OpGraphVal* val)
{
    if(!val)
        return false;

    // make space if the operand number is large
    if(this->input.size() <= (unsigned int) num )
        this->input.resize(num + 1, NULL);

    this->input[num] = val;

    val->output.push_back(this);
    val->output_operand.push_back(num);

    return true;
}

void OpGraphOp::parserUpdate(std::map<std::string, std::string> kvp)
{
    if(kvp.find("input") != kvp.end())
    {
        opcode = OPGRAPH_OP_INPUT;
    }
    else if(kvp.find("output") != kvp.end())
    {
        opcode = OPGRAPH_OP_OUTPUT;
    }
    else if(kvp.find("opcode") != kvp.end())
    {
        opcode = opcode_map[kvp["opcode"]];
    }

    if(kvp.find("cycle") != kvp.end())
    {
        // TODO: let the parser catch these exceptions
        scheduled_cycle =  std::stoi(kvp["cycle"]);
    }
}


OpGraphOp::~OpGraphOp()
{
}

OpGraphVal::OpGraphVal(string name)
    : OpGraphNode(name)
{
}

OpGraphVal::OpGraphVal(std::string name, OpGraphOp* input_op)
    : OpGraphNode(name)
{
    this->input = input_op;
    input_op->output = this;
}

OpGraphVal::~OpGraphVal()
{
}

OpGraph::OpGraph()
{
}

OpGraph::~OpGraph()
{
}

std::ostream& operator<<(std::ostream& output, const OpGraphOp& op)
{
    output << op.name << "(" << op.opcode <<")";
    return output;  // for multiple << operators.
}

std::ostream& operator<<(std::ostream& output, const OpGraphVal& val)
{
    output << val.name;
    return output;  // for multiple << operators.
}


bool OpGraph::scheduleASAP(unsigned int * latency)
{

    // create a copy of the list of all op nodes
    auto V = this->op_nodes;

    unsigned int num_scheduled = 0;
    // initialize all nodes
    FORALL(v, V)
    {
        if((*v)->input.size() == 0)
        {
            (*v)->scheduled_cycle = 0;
            (*v) = NULL; // invalidate from list
            num_scheduled++;
        }
        else
        {
            (*v)->scheduled_cycle = -1;
        }
    }

    int max_latency = 0;

    while(num_scheduled < V.size())
    {
        FORALL(v, V)
        {
            int max = 0;
            bool pred_sched = (*v);
            for(unsigned int i = 0; (*v) && i < (*v)->input.size(); i++) // for all node inputs
            {
                OpGraphVal* in = (*v)->input[i];
                max = std::max(max, in->input->scheduled_cycle); // TODO: verify:  latency needs to be the path between predecessor and current node
                if(in->input->scheduled_cycle == -1)
                {
                    pred_sched = false;
                    break;
                }
            }

            if(pred_sched) // all predecessors are scheduled
            {
                (*v)->scheduled_cycle = max + 1;
                (*v) = NULL; // invalidate from list
                num_scheduled++;
                max_latency = std::max(max, max_latency);
            }
        }
    }

    if(latency)
        *latency = max_latency;

    return true;
}

bool OpGraph::computeASAP(unsigned int * latency = NULL)
{
    // create a copy of the list of all op nodes
    auto V = this->op_nodes;

    unsigned int num_scheduled = 0;
    // initialize all nodes
    FORALL(v, V)
    {
        if((*v)->input.size() == 0)
        {
            (*v)->asap_cycle = 0;
            (*v) = NULL; // invalidate from list
            num_scheduled++;
        }
        else
        {
            (*v)->asap_cycle = -1;
        }
    }

    unsigned int max_latency = 0;

    std::vector<OpGraphOp*> toRemove;
    while(num_scheduled < V.size())
    {
        FORALL(v, V)
        {
            unsigned int max = 0;
            bool pred_sched = (*v);
            for(unsigned int i = 0; (*v) && i < (*v)->input.size(); i++) // for all node inputs
            {
                OpGraphVal* in = (*v)->input[i];
                std::cout << "Path latency (" << *(in->input) << ", " << **v << "): " << in->output_latency[i] << "\n";
                max = std::max(max, in->input->asap_cycle /*+ in->input->op_latency*/ + in->output_latency[i]); // TODO: verify:  latency needs to be the path between predecessor and current node
                if(in->input->asap_cycle == -1)
                {
                    pred_sched = false;
                    break;
                }
            }

            if(pred_sched) // all predecessors are scheduled
            {
                (*v)->asap_cycle = max; // TODO: should I factor in current op latency here?
                (*v) = NULL; // invalidate from list
                num_scheduled++;
                max_latency = std::max(max, max_latency);
            }
        }
    }

    if(latency)
        *latency = max_latency;

    return true;
}

bool OpGraph::computeALAP(unsigned int max_cycles)
{
    // create a copy of the list of all op nodes
    auto V = this->op_nodes;

    unsigned int num_scheduled = 0;

    // initialize all nodes
    FORALL(v, V)
    {
        if((*v)->output == NULL) // TODO: this is probably wrong - this needs to be finding all the output nodes
        {
            (*v)->alap_cycle = max_cycles;
            (*v) = NULL; // invalidate from list
            num_scheduled++;
        }
        else
        {
            (*v)->alap_cycle = -1;
        }
    }

    while(num_scheduled < V.size())
    {
        FORALL(v, V)
        {
            unsigned int min = 0;
            bool succ_sched = (*v);
            for(unsigned int i = 0; (*v) && i < (*v)->output->output.size(); i++) // for all op outputs
            {
                OpGraphVal* out = (*v)->output;
                OpGraphOp* succ = out->output[i];

                std::cout << "Path latency (" << **v << ", " << *succ << "): " << out->output_latency[i] << "\n";
                min = std::min(min, succ->alap_cycle /*- (*v)->op_latency*/ + out->output_latency[i]);

                if(succ->alap_cycle == -1)
                {
                    succ_sched = false;
                    break;
                }
            }

            if(succ_sched) // all sucessors are scheduled
            {
                (*v)->alap_cycle = min; // TODO: should I factor in current op latency here?
                (*v) = NULL; // invalidate from list
                num_scheduled++;
            }
        }
    }
    return true;
}

void OpGraph::debug_check()
{
#ifdef DEBUG
    //Check for null pointer
    for(auto it = this->op_nodes.begin(); it != this->op_nodes.end(); it++)
    {
        assert(*it);
    }
    for(auto it = this->val_nodes.begin(); it != this->val_nodes.end(); it++)
    {
        assert(*it);
    }

    // Check to make sure that all double link is set-up correctly from the input side
    for(auto temp_val : opgraph->val_nodes)
    {
        auto output = temp_val->output;
        for(auto temp_op : output)
        {
            auto temp_it = std::find(temp_op->input.begin(), temp_op->input.end(), temp_val);
            assert(temp_it != temp_op->input.end());
        }
    }
    // Check to make sure that all double link is set-up correctly from the output side
    for(auto temp_op : opgraph->op_nodes)
    {
        auto input = temp_op->input;
        for(auto temp_val : input)
        {
            auto temp_it = std::find(temp_val->output.begin(), temp_val->output.end(), temp_op);
            assert(temp_it != temp_val->output.end());
        }
    }
#endif
}

// This function uses a DFS to find the largest cycle in the graph starting with the input nodes
// It assumes the graph is not disjoint.
// If the graph is acyclic, it returns 0.
static void dfs_visit(int time , int & longest_cycle, std::map<OpGraphOp*, int> & dfs_colour, std::map<OpGraphOp*, int> & dfs_timestamp, OpGraphOp* op)
{
    // colour grey
    dfs_colour[op] = 1;

    dfs_timestamp[op] = time;
    time++;

    //if leaf node, we are done
    if(op->opcode == OPGRAPH_OP_OUTPUT || op->opcode == OPGRAPH_OP_STORE)
    {
        dfs_colour[op] = 2;
        return;
    }

    for(auto & n : op->output->output)
    {
        if(dfs_colour[n] == 0)
        {
            dfs_visit(time, longest_cycle, dfs_colour, dfs_timestamp, n);
        }
        else if(dfs_colour[n] == 1)
        {
            int size = dfs_timestamp[op] - dfs_timestamp[n] + 1;
            //cout << "found cycle at " << *op << " to " << *n << " size: " << size << "\n";
            if(size > longest_cycle)
                longest_cycle = size;
        }
    }
    dfs_colour[op] = 2;
    return;
}

int OpGraph::getMaxCycle()
{
    int result = 0;

    std::map<OpGraphOp*, int> dfs_colour;
    std::map<OpGraphOp*, int> dfs_timestamp;

    // Following Corman et al. page540, 2nd ed.
    // 0 - white
    // 1 - grey
    // 2 - black
    for(auto & n : op_nodes)
        dfs_colour[n] = 0;

    // Loop through all op nodes
    for(auto & op : op_nodes)
    {
        if(dfs_colour[op] == 0)
        {
            //cout << "Visiting input: " << *input_op << "\n";
            dfs_visit(0, result, dfs_colour, dfs_timestamp, op);
        }
    }
    return result;
}


void OpGraph::printDOTwithOps(std::ostream &s)
{
    unsigned int counter = 0;
    s << "digraph G {\n";

    // print op_nodes
    //  op_node map
    std::map<OpGraphOp*, std::string> opnode_map;
    counter = 0;
    for(auto it = this->op_nodes.begin(); it != this->op_nodes.end(); it++)
    {
        assert(*it);
        stringstream node_name;
        node_name << (*it)->opcode;

        opnode_map[(*it)] = node_name.str() + std::to_string(counter++);
        s << opnode_map[(*it)] << "[opcode=" << (*it)->opcode << "];\n";
    }

    // use val nodes to create all edges
    for(auto it = this->val_nodes.begin(); it != this->val_nodes.end(); it++)
    {
        std::string inputnode = opnode_map[(*it)->input];

        assert((*it)->output.size() == (*it)->output_operand.size());
        for(unsigned int o = 0; o < (*it)->output.size(); o++)
        {
            OpGraphOp* op = (*it)->output[o];
            unsigned int operand = (*it)->output_operand[o];
            s << inputnode << "->" << opnode_map[op] << "[operand=" << operand << "]; ";
            s << "//" << (*it)->input->name << "->" <<  op->name << "\n";
        }
    }
    s << "}\n";
}

void OpGraph::print_dot(std::ostream &s)
{
    unsigned int counter = 0;
    s << "digraph G {\n";

    // print op_nodes
    //  op_node map
    std::map<OpGraphOp*, std::string> opnode_map;
    counter = 0;
    for(auto it = this->op_nodes.begin(); it != this->op_nodes.end(); it++)
    {
        assert(*it);
        opnode_map[(*it)] = "node" + std::to_string(counter++);
        s << opnode_map[(*it)] << "[opcode=" << (*it)->opcode << "];\n";
    }

    // use val nodes to create all edges
    for(auto it = this->val_nodes.begin(); it != this->val_nodes.end(); it++)
    {
        std::string inputnode = opnode_map[(*it)->input];

        assert((*it)->output.size() == (*it)->output_operand.size());
        for(unsigned int o = 0; o < (*it)->output.size(); o++)
        {
            OpGraphOp* op = (*it)->output[o];
            unsigned int operand = (*it)->output_operand[o];
            s << inputnode << "->" << opnode_map[op] << "[operand=" << operand << "]; ";
            s << "//" << (*it)->input->name << "->" <<  op->name << "\n";
        }
    }
    s << "}\n";
}

