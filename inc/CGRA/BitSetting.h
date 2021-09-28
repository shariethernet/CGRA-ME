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

#ifndef __BITSETTING_H__
#define __BITSETTING_H__

#include <iosfwd>
#include <vector>

enum class BitSetting {
    LOW,
    HIGH,
    DONT_CARE_PREFER_LOW,
    DONT_CARE_PREFER_HIGH,
    HIGH_IMPEDANCE,
};

template<typename T>
T from_bool(bool b);

template<>
BitSetting from_bool<BitSetting>(bool b);
BitSetting force_to_rail(const BitSetting& bs);

std::ostream& operator<<(std::ostream& os, const BitSetting& bs);
std::ostream& operator<<(std::ostream& os, const std::vector<BitSetting>& bits);

struct BitSettingForVerilog {
    BitSetting val;
};

inline BitSettingForVerilog for_verilog(const BitSetting& bs) { return { bs }; }
std::ostream& operator<<(std::ostream& os, const BitSettingForVerilog& bs);

template<typename INTEGRAL>
std::vector<BitSetting>& push_back_int(std::vector<BitSetting>& v, const INTEGRAL& value, int num_bits) {
    for (int ibit = num_bits; ibit != 0; --ibit) {
        v.push_back(from_bool<BitSetting>((value >> (ibit-1)) % 2));
    }
    return v;
}

template<typename INTEGRAL>
std::vector<BitSetting> bitsettings_from_int(const INTEGRAL& value, int num_bits) {
	std::vector<BitSetting> result;
	push_back_int(result, value, num_bits);
	return result;
}


#endif /* __BITSETTING_H__ */
