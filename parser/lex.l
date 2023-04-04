%{
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "parser.h"
%}

%%
"("                     {return OBRACE;}
")"                     {return CBRACE;}
\"                      {return QUOTE;}
"."                     {return DOT;}
","                     {return COMMA;}
";"                     {return SEMICOLON;}



"open"                  {return TOK_OPEN;}
"create"                {return TOK_CREATE;}
"close"                 {return TOK_CLOSE;}

"addVertex"             {return TOK_ADD_NODE;}
"addEdge"               {return TOK_ADD_EDGE;}
"deleteEdge"            {return TOK_DELETE_EDGE;}
"V"                     {return TOK_NODES;}
"has"                   {return TOK_SELECT;}
"out"                   {return TOK_OUT;}

"eq"                    {return TOK_EQUAL;}
"gt"                    {return TOK_GREATER;}
"gte"                   {return TOK_GREATER_EQUAL;}
"lt"                    {return TOK_LESS;}
"lte"                   {return TOK_LESS_EQUAL;}
"neq"                   {return TOK_NOT_EQUAL;}
"like"                  {return TOK_LIKE;}
"ref"                   {return TOK_REFERENCE;}

"integer"               {return TOK_INTEGER;}
"float"                 {return TOK_FLOAT;}
"string"                {return TOK_STRING;}
"boolean"               {return TOK_BOOLEAN;}

"values"                {return TOK_VALUES;}
"delete()"              {return TOK_DELETE;}

"true"|"false"          {yylval.boolean = !strcmp(yytext, "true"); return BOOLEAN;}

[a-zA-Z0-9_]+\.[A-Za-z]{1,3}    {yylval.string = strdup(yytext); return FILENAME;}
[a-zA-Z][a-zA-Z0-9_ ,\.\/:\-\?\&=\%'@<>|\;\’#+!]*           {yylval.string = strdup(yytext); return WORD;}
(\.[0-9]+)|([0-9]+(\.[0-9]*)?([eE][+-]?[0-9]+)?)   {
      double f = atof(yytext);
      if (ceil(f) == f) {
        yylval.integer = atoi(yytext);
        return INTEGER;
      } else {
        yylval.decimal = f;
        return DECIMAL;
      }
}

\n                      {/*ignore */;}
[ \t]+                  {/* ignore whitespace */;}
%%
