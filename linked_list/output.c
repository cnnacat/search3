#include "output.h"
#include "../error/error.h"
#include <string.h>


output_node* init_output_node(wchar_t* file_name, wchar_t* file_path)
{
    output_node* new_node = (output_node*)malloc(sizeof(output_node));
    if (!new_node)
    {
        output_error(L"Failed to allocate memory for the output node(s)");
        return NULL;
    }

    new_node->file_name = _wcsdup(file_name);
    new_node->file_path = _wcsdup(file_path);
    new_node->next      = NULL; 

    return new_node;
}


void push_output_node(output_node** output_node_head, output_node* node)
{
    node->next = *output_node_head;
    *output_node_head = node;
}


void pop_output_node(output_node** output_node_head)
{
	if (!(*output_node_head))
		return;
	
    output_node* temp_node = (*output_node_head)->next;

    free((*output_node_head)->file_name);
    free((*output_node_head)->file_path);
    free(*output_node_head);

    *output_node_head = temp_node;
}
