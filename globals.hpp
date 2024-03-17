#pragma once
#include"stdafx.h"
#include"config.hpp"
namespace bw {
	extern std::string MessageBoxString;
	//extern std::shared_ptr<ftxui::ScreenInteractive> current_screen;
	extern bool show_modal;
	extern ftxui::Component MessageBoxComp;
	inline config_ptr global_config;
}
namespace bw::othello {
	namespace symbols {
		extern std::string col0;
		extern std::string col1;
		extern std::string none;
	}
	namespace numbers {
		extern std::string col0;
		extern std::string col1;
		extern std::string none;
	}
	enum { symb, numb };

	class charmap {
	public:
		charmap() = default;
		charmap(int st) :stl(st) {};
		std::string operator[](const int& col);
		int& current() {
			return stl;
		}
	private:
		int stl = symb;
	};
	extern charmap CharMap;
	extern std::vector<std::string> gamer_list;
	extern std::vector<std::string> size_list;
}
namespace bw::ataxx {

};
