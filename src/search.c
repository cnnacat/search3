#include "search.h"
#include "../unicode.h"
#include "../getopt/getopt.h"
#include "../error/error.h"
#include "../linked_list/output.h"
#include <fileapi.h>
#include <minwindef.h>
#include <stdlib.h>
#include <strsafe.h>
#include <wchar.h>
#include <wingdi.h>
#include <winnt.h>

enum
{
    PERFECT_MATCH = 415,
    SIMILAR_MATCH = 67
};

void hang()
{
    wprintf(L"Press ENTER to quit: ");
    getwchar();
    return;
}


int get_cli_args(
    int       argc,
    wchar_t*  argv[],
    wchar_t** directory_path,
    wchar_t** target)
{
    int return_code;
    int option_index;

    while (true)
    {
        static struct option long_option[] =
        {
            {L"target",    required_argument, 0, L'T'},
            {L"directory", optional_argument, 0, L'D'},
            {0,0,0,0}
        };

        return_code = getopt_long_w(
            argc,
            argv,
            L"D::",
            long_option,
            &option_index);

        if (return_code == -1)
            break;

        switch (return_code)
        {
        case L'T':
            {
                *target = _wcsdup(optarg);
                break;
            }

        case L'D':
            {
                wchar_t* arg_copy = _wcsdup(optarg);
                if (!arg_copy)
                {
                    wprintf(L"_wcsdup() failed in getopt\n");
                    return -1;
                }

                size_t length;
                HRESULT result = StringCchLengthW(
                    arg_copy,
                    MAX_PATH,
                    &length);

                if (FAILED(result))
                {
                    wprintf(L"StringCchLengthW() failed in getopt\n");
                    return -2;
                }

                if (length > 0
                    && arg_copy[length -1 ] == L'\\')
                    arg_copy[length - 1] = L'\0';

                *directory_path = arg_copy;
                break;
            }

        case L'?':
            break;

        default:
            abort();
        }
    }


    if (*directory_path == NULL)
    {
        *directory_path = _wcsdup(L".");
    }

    return 0;
}


size_t matching(wchar_t* string_1, wchar_t* string_2)
{
    size_t count = 0;

    while (*string_1 && *string_2)
    {
        if (*string_1 == *string_2)
            count++;

        string_1++;
        string_2++;
    }

    return count;

}


// I have no idea how to name this function. Basically calculates
// the amount of integers that should be considered "relevant"
// enough to be part of the output. 
// Uhh
// I mean like this: If target is "cat", and a file name is "car",
// then since car has "ca" in it, it should be part of the output.
int calculate_relevant_matches(wchar_t* file_name)
{
    size_t  length;
    HRESULT result;
    int     result_2;

    result = StringCchLengthW(file_name, MAX_PATH, &length);
    if (FAILED(result))
    {
        wprintf(L"bruh\n");
        return 1337;
    }

    // This assumes that the whatever file_name is given can fit within the
    // float data type. oh well. i mean, it should "TECHNICALLY" never fail.
    // but im not good enough to figure out a workaround. maybe eventually ill be good
    // enough. nah its not a maybe, im locked in.
    float do_math = (float)length / 2;
    result_2 = (int)(ceil(do_math));

    return result_2;
}


void search(  
    wchar_t*      directory_path,
    wchar_t*      target,
    output_node** head_output_node,
    output_node** perfect_match_head_output_node)
{
    WIN32_FIND_DATAW file_data;
    HANDLE           file_handle;
    wchar_t          search_path[MAX_PATH];

    if (_snwprintf_s(
        search_path, 
        MAX_PATH,
        _TRUNCATE,
        L"%s\\*",
        directory_path) == -1)
    {
        wprintf(L"Truncation happened during search path creation for %s\n", directory_path);
        return;
    }

    // I do not care (yet). Just skip.
    file_handle = FindFirstFileW(search_path, &file_data);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    
    do
    {
        wchar_t* file_name    = file_data.cFileName;
        bool is_dir           = file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        bool is_reparse_point = file_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
        bool is_excluded      = file_data.dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN); 
        bool is_dot_dir       = wcscmp(file_name, L".") == 0 || wcscmp(file_name, L"..") == 0;

        if (is_reparse_point
            || is_excluded
            || is_dot_dir)
            continue;

        if (_wcsicmp(file_name, target) == 0)
        {
            output_node* new_node = init_output_node(file_name, directory_path);
            if (!new_node)
            {
                wprintf(L"Failed allocation of perfect output node for file %s\n", file_name);
                continue;
            }
            push_output_node(perfect_match_head_output_node, new_node);
        }
        else if (matching(file_name, target) > calculate_relevant_matches(file_name)
            || wcsstr(file_name, target))
        {
            output_node* new_node = init_output_node(file_name, directory_path);
            if (!new_node)
            {
                wprintf(L"Failed allocation of an output node for file: %s\n", file_name);
                continue;
            }
            push_output_node(head_output_node, new_node);
        }
         
        if (is_dir)
        {
            wchar_t next_directory_path[MAX_PATH];

            if (_snwprintf_s(
                next_directory_path,
                MAX_PATH,
                _TRUNCATE,
                L"%s\\%s",
                directory_path,
                file_name) == -1)
            {
                continue; // Implement logging later
            }

            search(next_directory_path,
                target,
                head_output_node,
                perfect_match_head_output_node);
        }
    } while (FindNextFileW(file_handle, &file_data));

    FindClose(file_handle);
    return;
}


