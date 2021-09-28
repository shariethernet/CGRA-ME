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

#include <tuple>
#include <vector>
#include <iostream>
#include "adl.h"

namespace adl0 {

ADL::grid_position ADL::grid_area::division_begin(division div) const noexcept
{
    switch (div) {
        case division::NW:
        case division::WHOLE:
            return ADL::grid_position(top_, left_);
        case division::N:
            return ADL::grid_position(top_, left_+1);
        case division::NE:
            return ADL::grid_position(top_, right_);
        case division::W:
            return ADL::grid_position(top_+1, left_);
        case division::C:
            return ADL::grid_position(top_+1, left_+1);
        case division::E:
            return ADL::grid_position(top_+1, right_);
        case division::SW:
            return ADL::grid_position(bottom_, left_);
        case division::S:
            return ADL::grid_position(bottom_, left_+1);
        case division::SE:
            return ADL::grid_position(bottom_, right_);
        case division::INVALID:
            return ADL::grid_position(bottom_, right_+1);
    }
}


bool ADL::grid_area::const_iterator::valid_position(division div) const noexcept
{

    if (div == division::WHOLE)
        return current_.col >= area_->left_  && current_.col <= area_->right_ && current_.row >= area_->top_ && current_.row <= area_->bottom_;
    else if (area_->left_ == area_->right_ || area_->top_ == area_->bottom_) {
            return false;
    } else switch (div) {
        case division::NW:
            return current_.col == area_->left_  && current_.col <  area_->right_ && current_.row == area_->top_ && current_.row <  area_->bottom_;
        case division::NE:
            return current_.col >  area_->left_  && current_.col == area_->right_ && current_.row == area_->top_ && current_.row <  area_->bottom_;
        case division::SW:
            return current_.col == area_->left_  && current_.col <  area_->right_ && current_.row >  area_->top_ && current_.row == area_->bottom_;
        case division::SE:
            return current_.col >  area_->left_  && current_.col == area_->right_ && current_.row >  area_->top_ && current_.row == area_->bottom_;
        case division::N:
            return current_.col >  area_->left_  && current_.col <  area_->right_ && current_.row == area_->top_ && current_.row <  area_->bottom_;
        case division::S:
            return current_.col >  area_->left_  && current_.col <  area_->right_ && current_.row >  area_->top_ && current_.row == area_->bottom_;
        case division::W:
            return current_.col == area_->left_  && current_.col <  area_->right_ && current_.row >  area_->top_ && current_.row <  area_->bottom_;
        case division::E:
            return current_.col >  area_->left_  && current_.col == area_->right_ && current_.row >  area_->top_ && current_.row <  area_->bottom_;
        case division::C:
            return current_.col >  area_->left_  && current_.col <  area_->right_ && current_.row >  area_->top_ && current_.row <  area_->bottom_;
        case division::WHOLE:
            std::cerr << "There is a bug in mattern matching about divisions." << std::endl;
            return current_.col >= area_->left_  && current_.col <= area_->right_ && current_.row >= area_->top_ && current_.row <= area_->bottom_;
        case division::INVALID:
            return false;
    }
}

bool ADL::grid_area::const_iterator::move_next_position_in_same_division(void) noexcept
{
    switch (*div_it_) {
        case division::NW: case division::NE: case division::SW: case division::SE:
            return false;
        case division::N:
        case division::S:
            ++current_.col;
            if (current_.col >= area_->right_) {
                return false;
            }
            return true;
        case division::E:
        case division::W:
            ++current_.row;
            if (current_.row >= area_->bottom_) {
                return false;
            }
            return true;
        case division::C:
            ++current_.col;
            if (current_.col >= area_->right_) {
                ++current_.row;
                current_.col = area_->left_ + 1;
            }
            if (current_.row >= area_->right_ || current_.row >= area_->bottom_) {
                return false;
            }
            return true;
        case division::WHOLE:
            ++current_.col;
            if (current_.col > area_->right_) {
                ++current_.row;
                current_.col = area_->left_;
            }
            if (current_.row > area_->right_ || current_.row > area_->bottom_) {
                return false;
            }
            return true;
        case division::INVALID:
            return false;
    }
}

ADL::division ADL::read_division(const std::string &str)
{
    if (!str.compare("north")) {
        return ADL::division::N;
    } else if (!str.compare("east")) {
        return ADL::division::E;
    } else if (!str.compare("west")) {
        return ADL::division::W;
    } else if (!str.compare("south")) {
        return ADL::division::S;
    } else if (!str.compare("northeast")) {
        return ADL::division::NE;
    } else if (!str.compare("northwest")) {
        return ADL::division::NW;
    } else if (!str.compare("southeast")) {
        return ADL::division::SE;
    } else if (!str.compare("southwest")) {
        return ADL::division::SW;
    } else if (!str.compare("center")) {
        return ADL::division::C;
    } else if (!str.compare("whole")) {
        return ADL::division::WHOLE;
    } else  {
        std::cerr << "Unknown edge name: " << str << std::endl;
        return ADL::division::INVALID;
    }
}

}

