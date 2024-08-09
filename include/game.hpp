#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"gamer.hpp"
#include"tui/screen.hpp"
namespace bw {
	class basic_game {//基础的game
	public:
		enum { unready, ready, ongoing, ended, suspended };
		basic_game() = default;
		basic_game(int State) :st(State) {}
		virtual boost::cobalt::task<void> start(basic_gamer_ptr g0, basic_gamer_ptr g1) = 0;
		int& state() { return st; }
		virtual ~basic_game() = default;
	protected:
		virtual void suspend() {};
		int st = unready;
		//std::chrono::time_point<std::chrono::system_clock> begin, end;
	};
	using basic_game_ptr = std::shared_ptr<basic_game>;
}