#pragma once
#include"stdafx.h"
namespace bw::locale {
	using lang_map_t = std::unordered_map<int, std::wstring>;
	inline int language;
	inline lang_map_t lang_map;
	inline std::string langmap_path;

	inline bool exist_locale(int Language) {

	}

	inline void set_language(int Language) {

	}

	
}
namespace bw {
	inline std::string operator""_l(const char* str, std::size_t size) {
		return std::string(str, size);
	}
}