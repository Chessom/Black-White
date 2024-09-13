#pragma once
#include <numeric>
#include <type_traits>
#include "eval_methods.hpp"
#include "tree.hpp"
namespace bw::othello::ai {
    template<int BoardSize>
    using fast_move = std::conditional<BoardSize <= 8, uint64_t, coord>::type;
    template<int BoardSize>
    inline constexpr fast_move<BoardSize> pass_move() {
        if constexpr (BoardSize <= 8) {
            return 0;
        }
        else {
            return coord{ -1,-1 };
        }
    }
    template<int BoardSize>
    inline bool is_pass_move(const fast_move<BoardSize>& mv) {
        if constexpr (BoardSize <= 8) {
            return !mv;
        }
        else {
            return mv == coord{ -1,-1 };
        }
    }
    template<int BoardSize>
    inline coord fast_move_to_coord(const fast_move<BoardSize>& mv) {
        if constexpr (BoardSize <= 8) {
            return bitbrd_t<BoardSize>::bit2crd(mv);
        }
        else {
            return mv;
        }
    }
    template<int BoardSize>
    inline fast_move<BoardSize> coord_to_fast_move(const coord& mv) {
        if constexpr (BoardSize <= 8) {
            return bitbrd_t<BoardSize>::crd2bit(mv);
        }
        else {
            return mv;
        }
    }

    template<int BoardSize>
    struct mcts_tree_node {
        enum { inf = 1000 };
        enum { pass_move = 0 };
        color setter = none;
        fast_move<BoardSize> mv{};
        bool is_terminal = false;
        int game_times = 0;
        float pre_prob = 0.0f;
        float score = 0.0f;
    };

    template<int BoardSize>
    int crd_to_int(const coord& crd) {
        return crd.x * BoardSize + crd.y;
    }
    template<int BoardSize>
    coord int_to_crd(int crd) {
        return { crd / BoardSize,crd % BoardSize };
    }
#ifdef _HISTORY
    template<int BoardSize>
    struct fast_game {
        using state_type = game_state<BoardSize>;
        fast_game() = default;
        fast_game(const static_brd<BoardSize>& brd, color setter) :rounds(0) {
            states.reserve(BoardSize * BoardSize);
            states.push_back(state_type{ brd,setter,{} });
        }
        state_type& crt_state() {
            return states.back();
        }
        void move(const fast_move<BoardSize>& mv) {
            state_type st = crt_state();
            st.brd.applymove(mv, st.setter_color);
            st.setter_color = op_col(st.setter_color);
            states.push_back(st);
            ++rounds;
        }
        void pass() {
            state_type st = crt_state();
            st.setter_color = op_col(st.setter_color);
            states.push_back(st);
            ++rounds;
        }
        color winner() {
            state_type& st = crt_state();
            int num[2] = { st.brd.count(col0) ,st.brd.count(col1) };
            return num[col0] > num[col1] ? col0 : num[col0] < num[col1] ? col1 : none;
        }
    private:
        vector<state_type> states;
        int rounds = 0;
    };
#else
    template<int BoardSize>
    struct fast_game {
        using state_type = game_state<BoardSize>;
        fast_game() = default;
        fast_game(const static_brd<BoardSize>& brd, color setter) {
            crt = state_type{ brd,setter,{} };
        }
        state_type& crt_state() {
            return crt;
        }
        void move(const fast_move<BoardSize>& mv) {
            state_type st = crt_state();
            crt.brd.applymove(mv, crt.setter_color);
            crt.setter_color = op_col(crt.setter_color);
            ++rounds;
        }
        void pass() {
            crt.setter_color = op_col(crt.setter_color);
            ++rounds;
        }
        color winner() {
            state_type& st = crt_state();
            int num[2] = { st.brd.count(col0) ,st.brd.count(col1) };
            return num[col0] > num[col1] ? col0 : num[col0] < num[col1] ? col1 : none;
        }
    private:
        state_type crt;
        //vector<state_type> states;
        int rounds = 0;
    };
#endif
	template<int BoardSize, class EvalMethod>
	struct mcts {
        using node_type = mcts_tree_node<BoardSize>;
        using search_tree_type = tree<node_type>;
        using moves_type = fast_moves<BoardSize>;
        using iter_type = search_tree_type::iterator;
        mcts(color setter_color, const ai_option& option) :player(setter_color), option(option) {

        };
        
