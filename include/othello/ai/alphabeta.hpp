#pragma once
#include"eval_methods.hpp"
namespace bw::othello::ai {
	template<int BoardSize, class EvalMethod>
	struct alphabeta {
		alphabeta(color setter_color) :setter_col(setter_color) {};
		enum { inf = 1000 };
		float eval_end_game_board(const static_brd<BoardSize>& brd) {
			return EvalMethod::eval_end_game_board(brd, setter_col);
		}
		float eval_board(const static_brd<BoardSize>& brd, color setter_color) {
			return EvalMethod::eval_board(brd, setter_color, setter_col);
		}
		template<int Size>
		struct search_tree_node {
			game_state<Size> st;
			int depth;
			float alpha, beta, value;
		};
#ifdef BW_AI_TEST
		std::map<color, std::string> col2str = { {col0, std::string(gettext("black"))},{col1, std::string(gettext("white"))} };
		std::string level_str(int level, std::string tab = "    ") {
			std::string ret;
			for (int i = 0; i < level; ++i) {
				ret += tab;
			}
			return ret;
		}
#endif // BW_AI_TEST
		float nega_alphabeta_evaluate(const bitbrd_t<BoardSize>& brd, color setter_color, int depth, float alpha, float beta) {
			uint64_t mvs_0 = brd.getmoves(setter_color);
			if (!mvs_0) {
				uint64_t mvs_1 = brd.getmoves(op_col(setter_color));
				if (!mvs_1) { // leaf node
					return eval_end_game_board(brd);
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
					brd_.applymove(iter, setter_color);
					alpha = std::max(alpha, -nega_alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, -beta, -alpha));
					if (beta <= alpha) {
						break;
					}
					brd_ = brd;
				}
#ifdef BW_AI_TEST
				std::println("{}color:{} depth:{} alpha:{} beta:{}", level_str(3 - depth), col2str[setter_color], depth, alpha, beta);
#endif // BW_AI_TEST
			}
			return alpha;
		}
		float nega_alphabeta_evaluate(const arrbrd_t<BoardSize>& brd, color setter_color, int depth, float alpha, float beta) {
			moves mvs_0(brd, setter_color);
			if (mvs_0.empty()) {
				moves mvs_1(brd, op_col(setter_color));
				if (mvs_1.empty()) { // leaf node
					return eval_end_game_board(brd);
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
					brd_.applymove(crd, setter_color);
					alpha = std::max(alpha, -nega_alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, -beta, -alpha));
					if (beta <= alpha) {
						break; // Alpha cut-off
					}
					brd_ = brd;
				}
			}
			return alpha;
		}
		float alphabeta_evaluate(const bitbrd_t<BoardSize>& brd, color setter_color, int depth, float alpha, float beta) {
			uint64_t mvs_0 = brd.getmoves(setter_color);
			if (!mvs_0) {
				uint64_t mvs_1 = brd.getmoves(op_col(setter_color));
				if (!mvs_1) { // leaf node
					return brd.countpiece(setter_col) - brd.countpiece(op_col(setter_col));
				}
			}
			if (depth == 0) {
				uint64_t mvs_1 = brd.getmoves(op_col(setter_color));
				if (setter_color == setter_col) {
					return std::popcount(mvs_0);
				}
				else {
					return -std::popcount(mvs_0);
				}
			}
			if (setter_color != setter_col) {//min node
				if (!mvs_0) {
					beta = std::min(beta, alphabeta_evaluate(brd, op_col(setter_color), depth - 1, alpha, beta));
				}
				else {
					auto brd_ = brd;
					uint64_t iter = 0;
					for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
						iter = mvs_0 & (-ll(mvs_0));
						brd_.applymove(iter, setter_color);
						beta = std::min(beta, alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, alpha, beta));
						if (beta <= alpha) {
							break; // Beta cut-off
						}
						brd_ = brd;
					}
				}
				return beta;
			}
			else {//max node
				if (!mvs_0) {
					alpha = std::max(alpha, alphabeta_evaluate(brd, op_col(setter_color), depth - 1, alpha, beta));
				}
				else {
					auto brd_ = brd;
					uint64_t iter = 0;
					for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
						iter = mvs_0 & (-ll(mvs_0));
						brd_.applymove(iter, setter_color);
						alpha = std::max(alpha, alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, alpha, beta));
						if (beta <= alpha) {
							break; // Alpha cut-off
						}
						brd_ = brd;
					}
				}
				return alpha;
			}
		}
		float alphabeta_evaluate(const arrbrd_t<BoardSize>& brd, color setter_color, int depth, float alpha, float beta) {
			moves mvs_0(brd, setter_color);
			if (mvs_0.empty()) {
				moves mvs_1(brd, op_col(setter_color));
				if (mvs_1.empty()) { // leaf node
					return brd.countpiece(setter_col) - brd.countpiece(op_col(setter_col));
				}
			}
			if (depth == 0) {
				moves mvs_1(brd, op_col(setter_color));
				if (setter_color == setter_col) {
					return mvs_0.coords.size();
				}
				else {
					return -ll(mvs_0.coords.size());
				}
			}
			if (setter_color != setter_col) {//min node
				if (mvs_0.empty()) {
					beta = std::min(beta, alphabeta_evaluate(brd, op_col(setter_color), depth - 1, alpha, beta));
				}
				else {
					auto brd_ = brd;
					for (const auto& crd : mvs_0.coords) {
						brd_.applymove(crd, setter_color);
						beta = std::min(beta, alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, alpha, beta));
						if (beta <= alpha) {
							break; // Beta cut-off
						}
						brd_ = brd;
					}
				}
				return beta;
			}
			else {//max node
				if (mvs_0.empty()) {
					alpha = std::max(alpha, alphabeta_evaluate(brd, op_col(setter_color), depth - 1, alpha, beta));
				}
				else {
					auto brd_ = brd;
					for (const auto& crd : mvs_0.coords) {
						brd_.applymove(crd, setter_color);
						alpha = std::max(alpha, alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, alpha, beta));
						if (beta <= alpha) {
							break; // Alpha cut-off
						}
						brd_ = brd;
					}
				}
				return alpha;
			}
		}
		color setter_col;
	};
};