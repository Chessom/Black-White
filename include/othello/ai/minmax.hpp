#pragma once
#include"common.hpp"
namespace bw::othello::ai {
	template<int BoardSize, typename MovesType = moves>
	struct minmax {
		minmax(color setter_color) :setter_col(setter_color) {  };
		enum { inf = 1000 };
		struct tree_node {
			game_state<BoardSize> st;
			int depth;
			MovesType mvs[2];
			int value;
		};
		//std::vector<tree_node> tree_node_stack;

		int recursive_minmax_evaluate(const bitbrd_t<BoardSize>& brd, color setter_color, int depth) {
			uint64_t mvs_0 = brd.getmoves(setter_color);
			if (!mvs_0) {
				uint64_t mvs_1 = brd.getmoves(op_col(setter_color));
				if (!mvs_1) {//leaf node
					return brd.countpiece(setter_col) - brd.countpiece(op_col(setter_col));
				}
			}
			if (depth == 0) {
				uint64_t mvs_1 = brd.getmoves(op_col(setter_color));
				if (setter_color == setter_col) {
					return std::popcount(mvs_0) - 2 * std::popcount(mvs_1);
				}
				else {
					return std::popcount(mvs_1) - 2 * std::popcount(mvs_0);
				}
			}
			if (setter_color != setter_col) {
				int a = inf;
				auto brd_ = brd;
				uint64_t iter = 0;
				for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
					iter = mvs_0 & (-ll(mvs_0));
					brd_.applymove(iter, setter_color);
					a = std::min(a, recursive_minmax_evaluate(brd_, setter_col, depth - 1));
					brd_ = brd;
				}
				return a;
			}
			else {
				int b = -inf;
				auto brd_ = brd;
				uint64_t iter = 0;
				for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
					iter = mvs_0 & (-ll(mvs_0));
					brd_.applymove(iter, setter_color);
					b = std::max(b, recursive_minmax_evaluate(brd_, op_col(setter_color), depth - 1));
					brd_ = brd;
				}
				return b;
			}
			//if node is a terminal node or depth = 0
			//	return the heuristic value of node
			//	// 如果轮到对手走棋，是极小节点，选择一个得分最小的走法
			//if the adversary is to play at node
			//	let α : = +∞
			//	foreach child of node
			//	α : = min(α, minimax(child, depth - 1))
			//	// 如果轮到我们走棋，是极大节点，选择一个得分最大的走法
			//else { we are to play at node }
			//	let α : = -∞
			//	foreach child of node
			//	α : = max(α, minimax(child, depth - 1))
			//	return α;
			return 0;
		}

		int recursive_minmax_evaluate(const arrbrd_t<BoardSize>& brd, color setter_color, int depth) {
			moves mvs_0(brd, setter_color);
			if (!mvs_0.empty()) {
				moves mvs_1(brd, op_col(setter_color));
				if (!mvs_1.empty()) {//leaf node
					return brd.countpiece(setter_col) - brd.countpiece(op_col(setter_col));
				}
			}
			if (depth == 0) {
				moves mvs_1(brd, op_col(setter_color));
				if (setter_color == setter_col) {
					return mvs_0.coords.size() - 2 * mvs_1.coords.size();
				}
				else {
					return mvs_1.coords.size() - 2 * mvs_0.coords.size();
				}
			}
			if (setter_color != setter_col) {
				int a = inf;
				auto brd_ = brd;
				for (const auto& crd : mvs_0.coords) {
					brd_.applymove(crd, setter_color);
					a = std::min(a, recursive_minmax_evaluate(brd_, setter_col, depth - 1));
					brd_ = brd;
				}
				return a;
			}
			else {
				int b = -inf;
				auto brd_ = brd;
				for (const auto& crd : mvs_0.coords) {
					brd_.applymove(crd, setter_color);
					b = std::max(b, recursive_minmax_evaluate(brd_, op_col(setter_color), depth - 1));
					brd_ = brd;
				}
				return b;
			}
		}
		
		int alphabeta_evaluate(const bitbrd_t<BoardSize>& brd, color setter_color, int depth, int alpha, int beta) {
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
					return std::popcount(mvs_0) - 2 * std::popcount(mvs_1);
				}
				else {
					return std::popcount(mvs_1) - 2 * std::popcount(mvs_0);
				}
			}
			if (setter_color != setter_col) {
				int a = inf;
				auto brd_ = brd;
				uint64_t iter = 0;
				for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
					iter = mvs_0 & (-ll(mvs_0));
					brd_.applymove(iter, setter_color);
					a = std::min(a, alphabeta_evaluate(brd_, setter_col, depth - 1, alpha, beta));
					brd_ = brd;
					beta = std::min(beta, a);
					if (beta <= alpha) {
						break; // Beta cut-off
					}
				}
				return a;
			}
			else {
				int b = -inf;
				auto brd_ = brd;
				uint64_t iter = 0;
				for (; mvs_0; mvs_0 = mvs_0 & (mvs_0 - 1)) {
					iter = mvs_0 & (-ll(mvs_0));
					brd_.applymove(iter, setter_color);
					b = std::max(b, alphabeta_evaluate(brd_, op_col(setter_color), depth - 1, alpha, beta));
					brd_ = brd;
					alpha = std::max(alpha, b);
					if (beta <= alpha) {
						break; // Alpha cut-off
					}
				}
				return b;
			}
		}

		int end_game_evaluate() {

		}
		color setter_col;
	};
};