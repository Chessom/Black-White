#pragma once
#include"stdafx.h"
#include"core.hpp"
#ifdef _WIN32
#include<Windows.h>
#endif
namespace bw::env {
	enum class CPU { intel, amd };
	enum class GPU { nvidia, amd };
	inline int cpu, gpu;
	inline int winver;
	inline void set_codepage(int codepage) {
#ifdef _WIN32
		SetConsoleCP(codepage);
#else

#endif // _WIN32
	}
	inline void set_font(const std::wstring& wstr) {
#ifdef _WIN32
		CONSOLE_FONT_INFOEX info = { 0 };
		info.cbSize = sizeof(info);
		info.dwFontSize.Y = 16;
		info.FontWeight = FW_NORMAL;
		wcscpy(info.FaceName, wstr.c_str());
		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &info);
#else

#endif // _WIN32
	}
	inline void set_window_size(core::coord crd) {
#ifdef _WIN32
		HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SMALL_RECT winSize = { 0, 0 };
		winSize.Right = crd.x;
		winSize.Bottom = crd.y;
		SetConsoleWindowInfo(outputHandle, 1, &winSize);
		ftxui::Terminal::SetFallbackSize({ .dimx = crd.x,.dimy = crd.y });
#else

#endif // _WIN32
	}
	inline void set_window_size(int x, int y) {
		set_window_size({ x,y });
	}
}