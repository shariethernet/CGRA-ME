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

#include "dfgdot_driver.h"
#include "dfgdot.tab.h"

driver::driver ()
    : trace_scanning (false), trace_parsing (false)
{
}

driver::~driver ()
{
}

int driver::parse(const std::string &f)
{
    file = f;
    opgraph = new OpGraph();

    if(scan_begin())
        return 1;
    yy::parser parser(*this);
#ifdef DEBUG
    parser.set_debug_level(trace_parsing);
#endif
    int res = parser.parse();
    scan_end();
    if(res)
    {
        std::cout << "[ERROR] Parsing Failed, Delete Opgraph..." << std::endl;
        delete opgraph; // TODO: need to delete nodes too
    }
#ifdef DEBUG
    std::cerr << "RES:" << res << std::endl;
#endif
    return res;
}

bool driver::finalize()
{
    // populate inputs
    for(auto it = inputnodes.begin(); it != inputnodes.end(); it++)
    {
        opgraph->inputs.push_back((*it).second);
    }

    // populate outputs
    for(auto it = outputnodes.begin(); it != outputnodes.end(); it++)
    {
        opgraph->outputs.push_back((*it).second);
    }

    return true;
}

void driver::error (const yy::location& l, const std::string& m)
{
    std::cerr << l << ": " << m << std::endl;
}

void driver::error (const std::string& m)
{
    std::cerr << m << std::endl;
}

