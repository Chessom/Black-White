#pragma once
#include"gamer.hpp"
#include"tictactoe/move.hpp"
#include"tictactoe/board.hpp"
namespace bw::tictactoe {

	struct gamer :bw::basic_gamer {
		gamer() { gametype = core::gameid::tictactoe;; };
		gamer(core::color Color, int ID = 0, const std::string& Name = "", int GamerType = invalid)
			:basic_gamer(Color, ID, Name, GamerType, core::gameid::tictactoe) {
		};
		virtual boost::cobalt::task<move> getmove(board& brd, std::chrono::seconds limit = std::chrono::seconds(0)) = 0;
		virtual boost::cobalt::task<void> pass_msg(std::string message) = 0;
		virtual boost::cobalt::task<void> pass_move(move mv) = 0;
		virtual void cancel() {};
	};
	using gamer_ptr = std::shared_ptr<gamer>;
	using timdq = core::timer_deque_t<bw::tictactoe::move>;
	using timdq_ptr = std::shared_ptr<timdq>;
}