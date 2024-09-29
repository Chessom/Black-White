#pragma once
#include"eval_methods.hpp"
namespace bw::othello::ai {
	template<int BoardSize, class EvalMethod>
	struct alphabeta {
		alphabeta(color setter_color, const ai_option& option) :player(setter_color), option(option) {};
		enum { inf = 1000 };
		coord best_move(const static_brd<BoardSize> brd, color setter_color) {
			if (option.threads == 1) {
				return choose_move_single_thread(brd, setter_color);
			}
			else {
				return choose_move_multi_thread(brd, setter_color);
			}
		}
		void reset() {
			last_mvs.clear();
		}
		ai_option option;
		std::vector<std::pair<coord, float>> last_mvs;
	private:
		coord choose_move_single_thread(const static_brd<BoardSize>& brd, color c) {
			moves mvs;
			mvs.update(brd, c);
			move mv;
			float points = -inf;
			static_brd<BoardSize> board = brd;
			int index = 0;
			if (mvs.size != 0) {
				last_mvs.clear();
				for (int i = 0; i < mvs.size; ++i) {
					board.apply_move(mvs.coords[i], c);
					auto mark = -nega_alphabeta_evaluate(board, op_col(c), option.search_depth, -inf, inf);
					last_mvs.push_back({ mvs.coords[i], mark });
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
						*res = -nega_alphabeta_evaluate(new_brd, op_col(c), option.search_depth, -inf, inf);
						});
				}
				*res0 = -nega_alphabeta_evaluate(brd0, op_col(c), option.search_depth, -inf, inf);
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
		float eval_end_game_board(const static_brd<BoardSize>& brd, color setter_color) {
			auto value = e_mthd.eval_end_game_board(brd, player);
			if (setter_color == player) {
				return value;
			}
			else {
				return -value;
			}
		}
		float eval_board(const static_brd<BoardSize>& brd, color setter_color) {
			auto value = e_mthd.eval_board(brd, setter_color, player);
			if (setter_color == player) {
				return value;
			}
			else {
				return -value;
			}
		}
		template<int Size>
		struct search_tree_node {
			game_state<Size> st;
			int depth;
			float alpha, beta, value;
		};
		float nega_alphabeta_evaluate(const bitbrd_t<BoardSize>& brd, color setter_color, int depth, float alpha, float beta) {
			uint64_t mvs_0 = brd.getmoves(setter_color);
			if (!mvs_0) {
				uint64_t mvs_1 = brd.getmoves(op_col(setter_color));
				if (!mvs_1) { // leaf node
					return eval_end_game_board(brd, setter_color);
				}
			}
			if (depth == 0) {
				return eval_board(brd, setter_color);
			}
			if (!mvs_0) {
				alpha = std::max(alpha, -nega_alphabeta_evaluate(brd, op_col(setter_color), depth, -beta, -alpha));
			}
			else {
				auto brd_ = brd;
				uint64_t iter = 0;
				for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
					iter = mvs_0 & (-ll(mvs_0));
					brd_.apply_move(iter, setter_color);
					alpha = std::max(alpha, -nega_alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, -beta, -alpha));
					if (beta <= alpha) {
						break;
					}
					brd_ = brd;
				}
			}
			return alpha;
		}
		float nega_alphabeta_evaluate(const arrbrd_t<BoardSize>& brd, color setter_color, int depth, float alpha, float beta) {
			moves mvs_0(brd, setter_color);
			if (mvs_0.empty()) {
				moves mvs_1(brd, op_col(setter_color));
				if (mvs_1.empty()) { // leaf node
					return eval_end_game_board(brd, setter_color);
				}
			}
			if (depth == 0) {
				return eval_board(brd, setter_color);
			}
			if (mvs_0.empty()) {
				alpha = std::max(alpha, -nega_alphabeta_evaluate(brd, op_col(setter_color), depth - 1, -beta, -alpha));
			}
			else {
				auto brd_ = brd;
				for (const auto& crd : mvs_0.coords) {
					brd_.apply_move(crd, setter_color);
					alpha = std::max(alpha, -nega_alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, -beta, -alpha));
					if (beta <= alpha) {
						break; // Alpha cut-off
					}
					brd_ = brd;
				}
			}
			return alpha;
		}
		color player;
		EvalMethod e_mthd;
	};
};