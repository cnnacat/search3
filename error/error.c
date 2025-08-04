#include "error.h"
#include <string.h>
#include <strsafe.h>
#include <windows.h>


// For fun, never used in the actual program
void output_error(wchar_t* message)
{
	HRESULT cat_result;
	int output_result;


	// Can this even happen, if the program RUNS in the console?
	HWND console_window = GetConsoleWindow();
	if (!console_window)
		return;


	wchar_t error[4096] = L"Error: ";
	cat_result = StringCchCatW(
		error,
		4096,
		message);

	if (FAILED(cat_result))
	{
		MessageBoxW(
			console_window,
			L"output_error errored (string concat.) while trying to produce an error. huh.", 
			NULL,
			MB_OK | MB_ICONWARNING);

		return;
	}


	output_result = MessageBoxW(
		console_window,
		error,
		NULL,
		MB_OK | MB_ICONWARNING);

	if (output_result == 0)
	{
		wprintf(L"Failed to display a WinAPI message box");
		return;
	}

	return;
}
