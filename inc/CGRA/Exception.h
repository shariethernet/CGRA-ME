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

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <string>
#include <stdexcept>

struct cgrame_error : public std::runtime_error
{
    cgrame_error(const std::string & message)
        : std::runtime_error("[ERROR] " + message)
    {}
};

struct cgrame_mapper_error : public cgrame_error
{
    cgrame_mapper_error(const std::string & message)
        : cgrame_error("[MAPPER] " + message)
    {}
};

struct cgrame_visual_error : public cgrame_error
{
    cgrame_visual_error(const std::string & message)
        : cgrame_error("[VISUAL] " + message)
    {}
};

struct cgrame_adl_error : public cgrame_error
{
    cgrame_adl_error(const std::string & message)
        : cgrame_error("[ADL] " + message)
    {}
};

struct cgrame_model_error : public cgrame_error
{
    cgrame_model_error(const std::string & message)
        : cgrame_error("[MODEL] " + message)
    {}
};

#endif
