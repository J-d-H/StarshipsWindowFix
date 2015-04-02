// King of Dragon Pass Launcher.cpp
//

#include "stdafx.h"

struct SavedWndStyle {
	DWORD style;
	RECT pos;
};
void fixFullscreen(HWND hwnd) {
	auto text = cycleStrBuffer();

	// make fullscreen:
	SavedWndStyle sws;
	sws.style = GetWindowLong(hwnd, GWL_STYLE);
	GetWindowRect(hwnd, &sws.pos);
				
	HMONITOR hmon = MonitorFromWindow(hwnd,
										MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi ={sizeof(mi)};
	if (!GetMonitorInfo(hmon, &mi)) return;

	SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE);
	SetWindowPos(hwnd, HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top, 
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

HWND findGameWindow() {
	return FindWindow(NULL, L"Sid Meier's Starships");
}


int __cdecl _tmain(int argc, _TCHAR* argv[])
{
	HWND hwndMain = NULL;
	DWORD idProcess = 0;


	bool repeat = false;
	bool background = false;
	bool once = false;
	bool error = false;
	for (int i = 1; i < argc; ++i) {
		PWSTR arg = argv[i];
		if (arg != NULL && *arg == L'-') {
			WCHAR c;
			while ((c = *(++arg)) != L'\0') {
				if (c == L'r') repeat = true;
				else if (c == L'b') background = true;
				else if (c == L'o') once = true;
				else {
					error = true;
					break;
				}
			}
		}
	}
	error = error || (once && repeat) || (!once && !repeat);

	if (background) 
	{
		HWND hwnd = GetConsoleWindow();
		ShowWindow(hwnd, SW_HIDE);
	}
	else // !background
	{
		// setting codepage
		if (!SetConsoleCP(1252) || !SetConsoleOutputCP(1252)) {
			auto err = GetLastError();
			std::wcout << "Error setting CP1252: 0x" << std::hex << err << "\n" << GetErrorText(err) << std::endl;
		}

		std::locale::global(std::locale("english_England.1252"));
		std::wcout.imbue(std::locale());
		

		// Seting console font
		auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_FONT_INFOEX cfi = { sizeof(cfi) };
		//GetCurrentConsoleFontEx(hStdout, false, &cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = 0;
		cfi.dwFontSize.Y = 16;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Consolas");
		//wcscpy_s(cfi.FaceName, L"Arial");
		if (!SetCurrentConsoleFontEx(hStdout, false, &cfi)) {
			auto err = GetLastError();
			std::wcout << "Error setting console font: 0x" << std::hex << err << "\n" << GetErrorText(err) << std::endl;
		}
	}

	if (error) {
		std::wcout <<
			"Usage:\n"
			"Starships-Fix -(r|o)[b]\n"
			"\t-r\trepeat: waits infinitly for Sid Meier's Starships to fix the window.\n"
			"\t-o\tonce: fixes the window once and quits.\n"
			"\t-b\tbackground: hides in background.\n"
			"\n"
			"examples:\n"
			"\tStarships-Fix -o\n"
			"\tStarships-Fix -r\n"
			"\tStarships-Fix -rb\n"
			<< std::endl;
		return 0;
	}

	while (repeat || once) {
		once = false;
		std::wcout << "\n\nWaiting for Sid Meier's Starships" << std::flush;
		hwndMain = findGameWindow();
		while (!hwndMain) {
			for (size_t i = 0; i < 10 && !hwndMain; ++i) {
				Sleep(2500);
				hwndMain = findGameWindow();
			}
			std::wcout << '.' << std::flush;
		}

		std::wcout << ".\nfound.\n\nFixing fullscreen window..." << std::endl;

		fixFullscreen(hwndMain);

		std::wcout << " done.\n" << std::endl;

		if (repeat) {
			std::wcout << "Waiting until game closes ..." << std::flush;
			DWORD idProcess = 0;
			HANDLE hProcess = 0;
			GetWindowThreadProcessId(hwndMain, &idProcess);
			if (idProcess) hProcess = OpenProcess(SYNCHRONIZE, false, idProcess);
			if (hProcess) {
				WaitForSingleObject(hProcess, INFINITE);
				CloseHandle(hProcess);
			} else {
				std::wcout << "WARNING: cannot sync with game process." << std::endl;
				do {
					Sleep(1000);
					hwndMain = findGameWindow();
				} while (hwndMain);
			}

			std::wcout << " done.\n" << std::endl;
			Sleep(1000);
		}
	}

	return 0;
}
