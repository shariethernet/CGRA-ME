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

#ifndef DOTPARSER_DRIVER_HH
# define DOTPARSER_DRIVER_HH
# include <string>
# include <map>
# include <list>
# include "dfgdot.tab.h"

#include <CGRA/OpGraph.h>

class driver
{
    public:
        driver();
        virtual ~driver();

        // Handling the scanner.
        int scan_begin ();
        void scan_end ();
        bool trace_scanning;

        int count;
        OpGraph* opgraph;

        std::map<std::string, OpGraphOp*> nodes;
        std::map<std::string, OpGraphVal*> edges;

        std::map<std::string, OpGraphOp*> inputnodes;
        std::map<std::string, OpGraphOp*> outputnodes;

        bool finalize();

        // Run the parser on file F.
        // Return 0 on success.
        int parse(const std::string& f);
        // The name of the file being parsed.
        // Used later to pass the file name to the location tracker.
        std::string file;
        // Whether parser traces should be generated.
        bool trace_parsing;

        // Error handling.
        void error (const yy::location& l, const std::string& m);
        void error (const std::string& m);
};

# define YY_DECL yy::parser::symbol_type yylex(driver& d)
// ... and declare it for the parser's sake.
YY_DECL;

#endif

