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

#ifndef __CGRA_H__
#define __CGRA_H__

#include <memory>
#include <vector>

class CGRA;

#include <CGRA/Mapping.h>
#include <CGRA/Module.h>

/*
#include <set>
#include <algorithm>
#include <stdlib.h>
#include <memory>
#include <fstream>

*/
// This class holds the bitstream (in the map), and contains helper functions to add/remove items, and print the bitstream
class BitStream
{
    public:
        void append(const ConfigCell* ccell, const std::vector<BitSetting>& bits);
        void print(); // Print out the bistream
        void print_debug(); // Print out bistream in debug mode (with spaces between bits for differerent config cells)
        void print_testbench(std::ostream& os) const;

    private:
        std::unordered_map<const ConfigCell*, std::vector<BitSetting>> setting_storage;
        std::vector<const ConfigCell*> ccell_order;
};

// This class represents a top level CGRA architecture
class CGRA : public Module
{
    public:
        CGRA();
        BitStream genBitStream(const Mapping& mapping);  // Function that generates bitstream
        void genTimingConstraints(OpGraph * mappped_opgraph);   // Function that determines timing analysis of design mapped onto CGRA
        void genFloorPlan();                                    // Floorplans CGRA blocks. Only works well with a homogeneous array
        virtual ~CGRA();
        
        int maxII;
        int ROWS, COLS; // TODO: Are these necessary?
        
        std::shared_ptr<MRRG> getMRRG(int II);
    private:
        // CGRA variables
        std::vector<std::shared_ptr<MRRG>>      mrrgs; // Keeps references to all the MRRGs for each II
};
#endif

