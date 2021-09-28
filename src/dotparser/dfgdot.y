/* Mini Calculator */
/* calc.y */
%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.2"
%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
#include <iostream>
#include <string>
#include <map>

#include <CGRA/OpGraph.h>

class driver;

}
%param { driver& d}
%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &d.file;
  // clear out nodes map
  d.nodes.clear();
  d.edges.clear();
};

 %define parse.trace
 %define parse.error verbose


%code
{
#include "dfgdot_driver.h"
}

%start	graph
%token
  END  0  "end of file"
%token	<std::string>    ID
%token	<std::string>    edgeop
%token	DIGRAPH
%token	OB CB OS CS EQ SEMI
%type <std::map<std::string, std::string>>  a_list attr_list kvp

%%
graph       : DIGRAPH OB stmt_list CB { d.finalize(); }
            | DIGRAPH ID OB stmt_list CB {
                try
                {
                    d.count = std::stoi($2);
                }
                catch(const std::invalid_argument& ia)
                {
                #ifdef DEBUG
                    std::cerr << "Invalid argument: " << ia.what() << '\n';
                #endif
                }

                d.finalize();
            }

stmt_list   : stmt_list stmt
            | stmt


stmt        :   node_stmt SEMI
            |   edge_stmt SEMI

attr_list   :   OS CS {$$ = std::map<std::string, std::string>();}
            |   attr_list OS CS
            |   OS a_list CS {$$ = $2;}
            |   attr_list OS a_list CS {($1).insert(($3).begin(), ($3).end()); $$ = $1;}

a_list      :  a_list kvp {($1).insert(($2).begin(), ($2).end()); $$ = $1;}
            |  kvp { $$ = $1;}

kvp         :   ID {std::map<std::string, std::string> temp; temp[((std::string)($1))] = ((std::string)("")); $$ = temp;}
            |   ID EQ {std::map<std::string, std::string> temp; temp[((std::string)($1))] = ((std::string)("")); $$ = temp;}
            |   ID EQ ID {std::map<std::string, std::string> temp; temp[((std::string)($1))] = ((std::string)($3)); $$ = temp;}

edge_stmt   :   ID edgeop ID attr_list {
                    if(d.nodes.find($1) == d.nodes.end())
                    {
                        YYABORT;
                        /*d.nodes[$1] = new OpGraphOp();*/
                    }

                    if(d.nodes.find($3) == d.nodes.end())
                    {
                        YYABORT;
                        /*d.nodes[$3] = new OpGraphOp();*/
                    }

                    if(d.edges.find($1) == d.edges.end())
                    {
                        d.edges[$1] = new OpGraphVal($1 + "_val_output");
                      /*
                        if(d.inputnodes.find($1) != d.inputnodes.end() &&
                           d.outputnodes.find($3) != d.outputnodes.end())
                        {*/
                            d.opgraph->val_nodes.push_back(d.edges[$1]);
                        /*}*/
                    }

                    d.edges[$1]->input = d.nodes[$1];
                    d.nodes[$1]->output = d.edges[$1];

                    unsigned int operand;
                    try
                    {
                        operand = std::stoi($4["operand"]);
                    }
                    catch(const std::invalid_argument& ia)
                    {
                        std::cerr << "Invalid argument: " << ia.what() << '\n';
                        YYABORT;
                    }

                    if(d.nodes[$3]->input.size() <= operand)
                    {
                        d.nodes[$3]->input.resize(operand + 1);
                    }

                    d.nodes[$3]->input[operand] = d.edges[$1];
                    d.edges[$1]->output.push_back(d.nodes[$3]);
                    d.edges[$1]->output_operand.push_back(operand);

                }

node_stmt   : ID attr_list {
                                if(d.nodes.find($1) != d.nodes.end())
                                    d.nodes[$1]->parserUpdate($2);
                                else
                                {
                                    d.nodes[$1] = new OpGraphOp($1);
                                    d.nodes[$1]->parserUpdate($2);

                                    if($2.find("input") != $2.end())
                                    {
                                        d.inputnodes[$1] = d.nodes[$1];
                                    }
                                    else if($2.find("output") != $2.end())
                                    {
                                        d.outputnodes[$1] = d.nodes[$1];
                                    }
                                    /*
                                    else
                                    {
                                        d.opgraph->op_nodes.push_back(d.nodes[$1]);
                                    }
                                    */
                                    d.opgraph->op_nodes.push_back(d.nodes[$1]);
                                }
                           }
%%

void yy::parser::error (const location_type& l, const std::string& m)
{
    d.error (l, m);
}
