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

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"

#include <string>
#include <sstream>
#include <list>
#include <map>
#include <algorithm>
#include <cctype>
#include <fstream>

#include <CGRA/OpGraph.h>

#define DFG_LOWER_GEP_INSTR

using namespace llvm;

namespace
{
    cl::opt<bool> leaveUnnecessaryLeafNodes("no-leaf-removal", cl::desc("Don't remove the unnecessary leaf nodes from the generated DFG"));

    cl::opt<bool> leavePhiNodes("no-phi-removal", cl::desc("Don't remove the phi nodes from the generated DFG"));

    cl::opt<std::string> inputTagPairs("in-tag-pairs", cl::Positional, cl::desc("<input file that contains tag number and string pairs>"));

    cl::opt<std::string> loopTags("loop-tags", cl::desc("Input a list of loop tag names to generate DFG for"));

    OpGraphOpCode LLVMtoOp(Instruction * I)
    {
        switch(I->getOpcode())
        {
            case Instruction::PHI       :   return OPGRAPH_OP_PHI;
            case Instruction::Select    :   return OPGRAPH_OP_PHI;
            case Instruction::SExt      :   return OPGRAPH_OP_SEXT;
            case Instruction::ZExt      :   return OPGRAPH_OP_ZEXT;
            case Instruction::Trunc     :   return OPGRAPH_OP_TRUNC;
            case Instruction::Add       :   return OPGRAPH_OP_ADD;
            case Instruction::Sub       :   return OPGRAPH_OP_SUB;
            case Instruction::Shl       :   return OPGRAPH_OP_SHL;
            case Instruction::AShr      :   return OPGRAPH_OP_SHRA;
            case Instruction::LShr      :   return OPGRAPH_OP_SHRL;
            case Instruction::And       :   return OPGRAPH_OP_AND;
            case Instruction::Or        :   return OPGRAPH_OP_OR;
            case Instruction::Xor       :   return OPGRAPH_OP_XOR;
            case Instruction::SRem      :   return OPGRAPH_OP_DIV;
            case Instruction::URem      :   return OPGRAPH_OP_DIV;
            case Instruction::SDiv      :   return OPGRAPH_OP_DIV;
            case Instruction::UDiv      :   return OPGRAPH_OP_DIV;
            case Instruction::Mul       :   return OPGRAPH_OP_MUL;
            case Instruction::Load      :   return OPGRAPH_OP_LOAD;
            case Instruction::Store     :   return OPGRAPH_OP_STORE;
            case Instruction::GetElementPtr : return OPGRAPH_OP_GEP;
            case Instruction::ICmp      : return OPGRAPH_OP_ICMP;

            default: errs() << "could not look up:" << *I << "\n"; abort(); return OPGRAPH_OP_NOP;
        }
    }

    //Remove all unnecessay leaf nodes from the DFG graph
    void removeUnnecessaryLeafNodes(OpGraph * opgraph)
    {
        errs() << "Info: Removing Unnecessary Leaf Nodes!" << "\n";
        bool node_removed;
        do
        {
            node_removed = false;
            for(auto opnode_it = opgraph->op_nodes.begin(); opnode_it != opgraph->op_nodes.end(); )
            {
                // If the end node instruction is not store and it is not an output node
                if(((*opnode_it)->opcode != OPGRAPH_OP_STORE) && ((*opnode_it)->opcode != OPGRAPH_OP_OUTPUT))
                {
                    if((*opnode_it)->output->output.empty())
                    {
                        // Remove the link from parent OpGraphVal
                        auto temp_lambda = [opnode_it](OpGraphVal * item) -> void
                        {
                            auto child_op_it = std::find(item->output.begin(), item->output.end(), *opnode_it);
                            if(child_op_it != item->output.end())
                            {
                                auto index = child_op_it - item->output.begin();
                                item->output_operand.erase(item->output_operand.begin() + index); // Remove operand number
                                item->output.erase(child_op_it);
                            }
                        };
                        std::for_each((*opnode_it)->input.begin(), (*opnode_it)->input.end(), temp_lambda); // Apply removal to all parents

                        // Remove the corresponding OpGraphVal node
                        auto val_find_it = std::find(opgraph->val_nodes.begin(), opgraph->val_nodes.end(), (*opnode_it)->output);
                        if(val_find_it != opgraph->val_nodes.end())
                            opgraph->val_nodes.erase(val_find_it);
                        else
                            errs() << "Error: Could not find the corresponding value node when trying to remove unnecessary nodes!\n";

                        // Remove the opnode from the inputs list too if it is a input node
                        if((*opnode_it)->opcode == OPGRAPH_OP_INPUT)
                        {
                            auto temp_it = std::find(opgraph->inputs.begin(), opgraph->inputs.end(), *opnode_it);
                            if(temp_it != opgraph->inputs.end())
                                opgraph->inputs.erase(temp_it);
                            else
                                errs() << "Error: Could not find the the op node in the input list!\n";
                        }

                        opnode_it = opgraph->op_nodes.erase(opnode_it);
                        node_removed = true; // Removed this node
                    }
                    else
                        ++opnode_it;
                }
                else
                    ++opnode_it;
            }
        }
        while(node_removed); // Continue runs untill no change can be made
    }

