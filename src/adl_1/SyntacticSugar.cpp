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
#include <vector>
#include <tuple>
#include <iostream>
#include <sstream>

#include <ctype.h>

#include "pugixml.hpp"

#include "ADLParser.h"
#include "ADLStructs.h"
#include "SyntacticSugar.h"

namespace adl1 {

//This function populates the xmlData with the "mesh" attributes that the user defines
bool create_orthogonal_mesh(ADLStructs &xmlData, pugi::xml_node mesh)
{
    //Gather the operating rows and cols for the mesh
    std::stringstream colRange, rowRange;
    std::string colRangeStr;
    int startCol, endCol;

    //ToDo: error checking for ranges
    colRangeStr = mesh.attribute("col-range").value();
    colRange << colRangeStr;
    colRange >> startCol >> endCol;

    std::string rowRangeStr;
    int startRow, endRow;

    rowRangeStr = mesh.attribute("row-range").value();
    rowRange << rowRangeStr;
    rowRange >> startRow >> endRow;

    //Create the IO template that the mesh will use to add
    moduleTemplate meshIO;
    meshIO.name = "meshIO";

    //ToDo: account for different sizes
    meshIO.addPort("in", PORT_INPUT, 32);
    meshIO.addPort("out", PORT_OUTPUT, 32);
    meshIO.addPort("bidir", PORT_BIDIR, 32);
    std::map<std::string, std::string> m;
    m.insert(std::pair<std::string, std::string>("name", "io"));
    m.insert(std::pair<std::string, std::string>("size", "32"));
    meshIO.primitives.push_back(std::pair<std::string, std::map<std::string, std::string>>("IO", m));
    meshIO.connections.push_back(std::make_tuple("to", "io.in", "from", "this.in"));
    meshIO.connections.push_back(std::make_tuple("to", "this.out", "from", "io.out"));
    meshIO.connections.push_back(std::make_tuple("to", "this.bidir", "from", "io.bidir"));
    xmlData.moduleTemplates.push_back(std::pair<std::string, moduleTemplate>(meshIO.name, meshIO));

    //Get the names of the ports to add
    std::string outNorth, outEast, outWest, outSouth, inNorth, inEast, inWest, inSouth;
    outNorth = mesh.attribute("out-north").value();
    outWest = mesh.attribute("out-west").value();
    outEast = mesh.attribute("out-east").value();
    outSouth = mesh.attribute("out-south").value();
    inNorth = mesh.attribute("in-north").value();
    inEast = mesh.attribute("in-east").value();
    inWest = mesh.attribute("in-west").value();
    inSouth = mesh.attribute("in-south").value();
    std::map<int, std::string> ports;
    ports.insert(std::make_pair(0, outNorth));
    ports.insert(std::make_pair(1, outEast));
    ports.insert(std::make_pair(2, outSouth));
    ports.insert(std::make_pair(3, outWest));
    ports.insert(std::make_pair(4, inNorth));
    ports.insert(std::make_pair(5, inEast));
    ports.insert(std::make_pair(6, inSouth));
    ports.insert(std::make_pair(7, inWest));

    //First, create the IOs based on the user description
    std::string ioLayout = mesh.attribute("io").value();
    if(ioLayout == "every-side-port") {
        //std::cout << "Adding IO blocks
        //Add the top and bottom IOs
        for(int i = startCol + 1; i < endCol; i++) {
            std::string name = "block_" + std::to_string(startRow) + "_" + std::to_string(i);
            xmlData.subModules.push_back(std::pair<std::string, moduleTemplate>(name, meshIO));

            //Connect this block to the block below it
            std::string toName = name + ".in";
            std::string fromName = "block_" + std::to_string(startRow + 1) + "_" + std::to_string(i) + outNorth;
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));
            toName = "block_" + std::to_string(startRow + 1) + "_" + std::to_string(i) + inNorth;
            fromName = name + ".out";
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));

            name = "block_" + std::to_string(endRow) + "_" + std::to_string(i);
            xmlData.subModules.push_back(std::pair<std::string, moduleTemplate>(name, meshIO));

            //Connect this to the block above it
            fromName = name + ".out";
            toName = "block_" + std::to_string(endRow - 1) + "_" + std::to_string(i) + inSouth;
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));
            toName = name + ".in";
            fromName = "block_" + std::to_string(endRow - 1) + "_" + std::to_string(i) + outSouth;
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));
        }

        //Add the left and right IOs
        for(int i = startRow + 1; i < endRow; i++) {
            std::string name = "block_" + std::to_string(i) + "_" + std::to_string(startCol);
            xmlData.subModules.push_back(std::pair<std::string, moduleTemplate>(name, meshIO));

            //Connect this to the right of it
            std::string toName = name + ".in";
            std::string fromName = "block_" + std::to_string(i) + "_" + std::to_string(startCol + 1) + outWest;
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));
            fromName = name + ".out";
            toName = "block_" + std::to_string(i) + "_" + std::to_string(startCol + 1) + inWest;
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));

            name = "block_" + std::to_string(i) + "_" + std::to_string(endCol);
            xmlData.subModules.push_back(std::pair<std::string, moduleTemplate>(name, meshIO));

            toName = name + ".in";
            fromName = "block_" + std::to_string(i) + "_" + std::to_string(endCol - 1) + outEast;
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));
            fromName = name + ".out";
            toName = "block_" + std::to_string(i) + "_" + std::to_string(endCol - 1) + inEast;
            xmlData.connections.push_back(std::make_tuple("to", toName, "from", fromName));
        }
    }

    //Create the interior
    auto interior = mesh.child("interior");
    auto colAttr = interior.attribute("cols");
    auto rowAttr = interior.attribute("rows");
    std::string colAttrValue = colAttr.value();
    std::string rowAttrValue = rowAttr.value();
    int colInc = (colAttr) ? atoi(colAttrValue.c_str()) : 1;
    int rowInc = (rowAttr) ? atoi(rowAttrValue.c_str()) : 1;

    //Add the blocks
    for(int row = startRow + 1; row < endRow; row++) {
        for(int col = startCol + 1; col < endCol; col++) {
            xmlData.arch.setCurrentRow(row);
            xmlData.arch.setCurrentCol(col);

            if(interior.select_nodes("block").size() > (colInc*rowInc)) {
                std::cout << "Too many block declared in the pattern.\n";
                return false;
            }

            for(auto elem : interior.children("block")) {
                std::string identifier = elem.attribute("module").value();
                std::string name = "block_" + std::to_string(xmlData.arch.getCurrentRow()) + "_" + std::to_string(xmlData.arch.getCurrentCol());

            moduleTemplate temp;
                for(auto elem : xmlData.moduleTemplates) {
                    if(elem.first == identifier) {
                        temp = elem.second;
                        std::cout << "Module " << identifier << " found within templates\n";
                    }
                }
                xmlData.subModules.push_back(std::pair<std::string, moduleTemplate>(name, temp));
                xmlData.arch.setCurrentCol(xmlData.arch.getCurrentCol() + 1);
                if(xmlData.arch.getCurrentCol() > xmlData.arch.getPatternEndCol()) {
                    xmlData.arch.setCurrentCol(col);
                    xmlData.arch.setCurrentRow(xmlData.arch.getCurrentRow() + rowInc);
                }
            }
        }
    }

    //Add the inner connections
    //I do not include this into the above for loop so I can keep this as modular as possible
    for(int row = startRow + 2; row < endRow-1; row++) {
        for(int col = startCol + 2; col < endCol - 1; col++) {
            xmlData.arch.setCurrentRow(row);
            xmlData.arch.setCurrentCol(col);
            connect_inner_mesh_block(xmlData, ports);
        }
    }

    //Top and bottom row excluding corners
    for(int i = startCol + 2; i < endCol - 1; i++) {
        std::string opBlock = create_xml_block(startRow + 1, i);
        std::string target = create_xml_block(startRow + 1, i - 1);
        std::string from = opBlock + outWest;
        std::string to = target + inEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(startRow + 1, i + 1);
        from = opBlock + outEast;
        to = target + inWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(startRow + 2, i);
        from = opBlock + outSouth;
        to = target + inNorth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        opBlock = create_xml_block(endRow - 1, i);
        target = create_xml_block(endRow - 1, i - 1);
        from = opBlock + outWest;
        to = target + outEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(endRow - 1, i + 1);
        from = opBlock + outEast;
        to = target + inWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(endRow - 2, i);
        from = opBlock + outNorth;
        to = target + inSouth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    }

    //Left and right row excluding corners
    for(int i = startRow + 2; i < endRow - 1; i++) {
        std::string opBlock = create_xml_block(i, startCol + 1);
        std::string target = create_xml_block(i - 1, startCol + 1);
        std::string from = opBlock + outNorth;
        std::string to = target + inSouth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i + 1, startCol + 1);
        from = opBlock + outSouth;
        to = target + inNorth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i, startCol + 2);
        from = opBlock + outEast;
        to = target + inWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        opBlock = create_xml_block(i, endCol - 1);
        target = create_xml_block(i - 1, endCol - 1);
        from = opBlock + outNorth;
        to = target + inSouth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i + 1, endCol - 1);
        from = opBlock + outSouth;
        to = target + inNorth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i, endCol - 2);
        from = opBlock + outWest;
        to = target + inEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
    }



    //Connecting corners
    std::string opBlock = "block_" + std::to_string(startRow + 1) + "_" + std::to_string(startCol + 1);
    std::string targetBlock = "block_" + std::to_string(startRow + 2) + "_" + std::to_string(startCol + 1);
    std::string from = opBlock + outSouth;
    std::string to = targetBlock + inNorth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    targetBlock = "block_" + std::to_string(startRow + 1) + "_" + std::to_string(startCol + 2);
    from = opBlock + outEast;
    to = targetBlock + inWest;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    opBlock = "block_" + std::to_string(endRow - 1) + "_" + std::to_string(startCol + 1);
    targetBlock = "block_" + std::to_string(endRow - 2) + "_" + std::to_string(startCol + 1);
    from = opBlock + outNorth;
    to = targetBlock + inSouth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    targetBlock = "block_" + std::to_string(endRow - 1) + "_" + std::to_string(startCol + 2);
    from = opBlock + outEast;
    to = targetBlock + inWest;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    opBlock = "block_" + std::to_string(startRow + 1) + "_" + std::to_string(endCol - 1);
    targetBlock = "block_" + std::to_string(startRow + 1) + "_" + std::to_string(endCol - 2);
    from = opBlock + outWest;
    to = targetBlock + inEast;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    targetBlock = "block_" + std::to_string(startRow + 2) + "_" + std::to_string(endCol - 1);
    from = opBlock + outSouth;
    to = targetBlock + inNorth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    opBlock = "block_" + std::to_string(endRow - 1) + "_" + std::to_string(endCol - 1);
    targetBlock = "block_" + std::to_string(endRow - 2) + "_" + std::to_string(endCol - 1);
    from = opBlock + outNorth;
    to = targetBlock + inSouth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    targetBlock = "block_" + std::to_string(endRow - 1) + "_" + std::to_string(endCol - 2);
    from = opBlock + outWest;
    to = targetBlock + inEast;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));


}

