#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"basic_gamer.hpp"
#include"tui/screen.hpp"
namespace bw {
	class basic_game {//基础的game
	public:
		enum { unready, ready, ongoing, ended, suspended };
		basic_game() = default;
		basic_game(int State) :st(State) {}
		string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
		}
		virtual boost::cobalt::task<void> start() = 0;
		int& state() { return st; }
		virtual void end_game() = 0;
		virtual ~basic_game() = default;
		boost::signals2::signal<void()>
			pass_sig, regret_sig, end_sig, flush_sig, suspend_sig, save_sig;
	protected:
		virtual void suspend() {};
		int st = unready;
		//std::chrono::time_point<std::chrono::system_clock> begin, end;
	};
	using basic_game_ptr = std::shared_ptr<basic_game>;
}