    // Remove Phi Nodes from the generated DFG
    // TODO: Cover the cases where no input node is constant node(The case of normal if statements in loop)
    void removePhiNodes(OpGraph * opgraph)
    {
        errs() << "Info: Removing Phi Nodes!" << "\n";
        bool break_flag;
        do
        {
            break_flag = false;
            for(auto opnode_it = opgraph->op_nodes.begin(); opnode_it != opgraph->op_nodes.end(); ++opnode_it)
            {
                if((*opnode_it)->opcode == OPGRAPH_OP_PHI)
                {
                    // Find the parent const or input node and remove it
                    unsigned int special_nodes_counter = 0;
                    OpGraphVal * non_special_val;
                    auto is_special_node_lambda = [&special_nodes_counter, &non_special_val](OpGraphVal * item) -> void
                    {
                        // Mark const nodes to erase
                        if(item->input->opcode == OPGRAPH_OP_CONST || item->input->opcode == OPGRAPH_OP_INPUT)
                            ++special_nodes_counter;
                        else
                            non_special_val = item;
                    };
                    std::for_each((*opnode_it)->input.begin(), (*opnode_it)->input.end(), is_special_node_lambda);
                    // Only try to remove the phi node if there is only one non-const/input node left
                    if((*opnode_it)->input.size() - special_nodes_counter == 1)
                    {
                        // Find the link from the non-const/input parent to phi node and remove it
                        auto parent_to_phi_it = std::find(non_special_val->output.begin(), non_special_val->output.end(), *opnode_it);
                        if(parent_to_phi_it != non_special_val->output.end())
                        {
                            non_special_val->output.erase(parent_to_phi_it); // Remove the old link from phi's non-const/input parent to phi
                            non_special_val->output_operand.erase(non_special_val->output_operand.begin() + (parent_to_phi_it - non_special_val->output.begin()));
                        }
                        else
                            errs() << "Error: Could not find the non constant/input parent to the phi to be removed!" << "\n";

                        // Set up the links from the non-const/input parent to phi's children
                        for(auto phi_output_it = (*opnode_it)->output->output.begin(); phi_output_it != (*opnode_it)->output->output.end(); ++phi_output_it)
                        {
                            auto phi_child_it = std::find((*phi_output_it)->input.begin(), (*phi_output_it)->input.end(), (*opnode_it)->output);
                            if(phi_child_it != (*phi_output_it)->input.end())
                            {
                                (*phi_output_it)->input.erase(phi_child_it); // Remove the old link from phi's child to phi
                                (*phi_output_it)->input.push_back(non_special_val); // Set-up new link from phi's child to phi's non-const/input parent
                                non_special_val->output.push_back(*phi_output_it); // Set-up new link from phi's non-const/input parent to phi's child
                                non_special_val->output_operand.push_back((*opnode_it)->output->output_operand.at(phi_output_it - (*opnode_it)->output->output.begin())); // Set-up new oprand number from phi's non-const parent to phi's child
                            }
                            else
                                errs() << "Error: Could not find link to phi node from phi's child!" << "\n";
                        }

                        OpGraphVal * saved_val = (*opnode_it)->output;
                        OpGraphOp * saved_op = *opnode_it;

                        // Remove phi's all const/input parent (This operations will disable iterator)
                        auto find_op_lambda = [opgraph, opnode_it](OpGraphOp * item) -> bool
                        {
                            if((item->output->output.size() == 1) && (item->output->output.front() == *opnode_it)) // Assume the only child is phi node
                            {
                                if(item->opcode == OPGRAPH_OP_INPUT) // If phi's parent is a input node, remove it from input list
                                {
                                    auto input_node_it = std::find(opgraph->inputs.begin(), opgraph->inputs.end(), item);
                                    if(input_node_it != opgraph->inputs.end())
                                        opgraph->inputs.erase(input_node_it);
                                    else
                                        errs() << "Error: Could not find the input node to phi node in inputs!" << "\n";
                                }
                                return true;
                            }
                            else
                                return false;
                        };
                        opgraph->op_nodes.erase(std::remove_if(opgraph->op_nodes.begin(), opgraph->op_nodes.end(), find_op_lambda));

                        auto find_val_lambda = [opnode_it](OpGraphVal * item) -> bool
                        {
                            if((item->output.size() == 1) && (item->output.front() == *opnode_it)) // Assume the only child is phi node
                                return true;
                            else
                                return false;
                        };
                        opgraph->val_nodes.erase(std::remove_if(opgraph->val_nodes.begin(), opgraph->val_nodes.end(), find_val_lambda));

                        // Remove the corresponding OpGraphVal phi node
                        auto val_find_it = std::find(opgraph->val_nodes.begin(), opgraph->val_nodes.end(), saved_val);
                        if(val_find_it != opgraph->val_nodes.end())
                            opgraph->val_nodes.erase(val_find_it);
                        else
                            errs() << "Error: Could not find the corresponding value node for the phi node!" << "\n";
                        auto op_find_it = std::find(opgraph->op_nodes.begin(), opgraph->op_nodes.end(), saved_op);
                        if(op_find_it != opgraph->op_nodes.end())
                            opgraph->op_nodes.erase(op_find_it); // Remove this phi op node
                        else
                            errs() << "Error: Could not fint the phi node anymore after some deletion!" << "\n";

                        break_flag = true;
                        break; // Has to break because the removal of multiple nodes might break the iterator
                    }
                }
            }
        }
        while(break_flag);
    }

