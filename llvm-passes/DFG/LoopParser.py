#!/usr/bin/env python3

'''
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
'''

import sys
import os
import string

def loop_parser(input_source, output_source, output_tag):

    #Generate absolute path from reletive path
    dir = os.path.dirname(os.path.realpath("__file__"))
    input_src_path = os.path.join(dir, input_source)
    output_src_path = os.path.join(dir, output_source)
    output_tag_path = os.path.join(dir, output_tag)

    input_src_f = open(input_src_path, 'r')
    output_src_f = open(output_src_path, 'w')
    output_tag_f = open(output_tag_path, 'w')

    tag_string = "//DFGLoop:"
    tag_count = 1

    output_src_f.write("__attribute__((noinline)) void DFGLOOP_TAG(int index);\n")

    for line in input_src_f:
        tag_loc = line.find(tag_string)
        if tag_loc != -1: #Found a tag
            tag_name = line[tag_loc + len(tag_string) : -1].strip()
            output_tag_f.write(str(tag_count) + ' ' + tag_name + '\n')
            output_src_f.write("DFGLOOP_TAG(" + str(tag_count) + ");\n")
            tag_count+=1
        else:
            output_src_f.write(line)

    input_src_f.close()
    output_src_f.close()
    output_tag_f.close()
    return

if __name__ == "__main__":
    loop_parser(sys.argv[1], sys.argv[2], sys.argv[3])
