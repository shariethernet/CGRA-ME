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

#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <sstream>

#include <ctype.h>

#include "ADLParser.h"
#include "ADLStructs.h"

#include "pugixml.hpp"

namespace adl1 {

//This function fills the xmlData with the orthogonal mesh architecture for CGRA with the
//attributes specified in the xml document
bool create_orthogonal_mesh(ADLStructs &xmlData, pugi::xml_node mesh);

//This does the same as the above function
bool create_diag_mesh(ADLStructs &xmlData, pugi::xml_node diagonal);

//This function only adds lines and should be removed
bool connect_inner_mesh_block(ADLStructs &xmlData, std::map<int, std::string> ports);

}

