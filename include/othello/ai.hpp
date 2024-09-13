#pragma once
#include "ai/minmax.hpp"
#include "ai/alphabeta.hpp"
#include "ai/mcts.hpp"
#include <chrono>
namespace bw::othello::ai {
	REFLECTION(ai_option, s_mtd, e_mtd, threads, search_depth, device);
	struct evaluator {
		evaluator(color setter_color, int board_size) :setter(setter_color), board_size(board_size) { set_algo(); }
		coord best_move(const dynamic_brd& brd, color c) {
			switch (brd.size) {
			case 4:return best_move_static<4>(static_brd<4>(brd), c);
			case 6:return best_move_static<6>(static_brd<6>(brd), c);
			case 8:return best_move_static<8>(static_brd<8>(brd), c);
			case 10:return best_move_static<10>(static_brd<10>(brd), c);
			case 12:return best_move_static<12>(static_brd<12>(brd), c);
			case 14:return best_move_static<14>(static_brd<14>(brd), c);
			case 16:return best_move_static<16>(static_brd<16>(brd), c);
			default: std::unreachable();
			}
		}
		void set_algo() {
			switch (board_size) {
			case 4:set_algo_from_option<4>(); break;
			case 6:set_algo_from_option<6>(); break;
			case 8:set_algo_from_option<8>(); break;
			case 10:set_algo_from_option<10>(); break;
			case 12:set_algo_from_option<12>(); break;
			case 14:set_algo_from_option<14>(); break;
			case 16:set_algo_from_option<16>(); break;
			}
		}
		color setter;
		int board_size = 8;
		ai_option option;
	private:
		std::any algo;
		template<int BoardSize>
		inline int count_corner(const static_brd<BoardSize>& brd, color c) {
			return (brd.getcol({ 0,0 }) == c) + (brd.getcol({ 0,BoardSize - 1 }) == c) + (brd.getcol({ BoardSize - 1,0 }) == c) + (brd.getcol({ BoardSize - 1,BoardSize - 1 }) == c);
		}
		template<int BoardSize = 8>
		coord best_move_static(const static_brd<BoardSize>& brd, color c) {
			if (option.s_mtd == ai_option::search_method::alphabeta) {
				if (option.e_mtd == ai_option::eval_method::traits) {
					return std::any_cast<alphabeta<BoardSize, traits_eval<int>>&>(algo).best_move(brd, c);
				}
				else if (option.e_mtd == ai_option::eval_method::pattern) {
					return coord{};//TO DO
				}
				else if (option.e_mtd == ai_option::eval_method::nn) {
					return coord{};//TO DO
				}
			}
			else if (option.s_mtd == ai_option::search_method::mcts) {
				if (option.e_mtd == ai_option::eval_method::traits) {
					return std::any_cast<mcts<BoardSize, traits_eval<int>>&>(algo).best_move(brd, c);
				}
				else {
					return coord{};//TO DO
				}
			}
			else if (option.s_mtd == ai_option::search_method::minmax) {
				if (option.e_mtd == ai_option::eval_method::traits) {
					return std::any_cast<minmax<BoardSize, traits_eval<int>>&>(algo).best_move(brd, c);
				}
				else if (option.e_mtd == ai_option::eval_method::pattern) {
					return coord{};//TO DO
				}
				else if (option.e_mtd == ai_option::eval_method::nn) {
					return coord{};//TO DO
				}
			}
		}
		template<int BoardSize>
		void set_algo_from_option() {
			if (option.s_mtd == ai_option::search_method::alphabeta) {
				if (option.e_mtd == ai_option::eval_method::traits) {
					algo = alphabeta<BoardSize, traits_eval<int>>(setter, option);
				}
				else if (option.e_mtd == ai_option::eval_method::pattern) {
					//TO DO
				}
				else if (option.e_mtd == ai_option::eval_method::nn) {
					//TO DO
				}
			}
			else if (option.s_mtd == ai_option::search_method::mcts) {
				if (option.e_mtd == ai_option::eval_method::traits) {
					algo = mcts<BoardSize, traits_eval<int>>(setter, option);
				}
				else {

				}
			}
			else if (option.s_mtd == ai_option::search_method::minmax) {
				if (option.e_mtd == ai_option::eval_method::traits) {
					algo = minmax<BoardSize, traits_eval<int>>(setter, option);
				}
				else if (option.e_mtd == ai_option::eval_method::pattern) {
					//TO DO
				}
				else if (option.e_mtd == ai_option::eval_method::nn) {
					//TO DO
				}
			}
		}
	};
}