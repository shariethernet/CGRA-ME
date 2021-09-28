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

#ifndef __MAPPING_H___
#define __MAPPING_H___

#include <vector>
#include <iostream>

class Mapping;

#include <CGRA/CGRA.h>
#include <CGRA/MRRG.h>
#include <CGRA/OpGraph.h>

class Mapping
{
    public:
        Mapping(std::shared_ptr<CGRA> cgra, int II, std::shared_ptr<OpGraph> opgraph);
        ~Mapping();

        bool isMapped() const;
        void setMapped(bool);

        CGRA * getCGRA() const;
        int getII() const;

        // accessor functions to the mapping
              std::map<OpGraphNode*,std::vector<MRRGNode*>>& getMapping()       { return mapping; }
        const std::map<OpGraphNode*,std::vector<MRRGNode*>>& getMapping() const { return mapping; }

        const MRRGNode& getSingleMapping(OpGraphNode* key) const { return *mapping.at(key).at(0); }
        const std::vector<MRRGNode*>& getMappingList(OpGraphNode* key) const { return mapping.at(key); }

        void setMapping(std::map<OpGraphNode*,std::vector<MRRGNode*>> mapping);

        void  mapMRRGNode(OpGraphNode*, MRRGNode* node);
        void  unmapMRRGNode(OpGraphNode*, MRRGNode* node);

        // Result printing function
        void outputMapping(std::ostream & o = std::cout) const;
        void outputDetailedMapping(std::ostream & o = std::cout) const;

        // mapping verification
        bool verifyOpGraphMappingConnectivity();

              OpGraph& getOpGraph()       { return *opgraph; }
        const OpGraph& getOpGraph() const { return *opgraph; }

    private:
        std::map<OpGraphNode*,std::vector<MRRGNode*>> mapping;
        // TODO: add fanout-details
        std::shared_ptr<CGRA>       cgra;
        int                         II;
        std::shared_ptr<OpGraph>    opgraph;

        bool mapped;
};

#endif