        enum { inf = 1000 };
        coord best_move(const static_brd<BoardSize> brd, color setter_color) {
            if (option.threads == 1) {
                return choose_move_single_thread(brd, setter_color);
            }
            else {
                return choose_move_single_thread(brd, setter_color);
            }
        }
        ai_option option;
        ~mcts() {
            spdlog::trace("mcts destructed");
        }
    private:
        int rand_int() {
            return rand();
        }
        coord choose_move_single_thread(const static_brd<BoardSize>& brd, color c) {
            using namespace std::chrono;
            int simulate_times = 0;
            auto start = high_resolution_clock::now();
            long duration = 0;
            auto origin_game = fast_game<BoardSize>(brd, c);
            bool found_subtree = false;
            if (!search_tree.empty()) {
                auto head = search_tree.child(search_tree.begin(), last_move_index);
                auto sim_brd = fast_gm.crt_state().brd;
                auto mvs = static_brd<BoardSize>::deduce_moves(fast_gm.crt_state().brd, brd);
                auto op_color = op_col(c);
				bool found_move_history = false;
                if (mvs.empty()) {//我方连下
                    auto&& sub_tr = search_tree.move_out(head.begin());
                    search_tree.clear();
                    search_tree = std::move(sub_tr);
                    found_subtree = true;
                }
				else {
					do {
						for (auto& crd : mvs) {
							moves m(sim_brd, op_color);
							if (m.find(crd) != moves::npos) {
								sim_brd.applymove(crd, op_color);
							}
							else {
								break;
							}
						}
						if (sim_brd == brd) {
							found_move_history = true;
							break;
						}
                        sim_brd = fast_gm.crt_state().brd;
					} while (std::next_permutation(mvs.begin(), mvs.end()));
					if (found_move_history) {
						auto new_head = head;
                        int iter_times = 0;
                        while (new_head.node && new_head.begin() != new_head.end() && iter_times < mvs.size()) {
                            if (new_head->setter == player) {
                                new_head++;
                            }
                            else {
                                bool found = false;
								for (auto it = new_head.begin(); it != new_head.end(); ++it) {
									if (it->mv == coord_to_fast_move<BoardSize>(mvs[iter_times])) {
										new_head = it;
										found = true;
										break;
									}
								}
								if (!found) {
									break;
								}
								else {
									iter_times++;
                                    found = false;
								}
                            }
						}
                        if (iter_times == mvs.size()) {
							auto sub_tr = search_tree.move_out(new_head);
							search_tree = sub_tr;
							found_subtree = true;
                        }
                        else {
                            found_subtree = false;
                        }
					}
				}
            }
            fast_gm = origin_game;
            if (!found_subtree) {
                search_tree.clear();
				search_tree.set_head(
					node_type{
						op_col(c),
						{},
						false,
						0,
						0.0f,
						0.0f
					});
                expand(search_tree.begin());
            }
            while(true){
                auto node_iter = select();
                if (node_iter->game_times) {
					expand(node_iter);
                    if (!node_iter->is_terminal) {
						auto rnd_node_iter = node_iter.begin();
						std::advance(rnd_node_iter, rand_int() % node_iter.number_of_children());
						float result = simulate(rnd_node_iter);
						back_propagate(rnd_node_iter, result);
                    }
                    else {
                        float result = simulate(node_iter);
                        back_propagate(node_iter, result);
                    }
                }
                else {
					float result = simulate(node_iter);
					back_propagate(node_iter, result);
                }
                fast_gm = origin_game;

                simulate_times++;

                if (option.mcts_opt.simulations) {
                    if (simulate_times >= option.mcts_opt.simulations) {
                        break;
                    }
                }
                else if(option.time_limit){
                    if (simulate_times % 1000 == 0) {
                        duration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
                        if (duration >= option.time_limit * 1000) {
                            break;
                        }
                    }
                }
                else {
                    if (simulate_times >= ai_option::mcts_option::default_simulations) {
                        break;
                    }
                }
            }
            auto mv = get_best_move();
            fast_gm.move(mv);
            return fast_move_to_coord<BoardSize>(mv);
        }
        coord choose_move_multi_thread(const static_brd<BoardSize>& brd, color c) {
            return {};
        }
        auto select() {
            iter_type node_iter = search_tree.begin();
            while (node_iter.number_of_children()) {
                float max_ucb = -node_type::inf;
                auto max_iter = node_iter.begin();
                int total_visit = node_iter->game_times;
                for (auto it = node_iter.begin(); it != node_iter.end(); ++it) {
                    if (auto new_ucb = ucb(it, total_visit); new_ucb > max_ucb) {
                        max_iter = it;
                        max_ucb = new_ucb;
                    }
                }
                node_iter = max_iter;
				if (is_pass_move<BoardSize>(node_iter->mv)) {
					fast_gm.pass();
				}
				else {
					fast_gm.move(node_iter->mv);
				}
            }
            return node_iter;
        }
        void expand(auto node_iter) {
            auto& node = *node_iter;
            auto& st = fast_gm.crt_state();
            moves_type legal_moves = get_moves(st.brd, st.setter_color);
            if (empty_moves<BoardSize>(legal_moves)) {
                moves_type op_mvs = get_moves(st.brd, op_col(st.setter_color));
                if (empty_moves<BoardSize>(op_mvs)) {
                    node.is_terminal = true;
                    return;
                }
                else
                {
                    search_tree.append_child(node_iter, node_type{ st.setter_color, pass_move<BoardSize>(), false, 0, 1.0f, 0.0f});
                }
            }
            else
            {
                int children_size = 0;
                if constexpr (BoardSize <= 8) {
                    for (; legal_moves; legal_moves = legal_moves & (legal_moves - 1)) {
                        fast_move<BoardSize> iter = legal_moves & (-ll(legal_moves));
                        search_tree.append_child(node_iter, node_type{ st.setter_color, iter, false, 0, 0.0f, 0.0f});
                        children_size++;
                    }
                }
				else
				{
					for (const auto& mv : legal_moves) {
						search_tree.append_child(node_iter, node_type{ st.setter_color, mv, false, 0, 0.0f, 0.0f});
                        children_size++;
                    }
				}
                _pre_probs.clear();
                calculate_prior_probs(node_iter, _pre_probs);
                auto prob_it = _pre_probs.begin();
                for (auto it = node_iter.begin(); it != node_iter.end(); ++it, ++prob_it) {
                    it->pre_prob = *prob_it;
                }
            }
        }
        float simulate(auto node_iter) {
            auto node = *node_iter;
            if (node.is_terminal) {
                auto winner = fast_gm.winner();
                if (winner == player) {
                    return 1;
                }
                else if(winner == op_col(player)) {
                    return -1;
                }
                else{
                    return 0;
                }
                return e_mthd.eval_end_game_board(fast_gm.crt_state().brd, player);
            }
            if (is_pass_move<BoardSize>(node.mv)) {
                fast_gm.pass();
            }
            else {
                fast_gm.move(node.mv);
            }
            return evaluate(fast_gm.crt_state().brd, fast_gm.crt_state().setter_color);
            //return e_mthd.eval_board(fast_gm.crt_state().brd, fast_gm.crt_state().setter_color, player);
        }
        void back_propagate(auto node_iter, float result) {
            while (node_iter != nullptr) {
                auto& node = *node_iter;
                node.game_times++;
                node.score += node.setter == player ? result : -result;
                if (!search_tree.is_head(node_iter)) {
                    node_iter = search_tree.parent(node_iter);
                }
                else {
                    break;
                }
            }
        }
        static float ucb(const iter_type& node, int parent_num) {
            if (node->game_times == 0) return node_type::inf;
            auto value =
                //Q value
                node->score / node->game_times 
                //explore
                + 1.414f * node->pre_prob * std::sqrt(parent_num) / (1 + node->game_times);
            return value;
        }
        fast_move<BoardSize> get_best_move() {
            auto root = search_tree.begin();
            auto best_child = std::max_element(root.begin(), root.end(), [](const auto& a, const auto& b) {
                return a.game_times < b.game_times;
                });
            last_move_index = search_tree.index(best_child);
            last_move = best_child->mv;
            return best_child->mv;
        }
        moves_type get_moves(const static_brd<BoardSize>& brd, color col) {
            if constexpr (BoardSize <= 8) {
                return brd.getmoves(col);
            }
            else {
                moves mvs(brd, col);
                return mvs.coords;
            }
        }
        float evaluate(const static_brd<BoardSize>& brd, color setter) {
            float score = e_mthd.eval_board(brd, setter, player);
            float win_rate = std::tanh(score / 10.0f);
            return win_rate;
        }
        void calculate_prior_probs(iter_type node, std::vector<float>& ret) {
            float sum = 0;
            float min_val = node_type::inf;
			for (auto& mv_iter : node) {
				auto sim_gm = fast_gm;
				sim_gm.move(mv_iter.mv);
                float val = e_mthd.eval_board(sim_gm.crt_state().brd, sim_gm.crt_state().setter_color, player);
                ret.push_back(val);
                sum += val;
                min_val = std::min(val, min_val);
			}
            if (min_val < 0) {
                sum += -min_val * ret.size();
            }
            for (auto& v : ret) {
                v = (v + (min_val < 0 ? -min_val : 0)) / sum;
            }
            return;
        }
        EvalMethod e_mthd;
		color player;
        search_tree_type search_tree;
        fast_game<BoardSize> fast_gm;
        int last_move_index = 0;
        fast_move<BoardSize> last_move;
        std::vector<float> _pre_probs;
	};  
}