    // Hello - The first implementation, without getAnalysisUsage.
    struct DFGOut : public LoopPass
    {
        std::vector<OpGraph*> graphs;
        std::map<unsigned int, std::string> tag_pairs;
        std::vector<std::string> loop_tags;
        static char ID; // Pass identification, replacement for typeid
        DFGOut() : LoopPass(ID)
        {
            if(!inputTagPairs.empty())
            {
                std::ifstream in(inputTagPairs);
                for(std::string line; std::getline(in, line); )
                {
                    std::stringstream temp_sstream(line);
                    int temp_tag_num;
                    temp_sstream >> temp_tag_num;
                    std::string temp_tag_string;
                    temp_sstream >> temp_tag_string;
                    tag_pairs.emplace(temp_tag_num, std::move(temp_tag_string));
                }
                std::stringstream temp_sstream(loopTags);
                std::string temp_tag_string;
                while(temp_sstream >> temp_tag_string)
                    loop_tags.push_back(temp_tag_string);
            }
            else
                errs() << "Warning: No tag pair is provided as input, no DFG will be generated." << "\n";
            for(auto temp : tag_pairs)
                errs() << "Info: Tag detected: " << temp.first << " " << temp.second << "\n";
        }

        virtual bool runOnLoop(Loop *L, LPPassManager &LPM)
        {
            // Loop tag is in ths first basic block of the loop
            int found_tag_num = 0;
            for(auto it = L->getHeader()->begin(); it != L->getHeader()->end(); ++it)
            {
                if(isa<CallInst>(it))
                {
                    Function * func = dyn_cast<CallInst>(it)->getCalledFunction();
                    if((func != NULL) && (func->getName() == "DFGLOOP_TAG")) // The case where an indirect call happens
                        found_tag_num = cast<ConstantInt>(dyn_cast<CallInst>(it)->getArgOperand(0))->getValue().getZExtValue(); // found_tag_num has the the tag number
                }
            }
            if(!found_tag_num) // If there is no tag associated with this loop
                return false;

            std::string tag_name;

            auto tag_pairs_it = tag_pairs.find(found_tag_num);
            if(tag_pairs_it == tag_pairs.end())
            {
                errs() << "Error: Tag could not be found from the generated script, ignoring this loop." << "\n";
                return false;
            }
            else
            {
                auto tag_string_it = std::find(loop_tags.begin(), loop_tags.end(), tag_pairs_it->second);
                if(tag_string_it == loop_tags.end())
                {
                    return false; // Do not process this loop
                }
                // Process the loop otherwise
                tag_name = *tag_string_it;
                errs() << "Info: The loop with tag: " << tag_name << " is generating DFG." << "\n";
            }

            if(!L->empty()) // if empty there are no sub loops
                return false;

            unsigned int bb_count = L->getBlocks().size();
            if(bb_count > 1) // More then one basic block within a loop
            {
                errs() << "Error: Loop with tag: " << tag_name << " is not supported. This loop is ignored." << "\n";
                return false;
            }

            // Print the whole function
            //L->dump();

            // create new opgraph for loop
            OpGraph* opgraph = new OpGraph();

            // create map for the val output of each instruction
            std::map<Instruction*, OpGraphVal*> vals;

            // Loop through basic blocks and all instructions to
            // populate the initial vals map with instructions
            // Also, find inputs, and outputs of the inner loop and add appropriate ops
            for(Loop::block_iterator bb = L->block_begin(), e = L->block_end(); bb != e; ++bb)
            {
                // First create output vals for each output instuction and create map from instruction to the correct val node and 'fix' type conversions.
                for(BasicBlock::iterator i = (*bb)->begin(), e = (*bb)->end(); i != e; ++i)
                {
                    auto I = i;
                    if(I->getOpcode() == Instruction::Br || I->getOpcode() == Instruction::Call)
                        continue;
                    if(isa<CastInst>(I))
                    {
                        // Should update Instruction to Val map to implement this lin
                        Instruction * pre = dyn_cast<Instruction>(I->getOperand(0));
                        assert(pre);
                        vals[&*I] = vals[pre];
                        continue;
                    }

                    // create output val node
                    vals[&*I] = new OpGraphVal("");
                    opgraph->val_nodes.push_back(vals[&*I]);
                }
            }


            // Loop through basic blocks and all instructions again
            for(Loop::block_iterator bb = L->block_begin(), e = L->block_end(); bb != e; ++bb)
            {
                for(BasicBlock::iterator i = (*bb)->begin(), e = (*bb)->end(); i != e; ++i)
                {
                    auto I = i;
                    if(I->getOpcode() == Instruction::Br || I->getOpcode() == Instruction::Call)
                        continue;

                    auto itv = vals.find(&*I);
                    if(itv == vals.end())
                    {
                        // doesnt exist??
                        assert(1);
                    }
                }

                for(BasicBlock::iterator i = (*bb)->begin(), e = (*bb)->end(); i != e; ++i)
                {
                    auto I = i;
                    if(I->getOpcode() == Instruction::Br || I->getOpcode() == Instruction::Call)
                        continue;
                    // This block of code attempts to lower the GEP instruction to the DFG
#ifdef DFG_LOWER_GEP_INSTR
                    if(I->getOpcode() == Instruction::GetElementPtr)
                    {

                        OpGraphOp* output_op = NULL;
                        // if constant base ptr, then there should only be a constant op
                        if(I->getNumOperands() == 1)
                        {
                            // should get constant value here to translate it...
                            output_op = new OpGraphOp("const");
                            opgraph->op_nodes.push_back(output_op);
                        }
                        // else, start building add, mul, constant tree.
                        // convert into adds/muls/constants
                        else
                        {
                            gep_type_iterator GTI = gep_type_begin(&*I);
                            for(User::op_iterator i = I->op_begin() + 1, e = I->op_end(); i != e; ++i, ++GTI)
                            {
                                OpGraphOp*  offset_op = NULL;
                                /*
                                // Build a mask for high order bits.
                                const TargetData* TD = alloc->getTargetData();
                                unsigned IntPtrWidth = TD->getPointerSizeInBits();
                                uint64_t PtrSizeMask = ~0ULL >> (64-IntPtrWidth);

                                // apply mask
                                uint64_t Size = TD->getTypeAllocSize(GTI.getIndexedType()) & PtrSizeMask;

                                RTLWidth w("`MEMORY_CONTROLLER_ADDR_SIZE-1");
                                 */

                                // RTLOp *offset = rtl->addOp(RTLOp::Mul);
                                // offset->setOperand(0, rtl->addConst(utostr(Size), w));

                                if(ConstantInt *OpC = dyn_cast<ConstantInt>(*i))
                                {
                                    if(OpC->isZero())
                                        continue;

                                    /*
                                    // Handle a struct index, which adds its field offset.
                                    if(StructType *STy = dyn_cast<StructType>(*GTI))
                                    {
                                    return rtl->addConst(utostr(TD->getStructLayout(STy)->getElementOffset(OpC->getZExtValue())), w);
                                    }
                                    offset->setOperand(1, getConstantSignal(OpC));
                                     */

                                    // Else both operands are consts, so omit the output multiply and just generate a new constant
                                    //Operand0 = new OpGraphOp("const");
                                    //Operand1 = new OpGraphOp("const");
                                    offset_op = new OpGraphOp("gep_const", OPGRAPH_OP_CONST);
                                    opgraph->op_nodes.push_back(offset_op);
                                }
                                else
                                {
                                    Instruction *I = dyn_cast<Instruction>(*i);
                                    assert(I);
                                    offset_op = new OpGraphOp("gep_mul", OPGRAPH_OP_MUL);
                                    opgraph->op_nodes.push_back(offset_op);

                                    OpGraphOp* constant = new OpGraphOp("gep_const", OPGRAPH_OP_CONST);
                                    opgraph->op_nodes.push_back(constant);

                                    OpGraphVal* cval = new OpGraphVal("", constant);
                                    opgraph->val_nodes.push_back(cval);

                                    offset_op->setOperand(0, cval);
                                    offset_op->setOperand(1, vals[I]);
                                }

                                if(!output_op)
                                {
                                    output_op = offset_op;
                                }
                                else
                                {
                                    OpGraphOp * new_output_op = new OpGraphOp("gep_add", OPGRAPH_OP_ADD);
                                    opgraph->op_nodes.push_back(new_output_op);

                                    OpGraphVal* output_op_val = new OpGraphVal("gep_output_op_val", output_op);
                                    OpGraphVal* offset_op_val = new OpGraphVal("gep_offset_op_val", offset_op);

                                    opgraph->val_nodes.push_back(output_op_val);
                                    opgraph->val_nodes.push_back(offset_op_val);

                                    new_output_op->setOperand(0, output_op_val);
                                    new_output_op->setOperand(1, offset_op_val);
                                    output_op = new_output_op;
                                }
                            }
                        }
                        // set output val
                        vals[&*I] = new OpGraphVal("final_gep_output", output_op);
                        opgraph->val_nodes.push_back(vals[&*I]);

                        continue;
                    }
#endif
                    // Ignore cast instructions
                    if(isa<CastInst>(I))
                    {
                        continue;
                    }
                    // go through each instruction and build graph by
                    // creating an Op for the instruction
                    OpGraphOp* op = new OpGraphOp(I->getOpcodeName(), LLVMtoOp(&*I));
                    opgraph->op_nodes.push_back(op);

                    // create output val and add to vector (must create since LLVM follows SSA)
                    auto itv = vals.find(&*I);
                    if(itv == vals.end()) // see if we already created a val
                    {
                        assert(1);
                        vals[&*I] = new OpGraphVal("");
                        opgraph->val_nodes.push_back(vals[&*I]);
                    }

                    vals[&*I]->input = op;
                    op->output = vals[&*I];

                    unsigned int num_operands = I->getNumOperands();

                    // create input vals if they don't already exist (TODO: if they dont exist, they should be inputs?)
                    for(unsigned int r = 0; r < num_operands; r++)
                    {
                        Instruction * pre = dyn_cast<Instruction>(I->getOperand(r));
                        if(!pre)
                        {
                            // This converts all constants
                            if(Constant* C = dyn_cast<Constant>(I->getOperand(r)))
                            {
                                OpGraphOp* const_op = new OpGraphOp("const", OPGRAPH_OP_CONST);
                                opgraph->op_nodes.push_back(const_op);

                                OpGraphVal * v = new OpGraphVal("");
                                opgraph->val_nodes.push_back(v);

                                const_op->output  = v;
                                v->input    = const_op;

                                op->input.push_back(v);
                                v->output.push_back(op);
                                v->output_operand.push_back(r);
                            }
                            // When the parent is function argument it is changed to a input node?
                            else if(Argument* A = dyn_cast<Argument>(I->getOperand(r)))
                            {
                                OpGraphOp* in = new OpGraphOp("input", OPGRAPH_OP_INPUT);
                                opgraph->op_nodes.push_back(in);
                                opgraph->inputs.push_back(in);

                                OpGraphVal * v = new OpGraphVal("");
                                opgraph->val_nodes.push_back(v);

                                in->output  = v;
                                v->input    = in;

                                op->input.push_back(v);
                                v->output.push_back(op);
                                v->output_operand.push_back(r);
                            }
                            continue;
                        }

                        bool isInput = true;
                        for (const BasicBlock * BB : L->getBlocks())
                        {
                            if(pre->getParent() == BB)
                            {
                                isInput = false;
                                break;
                            }
                        }

                        if(!isInput)
                        {
                            auto itv = vals.find(pre);
                            if(itv == vals.end()) // see if we already created a val
                            {
                                // N.B. Dont need to check if it's an output, just need to create the val object
                                vals[pre] = new OpGraphVal("");
                                opgraph->val_nodes.push_back(vals[pre]);
                            }

                            vals[pre]->output.push_back(op);
                            vals[pre]->output_operand.push_back(r);
                            op->input.push_back(vals[pre]);
                        }
                        else // create new val for pre, also add to OpGraph inputs
                        {
                            OpGraphOp* in = new OpGraphOp("input", OPGRAPH_OP_INPUT);
                            opgraph->op_nodes.push_back(in);
                            opgraph->inputs.push_back(in);

                            OpGraphVal * v = new OpGraphVal("");
                            opgraph->val_nodes.push_back(v);

                            in->output  = v;
                            v->input    = in;

                            op->input.push_back(v);
                            v->output.push_back(op);
                            v->output_operand.push_back(r);
                        }
                    }// end for all operands

                    // Figure out if this is an output of the loop
                    bool isNotOutput = true;
                    for (const Use &U : I->uses())
                    {
                        //                  errs() << I->getParent()->getName() << ":::: " << *I << "\n";
                        const Instruction *I = cast<Instruction>(U.getUser());
                        bool use_is_in_loop = false;
                        for (const BasicBlock * BB : L->getBlocks())
                        {
                            if(I->getParent() == BB)
                            {
                                use_is_in_loop = true;
                                break;
                            }
                        }
                        isNotOutput = use_is_in_loop && isNotOutput;
                    }

                    // if it is an output, create output node
                    if(!isNotOutput)
                    {
                        OpGraphOp* out = new OpGraphOp("output", OPGRAPH_OP_OUTPUT);
                        out->input.push_back(vals[&*I]);
                        opgraph->op_nodes.push_back(out);
                        opgraph->outputs.push_back(out);

                        OpGraphVal * v = new OpGraphVal("");
                        opgraph->val_nodes.push_back(v);

                        out->output = v;
                        v->input = out;

                        vals[&*I]->output.push_back(out);
                        vals[&*I]->output_operand.push_back(0);
                    }
                }
            }

            if(!leavePhiNodes)
                removePhiNodes(opgraph);

            if(!leaveUnnecessaryLeafNodes)
                removeUnnecessaryLeafNodes(opgraph);

            opgraph->debug_check();

            std::ofstream f("graph_" + tag_name + ".dot", std::ios::out);
            opgraph->printDOTwithOps(f);

            return false;
        }

        virtual bool doFinalization()
        {
            errs() << "Info: Done all loops" << '\n';
            return false;
        }
    };
}

char DFGOut::ID = 0;
static RegisterPass<DFGOut> X("dfg-out", "DFG(Data Flow Graph) Output Pass",
        false /* Only looks at CFG */,
        false /* Analysis Pass */);
