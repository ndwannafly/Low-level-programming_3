%{
#include "request_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int yylex();
int yyparse();
void yyerror(const char *s);

request_tree tree = {.type = UNDEFINED};
bool array_list_created = false;

char *file_name;
int opened = 0;

extern char *yytext;

statement *cur_statement = NULL;

#define YYDEBUG_LEXER_TEXT yytext

%}
%union
{
	int integer;
	char *string;
	float decimal;
	int boolean;
	char *ref_name;
	int attribute_type;
	int compare_option;
}

%type<string> quoted_argument
%type<attribute_type> attribute_type

%token OBRACE CBRACE QUOTE DOT COMMA SEMICOLON

%token TOK_OPEN TOK_CREATE TOK_CLOSE
%token TOK_ADD_NODE TOK_NODES TOK_SELECT
%token TOK_ADD_EDGE TOK_DELETE_EDGE
%token TOK_EQUAL TOK_GREATER TOK_GREATER_EQUAL TOK_LESS TOK_LESS_EQUAL TOK_NOT_EQUAL TOK_LIKE TOK_REFERENCE
%token TOK_VALUES TOK_DELETE
%token TOK_OUT
%token <string> FILENAME

%token <integer> INTEGER
%token <decimal> DECIMAL
%token <boolean> BOOLEAN
%token <string> WORD
%token <string> STRING

%type <compare_option> compare_option

%token TOK_INTEGER
%token TOK_FLOAT
%token TOK_STRING
%token TOK_BOOLEAN

%start commands

%%

commands: /* empty */
	| commands command SEMICOLON {
	YYACCEPT;
	}
	;


command:
	open_file
	|
	create_file
	|
	close_file
	|
	add_edge
	|
	delete_edge
	|
	add_vertex
	|
	select_command
	;

select_command:
	select_nodes {
		tree.type = REQUEST_SELECT;
	}
	| select_command select_condition
	| select_command join
	| select_command delete_command
	;

open_file:
	TOK_OPEN OBRACE QUOTE FILENAME QUOTE CBRACE
	{
		tree.type = REQUEST_OPEN;
		tree.file_work.filename = malloc(sizeof(char) * strlen($4));
		strcpy(tree.file_work.filename, $4);
	};

create_file:
	TOK_CREATE OBRACE QUOTE FILENAME QUOTE CBRACE
	{
		tree.type = REQUEST_CREATE;
		tree.file_work.filename = malloc(sizeof(char) * strlen($4));
		strcpy(tree.file_work.filename, $4);
	};

close_file:
	TOK_CLOSE OBRACE CBRACE
	{
		tree.type = REQUEST_CLOSE;
	};


add_vertex:
	TOK_ADD_NODE OBRACE quoted_argument attribute_value_pairs CBRACE
	{
		tree.type = REQUEST_ADD_NODE;
		tree.add_node.schema_name = malloc(sizeof(char) * strlen($3));
		strcpy(tree.add_node.schema_name, $3);
	}
	;

add_edge:
    TOK_ADD_EDGE OBRACE quoted_argument COMMA INTEGER COMMA INTEGER CBRACE
    {
        tree.type = REQUEST_ADD_EDGE;
        tree.add_edge.schema_name = malloc(sizeof(char) * strlen($3));
        strcpy(tree.add_edge.schema_name, $3);
        tree.add_edge.node1 = $5;
        tree.add_edge.node2 = $7;
    }
    ;

delete_edge:
    TOK_DELETE_EDGE OBRACE quoted_argument COMMA INTEGER COMMA INTEGER CBRACE
        {
            tree.type = REQUEST_DELETE_EDGE;
            tree.delete_edge.schema_name = malloc(sizeof(char) * strlen($3));
            strcpy(tree.delete_edge.schema_name, $3);
            tree.delete_edge.node1 = $5;
            tree.delete_edge.node2 = $7;
        }
        ;

select_nodes:
	TOK_NODES OBRACE quoted_argument CBRACE
	{
		tree.schema_name = malloc(sizeof(char) * strlen($3));
		strcpy(tree.schema_name, $3);
		// printf("select nodes on %s\n", $3);
	}
	|
	TOK_NODES OBRACE CBRACE
    	{
    	}
	;

