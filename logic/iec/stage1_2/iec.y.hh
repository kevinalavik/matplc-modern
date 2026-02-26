/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_IEC_Y_HH_INCLUDED
# define YY_YY_IEC_Y_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    prev_declared_variable_name_token = 258, /* prev_declared_variable_name_token  */
    prev_declared_fb_name_token = 259, /* prev_declared_fb_name_token  */
    prev_declared_simple_type_name_token = 260, /* prev_declared_simple_type_name_token  */
    prev_declared_subrange_type_name_token = 261, /* prev_declared_subrange_type_name_token  */
    prev_declared_enumerated_type_name_token = 262, /* prev_declared_enumerated_type_name_token  */
    prev_declared_array_type_name_token = 263, /* prev_declared_array_type_name_token  */
    prev_declared_structure_type_name_token = 264, /* prev_declared_structure_type_name_token  */
    prev_declared_string_type_name_token = 265, /* prev_declared_string_type_name_token  */
    prev_declared_derived_function_name_token = 266, /* prev_declared_derived_function_name_token  */
    prev_declared_derived_function_block_name_token = 267, /* prev_declared_derived_function_block_name_token  */
    prev_declared_program_type_name_token = 268, /* prev_declared_program_type_name_token  */
    BOGUS_TOKEN_ID = 269,          /* BOGUS_TOKEN_ID  */
    pragma_token = 270,            /* pragma_token  */
    EN = 271,                      /* EN  */
    ENO = 272,                     /* ENO  */
    identifier_token = 273,        /* identifier_token  */
    integer_token = 274,           /* integer_token  */
    binary_integer_token = 275,    /* binary_integer_token  */
    octal_integer_token = 276,     /* octal_integer_token  */
    hex_integer_token = 277,       /* hex_integer_token  */
    real_token = 278,              /* real_token  */
    FALSE = 279,                   /* FALSE  */
    TRUE = 280,                    /* TRUE  */
    single_byte_character_string_token = 281, /* single_byte_character_string_token  */
    double_byte_character_string_token = 282, /* double_byte_character_string_token  */
    fixed_point_token = 283,       /* fixed_point_token  */
    fixed_point_d_token = 284,     /* fixed_point_d_token  */
    integer_d_token = 285,         /* integer_d_token  */
    fixed_point_h_token = 286,     /* fixed_point_h_token  */
    integer_h_token = 287,         /* integer_h_token  */
    fixed_point_m_token = 288,     /* fixed_point_m_token  */
    integer_m_token = 289,         /* integer_m_token  */
    fixed_point_s_token = 290,     /* fixed_point_s_token  */
    integer_s_token = 291,         /* integer_s_token  */
    fixed_point_ms_token = 292,    /* fixed_point_ms_token  */
    integer_ms_token = 293,        /* integer_ms_token  */
    TIME = 294,                    /* TIME  */
    T_SHARP = 295,                 /* T_SHARP  */
    TIME_OF_DAY = 296,             /* TIME_OF_DAY  */
    DATE = 297,                    /* DATE  */
    D_SHARP = 298,                 /* D_SHARP  */
    DATE_AND_TIME = 299,           /* DATE_AND_TIME  */
    BYTE = 300,                    /* BYTE  */
    WORD = 301,                    /* WORD  */
    DWORD = 302,                   /* DWORD  */
    LWORD = 303,                   /* LWORD  */
    LREAL = 304,                   /* LREAL  */
    REAL = 305,                    /* REAL  */
    SINT = 306,                    /* SINT  */
    INT = 307,                     /* INT  */
    DINT = 308,                    /* DINT  */
    LINT = 309,                    /* LINT  */
    USINT = 310,                   /* USINT  */
    UINT = 311,                    /* UINT  */
    UDINT = 312,                   /* UDINT  */
    ULINT = 313,                   /* ULINT  */
    WSTRING = 314,                 /* WSTRING  */
    STRING = 315,                  /* STRING  */
    BOOL = 316,                    /* BOOL  */
    DT = 317,                      /* DT  */
    TOD = 318,                     /* TOD  */
    ANY = 319,                     /* ANY  */
    ANY_DERIVED = 320,             /* ANY_DERIVED  */
    ANY_ELEMENTARY = 321,          /* ANY_ELEMENTARY  */
    ANY_MAGNITUDE = 322,           /* ANY_MAGNITUDE  */
    ANY_NUM = 323,                 /* ANY_NUM  */
    ANY_REAL = 324,                /* ANY_REAL  */
    ANY_INT = 325,                 /* ANY_INT  */
    ANY_BIT = 326,                 /* ANY_BIT  */
    ANY_STRING = 327,              /* ANY_STRING  */
    ANY_DATE = 328,                /* ANY_DATE  */
    ASSIGN = 329,                  /* ASSIGN  */
    DOTDOT = 330,                  /* DOTDOT  */
    TYPE = 331,                    /* TYPE  */
    END_TYPE = 332,                /* END_TYPE  */
    ARRAY = 333,                   /* ARRAY  */
    OF = 334,                      /* OF  */
    STRUCT = 335,                  /* STRUCT  */
    END_STRUCT = 336,              /* END_STRUCT  */
    direct_variable_token = 337,   /* direct_variable_token  */
    incompl_location_token = 338,  /* incompl_location_token  */
    VAR_INPUT = 339,               /* VAR_INPUT  */
    VAR_OUTPUT = 340,              /* VAR_OUTPUT  */
    VAR_IN_OUT = 341,              /* VAR_IN_OUT  */
    VAR_EXTERNAL = 342,            /* VAR_EXTERNAL  */
    VAR_GLOBAL = 343,              /* VAR_GLOBAL  */
    END_VAR = 344,                 /* END_VAR  */
    RETAIN = 345,                  /* RETAIN  */
    NON_RETAIN = 346,              /* NON_RETAIN  */
    R_EDGE = 347,                  /* R_EDGE  */
    F_EDGE = 348,                  /* F_EDGE  */
    AT = 349,                      /* AT  */
    standard_function_name_token = 350, /* standard_function_name_token  */
    FUNCTION = 351,                /* FUNCTION  */
    END_FUNCTION = 352,            /* END_FUNCTION  */
    CONSTANT = 353,                /* CONSTANT  */
    standard_function_block_name_token = 354, /* standard_function_block_name_token  */
    FUNCTION_BLOCK = 355,          /* FUNCTION_BLOCK  */
    END_FUNCTION_BLOCK = 356,      /* END_FUNCTION_BLOCK  */
    VAR_TEMP = 357,                /* VAR_TEMP  */
    VAR = 358,                     /* VAR  */
    PROGRAM = 359,                 /* PROGRAM  */
    END_PROGRAM = 360,             /* END_PROGRAM  */
    ACTION = 361,                  /* ACTION  */
    END_ACTION = 362,              /* END_ACTION  */
    TRANSITION = 363,              /* TRANSITION  */
    END_TRANSITION = 364,          /* END_TRANSITION  */
    FROM = 365,                    /* FROM  */
    TO = 366,                      /* TO  */
    PRIORITY = 367,                /* PRIORITY  */
    INITIAL_STEP = 368,            /* INITIAL_STEP  */
    STEP = 369,                    /* STEP  */
    END_STEP = 370,                /* END_STEP  */
    L = 371,                       /* L  */
    D = 372,                       /* D  */
    SD = 373,                      /* SD  */
    DS = 374,                      /* DS  */
    SL = 375,                      /* SL  */
    N = 376,                       /* N  */
    P = 377,                       /* P  */
    prev_declared_global_var_name_token = 378, /* prev_declared_global_var_name_token  */
    prev_declared_program_name_token = 379, /* prev_declared_program_name_token  */
    prev_declared_resource_name_token = 380, /* prev_declared_resource_name_token  */
    prev_declared_configuration_name_token = 381, /* prev_declared_configuration_name_token  */
    CONFIGURATION = 382,           /* CONFIGURATION  */
    END_CONFIGURATION = 383,       /* END_CONFIGURATION  */
    TASK = 384,                    /* TASK  */
    RESOURCE = 385,                /* RESOURCE  */
    ON = 386,                      /* ON  */
    END_RESOURCE = 387,            /* END_RESOURCE  */
    VAR_CONFIG = 388,              /* VAR_CONFIG  */
    VAR_ACCESS = 389,              /* VAR_ACCESS  */
    WITH = 390,                    /* WITH  */
    SINGLE = 391,                  /* SINGLE  */
    INTERVAL = 392,                /* INTERVAL  */
    READ_WRITE = 393,              /* READ_WRITE  */
    READ_ONLY = 394,               /* READ_ONLY  */
    EOL = 395,                     /* EOL  */
    sendto_identifier_token = 396, /* sendto_identifier_token  */
    LD = 397,                      /* LD  */
    LDN = 398,                     /* LDN  */
    ST = 399,                      /* ST  */
    STN = 400,                     /* STN  */
    NOT = 401,                     /* NOT  */
    S = 402,                       /* S  */
    R = 403,                       /* R  */
    S1 = 404,                      /* S1  */
    R1 = 405,                      /* R1  */
    CLK = 406,                     /* CLK  */
    CU = 407,                      /* CU  */
    CD = 408,                      /* CD  */
    PV = 409,                      /* PV  */
    IN = 410,                      /* IN  */
    PT = 411,                      /* PT  */
    AND = 412,                     /* AND  */
    AND2 = 413,                    /* AND2  */
    OR = 414,                      /* OR  */
    XOR = 415,                     /* XOR  */
    ANDN = 416,                    /* ANDN  */
    ANDN2 = 417,                   /* ANDN2  */
    ORN = 418,                     /* ORN  */
    XORN = 419,                    /* XORN  */
    ADD = 420,                     /* ADD  */
    SUB = 421,                     /* SUB  */
    MUL = 422,                     /* MUL  */
    DIV = 423,                     /* DIV  */
    MOD = 424,                     /* MOD  */
    GT = 425,                      /* GT  */
    GE = 426,                      /* GE  */
    EQ = 427,                      /* EQ  */
    LT = 428,                      /* LT  */
    LE = 429,                      /* LE  */
    NE = 430,                      /* NE  */
    CAL = 431,                     /* CAL  */
    CALC = 432,                    /* CALC  */
    CALCN = 433,                   /* CALCN  */
    RET = 434,                     /* RET  */
    RETC = 435,                    /* RETC  */
    RETCN = 436,                   /* RETCN  */
    JMP = 437,                     /* JMP  */
    JMPC = 438,                    /* JMPC  */
    JMPCN = 439,                   /* JMPCN  */
    SENDTO = 440,                  /* SENDTO  */
    OPER_NE = 441,                 /* OPER_NE  */
    OPER_GE = 442,                 /* OPER_GE  */
    OPER_LE = 443,                 /* OPER_LE  */
    OPER_EXP = 444,                /* OPER_EXP  */
    RETURN = 445,                  /* RETURN  */
    IF = 446,                      /* IF  */
    THEN = 447,                    /* THEN  */
    ELSIF = 448,                   /* ELSIF  */
    ELSE = 449,                    /* ELSE  */
    END_IF = 450,                  /* END_IF  */
    CASE = 451,                    /* CASE  */
    END_CASE = 452,                /* END_CASE  */
    FOR = 453,                     /* FOR  */
    BY = 454,                      /* BY  */
    DO = 455,                      /* DO  */
    END_FOR = 456,                 /* END_FOR  */
    WHILE = 457,                   /* WHILE  */
    END_WHILE = 458,               /* END_WHILE  */
    REPEAT = 459,                  /* REPEAT  */
    UNTIL = 460,                   /* UNTIL  */
    END_REPEAT = 461,              /* END_REPEAT  */
    EXIT = 462                     /* EXIT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 177 "iec.y"

    symbol_c 	*leaf;
    list_c	*list;
    char 	*ID;	/* token value */
    struct {
      symbol_c	*first;
      symbol_c	*second;
    } double_symbol; /* used by il_simple_operator_clash_il_operand */

#line 281 "iec.y.hh"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_IEC_Y_HH_INCLUDED  */
