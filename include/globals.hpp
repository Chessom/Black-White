#pragma once
#include"stdafx.h"
#include"config.hpp"
namespace bw {
	inline config_ptr global_config;
	inline boost::locale::generator locale_gen;
	inline std::string MessageBoxString = "";
	inline bool _show_modal = false;
	inline ftxui::Component MessageBoxComp = ftxui::Container::Horizontal({});
	inline std::function<void()> _msgbox_call_back = [] {};
}
namespace bw::othello {
	namespace symbols {
		//std::string col0 = reinterpret_cast<const char*>(u8"⚫"), col1 = reinterpret_cast<const char*>(u8"⚪"), none = reinterpret_cast<const char*>(u8"■");
		inline std::string col0 = "○", col1 = "●", none = "■";
	}
	namespace numbers {
		inline std::string col0 = "1 ", col1 = "2 ", none = "0 ";
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
	inline charmap CharMap{};
	inline std::vector<std::string> gamer_list;
	inline std::vector<std::string> size_list = { "4","6","8","10","12","14","16" };
	inline void initialize_global_variable() {
		gamer_list = { gettext("Human"),gettext("ComputerLow"),gettext("ComputerMiddle"),gettext("ComputerHigh"),gettext("RemoteGamer") };
	}
}
namespace bw::ataxx {

};
namespace bw::tictactoe {
	inline std::vector<std::string> gamer_list;
	inline void initialize_global_variable() {
		gamer_list = { gettext("Human"),gettext("Computer Random") };
	}
}
namespace bw {
	inline void initialize_globals() {
		othello::initialize_global_variable();
		tictactoe::initialize_global_variable();
	}
}
inline std::string bw::othello::charmap::operator[](const int& col) {
	std::string ret;
	if (stl == symb) {
		switch (col) {
		case 0:
			ret = symbols::col0;
			break;
		case 1:
			ret = symbols::col1;
			break;
		case -1:
			ret = symbols::none;
			break;
		}
	}
	if (stl == numb) {
		switch (col) {
		case 0:
			ret = numbers::col0;
			break;
		case 1:
			ret = numbers::col1;
			break;
		case -1:
			ret = numbers::none;
			break;
		}
	}
	return ret;
}