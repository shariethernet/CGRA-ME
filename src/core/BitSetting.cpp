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

#include <CGRA/BitSetting.h>
#include <CGRA/Exception.h>

#include <iostream>

template<>
BitSetting from_bool<BitSetting>(bool b) {
    if (b) {
        return BitSetting::HIGH;
    } else {
        return BitSetting::LOW;
    }
}

BitSetting force_to_rail(const BitSetting& bs) {
    switch (bs) {
        case BitSetting::LOW:
        case BitSetting::DONT_CARE_PREFER_LOW:
            return BitSetting::LOW;
        case BitSetting::HIGH:
        case BitSetting::DONT_CARE_PREFER_HIGH:
            return BitSetting::HIGH;
        default:
            throw cgrame_error("don't know how to force BitSetting to rail");
    }
}

std::ostream& operator<<(std::ostream& os, const BitSetting& bs) {
    switch (bs) {
        case BitSetting::LOW:
            return os << "0";
        case BitSetting::HIGH:
            return os << "1";
        case BitSetting::DONT_CARE_PREFER_LOW:
        case BitSetting::DONT_CARE_PREFER_HIGH:
            return os << "x";
        case BitSetting::HIGH_IMPEDANCE:
            return os << "z";
        default:
            throw cgrame_error("unhandled BitSetting in stream printing function");
    }    
}

std::ostream& operator<<(std::ostream& os, const std::vector<BitSetting>& bits) {
    os << '{';
    for (const auto& bit : bits) {
        os << bit;
    }
    os << '}';
    return os;
}

std::ostream& operator<<(std::ostream& os, const BitSettingForVerilog& bs) {
    switch (bs.val) {
        case BitSetting::LOW:
            return os << "1'b0";
        case BitSetting::HIGH:
            return os << "1'b1";
        case BitSetting::DONT_CARE_PREFER_LOW:
        case BitSetting::DONT_CARE_PREFER_HIGH:
            return os << "1'bx";
        case BitSetting::HIGH_IMPEDANCE:
            return os << "1'bz";
        default:
            throw cgrame_error("unhandled BitSettingForVerilog in stream printing function");
    }    
}
