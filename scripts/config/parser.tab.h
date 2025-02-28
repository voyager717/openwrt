<<<<<<< HEAD
/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.
=======
/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
<<<<<<< HEAD
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */
=======
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

<<<<<<< HEAD
/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

=======
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

<<<<<<< HEAD
/* Token kinds.  */
=======
/* Token type.  */
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
<<<<<<< HEAD
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_HELPTEXT = 258,              /* T_HELPTEXT  */
    T_WORD = 259,                  /* T_WORD  */
    T_WORD_QUOTE = 260,            /* T_WORD_QUOTE  */
    T_BOOL = 261,                  /* T_BOOL  */
    T_CHOICE = 262,                /* T_CHOICE  */
    T_CLOSE_PAREN = 263,           /* T_CLOSE_PAREN  */
    T_COLON_EQUAL = 264,           /* T_COLON_EQUAL  */
    T_COMMENT = 265,               /* T_COMMENT  */
    T_CONFIG = 266,                /* T_CONFIG  */
    T_DEFAULT = 267,               /* T_DEFAULT  */
    T_DEF_BOOL = 268,              /* T_DEF_BOOL  */
    T_DEF_TRISTATE = 269,          /* T_DEF_TRISTATE  */
    T_DEPENDS = 270,               /* T_DEPENDS  */
    T_ENDCHOICE = 271,             /* T_ENDCHOICE  */
    T_ENDIF = 272,                 /* T_ENDIF  */
    T_ENDMENU = 273,               /* T_ENDMENU  */
    T_HELP = 274,                  /* T_HELP  */
    T_HEX = 275,                   /* T_HEX  */
    T_IF = 276,                    /* T_IF  */
    T_IMPLY = 277,                 /* T_IMPLY  */
    T_INT = 278,                   /* T_INT  */
    T_MAINMENU = 279,              /* T_MAINMENU  */
    T_MENU = 280,                  /* T_MENU  */
    T_MENUCONFIG = 281,            /* T_MENUCONFIG  */
    T_MODULES = 282,               /* T_MODULES  */
    T_ON = 283,                    /* T_ON  */
    T_OPEN_PAREN = 284,            /* T_OPEN_PAREN  */
    T_OPTIONAL = 285,              /* T_OPTIONAL  */
    T_PLUS_EQUAL = 286,            /* T_PLUS_EQUAL  */
    T_PROMPT = 287,                /* T_PROMPT  */
    T_RANGE = 288,                 /* T_RANGE  */
    T_RESET = 289,                 /* T_RESET  */
    T_SELECT = 290,                /* T_SELECT  */
    T_SOURCE = 291,                /* T_SOURCE  */
    T_STRING = 292,                /* T_STRING  */
    T_TRISTATE = 293,              /* T_TRISTATE  */
    T_VISIBLE = 294,               /* T_VISIBLE  */
    T_EOL = 295,                   /* T_EOL  */
    T_ASSIGN_VAL = 296,            /* T_ASSIGN_VAL  */
    T_OR = 297,                    /* T_OR  */
    T_AND = 298,                   /* T_AND  */
    T_EQUAL = 299,                 /* T_EQUAL  */
    T_UNEQUAL = 300,               /* T_UNEQUAL  */
    T_LESS = 301,                  /* T_LESS  */
    T_LESS_EQUAL = 302,            /* T_LESS_EQUAL  */
    T_GREATER = 303,               /* T_GREATER  */
    T_GREATER_EQUAL = 304,         /* T_GREATER_EQUAL  */
    T_NOT = 305                    /* T_NOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
=======
    T_HELPTEXT = 258,
    T_WORD = 259,
    T_WORD_QUOTE = 260,
    T_ALLNOCONFIG_Y = 261,
    T_BOOL = 262,
    T_CHOICE = 263,
    T_CLOSE_PAREN = 264,
    T_COLON_EQUAL = 265,
    T_COMMENT = 266,
    T_CONFIG = 267,
    T_DEFAULT = 268,
    T_DEFCONFIG_LIST = 269,
    T_DEF_BOOL = 270,
    T_DEF_TRISTATE = 271,
    T_DEPENDS = 272,
    T_ENDCHOICE = 273,
    T_ENDIF = 274,
    T_ENDMENU = 275,
    T_HELP = 276,
    T_HEX = 277,
    T_IF = 278,
    T_IMPLY = 279,
    T_INT = 280,
    T_MAINMENU = 281,
    T_MENU = 282,
    T_MENUCONFIG = 283,
    T_MODULES = 284,
    T_ON = 285,
    T_OPEN_PAREN = 286,
    T_OPTION = 287,
    T_OPTIONAL = 288,
    T_PLUS_EQUAL = 289,
    T_PROMPT = 290,
    T_RANGE = 291,
    T_RESET = 292,
    T_SELECT = 293,
    T_SOURCE = 294,
    T_STRING = 295,
    T_TRISTATE = 296,
    T_VISIBLE = 297,
    T_EOL = 298,
    T_ASSIGN_VAL = 299,
    T_OR = 300,
    T_AND = 301,
    T_EQUAL = 302,
    T_UNEQUAL = 303,
    T_LESS = 304,
    T_LESS_EQUAL = 305,
    T_GREATER = 306,
    T_GREATER_EQUAL = 307,
    T_NOT = 308
  };
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
<<<<<<< HEAD
union YYSTYPE
{

=======

union YYSTYPE
{


>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	char *string;
	struct symbol *symbol;
	struct expr *expr;
	struct menu *menu;
	enum symbol_type type;
	enum variable_flavor flavor;


};
<<<<<<< HEAD
=======

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

<<<<<<< HEAD

int yyparse (void);


=======
int yyparse (void);

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
