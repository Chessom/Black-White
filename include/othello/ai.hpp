#pragma once
#include "ai/minmax.hpp"
#include "ai/alphabeta.hpp"
#include <boost/threadpool.hpp>
#include <syncstream>
#include <chrono>
namespace bw::othello::ai {
	struct ai_option{
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
		int threads = 1;
		int search_depth = 5;
		device_type device = device_type::cpu;
	};
	REFLECTION(ai_option, s_mtd, e_mtd, threads, search_depth, device);
	struct evaluator {
		evaluator(color setter_color) :setter(setter_color) {}
		template<int BoardSize = 8>
		float evaluate(const static_brd<BoardSize>& brd, color c) {
			if (option.s_mtd == ai_option::search_method::alphabeta) {
				if (option.e_mtd == ai_option::eval_method::traits) {
					alphabeta<BoardSize, traits_eval> algo(setter);
					return algo.nega_alphabeta_evaluate(brd, c, option.search_depth, -alphabeta<BoardSize, traits_eval>::inf, alphabeta<BoardSize, traits_eval>::inf);
				}
				else if (option.e_mtd == ai_option::eval_method::pattern) {
					return 0;//TO DO
				}
				else if(option.e_mtd == ai_option::eval_method::nn){
					return 0;//TO DO
				}
			}
		}
		float evaluate_dynamic(const dynamic_brd& brd, color c) {
			switch (brd.size) {
			case 4:return evaluate<4>(static_brd<4>(brd), c);
			case 6:return evaluate<6>(static_brd<6>(brd), c);
			case 8:return evaluate<8>(static_brd<8>(brd), c);
			case 10:return evaluate<10>(static_brd<10>(brd), c);
			case 12:return evaluate<12>(static_brd<12>(brd), c);
			case 14:return evaluate<14>(static_brd<14>(brd), c);
			case 16:return evaluate<16>(static_brd<16>(brd), c);
			}
			return 0;
		}
		coord best_move(const dynamic_brd& brd, color c) {
			if (option.threads == 1) {
				return choose_move_single_thread(brd, c);
			}
			else
			{
				return choose_move_multi_thread(brd, c);
			}
		}
		color setter;
		ai_option option;
	private:
		template<int BoardSize>
		inline int count_corner(const static_brd<BoardSize>& brd, color c) {
			return (brd.getcol({ 0,0 }) == c) + (brd.getcol({ 0,BoardSize - 1 }) == c) + (brd.getcol({ BoardSize - 1,0 }) == c) + (brd.getcol({ BoardSize - 1,BoardSize - 1 }) == c);
		}
		coord choose_move_single_thread(const dynamic_brd& brd, color c) {
			moves mvs;
			mvs.update(brd, c);
			move mv;
			float points = -100000;
			dynamic_brd board = brd;
			int index = 0;
			if (mvs.size != 0) {
				for (int i = 0; i < mvs.size; ++i) {
					board.applymove(mvs.coords[i], c);
					auto mark = -evaluate_dynamic(board, op_col(c));
					if (mark > points) {
						points = mark;
						index = i;
					}
					board = brd;
				}
			}
			return mvs.coords[index];
		}
		coord choose_move_multi_thread(const dynamic_brd& brd, color c) {
			moves mvs;
			mvs.update(brd, c);
			move mv;
			vector<std::shared_ptr<float>> scores;
			int index = 0;
			float points = -10000;
			if (mvs.size != 0) {
				boost::threadpool::pool pool(option.threads - 1);
				dynamic_brd brd0 = brd;
				brd0.applymove(mvs.coords[0], c);
				auto res0 = std::make_shared<float>(0.0f);
				scores.push_back(res0);
				for (int i = 1; i < mvs.size; ++i) {
					dynamic_brd new_brd = brd;
					new_brd.applymove(mvs.coords[i], c);
					auto res = std::make_shared<float>(0.0f);
					scores.push_back(res);
					pool.schedule([&scores, new_brd, i,res, c, this] {
						*res = -evaluate_dynamic(new_brd, op_col(c));
					});
				}
				*res0 = -evaluate_dynamic(brd0, op_col(c));
				pool.wait();
			}
			for (int i = 0; i < mvs.size; ++i) {
				if (auto score = *scores[i]; score > points) {
					points = score;
					index = i;
				}
			}
			return mvs.coords[index];
		}
	};
}