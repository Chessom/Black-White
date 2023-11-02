#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"gamer.hpp"
#include"othello.move.hpp"
#include"othello.moves.hpp"
#include"othello.board.hpp"
namespace bw {
	class mtxdeque {
	public:
		std::deque<bw::core::coord> q;
		std::mutex mtx;
		std::condition_variable conv;
	};
	using mtxdq_ptr = std::shared_ptr<mtxdeque>;
}
namespace bw::othello {
	
	class gamer :public bw::basic_gamer {
	public:
		enum { pass, end, suspended };
		gamer() { gametype = core::gameid::unspecific; };
		gamer(core::color Color, int ID = 0, const std::string& Name = "", int GamerType = invalid)
			:basic_gamer(Color, ID, Name, GamerType, core::gameid::othello) {
		};
		virtual move getmove(dynamic_brd& brd, std::chrono::seconds limit = std::chrono::seconds(0)) = 0;
		virtual std::string get_name() = 0;
		virtual ~gamer() = default;
		bool is_human() const {
			return gamertype == human;
		}
		bool is_computer() const {
			return gamertype == computer;
		}
		moves mvs;
		mtxdq_ptr dqptr = nullptr;
	};
	using gamer_ptr = std::shared_ptr<gamer>;
}