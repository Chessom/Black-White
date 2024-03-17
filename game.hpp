#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"gamer.hpp"
namespace bw {
	class basic_game {
	public:
		enum { unready, ready, ongoing, ended, suspended };
		basic_game() = default;
		basic_game(int State) :st(State) {}
/*
* game的基本逻辑：
* 1.更换执子人，并检查当下执子人是否可下
* 2.如果不可下棋，判断对局是否结束
* 3.	如果结束，则宣布对局结果
* 4.	如果未结束，继续对局（特定于具体棋种）
* 5.如果可下棋，则调用执子人的gamer.getmove()，获得move
* 6.	move的有效性由执子人的gamer承担，game默认认为move有效。
* 7.	
* 8.	执行brd.applymove()
* 9.	
*/
		virtual void start(basic_gamer_ptr g0, basic_gamer_ptr g1) = 0;
		int& state() { return st; }
		virtual ~basic_game() = default;
	protected:
		virtual void suspend() {};
		int st = unready;
		std::chrono::time_point<std::chrono::system_clock> begin, end;
	};
}