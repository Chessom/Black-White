#pragma once
#include"common.hpp"
namespace bw::othello::ai {
	template<int BoardSize, typename EvalMethod, typename MovesType = moves>
	struct minmax {
		minmax(color setter_color, const ai_option& option) :player(setter_color), option(option) {};
		enum { inf = 1000 };
		coord best_move(const static_brd<BoardSize> brd, color setter_color) {
			if (option.threads == 1) {
				return choose_move_single_thread(brd, setter_color);
			}
			else {
				return choose_move_multi_thread(brd, setter_color);
			}
		}
		ai_option option;
	private:
		coord choose_move_single_thread(const static_brd<BoardSize>& brd, color c) {
			moves mvs;
			mvs.update(brd, c);
			move mv;
			float points = -inf;
			static_brd<BoardSize> board = brd;
			int index = 0;
			if (mvs.size != 0) {
				for (int i = 0; i < mvs.size; ++i) {
					board.apply_move(mvs.coords[i], c);
					auto mark = recursive_minmax_evaluate(board, op_col(c), option.search_depth);
					if (mark > points) {
						points = mark;
						index = i;
					}
					board = brd;
				}
			}
			return mvs.coords[index];
		}
		coord choose_move_multi_thread(const static_brd<BoardSize>& brd, color c) {
			moves mvs;
			mvs.update(brd, c);
			move mv;
			vector<std::shared_ptr<float>> scores;
			int index = 0;
			float points = -inf;
			if (mvs.size != 0) {
				boost::threadpool::pool pool(option.threads - 1);
				static_brd<BoardSize> brd0 = brd;
				brd0.apply_move(mvs.coords[0], c);
				auto res0 = std::make_shared<float>(0.0f);
				scores.push_back(res0);
				for (int i = 1; i < mvs.size; ++i) {
					static_brd<BoardSize> _brd = brd;
					_brd.apply_move(mvs.coords[i], c);
					auto res = std::make_shared<float>(0.0f);
					scores.push_back(res);
					pool.schedule([new_brd = std::move(_brd), res, c, this] {
						*res = recursive_minmax_evaluate(new_brd, op_col(c), option.search_depth);
						});
				}
				*res0 = recursive_minmax_evaluate(brd0, op_col(c), option.search_depth);
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
		struct tree_node {
			game_state<BoardSize> st;
			int depth;
			MovesType mvs[2];
			int value;
		};
		//std::vector<tree_node> tree_node_stack;
		float eval_end_game_board(const static_brd<BoardSize>& brd) {
			return e_mthd.eval_end_game_board(brd, player);
		}
		float eval_board(const static_brd<BoardSize>& brd, color setter_color) {
			return e_mthd.eval_board(brd, setter_color, player);
		}
		int recursive_minmax_evaluate(const bitbrd_t<BoardSize>& brd, color setter_color, int depth) {
			uint64_t mvs_0 = brd.getmoves(setter_color);
			if (!mvs_0) {
				uint64_t mvs_1 = brd.getmoves(op_col(setter_color));
				if (!mvs_1) {//leaf node
					return eval_end_game_board(brd);
				}
			}
			if (depth == 0) {
				return eval_board(brd, setter_color);
			}
			if (setter_color != player) {
				int a = inf;
				auto brd_ = brd;
				uint64_t iter = 0;
				
				for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
					iter = mvs_0 & (-ll(mvs_0));
					brd_.apply_move(iter, setter_color);
					a = std::min(a, recursive_minmax_evaluate(brd_, player, depth - 1));
					brd_ = brd;
				}
				if (!mvs_0) {
					a = std::min(a, recursive_minmax_evaluate(brd_, player, depth - 1));
				}
				return a;
			}
			else {
				int b = -inf;
				auto brd_ = brd;
				uint64_t iter = 0;
				for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
					iter = mvs_0 & (-ll(mvs_0));
					brd_.apply_move(iter, setter_color);
					b = std::max(b, recursive_minmax_evaluate(brd_, op_col(setter_color), depth - 1));
					brd_ = brd;
				}
				if (!mvs_0) {
					b = std::max(b, recursive_minmax_evaluate(brd_, op_col(setter_color), depth - 1));
				}
				return b;
			}
			return 0;
		}

		int recursive_minmax_evaluate(const arrbrd_t<BoardSize>& brd, color setter_color, int depth) {
			moves mvs_0(brd, setter_color);
			if (!mvs_0.empty()) {
				moves mvs_1(brd, op_col(setter_color));
				if (!mvs_1.empty()) {//leaf node
					return eval_end_game_board(brd);
				}
			}
			if (depth == 0) {
				return eval_board(brd, setter_color);
			}
			if (setter_color != player) {
				int a = inf;
				auto brd_ = brd;
				for (const auto& crd : mvs_0.coords) {
					brd_.apply_move(crd, setter_color);
					a = std::min(a, recursive_minmax_evaluate(brd_, player, depth - 1));
					brd_ = brd;
				}
				if (!mvs_0.empty()) {
					a = std::min(a, recursive_minmax_evaluate(brd_, player, depth - 1));
				}
				return a;
			}
			else {
				int b = -inf;
				auto brd_ = brd;
				for (const auto& crd : mvs_0.coords) {
					brd_.apply_move(crd, setter_color);
					b = std::max(b, recursive_minmax_evaluate(brd_, op_col(setter_color), depth - 1));
					brd_ = brd;
				}
				if (!mvs_0.empty()) {
					b = std::max(b, recursive_minmax_evaluate(brd_, op_col(setter_color), depth - 1));
				}
				return b;
			}
		}
		
		int end_game_evaluate() {

		}
		EvalMethod e_mthd;
		color player;
	};
};