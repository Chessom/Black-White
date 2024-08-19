#pragma once
#include"stdafx.h"
#include"config.hpp"
namespace bw {
	inline config_ptr global_config;
	inline boost::locale::generator locale_gen;
	inline std::string MessageBoxString = "";
	inline bool _show_modal = false;
	inline ftxui::Component MessageBoxComp = ftxui::Container::Vertical({});
	inline std::function<void()> _msgbox_call_back = [] {};
	template<typename E>
	constexpr auto enum_count() {
		static_assert(std::is_enum_v<E>, "E must be an enum type");
		constexpr auto first = static_cast<std::underlying_type_t<E>>(E::first);
		constexpr auto last = static_cast<std::underlying_type_t<E>>(E::last);
		return last - first + 1;
	}
}
namespace bw::othello {
	namespace symbols {
		inline std::string col0 = "○ ", col1 = "● ", none = "■ ";
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
	enum class detailed_type :int { human = 0, computer_random, computer_ai, lua_script_gamer, python_script_gamer, remote_tcp_gamer, online, invalid };
	inline charmap CharMap{};
	inline std::vector<std::string> gamer_list;
	inline std::vector<std::string> size_list = { "4","6","8","10","12","14","16" };
	inline void initialize_global_variable() {
		gamer_list.resize(6);
		gamer_list[std::to_underlying(detailed_type::human)] = gettext("Human");
		gamer_list[std::to_underlying(detailed_type::computer_random)] = gettext("ComputerRandom");
		gamer_list[std::to_underlying(detailed_type::computer_ai)] = gettext("ComputerAI");
		gamer_list[std::to_underlying(detailed_type::lua_script_gamer)] = gettext("Lua Script Gamer");
		gamer_list[std::to_underlying(detailed_type::python_script_gamer)] = gettext("Python Script Gamer");
		gamer_list[std::to_underlying(detailed_type::remote_tcp_gamer)] = gettext("RemoteTCPGamer");
	}
}
namespace bw::ataxx {

};
namespace bw::tictactoe {
	enum class detailed_type :int { human = 0, computer_random, computer_ai, lua_script_gamer, remote_tcp_gamer, invalid };
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