#pragma once

#include <windows.h>
#include <wchar.h>

typedef struct output_node
{
	wchar_t* file_name;
	wchar_t* file_path;
	
	struct output_node* next;

} output_node;


output_node* init_output_node (wchar_t* file_name, wchar_t* file_path);
void         push_output_node (output_node** output_node_head, output_node* node);
void         pop_output_node  (output_node** output_node_head);