//This function populates the xmlData wit the "diagonal" attributes that the user defines
bool create_diag_mesh(ADLStructs &xmlData, pugi::xml_node diagonal)
{
    //Gather the operating rows and cols for the mesh
    std::stringstream colRange, rowRange;
    std::string colRangeStr;
    int startCol, endCol;

    //ToDo: error checking for ranges
    colRangeStr = diagonal.attribute("col-range").value();
    colRange << colRangeStr;
    colRange >> startCol >> endCol;

    std::string rowRangeStr;
    int startRow, endRow;

    rowRangeStr = diagonal.attribute("row-range").value();
    rowRange << rowRangeStr;
    rowRange >> startRow >> endRow;

    //Create the IO for the diagonal architecture
    moduleTemplate diagIO;
    diagIO.name = "diagIO";

    //Add primitives to diagIO
    std::map<std::string, std::string> m;
    m.insert(std::make_pair("name", "io0"));
    m.insert(std::make_pair("size", "32"));
    diagIO.primitives.push_back(std::make_pair("IO", m));
    (m.find("name"))->second = "io1";
    diagIO.primitives.push_back(std::make_pair("IO", m));
    (m.find("name"))->second = "io2";
    diagIO.primitives.push_back(std::make_pair("IO", m));
   // (m.find("name"))->second = "io3";
   // diagIO.primitives.push_back(std::make_pair("IO", m));

    //Add the ports
    for(int i = 0; i < 3; i++) {
        diagIO.ports["in" + std::to_string(i)]      = std::make_tuple(PORT_INPUT, 32);
        diagIO.ports["out" + std::to_string(i)]     = std::make_tuple(PORT_OUTPUT, 32);
        diagIO.ports["bidir" + std::to_string(i)]   = std::make_tuple(PORT_BIDIR, 32);
        diagIO.connections.push_back(std::make_tuple("to", "io" + std::to_string(i) + ".in", "from", "this.in" + std::to_string(i)));
        diagIO.connections.push_back(std::make_tuple("to", "this.out" + std::to_string(i), "from", "io" + std::to_string(i) + ".out"));
        diagIO.connections.push_back(std::make_tuple("to", "this.bidir" + std::to_string(i), "from", "io" + std::to_string(i) + ".bidir"));
    }

    //Create the corner module for the IO
    moduleTemplate cornerIO;
    cornerIO.name = "cornerIO";
    std::map<std::string, std::string> n;
    n.insert(std::make_pair("name", "io0"));
    n.insert(std::make_pair("size", "32"));
    cornerIO.primitives.push_back(std::make_pair("IO", n));
    (n.find("name"))->second = "io1";
    cornerIO.primitives.push_back(std::make_pair("IO", n));

    for(int i = 0; i < 2; i++) {
        cornerIO.addPort("in" + std::to_string(i), PORT_INPUT, 32);
        cornerIO.addPort("out" + std::to_string(i), PORT_OUTPUT, 32);
        cornerIO.addPort("bidir" + std::to_string(i), PORT_BIDIR, 32);
        cornerIO.connections.push_back(std::make_tuple("to", "io" + std::to_string(i) + ".in", "from", "this.in" + std::to_string(i)));
        cornerIO.connections.push_back(std::make_tuple("to", "this.out" + std::to_string(i), "from", "io" + std::to_string(i) + ".out"));
        cornerIO.connections.push_back(std::make_tuple("to", "this.bidir" + std::to_string(i), "from", "io" + std::to_string(i) + ".bidir"));

    }
    xmlData.moduleTemplates.push_back(std::pair<std::string, moduleTemplate>(cornerIO.name, cornerIO));
    xmlData.moduleTemplates.push_back(std::pair<std::string, moduleTemplate>(diagIO.name, diagIO));

    //Get the names of the ports
    //There is def an easier way to do this, I'm just being lazy (or is it being dumb?!?!?)
    std::string outNorth, outNorthEast, outEast, outSouthEast, outSouth, outSouthWest, outWest, outNorthWest, inNorth, inNorthEast, inEast, inSouthEast, inSouth, inSouthWest, inWest, inNorthWest;

    outNorth = diagonal.attribute("out-north").value();
    outNorthWest = diagonal.attribute("out-northwest").value();
    outWest = diagonal.attribute("out-west").value();
    outNorthEast = diagonal.attribute("out-northwest").value();
    outEast = diagonal.attribute("out-east").value();
    outSouthEast = diagonal.attribute("out-southeast").value();
    outSouthWest = diagonal.attribute("out-southwest").value();
    outSouth = diagonal.attribute("out-south").value();
    inNorth = diagonal.attribute("in-north").value();
    inNorthEast = diagonal.attribute("in-northeast").value();
    inEast = diagonal.attribute("in-east").value();
    inNorthWest = diagonal.attribute("in-northwest").value();
    inWest = diagonal.attribute("in-west").value();
    inSouthWest = diagonal.attribute("in-southwest").value();
    inSouth = diagonal.attribute("in-south").value();
    inSouthEast = diagonal.attribute("in-southeast").value();

    std::string ioLayout = diagonal.attribute("io").value();

    if(ioLayout == "every-side-port") {
        //Add top and bottom IOs
        for(int i = startCol + 1; i < endCol; i++) {
            std::string name = create_xml_block(startRow, i);
            if(i != startCol + 1 && i != endCol - 1) {
                xmlData.subModules.push_back(std::make_pair(name, diagIO));
                std::string target = create_xml_block(startRow + 1, i);
                xmlData.connections.push_back(std::make_tuple("to", target + inNorthWest, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inNorth, "from", name + ".out1"));
                xmlData.connections.push_back(std::make_tuple("to", target + inNorthEast, "from", name + ".out2"));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outNorthWest));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outNorth));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in2", "from", target + outNorthEast));
                xmlData.ports.push_back(std::make_tuple("io_top_" + std::to_string(i) + "NW", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_top_" + std::to_string(i) + "NW", "from", name + ".bidir0"));
                xmlData.ports.push_back(std::make_tuple("io_top_" + std::to_string(i) + "N", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_top_" + std::to_string(i) + "N", "from", name + ".bidir1"));
                xmlData.ports.push_back(std::make_tuple("io_top_" + std::to_string(i) + "NE", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_top_" + std::to_string(i) + "NE", "from", name + ".bidir2"));

                name = create_xml_block(endRow, i);
                xmlData.subModules.push_back(std::make_pair(name, diagIO));
                target = create_xml_block((endRow-1), i);
                xmlData.connections.push_back(std::make_tuple("to", target + inSouthWest, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inSouth, "from", name + ".out1"));
                xmlData.connections.push_back(std::make_tuple("to", target + inSouthEast, "from", name + ".out2"));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outSouthWest));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outSouth));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in2", "from", target + outSouthEast));
                xmlData.ports.push_back(std::make_tuple("io_bottom_" + std::to_string(i) + "SW", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_bottom_" + std::to_string(i) + "SW", "from", name + ".bidir0"));
                xmlData.ports.push_back(std::make_tuple("io_bottom_" + std::to_string(i) + "S", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_bottom_" + std::to_string(i) +"S", "from", name + ".bidir1"));
                xmlData.ports.push_back(std::make_tuple("io_bottom_" + std::to_string(i) + "SE", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_bottom_" + std::to_string(i) + "SE", "from", name + ".bidir2"));



            }
            else if(i == startCol + 1) {
                xmlData.subModules.push_back(std::make_pair(name, cornerIO));
                std::string target = create_xml_block(startRow + 1, i);
                //xmlData.connections.push_back(std::make_tuple("to", target + inNorthWest, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inNorth, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inNorthEast, "from", name + ".out1"));
                //xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outNorthWest));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outNorth));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outNorthEast));
                xmlData.ports.push_back(std::make_tuple("io_top_" + std::to_string(i) + "NW", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_top_" + std::to_string(i) + "NW", "from", name + ".bidir0"));
                xmlData.ports.push_back(std::make_tuple("io_top_" + std::to_string(i) + "N", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_top_" + std::to_string(i) + "N", "from", name + ".bidir1"));



                name = create_xml_block(endRow, i);
                xmlData.subModules.push_back(std::make_pair(name, cornerIO));
                target = create_xml_block((endRow-1), i);
                //xmlData.connections.push_back(std::make_tuple("to", target + inSouthWest, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inSouth, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inSouthEast, "from", name + ".out1"));
                //xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outSouthWest));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outSouth));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outSouthEast));
                xmlData.ports.push_back(std::make_tuple("io_bottom_" + std::to_string(i) + "SW", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_bottom_" + std::to_string(i) + "SW", "from", name + ".bidir0"));
                xmlData.ports.push_back(std::make_tuple("io_bottom_" + std::to_string(i) + "S", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_bottom_" + std::to_string(i) + "S", "from", name + ".bidir1"));

            }
            else if(i == endCol-1) {
                xmlData.subModules.push_back(std::make_pair(name, cornerIO));
                std::string target = create_xml_block(startRow + 1, i);
                xmlData.connections.push_back(std::make_tuple("to", target + inNorthWest, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inNorth, "from", name + ".out1"));
                //xmlData.connections.push_back(std::make_tuple("to", target + inNorthEast, "from", name + ".out2"));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outNorthWest));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outNorth));
                //xmlData.connections.push_back(std::make_tuple("to", name + ".in2", "from", target + outNorthEast));
                xmlData.ports.push_back(std::make_tuple("io_top_" + std::to_string(i) + "NW", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_top_" + std::to_string(i) + "NW", "from", name + ".bidir0"));
                xmlData.ports.push_back(std::make_tuple("io_top_" + std::to_string(i) + "N", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_top_" + std::to_string(i) + "N", "from", name + ".bidir1"));

                name = create_xml_block(endRow, i);
                xmlData.subModules.push_back(std::make_pair(name, cornerIO));
                target = create_xml_block((endRow-1), i);
                xmlData.connections.push_back(std::make_tuple("to", target + inSouthWest, "from", name + ".out0"));
                xmlData.connections.push_back(std::make_tuple("to", target + inSouth, "from", name + ".out1"));
                //xmlData.connections.push_back(std::make_tuple("to", target + inSouthEast, "from", name + ".out2"));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outSouthWest));
                xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outSouth));
                //xmlData.connections.push_back(std::make_tuple("to", name + ".in2", "from", target + outSouthEast));
                xmlData.ports.push_back(std::make_tuple("io_bottom_" + std::to_string(i) + "SW", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_bottom_" + std::to_string(i) + "SW", "from", name + ".bidir0"));
                xmlData.ports.push_back(std::make_tuple("io_bottom_" + std::to_string(i) + "S", PORT_BIDIR, 32));
                xmlData.connections.push_back(std::make_tuple("to", "this.io_bottom_" + std::to_string(i) + "S", "from", name + ".bidir1"));


            }
        }

        //Left and right IOs
        for(int i = startRow + 1; i < endRow; i++) {
            std::string name = create_xml_block(i, startCol);
            xmlData.subModules.push_back(std::make_pair(name, diagIO));
            std::string target = create_xml_block(i, startCol + 1);
            xmlData.connections.push_back(std::make_tuple("to", target + inNorthWest, "from", name + ".out0"));
            xmlData.connections.push_back(std::make_tuple("to", target + inWest, "from", name + ".out1"));
            xmlData.connections.push_back(std::make_tuple("to", target + inSouthWest, "from", name + ".out2"));
            xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outNorthWest));
            xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outWest));
            xmlData.connections.push_back(std::make_tuple("to", name + ".in2", "from", target + outSouthWest));
            xmlData.ports.push_back(std::make_tuple("io_left_" + std::to_string(i) + "NW", PORT_BIDIR, 32));
            xmlData.connections.push_back(std::make_tuple("to", "this.io_left_" + std::to_string(i) + "NW", "from", name + ".bidir0"));
            xmlData.ports.push_back(std::make_tuple("io_left_" + std::to_string(i) + "W", PORT_BIDIR, 32));
            xmlData.connections.push_back(std::make_tuple("to", "this.io_left_" + std::to_string(i) + "W", "from", name + ".bidir1"));
            xmlData.ports.push_back(std::make_tuple("io_left_" + std::to_string(i) + "SW", PORT_BIDIR, 32));
            xmlData.connections.push_back(std::make_tuple("to", "this.io_left_" + std::to_string(i) + "SW", "from", name + ".bidir2"));




            name = create_xml_block(i, endCol);
            xmlData.subModules.push_back(std::make_pair(name, diagIO));
            target = create_xml_block(i, endCol - 1);
            xmlData.connections.push_back(std::make_tuple("to", target + inNorthEast, "from", name + ".out0"));
            xmlData.connections.push_back(std::make_tuple("to", target + inEast, "from", name + ".out1"));
            xmlData.connections.push_back(std::make_tuple("to", target + inSouthEast, "from", name + ".out2"));
            xmlData.connections.push_back(std::make_tuple("to", name + ".in0", "from", target + outNorthEast));
            xmlData.connections.push_back(std::make_tuple("to", name + ".in1", "from", target + outEast));
            xmlData.connections.push_back(std::make_tuple("to", name + ".in2", "from", target + outSouthEast));
            xmlData.ports.push_back(std::make_tuple("io_right_" + std::to_string(i) + "NE", PORT_BIDIR, 32));
            xmlData.connections.push_back(std::make_tuple("to", "this.io_right_" + std::to_string(i) + "NE", "from", name + ".bidir0"));
            xmlData.ports.push_back(std::make_tuple("io_right_" + std::to_string(i) + "E", PORT_BIDIR, 32));
            xmlData.connections.push_back(std::make_tuple("to", "this.io_right_" + std::to_string(i) + "E", "from", name + ".bidir1"));
            xmlData.ports.push_back(std::make_tuple("io_right_" + std::to_string(i) + "SE", PORT_BIDIR, 32));
            xmlData.connections.push_back(std::make_tuple("to", "this.io_right_" + std::to_string(i) + "SE", "from", name + ".bidir2"));

        }
    }

    //Create the interior
    auto interior = diagonal.child("interior");
    auto colAttr = interior.attribute("cols");
    auto rowAttr = interior.attribute("rows");
    std::string colAttrValue = colAttr.value();
    std::string rowAttrValue = rowAttr.value();
    int colInc = (colAttr) ? atoi(colAttrValue.c_str()) : 1;
    int rowInc = (rowAttr) ? atoi(rowAttrValue.c_str()) : 1;

    //Add the blocks
    for(int row = startRow + 1; row < endRow; row++) {
        for(int col = startCol + 1; col < endCol; col++) {
            xmlData.arch.setCurrentRow(row);
            xmlData.arch.setCurrentCol(col);

            if(interior.select_nodes("block").size() > (colInc*rowInc)) {
                std::cout << "Too many block declared in the pattern.\n";
                return false;
            }

            for(auto elem : interior.children("block")) {
                std::string identifier = elem.attribute("module").value();
                std::string name = "block_" + std::to_string(xmlData.arch.getCurrentRow()) + "_" + std::to_string(xmlData.arch.getCurrentCol());

            moduleTemplate temp;
                for(auto elem : xmlData.moduleTemplates) {
                    if(elem.first == identifier) {
                        temp = elem.second;
                        std::cout << "Module " << identifier << " found within templates\n";
                    }
                }
                xmlData.subModules.push_back(std::pair<std::string, moduleTemplate>(name, temp));
                xmlData.arch.setCurrentCol(xmlData.arch.getCurrentCol() + 1);
                if(xmlData.arch.getCurrentCol() > xmlData.arch.getPatternEndCol()) {
                    xmlData.arch.setCurrentCol(col);
                    xmlData.arch.setCurrentRow(xmlData.arch.getCurrentRow() + rowInc);
                }
            }
        }
    }

    //Connect the inner blocks
    //I do not include this above because I want to make it modular
    for(int row = startRow + 2; row < endRow - 1; row++) {
        for(int col = startCol + 2; col < endCol - 1; col++) {
            std::string opBlock = create_xml_block(row, col);

            //Above Block
            std::string target = create_xml_block((row-1), col);
            std::string from = opBlock + outNorth;
            std::string to = target + inSouth;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

            //Above Right Block
            target = create_xml_block((row-1), col +1);
            from = opBlock + outNorthEast;
            to = target + inSouthWest;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

            //Above Left Block
            target = create_xml_block((row-1), col - 1);
            from = opBlock + outNorthWest;
            to = target + inSouthEast;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

            //Below Block
            target = create_xml_block((row+1), col);
            from = opBlock + outSouth;
            to = target + inNorth;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

            //Below Right Block
            target = create_xml_block((row+1), col + 1);
            from = opBlock + outSouthEast;
            to = target + inNorthWest;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

            //Below Left Block
            target = create_xml_block((row+1), col - 1);
            from = opBlock + outSouthWest;
            to = target + inNorthEast;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

            //Left Block
            target = create_xml_block((row), col-1);
            from = opBlock + outWest;
            to = target + inEast;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

            //Right Block
            target = create_xml_block((row), col+1);
            from = opBlock + outEast;
            to = target + inWest;
            xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
        }
    }

    //Connect top and bottom col
    for(int i = startCol + 2; i < endCol - 1; i++) {
        std::string opBlock = create_xml_block(startRow + 1, i);
        std::string target = create_xml_block(startRow + 1, i - 1);
        std::string from = opBlock + outWest;
        std::string to = target + inEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(startRow + 1, i + 1);
        from = opBlock + outEast;
        to = target + inWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(startRow + 2, i);
        from = opBlock + outSouth;
        to = target + inNorth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(startRow + 2, i - 1);
        from = opBlock + outSouthWest;
        to = target + inNorthEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(startRow + 2, i + 1);
        from = opBlock + outSouthEast;
        to = target + inNorthWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        opBlock = create_xml_block(endRow - 1, i);
        target = create_xml_block(endRow - 1, i - 1);
        from = opBlock + outWest;
        to = target + inEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(endRow - 1, i + 1);
        from = opBlock + outEast;
        to = target + inWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(endRow - 2, i);
        from = opBlock + outNorth;
        to = target + inSouth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(endRow - 2, i - 1);
        from = opBlock + outNorthWest;
        to = target + inSouthEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(endRow - 2, i + 1);
        from = opBlock + outNorthEast;
        to = target + inSouthWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
    }

    //Connect the left and right row
    for(int i = startRow + 2; i < endRow - 1; i++) {
        std::string opBlock = create_xml_block(i, startCol + 1);
        std::string target = create_xml_block(i - 1, startCol + 1);
        std::string from = opBlock + outNorth;
        std::string to = target + inSouth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i + 1, startCol + 1);
        from = opBlock + outSouth;
        to = target + inNorth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i, startCol + 2);
        from = opBlock + outEast;
        to = target + inWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i-1, startCol + 2);
        from = opBlock + outNorthEast;
        to = target + inSouthWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i + 1, startCol + 2);
        from = opBlock + outSouthEast;
        to = target + inNorthWest;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        opBlock = create_xml_block(i, endCol - 1);
        target = create_xml_block(i - 1, endCol - 1);
        from = opBlock + outNorth;
        to = target + inSouth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i + 1, endCol - 1);
        from = opBlock + outSouth;
        to = target + inNorth;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i, endCol - 2);
        from = opBlock + outWest;
        to = target + inEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i - 1, endCol - 2);
        from = opBlock + outNorthWest;
        to = target + inSouthEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

        target = create_xml_block(i + 1, endCol - 2);
        from = opBlock + outSouthWest;
        to = target + inNorthEast;
        xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
    }
    //Corners
    //Top left block
    std::string opBlock = create_xml_block(startRow + 1, startCol + 1);
    std::string target = create_xml_block(startRow + 1, startCol + 2);
    std::string from = opBlock + outEast;
    std::string to = target + inWest;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(startRow + 2, startCol + 2);
    from = opBlock + outSouthEast;
    to = target + inNorthWest;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(startRow + 2, startCol + 1);
    from = opBlock + outSouth;
    to = target + inNorth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    opBlock = create_xml_block(endRow - 1, startCol +  1);
    target = create_xml_block(endRow - 1, startCol + 2);
    from = opBlock + outEast;
    to = target + inWest;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(endRow - 2, startCol + 2);
    from = opBlock + outNorthEast;
    to = target + inSouthWest;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(endRow - 2, startCol + 1);
    from = opBlock + outNorth;
    to = target + inSouth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    opBlock = create_xml_block(startRow + 1, endCol - 1);
    target = create_xml_block(startRow + 1, endCol - 2);
    from = opBlock + outWest;
    to = target + inEast;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(startRow + 2, endCol - 2);
    from = opBlock + outSouthWest;
    to = target + inNorthEast;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(startRow + 2, endCol - 1);
    from = opBlock + outSouth;
    to = target + inNorth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    opBlock = create_xml_block(endRow - 1, endCol - 1);
    target = create_xml_block(endRow - 2, endCol - 1);
    from = opBlock + outNorth;
    to = target + inSouth;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(endRow - 2, endCol - 2);
    from = opBlock + outNorthWest;
    to = target + inSouthEast;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    target = create_xml_block(endRow - 1, endCol - 2);
    from = opBlock + outWest;
    to = target + inEast;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
}

//This function creates the connections for a block on the inside of a mesh pattern
bool connect_inner_mesh_block(ADLStructs &xmlData, std::map<int, std::string> ports)
{
    int row = xmlData.arch.getCurrentRow();
    int col = xmlData.arch.getCurrentCol();
    std::string opBlock = "block_" + std::to_string(row) + "_" + std::to_string(col);

    //Above block
    std::string targetBlock = "block_" + std::to_string(row-1) + "_" + std::to_string(col);
    std::string from = opBlock + (ports.find(0)->second);
    std::string to = targetBlock + (ports.find(6)->second);
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    //Bottom block
    targetBlock = "block_" + std::to_string(row+1) + "_" + std::to_string(col);
    from = opBlock + (ports.find(2)->second);
    to = targetBlock + (ports.find(4)->second);
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    //West block
    targetBlock = "block_" + std::to_string(row) + "_" + std::to_string(col-1);
    from = opBlock + ports.find(3)->second;
    to = targetBlock + ports.find(5)->second;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));

    //West block
    targetBlock = "block_" + std::to_string(row) + "_" + std::to_string(col + 1);
    from = opBlock + ports.find(1)->second;
    to = targetBlock + ports.find(7)->second;
    xmlData.connections.push_back(std::make_tuple("to", to, "from", from));
}

}

