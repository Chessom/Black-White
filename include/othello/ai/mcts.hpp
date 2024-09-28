#pragma once
#include <numeric>
#include <type_traits>
#include "eval_methods.hpp"
#include "tree.hpp"
#include "timer.hpp"
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
    inline bool is_over(const static_brd<BoardSize>& brd) {
        if constexpr (BoardSize <= 8) {
            return !(brd.getmoves(col0) || brd.getmoves(col1));
        }
        else {
            color col = col0;
            using drc_t = int;
            using namespace directions;
            color opcol = col ^ 1;
            for (int x = 0; x < BoardSize; ++x) {
                for (int y = 0; y < BoardSize; ++y) {
                    coord crd{ x,y };
                    if (brd.get_col(crd) == none) {
                        for (drc_t drc = R; drc <= UR; ++drc) {
                            coord iter(crd);
                            if (brd.in_board(iter.to_next(drc)) && brd.get_col(iter) != none) {
                                opcol = brd.get_col(iter);
                                col = op_col(opcol);
                                color c = 0;
                                while (brd.in_board(iter.to_next(drc))) {
                                    c = brd.get_col(iter);
                                    if (c == col) {
                                        return false;
                                    }
                                    else if (c == none) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }
    }
    template<typename T>
    void to_string_helper(const tree<T>& t, typename tree<T>::iterator node, std::ostringstream& oss, const std::string& prefix, bool is_last) {
        oss << prefix;
        if (is_last) {
            oss << "└-";
        }
        else {
            oss << "├-";
        }
        oss << *node << "\n";

        std::string new_prefix = prefix + (is_last ? "  " : "| ");

        auto child = node.begin();
        while (child != node.end()) {
            bool last_child = (std::next(child) == t.end(node));
            to_string_helper(t, child, oss, new_prefix, last_child);
            ++child;
        }
    }
    template<typename T>
    std::string to_string(const tree<T>& t) {
        if (t.empty()) return "";
        std::ostringstream oss;
        to_string_helper(t, t.begin(), oss, "", true);
        return oss.str();
    }

    template<int BoardSize>
    struct mcts_tree_node {
        enum { inf = 100000 };
        enum { pass_move = 0 };
        color setter = none;//当前节点后应当谁下？
        fast_move<BoardSize> mv{};
        bool is_terminal = false;
        int game_times = 0;
        float policy = 0.0f;
        float value = 0.0f;
    };
    template<int BoardSize>
    std::ostream& operator<<(std::ostream& os, const mcts_tree_node<BoardSize>& node) {
        os << fast_move_to_coord<BoardSize>(node.mv)
            << " N:" << node.game_times
            << " Q:" << (node.game_times ? node.value / node.game_times : 0)
            << " " << (node.setter == col0 ? "black" : "white");
        return os;
    }
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
            st.brd.apply_move(mv, st.setter_color);
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
        fast_game() {
            static_brd<BoardSize> brd{};
            brd.initialize();
            crt = state_type{ brd,col0 };
        };
        fast_game(const static_brd<BoardSize>& brd, color setter) {
            crt = state_type{ brd,setter,{} };
        }
        state_type& crt_state() {
            return crt;
        }
        bool end() {
            return is_over<BoardSize>(crt.brd);
        }
        void pass_or_move(const fast_move<BoardSize>& mv) {
            if (is_pass_move<BoardSize>(mv)) {
                pass();
            }
            else {
                move(mv);
            }
        }
        void move(const fast_move<BoardSize>& mv) {
            state_type st = crt_state();
            crt.brd.apply_move(mv, crt.setter_color);
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
    template<int BoardSize>
    struct selfplay_data {
        std::array<float, BoardSize* BoardSize> brd;
        std::array<float, BoardSize* BoardSize> probs;
        float value;
        color col;
    };
    template<int BoardSize>
    using sfplay_dataset = std::vector<selfplay_data<BoardSize>>;
	template<int BoardSize, class EvalMethod>
	struct mcts {
        using node_type = mcts_tree_node<BoardSize>;
        using search_tree_type = tree<node_type>;
        using moves_type = fast_moves<BoardSize>;
        using iter_type = search_tree_type::iterator;
        mcts(color setter_color, const ai_option& option) :player(setter_color), option(option){
            explore_factor = option.mcts_opt.explore_factor;
            std::random_device rnd;
            srand(rnd());
        };
        coord best_move(const static_brd<BoardSize> brd, color setter_color) {
            if (option.threads == 1) {
                return choose_move_single_thread(brd, setter_color);
            }
            else {
                return choose_move_single_thread(brd, setter_color);
            }
        }
        void print_search_tree(std::ostream& os= std::cout) {
            os << to_string(search_tree);
        }
        void set_evaluator(const EvalMethod& mthd) {
            e_mthd = mthd;
        }
        void set_evaluator(EvalMethod&& mthd) {
            e_mthd = mthd;
        }
        ai_option option;
        std::vector<std::pair<coord, float>> last_mvs;
        sfplay_dataset<BoardSize> self_play() {
            sfplay_dataset<BoardSize> dataset{};
            fast_gm = fast_game<BoardSize>();
            while (!fast_gm.end()) {
                search(search_tree.begin());

            }
            return dataset;
        }
        std::vector<float> get_actual_probs() {
            auto root = search_tree.begin();
            last_mvs.clear();
        }
        ~mcts() {
            spdlog::trace("mcts destructed");
        }
    private:
        int rand_int() {
            return rand();
        }
        coord choose_move_single_thread(const static_brd<BoardSize>& brd, color c) {
            bool found_subtree = false;
            if (!search_tree.empty()) {
                found_subtree = reuse_search_tree(brd, c);
            }
            if (!found_subtree) {
                search_tree.clear();
				search_tree.set_head(
					node_type{
						op_col(c),
						{},
						false,
						1,
						0.0f,
						0.0f
					});
            }
            fast_gm = fast_game<BoardSize>(brd, c);
            search(search_tree.begin());
            auto mv = get_best_move();
            fast_gm.move(mv);
            return fast_move_to_coord<BoardSize>(mv);
        }
        void search(iter_type head) {
            using namespace std::chrono;
            int simulate_times = 0;
            auto origin_game = fast_gm;
            auto start = high_resolution_clock::now();
            long duration = 0;
            while (true) {
                auto node_iter = select(head);
                if (!node_iter->is_terminal) {
                    expand(node_iter);
                    auto rnd_node_iter = node_iter.begin();
                    std::advance(rnd_node_iter, rand_int() % node_iter.number_of_children());
                    fast_gm.pass_or_move(rnd_node_iter->mv);
                    float result = simulate(rnd_node_iter);
                    back_propagate(rnd_node_iter, result);
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
                else if (option.time_limit) {
                    if (simulate_times % 1000 == 0) {
                        duration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
                        if (duration >= option.time_limit * 1000) {
                            break;
                        }
                    }
                }
                else [[unlikely]] {
                    if (simulate_times >= ai_option::mcts_option::default_simulations) {
                        break;
                    }
                }
            }
        }
        coord choose_move_multi_thread(const static_brd<BoardSize>& brd, color c) {
            return {};
        }
        bool reuse_search_tree(const static_brd<BoardSize>& brd, color setter) {
            bool found_subtree = false;
            auto head = search_tree.child(search_tree.begin(), last_move_index);
            auto sim_brd = fast_gm.crt_state().brd;
            auto mvs = static_brd<BoardSize>::deduce_moves(fast_gm.crt_state().brd, brd);
            auto op_color = op_col(setter);
            bool found_move_history = false;
            if (mvs.empty()) {
                if (head.begin() != head.end()) {
					auto&& sub_tr = search_tree.move_out(head.begin());
					search_tree = std::move(sub_tr);
					found_subtree = true;
                }
                else {
                    found_subtree = false;
                }
            }
            else {
                do {
                    for (auto& crd : mvs) {
                        moves m(sim_brd, op_color);
                        if (m.find(crd) != moves::npos) {
                            sim_brd.apply_move(crd, op_color);
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
                        if (new_head->setter != setter) {
                            new_head = new_head.begin();
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
            return found_subtree;
        }
        iter_type select(iter_type node_iter) {
            while (node_iter.number_of_children()) {
                float max_ucb = -node_type::inf;
                int total_visit = node_iter->game_times;
                ucb_max_nodes.clear();
                for (auto it = node_iter.begin(); it != node_iter.end(); ++it) {
                    auto new_ucb = ucb(it, total_visit);
                    if (new_ucb > max_ucb) {
                        ucb_max_nodes.clear();
                        ucb_max_nodes.push_back(it);
                        max_ucb = new_ucb;
                    }
                    else if (std::abs(new_ucb - max_ucb) <= 0.0001) {
                        ucb_max_nodes.push_back(it);
                    }
                }
                node_iter = ucb_max_nodes[rand_int() % ucb_max_nodes.size()];
				if (is_pass_move<BoardSize>(node_iter->mv)) {
					fast_gm.pass();
				}
				else {
					fast_gm.move(node_iter->mv);
				}
            }
            return node_iter;
        }
        void expand(iter_type node_iter) {
            auto& node = *node_iter;
            auto& st = fast_gm.crt_state();
            moves_type legal_moves = get_moves(st.brd, st.setter_color);
            if (empty_moves<BoardSize>(legal_moves)) {
                search_tree.append_child(node_iter, node_type{ st.setter_color, pass_move<BoardSize>(), false, 0, 1.0f, 0.0f });
            }
            else
            {
                int children_size = 0;
                if constexpr (BoardSize <= 8) {
                    auto sim_brd = st.brd;
                    for (; legal_moves; legal_moves = legal_moves & (legal_moves - 1)) {
                        fast_move<BoardSize> iter = legal_moves & (-ll(legal_moves));
                        sim_brd.apply_move(iter, st.setter_color);
                        search_tree.append_child(node_iter, node_type{ st.setter_color, iter, is_over<BoardSize>(sim_brd), 0, 0.0f, 0.0f });
                        children_size++;
                        sim_brd = st.brd;
                    }
                }
                else {
                    auto sim_brd = st.brd;
                    for (const auto& mv : legal_moves) {
                        sim_brd.apply_move(mv, st.setter_color);
                        search_tree.append_child(node_iter, node_type{ st.setter_color, mv, is_over<BoardSize>(sim_brd), 0, 0.0f, 0.0f });
                        children_size++;
                        sim_brd = st.brd;
                    }
                }
                _pre_probs.clear();
                _pre_probs.reserve(children_size);
                calculate_prior_probs(node_iter, _pre_probs);
                auto prob_it = _pre_probs.begin();
                for (auto it = node_iter.begin(); it != node_iter.end(); ++it, ++prob_it) {
                    it->policy = *prob_it;
                }
            }
        }
        float simulate(iter_type node_iter) {
            auto& node = *node_iter;
            if (node.is_terminal) {
                auto winner = fast_gm.winner();
                if (winner == node.setter) {
                    return 1;
                }
                else if(winner == op_col(node.setter)) {
                    return -1;
                }
                else{
                    return 0;
                }
            }
            else {
                return evaluate(fast_gm.crt_state().brd, fast_gm.crt_state().setter_color, node.setter);
            }
        }
        void back_propagate(iter_type node_iter, float result) {
            auto node_player = node_iter->setter;
            while (node_iter != nullptr) {
                auto& node = *node_iter;
                node.game_times++;
                //node.value += node.setter == node_player ? result : -result;
                node.value += result;
                result = -result;
                if (!search_tree.is_head(node_iter)) {
                    node_iter = search_tree.parent(node_iter);
                }
                else {
                    break;
                }
            }
        }
        float ucb(const iter_type& node, int parent_num) {
            float Q = node->game_times ? node->value / node->game_times : 0;
            float U = explore_factor * node->policy * std::sqrt(parent_num) / (1 + node->game_times);
            return Q + U;
        }
        fast_move<BoardSize> get_best_move() {
            auto root = search_tree.begin();
            auto best_child = root.begin();
            last_mvs.clear();
            /*std::vector<int> visits;
            for (auto it = root.begin(); it != root.end(); ++it) {
                visits.push_back(it->game_times);
                last_mvs.push_back({ fast_move_to_coord<BoardSize>(it->mv),float(it->game_times) });
            }
            auto dis = std::discrete_distribution<>(visits.begin(), visits.end());
            static std::random_device d;
            static std::default_random_engine rnd(d());
            std::advance(best_child, dis(rnd));*/
            for (auto it = root.begin(); it != root.end(); ++it) {
                if (it->game_times > best_child->game_times) {
                    best_child = it;
                }
                last_mvs.push_back({ fast_move_to_coord<BoardSize>(it->mv),float(it->game_times) });
            }
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
        void softmax(std::vector<float>& vec) {
            float sum = 0;
            for (auto& p : vec) {
                p = std::exp(p);
                sum += p;
            }
            for (auto& p : vec) {
                p = p / sum;
            }
        }
        float evaluate(const static_brd<BoardSize>& brd, color setter, color player) {
            float value = e_mthd.eval_board(brd, setter, player);
            return std::tanh(value / 10.0f);
            /*auto sim_gm = fast_game<BoardSize>(brd, setter);
            moves mvs[2];
            while (true) {
                auto& st = sim_gm.crt_state();
                mvs[st.setter_color].update(st.brd, st.setter_color);
                if (mvs[st.setter_color].empty()) {
                    mvs[op_col(st.setter_color)].update(st.brd, op_col(st.setter_color));
                    if (mvs[op_col(st.setter_color)].empty()) {
                        auto winner = sim_gm.winner();
                        if (winner == none) {
                            return 0;
                        }
                        else if (winner==player) {
                            return 1;
                        }
                        else {
                            return -1;
                        }
                    }
                    else {
                        sim_gm.pass();
                        continue;
                    }
                }
                else {
                    sim_gm.move(coord_to_fast_move<BoardSize>(mvs[st.setter_color].coords[rand_int() % mvs[st.setter_color].coords.size()]));
                }
            }*/
        }
        float evaluate_end_game(const static_brd<BoardSize>& brd, color setter, color player) {
            float value = e_mthd.eval_end_game_board(brd, setter, player);
            return std::tanh(value / 30);
        }
        void calculate_prior_probs(iter_type node, std::vector<float>& ret) {//node下面的
            auto setter_col = fast_gm.crt_state().setter_color;
            auto sim_gm = fast_gm;
			for (auto& mv_iter : node) {
				sim_gm.move(mv_iter.mv);
                float val = e_mthd.eval_board(sim_gm.crt_state().brd, sim_gm.crt_state().setter_color, setter_col);
                ret.push_back(val / 20);
                sim_gm = fast_gm;
			}
            softmax(ret);
            /*int children_size = search_tree.number_of_children(node);
            for (int i = 0; i < children_size; ++i) {
                ret.push_back(1.0f / children_size);
            }
            return;*/
        }
        std::vector<float> generate_dirichlet_noise(int size, float alpha) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::gamma_distribution<float> gamma(alpha, 1.0);

            std::vector<float> noise(size);
            for (int i = 0; i < size; ++i) {
                noise[i] = gamma(gen);
            }

            float sum = std::accumulate(noise.begin(), noise.end(), 0.0f);
            for (auto& n : noise) {
                n /= sum;
            }

            return noise;
        }

        void add_dirichlet_noise(std::vector<float>& probs, float alpha, float epsilon) {
            auto noise = generate_dirichlet_noise(probs.size(), alpha);
            for (size_t i = 0; i < probs.size(); ++i) {
                probs[i] = (1 - epsilon) * probs[i] + epsilon * noise[i];
            }
        }

        EvalMethod e_mthd;
		color player;
        search_tree_type search_tree;
        fast_game<BoardSize> fast_gm;
        int last_move_index = 0;
        fast_move<BoardSize> last_move = {};
        std::vector<iter_type> ucb_max_nodes = {};
        std::vector<float> _pre_probs;
        float explore_factor = 2.0f;
	};  
}