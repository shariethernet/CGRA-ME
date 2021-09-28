/* Mini Calculator */
/* calc.lex */

%{
#include <string>
#include <iostream>
#include "dfgdot_driver.h"
#include "dfgdot.tab.h"

//int yyerror(char *s);

static yy::location loc;
%}

%option noyywrap nounput batch debug noinput
%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  // Code run each time yylex is called.
  loc.step();
%}

[ \t]+   loc.step();
[\n]     loc.lines(yyleng); loc.step();

"["   { return yy::parser::make_OS(loc);}
"]"   { return yy::parser::make_CS(loc);}
"{"   { return yy::parser::make_OB(loc);}
"}"   { return yy::parser::make_CB(loc);}
"="   { return yy::parser::make_EQ(loc);}
";"   { return yy::parser::make_SEMI(loc);}
"digraph"   { return yy::parser::make_DIGRAPH(loc);}
"->"        { return yy::parser::make_edgeop(std::string(yytext),loc);}

\"[a-zA-Z_0-9]+\"           { return yy::parser::make_ID(std::string(yytext),loc);}
[a-zA-Z_]+[0-9]*            { return yy::parser::make_ID(std::string(yytext),loc);}
[a-zA-Z\200-\377_]+[0-9]*   { return yy::parser::make_ID(std::string(yytext),loc);}
[0-9]+                      { return yy::parser::make_ID(std::string(yytext),loc);}

"//"[^\n]* {loc.step(); continue;}

.          d.error(loc, "invalid character");
<<EOF>>    return yy::parser::make_END(loc);
%%


int driver::scan_begin ()
{
    int res = 0;

    yy_flex_debug = trace_scanning;

    if (!(yyin = fopen (file.c_str (), "r")))
    {
        error ("cannot open " + file + ": " + strerror(errno));
        return 1;
    }
    yyrestart(yyin);
    return 0;
}

void driver::scan_end()
{
    fclose (yyin);
}

