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

#ifndef __USERARCHS__H__
#define __USERARCHS__H__

#include <utility>
#include <map>
#include <string>

#include <CGRA/CGRA.h>
#include <CGRA/Module.h>
#include <CGRA/ModuleComposites.h>

class UserArchs
{
    public:
        UserArchs()
        {
            // Follow the syntax below, add your fuction pointer, description, and default arguments to archs
            /*
             *                              Function Pointer                                 Description
             *                                      |                                             |
             archs.insert(std::make_pair(createSimpleArchOrth, std::make_pair("Simple Orthogonal CGRA Architecture", std::map<std::string, std::string>
             {
             {"cols", "4"},
             {"rows", "4"},              <--- Default arguments
             {"homogeneous_fu", "1"},
             }
             )));
             */
            archs.push_back(std::make_pair(createSimpleArchOrth, std::make_pair("Simple Orthogonal CGRA Architecture", std::map<std::string, std::string>
                            {
                            {"cols", "4"}, // Columns
                            {"rows", "4"}, // Rows
                            {"homogeneous_fu", "1"}, // Homogeneous Function Unit ( 1 = true, 0 = false)
                            }
                            )));

            archs.push_back(std::make_pair(createSimpleArchDiag, std::make_pair("Simple Diagonal CGRA Architecture", std::map<std::string, std::string>
                            {
                            {"cols", "4"}, // Columns
                            {"rows", "4"}, // Rows
                            {"homogeneous_fu", "1"}, // Homogeneous Function Unit ( 1 = true, 0 = false)
                            }
                            )));

            archs.push_back(std::make_pair(createAdresArch, std::make_pair("Adres CGRA Architecture", std::map<std::string, std::string>
                            {
                            {"cols", "4"}, // Columns
                            {"rows", "4"}, // Rows
                            {"toroid", "1"}, // Toroid Connection
                            {"fu_type", "0"}, // Heterogeneous Function Unit Type (0 = homogeneous, 1 = alternate-by-row, 2 = alternate-by-column
                            {"rf_cols", "1"}, // Register File Columns
                            {"rf_rows", "1"}, // Register File Rows
                            }
                            )));
        }

        std::vector<std::pair<std::unique_ptr<CGRA> (*) (const std::map<std::string, std::string>&), std::pair<std::string, std::map<std::string, std::string>>>> archs;

    private:
        // Add your function that create the CGRA in-memory architecture, following the prototype below
        static std::unique_ptr<CGRA> createSimpleArchOrth(const std::map<std::string, std::string> & args);
        static std::unique_ptr<CGRA> createSimpleArchDiag(const std::map<std::string, std::string> & args);
        static std::unique_ptr<CGRA> createAdresArch(const std::map<std::string, std::string> & args);
};

#endif