// mess
void unwind_search(
    wchar_t*      target,
    output_node** head_output_node,
    output_node** perfect_match_head_output_node)
{

    if (!(*head_output_node) 
        && !(*perfect_match_head_output_node))
    {
        wprintf(L"\nNo matches found to the target: %s\n", target);
    }

    else if (!(*perfect_match_head_output_node)
        && (*head_output_node))
    {
        wprintf(L"\nNo perfect match(es) to target: %s\n", target);
        wprintf(L"Similar file names to the target found:\n");

        while ((*head_output_node))
        {
            wchar_t* file_name = (*head_output_node)->file_name;
            wchar_t* file_path = (*head_output_node)->file_path;

            wprintf(L"Similar file: %s\n", file_name);
            wprintf(L"Located in directory: %s\n\n", file_path);

            pop_output_node(head_output_node);
        }
    }

    else if ((*perfect_match_head_output_node) 
        && !(*head_output_node))
    {
        wprintf(L"\nPerfect match(es) found for target: %s\n", target);

        while ((*perfect_match_head_output_node))
        {
            wchar_t* file_name = (*perfect_match_head_output_node)->file_name;
            wchar_t* file_path = (*perfect_match_head_output_node)->file_path;

            wprintf(L"Exact file name: %s\n", file_name);
            wprintf(L"Located in directory: %s\n\n", file_path);

            pop_output_node(perfect_match_head_output_node);
        }
    }

    else if ((*perfect_match_head_output_node)
        && (*head_output_node))
    {
        wprintf(L"\nPerfect match(es) found for target: %s\n", target);

        while ((*perfect_match_head_output_node))
        {
            wchar_t* file_name = (*perfect_match_head_output_node)->file_name;
            wchar_t* file_path = (*perfect_match_head_output_node)->file_path;

            wprintf(L"Exact file name: %s\n", file_name);
            wprintf(L"Located in directory: %s\n\n", file_path);

            pop_output_node(perfect_match_head_output_node);
        }

        wprintf(L"Printing additional similar matches...\n");

        while ((*head_output_node))
        {
            wchar_t* file_name = (*head_output_node)->file_name;
            wchar_t* file_path = (*head_output_node)->file_path;

            wprintf(L"Similar file: %s\n", file_name);
            wprintf(L"Located in directory: %s\n\n", file_path);

            pop_output_node(head_output_node);
        }
    }

    return;
}


int wmain(int argc, wchar_t* argv[])
{
    if (argc == 1)
    {
        wprintf(L"\nUsage: search3 --target=file_name\n");
        hang();
        exit(-1);
    }

    // Set CMD output to Unicode
    _setmode(_fileno(stdout), _O_U16TEXT);


    wchar_t*     directory_path                 = NULL;
    wchar_t*     target                         = NULL;
    output_node* head_output_node               = NULL;
    output_node* perfect_match_head_output_node = NULL;
    // LOL im doing this so i DONT have to sort the same linked list for
    // perfect matches. yeah im doing this because i dont want to sort something.
    // because i shouldnt have to. just separate the data.

    // actually i might want to alphabeitize (beitize(alphabizate)) output. 

    get_cli_args(
        argc,
        argv,
        &directory_path,
        &target);

    if (target == NULL
        || wcscmp(target, L"") == 0)
    {
        wprintf(L"\nCannot search for nothing. Program will cancel.\n");
        hang();
        exit(-1);
    }

    search(
        directory_path,
        target,
        &head_output_node,
        &perfect_match_head_output_node);

    unwind_search(
        target,
        &head_output_node,
        &perfect_match_head_output_node);  

    free(directory_path);
    free(target);
    hang();
    return 0;
}