select_condition:
	| DOT TOK_SELECT OBRACE select_statements CBRACE
	{
		//CHECK array_list_created to create tree.statements
		if(!array_list_created) {
			tree.statements = arraylist_create();
			array_list_created = true;
		}
		arraylist_add(tree.statements, cur_statement);
		//make new empty statement
		cur_statement = NULL;
	}
	;

join:
	| DOT TOK_OUT OBRACE quoted_argument CBRACE
	{
		if(!array_list_created) {
			tree.statements = arraylist_create();
			array_list_created = true;
		}
		cur_statement = malloc(sizeof(statement));
		cur_statement->type = OUT;
		cur_statement->attr_name = malloc(sizeof(char) * strlen($4));
		strcpy(cur_statement->attr_name, $4);
		arraylist_add(tree.statements, cur_statement);
		cur_statement = NULL;
		// printf("join %s\n", $4);
	}
	;

delete_command:
	| DOT TOK_DELETE {
		if(!array_list_created) {
			tree.statements = arraylist_create();
			array_list_created = true;
		}
		cur_statement = malloc(sizeof(statement));
		cur_statement->type = DELETE;
		arraylist_add(tree.statements, cur_statement);
		cur_statement = NULL;
		// printf("delete command\n");
	}
	;

select_statements:
	| select_statements select_statement COMMA {

	}
	| select_statements select_statement {

	}
	;

select_statement:
	quoted_argument COMMA compare_option OBRACE INTEGER CBRACE {
		if(cur_statement == NULL) {
			cur_statement = malloc(sizeof(statement));
			cur_statement->type = SELECT_CONDITION;
			cur_statement->conditions = arraylist_create();
		}
		select_condition *cond = malloc(sizeof(select_condition));
		cond->attr_name = malloc(sizeof(char) * strlen($1));
		strcpy(cond->attr_name, $1);
		cond->option = $3;
		cond->type = ATTR_TYPE_INTEGER;
		cond->value.integer_value = $5;
		arraylist_add(cur_statement->conditions, cond);
	}
	| quoted_argument COMMA compare_option OBRACE DECIMAL CBRACE {
		if(cur_statement == NULL) {
			cur_statement = malloc(sizeof(statement));
			cur_statement->type = SELECT_CONDITION;
			cur_statement->conditions = arraylist_create();
		}
		select_condition *cond = malloc(sizeof(select_condition));
		cond->attr_name = malloc(sizeof(char) * strlen($1));
		strcpy(cond->attr_name, $1);
		cond->option = $3;
		cond->type = ATTR_TYPE_FLOAT;
		cond->value.float_value = $5;
		arraylist_add(cur_statement->conditions, cond);
	}
	| quoted_argument COMMA compare_option OBRACE BOOLEAN CBRACE {
		if(cur_statement == NULL) {
			cur_statement = malloc(sizeof(statement));
			cur_statement->type = SELECT_CONDITION;
			cur_statement->conditions = arraylist_create();
		}
		select_condition *cond = malloc(sizeof(select_condition));
		cond->attr_name = malloc(sizeof(char) * strlen($1));
		strcpy(cond->attr_name, $1);
		cond->option = $3;
		cond->type = ATTR_TYPE_BOOLEAN;
		cond->value.bool_value = $5;
		arraylist_add(cur_statement->conditions, cond);
	}
	| quoted_argument COMMA compare_option OBRACE quoted_argument CBRACE {
		if(cur_statement == NULL) {
			cur_statement = malloc(sizeof(statement));
			cur_statement->type = SELECT_CONDITION;
			cur_statement->conditions = arraylist_create();
		}
		select_condition *cond = malloc(sizeof(select_condition));
		cond->attr_name = malloc(sizeof(char) * strlen($1));
		strcpy(cond->attr_name, $1);
		cond->option = $3;
		cond->type = ATTR_TYPE_STRING;
		cond->value.string_value = $5;
		arraylist_add(cur_statement->conditions, cond);
	}
	;

attribute_value_pairs:
	| attribute_value_pairs COMMA attribute_value_pair {

	};

