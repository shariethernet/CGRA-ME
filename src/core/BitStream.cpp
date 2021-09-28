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

#include <CGRA/CGRA.h>

#include <numeric>
#include <ostream>

void BitStream::append(const ConfigCell* ccell, const std::vector<BitSetting>& bits) {
    if (not setting_storage.emplace(ccell, bits).second) {
        throw cgrame_error("adding duplicate config setting!");
    }
    ccell_order.push_back(ccell);
}

// Regular bitstream printing
void BitStream::print()
{
    for (const auto& ccell : ccell_order) {
        std::cout << setting_storage.at(ccell);
    }
};

// Bitstream printing in debug mode, adds spaces between bits in different indexes
void BitStream::print_debug()
{
    for (const auto& ccell : ccell_order) {
        std::cout << ccell->getName() << ": " << setting_storage.at(ccell) << '\n';
    }
};

// Prints out bits in reverse order for a ModelSim testbench
void BitStream::print_testbench(std::ostream& os) const
{
    static const char* const total_num_bits_localparam =   "TOTAL_NUM_BITS";
    static const char* const clock_sig =                   "clock";
    static const char* const enable_sig =                  "enable";
    static const char* const sync_reset_sig =              "sync_reset";
    static const char* const bitstream_sig =               "bitstream";
    static const char* const done_sig =                    "done";
    static const char* const storage =                     "storage";
    static const char* const storage_pos =                 "next_pos";

    const int total_num_bits = std::accumulate(begin(setting_storage), end(setting_storage), 0, [&](auto&& accum, auto&& ccell_and_setting) {
        return accum + ccell_and_setting.first->getStorageSize();
    });

    os <<
        "module CGRA_configurator(\n"
        "    input      "<<clock_sig<<",\n"
        "    input      "<<enable_sig<<",\n"
        "    input      "<<sync_reset_sig<<",\n"
        "\n"
        "    output reg "<<bitstream_sig<<",\n"
        "    output reg "<<done_sig<<"\n"
        ");\n"
        "\n"
        "    localparam "<<total_num_bits_localparam<<" = " << total_num_bits << ";\n"
    ;

    os << "\treg [0:"<<total_num_bits_localparam<<"-1] "<<storage<<" = {\n";
    for (auto ccell_it = rbegin(ccell_order); ccell_it != rend(ccell_order); ++ccell_it) {
        const auto& ccell = *ccell_it;
        const auto& setting = setting_storage.at(ccell);
        os << "\t\t";
        for (auto bitsetting_it = rbegin(setting); bitsetting_it != rend(setting); ++bitsetting_it) {
            const auto& bitsetting = *bitsetting_it;
            os << for_verilog(bitsetting);
            if (std::next(ccell_it) != rend(ccell_order) || std::next(bitsetting_it) != rend(setting)) {
                os << ',';
            }
        }
        os << " // " << ccell->getAllConnectedPorts().at(0)->getModule().parent->getName() << "::" << ccell->getName() << '\n';
    }

    os <<
        "	};\n"
        "\n"
        "	reg [31:0] "<<storage_pos<<";\n"
        "	always @(posedge "<<clock_sig<<") begin\n"
        "		if (sync_reset) begin\n"
        "			"<<storage_pos<<" <= 0;\n"
        "			"<<bitstream_sig<<" <= 1'bx;\n"
        "			"<<done_sig<<" <= 0;\n"
        "		end else if ("<<storage_pos<<" >= "<<total_num_bits_localparam<<") begin\n"
        "			"<<done_sig<<" <= 1;\n"
        "			"<<bitstream_sig<<" <= 1'bx;\n"
        "		end else if ("<<enable_sig<<") begin\n"
        "			"<<bitstream_sig<<" <= "<<storage<<"["<<storage_pos<<"];\n"
        "			"<<storage_pos<<" <= "<<storage_pos<<" + 1;\n"
        "		end\n"
        "	end\n"
        "endmodule\n"
    ;
}
