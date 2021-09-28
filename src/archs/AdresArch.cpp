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

/**

  This is adres arch for mapping.
  All of the archs wont produce accurate RTL.
  Currently there are no configuration cells in this designs.

 **/

#include <CGRA/Exception.h>
#include <CGRA/user-inc/UserArchs.h>
#include <CGRA/user-inc/UserModules.h>

std::unique_ptr<CGRA> UserArchs::createAdresArch(const std::map<std::string, std::string> & args)
{
    int cols;
    int rows;
    int toroid;
    int hetero_fu_type;
    int rf_cols;
    int rf_rows;
    try
    {
        cols = std::stoi(args.at("cols"));
        rows = std::stoi(args.at("rows"));
        toroid = std::stoi(args.at("toroid"));
        hetero_fu_type = std::stoi(args.at("fu_type"));
        rf_cols = std::stoi(args.at("rf_cols"));
        rf_rows = std::stoi(args.at("rf_rows"));
    }
    catch(const std::out_of_range & e)
    {
        throw cgrame_error("C++ Architecture Argument Error");
    }

    /* Verify if rf_rows and rf_cols are of valid values */
    if(((rows - 1) % rf_rows) != 0) // top row excluded
        throw cgrame_error("Create Adres Arch Error");

    if((cols % rf_cols) != 0)
        throw cgrame_error("Create Adres Arch Error");

    const unsigned RF_SIZE = rf_rows * rf_cols;
    std::unique_ptr<CGRA> result(new CGRA());

    /* IO Ports */
    for (int i = 0; i < cols; i++)
    {
        result->addPort("ext_io_top_" + std::to_string(i), PORT_BIDIR);
        result->addSubModule(new IO("io_top_" + std::to_string(i), 32));
        result->addConnection(
            "io_top_" + std::to_string(i) + ".bidir",
            "this.ext_io_top_" + std::to_string(i)
            ); // to top-level external pin
    }

    /* Memory Ports */
    for (int i = 0; i < rows; i++)
    {
        result->addSubModule(new MemPort("mem_" + std::to_string(i), cols));
    }

    /* Adres Processing Elements */
    for (int c = 0; c < cols; c++)
    {
        for (int r = 0; r < rows; r++)
        {
            // select fu_type based on position: 'vliw' for top row, 'cga' for everything else
            std::string fu_type = "cga";
            if ((r == 0 || hetero_fu_type == 0) ||
                    (hetero_fu_type == 1 && ((r % 2) == 0)) ||
                    (hetero_fu_type == 2 && ((c % 2) == 0))) {
                fu_type = "vliw";
            }

            // PE needs inputs for 3~4 adjacent PEs, 1 mem port, and 1 regfile
            result->addSubModule(
                new AdresPE(
                  "pe_c" + std::to_string(c) + "_r" + std::to_string(r),
                  ((toroid && r == 0)?6:5),
                  fu_type));
        }
    }

    /* IO-PE connections */
    for (int c = 0; c < cols; c++)
    {
        std::string io_n = "io_top_" + std::to_string(c);
        std::string blk_n = "pe_c" + std::to_string(c) + "_r0";
        result->addConnection(io_n + ".out", blk_n + ".in0"); //io->blk
        result->addConnection(blk_n + ".out", io_n + ".in"); //blk->io
    }

    /* Mem-PE connections */
    for(int r = 0; r < rows; r++)
    {
        std::string mem_n = "mem_" + std::to_string(r);

        for(int c = 0; c < cols; c++)
        {
            std::string blk_n = "pe_c" + std::to_string(c) + "_r" + std::to_string(r);
            result->addConnection(blk_n + ".out", mem_n + ".in" + std::to_string(c)); // to io
            result->addConnection(mem_n + ".out", blk_n + ((toroid && r == 0)?".in5":".in4")); // to block
        }
    }

    // Connect VLIW FUs (top row) to one shared register file
    result->addSubModule(new RegisterFile("drf", cols, 2*cols, 3, 32));
    for (int c = 0; c < cols; c++)
    {
        std::string blk  = "pe_c" + std::to_string(c) + "_r0";

        result->addConnection(blk + ".fu_to_rf", "drf.in" + std::to_string(c));
        result->addConnection("drf.out" + std::to_string(c*2), blk + ".rf_to_muxa");
        result->addConnection("drf.out" + std::to_string(c*2+1), blk + ".rf_to_muxout");

        result->addConfig(
            new ConfigCell("DrfWE" + std::to_string(c)),
            {"drf.WE" + std::to_string(c)});
        result->addConfig(
            new ConfigCell("DrfAddrIn" + std::to_string(c)),
            {"drf.address_in" + std::to_string(c)});
        result->addConfig(
            new ConfigCell("DrfAddrOut" + std::to_string(c*2)),
            {"drf.address_out" + std::to_string(c*2)});
        result->addConfig(
            new ConfigCell("DrfAddrOut" + std::to_string(c*2+1)),
            {"drf.address_out" + std::to_string(c*2+1)});
    }

    // Instantiate External Register Files (CGA FUs only; excludes top row VLIW FUs)
    for (int c = 0; c < cols; c += rf_cols)
    {
        for (int r = 1; r < rows; r += rf_rows)
        {
            // the name represents the minimal indices of the corner(top left) of RF
            std::string rf = "rf_c" + std::to_string(c) + "_r" + std::to_string(r);
            std::string blk = "pe_c" + std::to_string(c) + "_r" + std::to_string(r);
            result->addSubModule(new RegisterFile(rf, RF_SIZE, 2, RF_SIZE, 32));
            result->addConnection(rf + ".out0", blk + ".rf_to_muxa");
            result->addConnection(rf + ".out1", blk + ".rf_to_muxout");

            std::string rfcc = "RfC" + std::to_string(c) + "R" + std::to_string(r);
            result->addConfig(new ConfigCell(rfcc + "WE"), {rf + ".WE0"});
            result->addConfig(new ConfigCell(rfcc + "AddrIn0"), {rf + ".address_in0"});
            result->addConfig(new ConfigCell(rfcc + "AddrOut0"), {rf + ".address_out0"});
            result->addConfig(new ConfigCell(rfcc + "AddrOut1"), {rf + ".address_out1"});
        }
    }
    // Connect external RFs to PEs
    for (int c = 0; c < cols; c++)
    {
        for (int r = 1; r < rows; r++)
        {
            int c_mod = c % rf_cols;
            int r_mod = (r - 1) % rf_rows;
            int c_rf = c - c_mod;
            int r_rf = r - r_mod;
            int rf_in_i = 2 * r_mod + c_mod;
            std::string rf_in = std::to_string(rf_in_i);
            std::string rf = "rf_c" + std::to_string(c_rf) + "_r" + std::to_string(r_rf);
            std::string blk = "pe_c" + std::to_string(c) + "_r" + std::to_string(r);
            result->addConnection(blk + ".fu_to_rf", rf + ".in" + rf_in);
        }
    }

    // NORTH/SOUTH inter-PE connections
    for (int c = 0; c < cols; c++)
    {
        for (int r = 0; r < rows - 1; r++)
        {
            std::string blk_n_c_r  = "pe_c" + std::to_string(c) + "_r" + std::to_string(r);
            std::string blk_n_c_r1 = "pe_c" + std::to_string(c) + "_r" + std::to_string(r + 1);

            // north / south connections
            result->addConnection(blk_n_c_r + ".out", blk_n_c_r1 + ".in0");
            result->addConnection(blk_n_c_r1 + ".out", blk_n_c_r + ".in2");
        }
    }
    if (toroid)
    { // toroid connection for top and bottom
        for (int c = 0; c < cols; c++)
        {
            std::string blk_n_c_r  = "pe_c" + std::to_string(c) + "_r" + std::to_string(rows - 1);
            std::string blk_n_c_r1 = "pe_c" + std::to_string(c) + "_r" + std::to_string(0);
            result->addConnection(blk_n_c_r + ".out", blk_n_c_r1 + ".in4"); // special port of top row PE
            result->addConnection(blk_n_c_r1 + ".out", blk_n_c_r + ".in2");
        }
    }

    // EAST/WEST inter-PE connections
    int colBound = (toroid) ? cols : cols - 1; // allow 'wraparound' if requested
    for (int c = 0; c < colBound; c++)
    {
        for (int r = 0; r < rows; r++)
        {
            std::string blk_n_c_r  = "pe_c" + std::to_string(c) + "_r" + std::to_string(r);
            std::string blk_n_c1_r = "pe_c" + std::to_string((c+1) % cols) + "_r" + std::to_string(r);

            // east / west connections
            result->addConnection(blk_n_c_r +  ".out", blk_n_c1_r + ".in3");
            result->addConnection(blk_n_c1_r + ".out", blk_n_c_r + ".in1");
        }
    }
    return result;
}
