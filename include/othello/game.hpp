#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"game.hpp"
#include"othello/moves.hpp"
#include"othello/gamer.hpp"
#include"othello/board.hpp"
#include"tui/components.hpp"
#include"tui/ftxui_screen.hpp"
#include"online/signals.hpp"
namespace bw::othello {
	namespace components {
		class BoardBase;
	};
	struct aspect {
		dynamic_brd brd;
		color col;
	};
	using aspects = std::vector<aspect>;
	class game :public basic_game, public std::enable_shared_from_this<game> {
		friend class components::BoardBase;
	public:
		game(basic_gamer_ptr g0, basic_gamer_ptr g1, int brd_size = default_size, bw::components::ftxui_screen_ptr Screen = nullptr) :basic_game(ready), brd(brd_size) {
			g[col0] = std::move(std::dynamic_pointer_cast<gamer>(g0));
			g[col1] = std::move(std::dynamic_pointer_cast<gamer>(g1));
			if (g[col0] == nullptr || g[col1] == nullptr) {
				ui::msgbox(gettext("Invalid Gamer!"));
				throw std::invalid_argument("Invalid Gamer!");
			}
			if (g[col0]->col != op_col(g[col1]->col)) {
				ui::msgbox(gettext("Invalid color!"));
				throw std::invalid_argument("Invalid Gamer color!");
			}

			brd.initialize();
			st = ready;
		}
		boost::cobalt::task<void> start() override {
			auto self = shared_from_this();

			st = ongoing;
			brd.initialize();
			color op = core::op_col(col);
			game_aspects.emplace_back(brd, none);//用来处理第一次悔棋情况

			while (true) {
				mvs[col].update(brd, col);
				if (mvs[col].size == 0) {
					mvs[op].update(brd, op);
					if (mvs[op].size == 0) {
						//announce("End");
						end_sig();
						st = ended;
						end_game();
						break;
					}
					else {
						pass_sig();
						col = op;
						op = op_col(col);
					}
					continue;
				}
				move mv{ .mvtype = move::invalid,.msg = "None" };
				try {
					mv = co_await g[col]->getmove(brd);

					co_await g[op_col(col)]->pass_move(mv);
				}
				catch (const std::exception& e) {
					ui::msgbox(gbk2utf8(e.what()));
					st = game::ended;
					mv.mvtype = move::quit;
				}

				switch (mv.mvtype) {
				case move::mv:
					brd.applymove(mv.pos, col);
					//screen->post_event("Flush");
					flush_sig();
					break;
				case move::regret:
					if (brd.count(col0) + brd.count(col1) < 6) {
						co_await g[col]->pass_msg(gettext("You can't regret it now!"));
						continue;
					}
					if (cannot_regret()) {
						co_await g[col]->pass_msg(gettext("Regretting is not supported now!"));
					}
					if (col == game_aspects.back().col) {//和上次的是同一个人
						game_aspects.pop_back();
						brd = game_aspects.back().brd;
						//msgbox("Regret type:1");
					}
					else {
						game_aspects.pop_back();
						game_aspects.pop_back();
						brd = game_aspects.back().brd;
					}
					flush_sig();
					wait_for_gamer(col);
					continue;
					break;
				case move::suspend:
					st = suspended;
					suspend();
					co_return;
					break;
				case move::quit:
					st = ended;
					co_return;
					break;
				case move::str:
					continue;
					break;
				case move::invalid:
					ui::msgbox(std::format("Invalid move:pos:{},type:{},\nmsg:{}", mv.pos, mv.mvtype, mv.msg));
					st = ended;
					co_return;
					break;
				default:
					ui::msgbox(std::format("{}:{}", gettext("move type id"), mv.mvtype));
					st = ended;
					break;
				}
				game_aspects.emplace_back(brd, col);
				col = op;//change the setter
				op = op_col(col);
			}
		}
		bool valid_move(color c, const coord& crd) {
			return mvs[c].find(crd) != moves::npos;
		}
		void wait_for_gamer(color c) {

			return;
		}
		dynamic_brd current_board() {
			return brd;
		}
		color current_color() const {
			return col;
		}
		gamer_ptr current_gamer() const {
			return g[col];
		}
		void end_game() override {
			g[col]->cancel();
			g[col ^ 1]->cancel();
			bw::online::signals::end_game(st == ended ? game_msg::ok : game_msg::gamer_quit_or_disconnect);
			st = ended;
		}
		bool cannot_regret() {
			return 
				g[col]->gamertype==gamer::online||
				g[op_col(col)]->gamertype==gamer::online||
				g[col]->gamertype == gamer::remote ||
				g[op_col(col)]->gamertype == gamer::remote;
		}
		virtual ~game() = default;
	protected:
		void suspend() {

		}
		gamer_ptr g[2] = { nullptr,nullptr };
		dynamic_brd brd;
		color col = col0;//current color
		moves mvs[2];
		aspects game_aspects;
	};
	using game_ptr = std::shared_ptr<game>;
	template<typename Board>
	class light_human_game {
	public:
		light_human_game() = default;
		light_human_game(int size) {
			if constexpr (std::is_same<Board, dynamic_brd>::value) {
				brd.resize(size);
			}
		}
		void start() {
			brd.initialize();
			color op = 0;
			while (true) {
				
				op = op_col(col);
				std::println("{:sm<}", brd);
				mvs[col].update(brd, col);
				if (mvs[col].size == 0) {
					mvs[op].update(brd, op);
					if (mvs[op].size == 0) {
						std::println("Game end.");
						std::print("Black:{}\nWhite:{}\n", brd.countpiece(col0), brd.countpiece(col1));
						break;
					}
					std::println("{} pass.", col2str[col]);
					col = op;
					op = op_col(col);
					continue;
				}
				coord mv;
				std::string str;
				while (true) {
					std::println("Moves:{:c}", mvs[col]);
					std::print("Input {} move:", col2str[col]);
					std::cin >> str;
					for (auto& c : str) {
						c = std::toupper(c);
					}
					if (!is_valid_movestr(str) || mvs[col].find(fromstr(str)) == moves::npos) {
						std::println("Invalid move.");
						continue;
					}
					else {
						mv = fromstr(str);
						break;
					}
				}
				brd.applymove(mv, col);
				col = op;
			}
		}

	private:
		Board brd;
		moves mvs[2];
		color col = col0;
		std::map<color, std::string> col2str = { {col0, std::string(gettext("black"))},{col1, std::string(gettext("white"))} };
		coord fromstr(const std::string& s) {
			return coord(s[1] - '1', s[0] - 'A');
		}
		bool is_valid_movestr(const std::string& s) {
			return s[0] >= 'A' && s[0] <= 'Z' && s[1] >= '0' && s[1] <= '9';
		}
	};

	template<typename Board>
	int quick_self_play(int times) {
		return 0;
	}
}