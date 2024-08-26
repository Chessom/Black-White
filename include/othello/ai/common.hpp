#pragma once
#include"othello/board.hpp"
#include"othello/moves.hpp"
namespace bw::othello::ai {
	template<int BoardSize = 8>
	struct game_state
	{
		static_brd<BoardSize> brd;
		color player_color;
	};
}