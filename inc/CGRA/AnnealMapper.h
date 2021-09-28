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

#ifndef ___ANNEALMAPPER_H__
#define ___ANNEALMAPPER_H__

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <CGRA/CGRA.h>
#include <CGRA/OpGraph.h>
#include <CGRA/Mapper.h>
#include <CGRA/Mapping.h>

typedef struct
{
    std::map<OpGraphNode*, std::vector<MRRGNode*>> mapping;
} OpMapping;

class AnnealMapper : public Mapper
{
    public:
        // Custom setup with variable args
        AnnealMapper(std::shared_ptr<CGRA> cgra, int timelimit, const std::map<std::string, std::string> & args);
        // Default setup
        AnnealMapper(std::shared_ptr<CGRA> cgra);               
        // Custom setup
        AnnealMapper(std::shared_ptr<CGRA> cgra, int timelimit,  int rand_seed, float initial_penalty, float penalty_factor, float const_temp_factor, int swap_factor, float cold_accept_rate); 
        ~AnnealMapper() = default;

        Mapping mapOpGraph(std::shared_ptr<OpGraph> opgraph, int II) override;


    protected:
        int     rand_seed;
        float   pfactor;
        float   pfactor_factor;
        float   const_temp_factor;
        int     swap_factor;
        float   cold_accept_rate;
        float   updateTempConst(float temp);

    private:
        bool inner_place_and_route_loop(OpGraph* opgraph, MRRG* mrrg, float temp, float* accept_rate);
        MRRGNode* getCandidateFU(MRRG* mrrg, OpGraphOp* op);
        MRRGNode* getRandomUnoccupiedFU(MRRG* mrrg, OpGraphOp* op);
        OpGraphOp* getOpNodePtr(OpGraph* opgraph, MRRGNode* n);

        OpMapping ripUpOp(OpGraphOp* op, float* cost = NULL);
        void restoreOp(OpMapping oldmap);

        bool routeOp(OpGraphOp* op, MRRG* mrrg);
        bool routeVal(OpGraphVal* val);
        bool placeOp(OpGraphOp* op, MRRGNode* n);

        bool checkOveruse(MRRG* mrrg);
        
        // mapping/unmapping
        void  mapMRRGNode(OpGraphNode*, MRRGNode* node);
        void  unmapMRRGNode(OpGraphNode*, MRRGNode* node);
        std::vector<MRRGNode*> unmapAllMRRGNodes(OpGraphNode*);
        void mapAllMRRGNodes(OpGraphNode*, std::vector<MRRGNode*> nodes);
        MRRGNode* getMappedMRRGNode(OpGraphOp* op);

        // mapping and occupancy
        std::map<MRRGNode*,int> occupancy;
        std::map<OpGraphNode*, std::vector<MRRGNode*>> mapping;

        // Costing
        float getTotalOpCost(OpGraphOp* op);
        float getCost(MRRGNode* n);
        float getCost(MRRG* n);
        float getCost(OpGraphNode* n);
        float getCost(OpGraph* opgraph);
        bool compare_mrrg_node_cost(MRRGNode* a, MRRGNode* b);

};

#endif

