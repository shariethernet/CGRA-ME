// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.

#line 37 "dfgdot.tab.cc" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "dfgdot.tab.h"

// User implementation prologue.

#line 51 "dfgdot.tab.cc" // lalr1.cc:412
// Unqualified %code blocks.
#line 37 "dfgdot.y" // lalr1.cc:413

#include "dfgdot_driver.h"

#line 57 "dfgdot.tab.cc" // lalr1.cc:413


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
#line 143 "dfgdot.tab.cc" // lalr1.cc:479

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (driver& d_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      d (d_yyarg)
  {}

  parser::~parser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  inline
  parser::by_state::by_state ()
    : state (empty_state)
  {}

  inline
  parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  parser::by_state::clear ()
  {
    state = empty_state;
  }

  inline
  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  inline
  parser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  parser::symbol_number_type
  parser::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  inline
  parser::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  parser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
      switch (that.type_get ())
    {
      case 16: // attr_list
      case 17: // a_list
      case 18: // kvp
        value.move< std::map<std::string, std::string> > (that.value);
        break;

      case 3: // ID
      case 4: // edgeop
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

  inline
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
      switch (that.type_get ())
    {
      case 16: // attr_list
      case 17: // a_list
      case 18: // kvp
        value.copy< std::map<std::string, std::string> > (that.value);
        break;

      case 3: // ID
      case 4: // edgeop
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  inline
  void
  parser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  parser::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  inline parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::parse ()
  {
    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    // User initialization code.
    #line 24 "dfgdot.y" // lalr1.cc:741
{
  // Initialize the initial location.
  yyla.location.begin.filename = yyla.location.end.filename = &d.file;
  // clear out nodes map
  d.nodes.clear();
  d.edges.clear();
}

#line 442 "dfgdot.tab.cc" // lalr1.cc:741

    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            symbol_type yylookahead (yylex (d));
            yyla.move (yylookahead);
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
        switch (yyr1_[yyn])
    {
      case 16: // attr_list
      case 17: // a_list
      case 18: // kvp
        yylhs.value.build< std::map<std::string, std::string> > ();
        break;

      case 3: // ID
      case 4: // edgeop
        yylhs.value.build< std::string > ();
        break;

      default:
        break;
    }


      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 2:
#line 52 "dfgdot.y" // lalr1.cc:859
    { d.finalize(); }
#line 563 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 3:
#line 53 "dfgdot.y" // lalr1.cc:859
    {
                try
                {
                    d.count = std::stoi(yystack_[3].value.as< std::string > ());
                }
                catch(const std::invalid_argument& ia)
                {
                #ifdef DEBUG
                    std::cerr << "Invalid argument: " << ia.what() << '\n';
                #endif
                }
                
                d.finalize();
            }
#line 582 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 8:
#line 90 "dfgdot.y" // lalr1.cc:859
    {yylhs.value.as< std::map<std::string, std::string> > () = std::map<std::string, std::string>();}
#line 588 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 10:
#line 92 "dfgdot.y" // lalr1.cc:859
    {yylhs.value.as< std::map<std::string, std::string> > () = yystack_[1].value.as< std::map<std::string, std::string> > ();}
#line 594 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 11:
#line 93 "dfgdot.y" // lalr1.cc:859
    {(yystack_[3].value.as< std::map<std::string, std::string> > ()).insert((yystack_[1].value.as< std::map<std::string, std::string> > ()).begin(), (yystack_[1].value.as< std::map<std::string, std::string> > ()).end()); yylhs.value.as< std::map<std::string, std::string> > () = yystack_[3].value.as< std::map<std::string, std::string> > ();}
#line 600 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 12:
#line 95 "dfgdot.y" // lalr1.cc:859
    {(yystack_[1].value.as< std::map<std::string, std::string> > ()).insert((yystack_[0].value.as< std::map<std::string, std::string> > ()).begin(), (yystack_[0].value.as< std::map<std::string, std::string> > ()).end()); yylhs.value.as< std::map<std::string, std::string> > () = yystack_[1].value.as< std::map<std::string, std::string> > ();}
#line 606 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 13:
#line 96 "dfgdot.y" // lalr1.cc:859
    { yylhs.value.as< std::map<std::string, std::string> > () = yystack_[0].value.as< std::map<std::string, std::string> > ();}
#line 612 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 14:
#line 98 "dfgdot.y" // lalr1.cc:859
    {std::map<std::string, std::string> temp; temp[((std::string)(yystack_[0].value.as< std::string > ()))] = ((std::string)("")); yylhs.value.as< std::map<std::string, std::string> > () = temp;}
#line 618 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 15:
#line 99 "dfgdot.y" // lalr1.cc:859
    {std::map<std::string, std::string> temp; temp[((std::string)(yystack_[1].value.as< std::string > ()))] = ((std::string)("")); yylhs.value.as< std::map<std::string, std::string> > () = temp;}
#line 624 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 16:
#line 100 "dfgdot.y" // lalr1.cc:859
    {std::map<std::string, std::string> temp; temp[((std::string)(yystack_[2].value.as< std::string > ()))] = ((std::string)(yystack_[0].value.as< std::string > ())); yylhs.value.as< std::map<std::string, std::string> > () = temp;}
#line 630 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 17:
#line 112 "dfgdot.y" // lalr1.cc:859
    {
                    if(d.nodes.find(yystack_[3].value.as< std::string > ()) == d.nodes.end())
                    {
                        YYABORT;
                        /*d.nodes[$1] = new OpGraphOp();*/
                    }

                    if(d.nodes.find(yystack_[1].value.as< std::string > ()) == d.nodes.end())
                    {
                        YYABORT;
                        /*d.nodes[$3] = new OpGraphOp();*/
                    }

                    if(d.edges.find(yystack_[3].value.as< std::string > ()) == d.edges.end())
                    {
                        d.edges[yystack_[3].value.as< std::string > ()] = new OpGraphVal(yystack_[3].value.as< std::string > () + "_val_output");
                      /* 
                        if(d.inputnodes.find($1) != d.inputnodes.end() &&
                           d.outputnodes.find($3) != d.outputnodes.end())
                        {*/
                            d.opgraph->val_nodes.push_back(d.edges[yystack_[3].value.as< std::string > ()]);
                        /*}*/
                    }

                    d.edges[yystack_[3].value.as< std::string > ()]->input = d.nodes[yystack_[3].value.as< std::string > ()];
                    d.nodes[yystack_[3].value.as< std::string > ()]->output = d.edges[yystack_[3].value.as< std::string > ()];

                    unsigned int operand;
                    try
                    {
                        operand = std::stoi(yystack_[0].value.as< std::map<std::string, std::string> > ()["operand"]);
                    }
                    catch(const std::invalid_argument& ia)
                    {
                        std::cerr << "Invalid argument: " << ia.what() << '\n';
                        YYABORT;
                    }
                    
                    if(d.nodes[yystack_[1].value.as< std::string > ()]->input.size() <= operand)
                    {
                        d.nodes[yystack_[1].value.as< std::string > ()]->input.resize(operand + 1);
                    }

                    d.nodes[yystack_[1].value.as< std::string > ()]->input[operand] = d.edges[yystack_[3].value.as< std::string > ()];
                    d.edges[yystack_[3].value.as< std::string > ()]->output.push_back(d.nodes[yystack_[1].value.as< std::string > ()]);
                    d.edges[yystack_[3].value.as< std::string > ()]->output_operand.push_back(operand);

                }
#line 683 "dfgdot.tab.cc" // lalr1.cc:859
    break;

  case 18:
#line 169 "dfgdot.y" // lalr1.cc:859
    {
                                if(d.nodes.find(yystack_[1].value.as< std::string > ()) != d.nodes.end())
                                    d.nodes[yystack_[1].value.as< std::string > ()]->parserUpdate(yystack_[0].value.as< std::map<std::string, std::string> > ());
                                else
                                {
                                    d.nodes[yystack_[1].value.as< std::string > ()] = new OpGraphOp(yystack_[1].value.as< std::string > ());
                                    d.nodes[yystack_[1].value.as< std::string > ()]->parserUpdate(yystack_[0].value.as< std::map<std::string, std::string> > ());

                                    if(yystack_[0].value.as< std::map<std::string, std::string> > ().find("input") != yystack_[0].value.as< std::map<std::string, std::string> > ().end())
                                    {
                                        d.inputnodes[yystack_[1].value.as< std::string > ()] = d.nodes[yystack_[1].value.as< std::string > ()];
                                    }
                                    else if(yystack_[0].value.as< std::map<std::string, std::string> > ().find("output") != yystack_[0].value.as< std::map<std::string, std::string> > ().end())
                                    {
                                        d.outputnodes[yystack_[1].value.as< std::string > ()] = d.nodes[yystack_[1].value.as< std::string > ()];
                                    }
                                    /*
                                    else
                                    {
                                        d.opgraph->op_nodes.push_back(d.nodes[$1]);
                                    }
                                    */
                                    d.opgraph->op_nodes.push_back(d.nodes[yystack_[1].value.as< std::string > ()]);
                                }
                           }
#line 713 "dfgdot.tab.cc" // lalr1.cc:859
    break;


#line 717 "dfgdot.tab.cc" // lalr1.cc:859
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -25;

  const signed char parser::yytable_ninf_ = -1;

  const signed char
  parser::yypact_[] =
  {
      15,    16,    21,     9,    20,   -25,    20,     8,    -1,   -25,
      13,    14,    10,    23,    -2,    19,   -25,   -25,   -25,   -25,
     -25,    22,    18,   -25,     0,   -25,     1,    19,    26,   -25,
     -25,   -25,     2,   -25,   -25
  };

  const unsigned char
  parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     1,     0,     0,     0,     5,
       0,     0,     0,     0,     0,    18,     2,     4,     7,     6,
       3,     0,    14,     8,     0,    13,     0,    17,    15,    10,
      12,     9,     0,    16,    11
  };

  const signed char
  parser::yypgoto_[] =
  {
     -25,   -25,    25,     6,    11,     7,   -24,   -25,   -25
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     2,     8,     9,    15,    24,    25,    10,    11
  };

  const unsigned char
  parser::yytable_[] =
  {
      30,    22,     7,    22,    22,    22,    16,    23,    30,    29,
      31,    34,    13,     7,    17,     6,    14,    20,    17,     3,
       1,     5,     4,     7,    18,    19,    21,    26,    28,    33,
      14,    12,    27,    32
  };

  const unsigned char
  parser::yycheck_[] =
  {
      24,     3,     3,     3,     3,     3,     7,     9,    32,     9,
       9,     9,     4,     3,     8,     6,     8,     7,    12,     3,
       5,     0,     6,     3,    11,    11,     3,     8,    10,     3,
       8,     6,    21,    26
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     5,    13,     3,     6,     0,     6,     3,    14,    15,
      19,    20,    14,     4,     8,    16,     7,    15,    11,    11,
       7,     3,     3,     9,    17,    18,     8,    16,    10,     9,
      18,     9,    17,     3,     9
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    12,    13,    13,    14,    14,    15,    15,    16,    16,
      16,    16,    17,    17,    18,    18,    18,    19,    20
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     4,     5,     2,     1,     2,     2,     2,     3,
       3,     4,     2,     1,     1,     2,     3,     4,     2
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "ID", "edgeop", "DIGRAPH",
  "OB", "CB", "OS", "CS", "EQ", "SEMI", "$accept", "graph", "stmt_list",
  "stmt", "attr_list", "a_list", "kvp", "edge_stmt", "node_stmt", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned char
  parser::yyrline_[] =
  {
       0,    52,    52,    53,    74,    75,    78,    79,    90,    91,
      92,    93,    95,    96,    98,    99,   100,   112,   169
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG



} // yy
#line 1099 "dfgdot.tab.cc" // lalr1.cc:1167
#line 220 "dfgdot.y" // lalr1.cc:1168


void
yy::parser::error (const location_type& l, const std::string& m)
{
    d.error (l, m);
}
