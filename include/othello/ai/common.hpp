#pragma once
#include"othello/board.hpp"
#include"othello/moves.hpp"
#include<boost/threadpool.hpp>
namespace bw::othello::ai {
	template<int BoardSize = 8>
	struct game_state
	{
		static_brd<BoardSize> brd;
		color setter_color = col0;
		fast_moves<BoardSize> mvs[2] = {};
	};
	struct ai_option {
		enum class search_method {
			alphabeta = 0,
			mcts,
			minmax,
		};
		enum class eval_method {
			traits = 0,//traditional traits
			pattern,//traditional pattern
			nn//nerual network
		};
		enum class device_type {
			cpu = 0,
			gpu,
		};
		search_method s_mtd = search_method::alphabeta;
		eval_method e_mtd = eval_method::traits;
		device_type device = device_type::cpu;
		int threads = 1;
		int search_depth = 5;
		int time_limit = 0;
		struct mcts_option {
			enum { default_simulations = 1000 };
			int simulations = default_simulations;
			float explore_factor = 4.0;
		};
		mcts_option mcts_opt;
	};
}