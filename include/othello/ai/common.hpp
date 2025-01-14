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
			traits = 0,//traditional hand written traits
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
			float temp = 1.0f;
			float fpu = 0.0f;
			bool operator==(const mcts_option& option) const {
				return simulations == option.simulations
					&& explore_factor == option.explore_factor
					&& temp == option.temp
					&& fpu == option.fpu;
			}
		} mcts_opt;
		bool operator==(const ai_option& option) const {
			return s_mtd == option.s_mtd
				&& e_mtd == option.e_mtd
				&& device == option.device
				&& threads == option.threads
				&& search_depth == option.search_depth
				&& time_limit == option.time_limit
				&& mcts_opt == option.mcts_opt;
		}
	};
}