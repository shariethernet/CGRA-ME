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

#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <memory>
#include <iostream>
#include <iomanip>
#include <iterator>

#include <linux/limits.h>
#include <unistd.h>
#include <libgen.h>

#include <CGRA/user-inc/UserArchs.h>

#include <CGRA/Exception.h>
#include <CGRA/Mapper.h>
#include <CGRA/Mapping.h>

#include <CGRA/dotparse.h>
#include <CGRA/adlparse.h>

#include <CGRA/Visual.h>

#include <cxxopts.hpp>

#include <mini.hpp>

#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
    std::cout << std::endl;
    std::cout << "CGRA - Modelling and Exploration Version 1.0 (http://cgra-me.ece.utoronto.ca/)" << std::endl;
    std::cout << "Copyright (c) 2015-2018 University of Toronto. All Rights Reserved." << std::endl;
    std::cout << "For research and academic purposes only. Commercial use is prohibited." << std::endl;
    std::cout << "Please email questions to: Xander Chin(xan@ece.utoronto.ca)" << std::endl;
    std::cout << "Compiled: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << std::endl;

    UserArchs userarchs;

    std::string arch_filename, dfg_filename, hdl_dirpath;
    int arch_id;
    std::string arch_opts;
    MapperType mapper_type;
    std::string mapper_opts;
    int II;
    double timelimit;
    bool printarch;
    bool printop;
    bool genverilog;
    bool make_testbench;
    int adl;

    try
    {
        cxxopts::Options options("CGRA-ME", "CGRA - Modelling and Exploration");

        options.add_options()
            ("h,help", "Print Help")
            ("x,xml", "Use Architecture Description Language File", cxxopts::value<std::string>(), "<Filepath>")
            ("p,parser", "Which ADL parser to use (0 = XML specs v0, 1 = XML specs v1)", cxxopts::value<int>()->default_value("1"), "<#>")
            ("c,cpp", "Use C++ Architecture, ID # Generated from --arch-list", cxxopts::value<int>(), "<#>")
            ("arch-list", "Show the List of Avaliable C++ Architectures with IDs", cxxopts::value<bool>())
            ("arch-opts", "C++ Architecture Options that Overwrites the Default Ones (<Key>=<Value> Pairs, Separate by Space, and Close by Quotation Marks)", cxxopts::value<std::string>(), "<\"opts\">")
            ("arch-opts-list", "Show the List of Avaliable Options for a C++ Architecture, ID # Generated from --arch-list", cxxopts::value<int>(), "<#>")
            ("i,II", "Architecture Contexts", cxxopts::value<int>()->default_value("1"), "<#>")
            ("g,dfg", "The DFG file to map in dot format", cxxopts::value<std::string>())
            ("m,mapper", "Which Mapper to Use (0 = ILP, 1 = Simulated Annealing)", cxxopts::value<int>()->default_value("0"), "<#>")
            ("mapper-opts", "Mapper Options that Overwrites the Default Ones (<Key>=<Value> Pairs, Separate by Space, and Close by Quotation Marks)", cxxopts::value<std::string>(), "<\"opts\">")
            ("v,visual", "Output visualization directory after mapping", cxxopts::value<bool>())
            ("t,timelimit", "Mapper Timelimit", cxxopts::value<double>()->default_value("7200.0"), "<#>")
            ("a,print-arch", "Print Architecture to stdout", cxxopts::value<bool>())
            ("o,print-op", "Print Operation Graph to stdout", cxxopts::value<bool>())
            ("gen-verilog", "Generate Verilog Implementation of Architecture and Dump to Specified Directory", cxxopts::value<std::string>(), "<Directorypath>")
            ("gen-testbench", "Generate testbench for use in a simulation of the DFG with configuration bitstream", cxxopts::value<bool>())
            ;

        options.parse(argc, argv);

        genverilog = options.count("gen-verilog");

        if(options.count("help")) // Print help
        {
            std::cout << options.help({""}) << std::endl;
            return 0;
        }

        if(options.count("arch-list")) // Print the list of avaliable CGRA architecture
        {
            std::cout << "List of C++ CGRA Architecture with ID: " << std::endl;
            int count = 0;
            for(const auto & arch : userarchs.archs)
                std::cout << count++ << ". " << arch.second.first << std::endl;
            return 0;
        }

        if(options.count("arch-opts-list")) // Print the list of avaliable options for a CGRA architecture
        {
            int arch_index(options["arch-opts-list"].as<int>());
            if(!(arch_index >= 0 && arch_index < userarchs.archs.size()))
            {
                std::cout << "[ERROR] The C++ Architecture ID Given Is out of Bound, Run with --arch-list to See the List of Archs with IDs" << std::endl;
                return 1;
            }
            const auto & arch_entry = *std::next(userarchs.archs.begin(), arch_index);
            const auto & arch_default_args = arch_entry.second.second;
            std::cout << "List of Options for: " << arch_entry.second.first << std::endl;
            std::cout << "Option:" << std::setw(80) << "Default Value:" << std::endl;
            for(const auto & arch_arg : arch_default_args)
                std::cout << std::left << std::setw(40) << arch_arg.first << std::right << std::setw(40) << arch_arg.second << std::endl;
            return 1;
        }

        if(!genverilog && !options.count("dfg"))
        {
            //std::cout << "[ERROR] DFG File Path is Missing, Exiting..." << std::endl;
            std::cout << options.help({""}) << std::endl;
            return 1;
        }

        if(options.count("xml") && options.count("cpp"))
        {
            std::cout << "[ERROR] You Can Only Choose Either ADL or C++ but Not Both" << std::endl;
            return 1;
        }
        else if(!options.count("xml") && !options.count("cpp"))
        {
            std::cout << "[ERROR] No Architecture Specified, You Can Use Either ADL or C++" << std::endl;
            return 1;
        }

        if(options.count("cpp"))
        {
            int cpp_index(options["cpp"].as<int>());
            if(!(cpp_index >= 0 && cpp_index < userarchs.archs.size()))
            {
                std::cout << "[ERROR] The C++ Architecture ID Given Is out of Bound, Run with --arch-list to See the List of Archs with IDs" << std::endl;
                return 1;
            }
        }

        dfg_filename = options["dfg"].as<std::string>();
        arch_filename = options["xml"].as<std::string>();
        hdl_dirpath = options["gen-verilog"].as<std::string>();
        arch_id = options["cpp"].as<int>();
        arch_opts = options["arch-opts"].as<std::string>();
        mapper_type = static_cast<MapperType>(options["mapper"].as<int>());
        mapper_opts = options["mapper-opts"].as<std::string>();
        II = options["II"].as<int>();
        timelimit = options["timelimit"].as<double>();
        printarch = options["print-arch"].as<bool>();
        printop = options["print-op"].as<bool>();
        adl = options["parser"].as<int>();
        make_testbench = options["gen-testbench"].as<bool>();
    }
    catch(const cxxopts::OptionException & e)
    {
        std::cout << "[ERROR] Error Parsing Options: " << e.what() << std::endl;
        return 1;
    }

    try
    {
        char full_path[PATH_MAX] = {0}; // initialize to zero, so we don't have to set the null char ourselves
        ssize_t count = readlink("/proc/self/exe", full_path, PATH_MAX-1);
        std::string exe_path;
        if(count != -1)
        {
            exe_path = std::string(dirname(full_path));
        }
        else
        {
            std::cout << "[ERROR] Readlink is not able to get the executable path" << std::endl;
            return 1;
        }

        std::shared_ptr<CGRA> arch = NULL;

        if(arch_filename.empty()) // Use C++
        {
            std::cout << "[INFO] Creating Architecture #" << arch_id << " from C++..." << std::endl;
            auto & arch_entry = *std::next(userarchs.archs.begin(), arch_id);
            std::cout << "[INFO] Architecture Name: " << arch_entry.second.first << std::endl;
            auto & arch_default_args = arch_entry.second.second;
            std::stringstream ss_arch_opts(arch_opts);
            std::string opt_pair;
            while(ss_arch_opts >> opt_pair)
            {
                auto n = opt_pair.find('=');
                if(n == std::string::npos)
                {
                    std::cout <<"[WARNING] Ill Formated C++ Architrecture Option, Skipping: " << opt_pair << std::endl;
                    continue;
                }
                auto key = opt_pair.substr(0, n);
                auto value = opt_pair.substr(n + 1, std::string::npos);
                auto it = arch_default_args.find(key);
                if(it != arch_default_args.end())
                {
                    std::cout << "[INFO] Overwritting C++ Architecture Parameter: " << key << " to " << value << " (Default: " << arch_default_args.at(key) << ")" << std::endl;
                    arch_default_args[key] = value; // Overwrite default option
                }
                else
                    std::cout << "[WARNING] C++ Architecture Parameter: " << key << " doesn't exist, Skipping: " << key << " = " << value << std::endl;
            }
            std::cout << "[INFO] Creating \"" << arch_entry.second.first << "\" Architecture from C++..." << std::endl;
            arch = arch_entry.first(arch_default_args); // Create C++ Arch
        }
        else // Use ADL
        {
            std::cout << "[INFO] Creating Architecture from XML..." << std::endl;
            if(adl)
            {
                std::string adl_template_filename = exe_path + "/module_templates.xml";
                std::ifstream adl_template_file(adl_template_filename);
                if (adl_template_file.good())
                    arch = adl1::parseADL(adl_template_filename, arch_filename);
                else
                {
                    std::cout << "[ERROR] Missing module_templates.xml, Cannot Read Default Module Templates" << std::endl;
                    return 1;
                }
            }
            else
                arch = adl0::parseADL(arch_filename);
        }

        // Print Architecture if requested
        if(printarch)
        {
            std::cout << std::endl << "[ARCHGRAPH]" << std::endl;
            arch->print();
            arch->print_dot();
        }

        // Generate Verilog implementation and exit
        if(genverilog)
        {
            // Verify if directory exists
            struct stat fs_info;
            auto res = stat(hdl_dirpath.c_str(), &fs_info);
            if (res != 0)
            {
                std::cout << "[ERROR] Cannot open \"" + hdl_dirpath + "\"";
                std::cout << std::endl;
                return 1;
            }
            bool is_dir = fs_info.st_mode & S_IFDIR;
            if (!is_dir) // make sure it is directory
            {
                std::cout << "[ERROR] \"" + hdl_dirpath + "\" is not a directory";
                std::cout << std::endl;
                return 1;
            }

            std::cout << "[INFO] Generating Verilog Implementation of Specified Architecture" << std::endl;
            if (hdl_dirpath.back() != '/')
                hdl_dirpath = hdl_dirpath + "/";
            arch->genVerilog(hdl_dirpath);
            return 0;
        }

        // Creating OpGraph
        std::cout << "[INFO] Parsing DFG..." << std::endl;
        // Need to do std::move for GCC 6.2 (possibly others)
        std::shared_ptr<OpGraph> opgraph = parseOpGraph(dfg_filename.c_str());


        // Print OpGraph if requested
        if(printop)
        {
            std::cout << std::endl << "[OPGRAPH]" << std::endl;
            opgraph->print_dot();
        }

        std::ifstream ini_file(exe_path + "/mapper_config.ini");
        if(!ini_file)
        {
            std::cout << "[ERROR] Missing mapper_config.ini, Cannot Read Default Parameters for Mappers" << std::endl;
            return 1;
        }
        std::string ini_str((std::istreambuf_iterator<char>(ini_file)), std::istreambuf_iterator<char>());

        mINI config_ini;
        std::map<std::string, std::string> mapper_args;
        if(config_ini.parse(ini_str)) // INI Parse OK
        {
            mapper_args = config_ini;
        }
        else // INI Parse ERROR
        {
            std::cout <<"[ERROR] Error in INI Parser, Check You mapper_config.ini" << std::endl;
            return 1;
        }

        std::stringstream ss_mapper_opts(mapper_opts);
        std::string opt_pair;
        while(ss_mapper_opts >> opt_pair)
        {
            auto n = opt_pair.find('=');
            if(n == std::string::npos)
            {
                std::cout <<"[WARNING] Ill Formated Mapper Option, Skipping: " << opt_pair << std::endl;
                continue;
            }
            auto key = opt_pair.substr(0, n);
            auto value = opt_pair.substr(n + 1, std::string::npos);
            auto it = mapper_args.find(key);
            if(it != mapper_args.end())
            {
                std::cout << "[INFO] Overwritting Mapper Parameter: " << key << " to " << value << " (Default: " << mapper_args.at(key) << ")" << std::endl;
                mapper_args[key] = value; // Overwrite default option
            }
            else
                std::cout << "[WARNING] Mapper Parameter: " << key << " doesn't exist, Skipping: " << key << " = " << value << std::endl;
        }

        std::cout << "[INFO] Creating Mapper..." << std::endl;
        auto mapper = Mapper::createMapper(mapper_type, arch, timelimit, mapper_args);

        Mapping mapping_result = mapper->mapOpGraph(opgraph, II);

        if(mapping_result.isMapped())
        {
            std::cout << std::endl;
            genMappingVisual(exe_path, mapping_result);
            mapping_result.outputMapping();

            if (make_testbench)
            {
                std::ofstream tb_file("testbench.v");
                arch->genBitStream(mapping_result).print_testbench(tb_file);
            }

            return 0;
        }
        else
            return 1;
    }
    catch(const cgrame_error & e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 1;
};