attribute_value_pair:

	quoted_argument COMMA INTEGER {
		if(!array_list_created) {
			tree.add_node.attribute_values = arraylist_create();
			array_list_created = true;
		}
		attr_value *attr_val = malloc(sizeof(attr_value));
		*attr_val = (attr_value) {
			.attr_name = malloc(sizeof(char) * strlen($1)),
			.type = ATTR_TYPE_INTEGER,
			.value = (union value) {.integer_value = $3}
		};
		strcpy(attr_val->attr_name, $1);
		arraylist_add(tree.add_node.attribute_values, attr_val);
	}
	| quoted_argument COMMA DECIMAL{
		if(!array_list_created) {
			tree.add_node.attribute_values = arraylist_create();
			array_list_created = true;
		}
		attr_value *attr_val = malloc(sizeof(attr_value));
		*attr_val = (attr_value) {
			.attr_name = malloc(sizeof(char) * strlen($1)),
			.type = ATTR_TYPE_FLOAT,
			.value = (union value) {.float_value = $3}
		};
		strcpy(attr_val->attr_name, $1);
		arraylist_add(tree.add_node.attribute_values, attr_val);
	}
	| quoted_argument COMMA BOOLEAN{
		if(!array_list_created) {
			tree.add_node.attribute_values = arraylist_create();
			array_list_created = true;
		}
		attr_value *attr_val = malloc(sizeof(attr_value));
		*attr_val = (attr_value) {
			.attr_name = malloc(sizeof(char) * strlen($1)),
			.type = ATTR_TYPE_BOOLEAN,
			.value = (union value) {.bool_value = $3}
		};
		strcpy(attr_val->attr_name, $1);
		arraylist_add(tree.add_node.attribute_values, attr_val);
	}
	| quoted_argument COMMA quoted_argument{
		if(!array_list_created) {
			tree.add_node.attribute_values = arraylist_create();
			array_list_created = true;
		}
		attr_value *attr_val = malloc(sizeof(attr_value));
		*attr_val = (attr_value) {
			.attr_name = malloc(sizeof(char) * strlen($1)),
			.type = ATTR_TYPE_STRING,
			.value = (union value) {.string_value = malloc(sizeof(char) * strlen($3))}
		};
		strcpy(attr_val->value.string_value, $3);
		strcpy(attr_val->attr_name, $1);
		arraylist_add(tree.add_node.attribute_values, attr_val);
	}
	;

attribute_pairs:
	| attribute_pairs COMMA attribute_pair {
	}
	;

attribute_type:
	TOK_INTEGER {
		$$ = ATTR_TYPE_INTEGER;
	}
	| TOK_STRING {
		$$ = ATTR_TYPE_STRING;
	}
	| TOK_FLOAT {
		$$ = ATTR_TYPE_FLOAT;
	}
	| TOK_BOOLEAN {
		$$ = ATTR_TYPE_BOOLEAN;
	}
	;

attribute_pair:
	quoted_argument COMMA attribute_type {
		if (!array_list_created) {
		tree.add_schema.attribute_declarations = arraylist_create();
		array_list_created = true;
		}
		attribute_declaration *attr_decl = malloc(sizeof(attribute_declaration));
		*attr_decl = (attribute_declaration) {
		.attr_name = malloc(sizeof(char) * strlen($1)),
		.type = $3,
		};
		strcpy(attr_decl->attr_name, $1);
		arraylist_add(tree.add_schema.attribute_declarations, attr_decl);
	}
	;

compare_option:
	TOK_EQUAL {
		$$ = OPTION_EQUAL;
	}
	| TOK_GREATER {
		$$ = OPTION_GREATER;
	}
	| TOK_GREATER_EQUAL {
		$$ = OPTION_GREATER_EQUAL;
	}
	| TOK_LESS {
		$$ = OPTION_LESS;
	}
	| TOK_LESS_EQUAL {
		$$ = OPTION_LESS_EQUAL;
	}
	| TOK_NOT_EQUAL {
		$$ = OPTION_NOT_EQUAL;
	}
	| TOK_LIKE {
		$$ = OPTION_LIKE;
	}
	| TOK_REFERENCE {
	    $$ = OPTION_REFERENCE;
	}
	;


quoted_argument:
	QUOTE WORD QUOTE
	{
		$$ = $2;
	}
	;
%%

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n%s\n",str, yytext);
}

int yywrap()
{
	return 1;
}

request_tree get_request_tree(){
	return tree;
}