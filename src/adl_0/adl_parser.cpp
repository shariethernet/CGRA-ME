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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(XML_VALIDATION)
#include <libxml++/libxml++.h>
#endif

#include "pugixml.hpp"

#include <tuple>
#include <vector>
#include <iostream>
#include "adl.h"

namespace adl0 {

std::shared_ptr<CGRA> parseADL(std::string arch_file)
{
#if defined(XML_VALIDATION)
    // TODO: FIX setting the correct schema file
    if(argc > 2 )
        rngschemafile = argv[2];
    else
        rngschemafile = "../../language/schema.rng";

    try
    {
        xmlpp::DomParser parser;
        // parser.set_validate();
        parser.set_substitute_entities();
        parser.parse_file(arch_file);

        xmlpp::RelaxNGValidator rnvValidator(rngschemafile);
        if(parser)
        {
            rnvValidator.validate(parser.get_document());
        }
#endif

        pugi::xml_document doc;
        auto source = arch_file.c_str();

        pugi::xml_parse_result result = doc.load_file(source);

        if(result)
        {
            std::cout << "XML [" << source << "] parsed without errors this should show up" << std::endl;
            ADL p;
            std::shared_ptr<CGRA> cgra(p.parse(doc).release());
           // std::cout << "Pringing the contents of the module\n";

            //cgra->print();

          //  std::cout << "Power consumed: " << cgra->power() << "\n";
           // std::cout << "Area consumed: " << cgra->area() << "\n";

           // abort();
            return cgra;
        }
        else
        {
            std::cout << "XML [" << source << "] parsed with errors" << std::endl;
            std::cout << "Error description: " << result.description() << std::endl;
            std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "])" << std::endl;
        }
#if defined(XML_VALIDATION)
    }
    catch(const xmlpp::internal_error& ex)
    {
        std::cout << "Exception caught: " << ex.what() << std::endl;
    }
#endif

    return NULL;
}

}

