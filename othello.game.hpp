#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"game.hpp"
#include"othello.moves.hpp"
#include"othello.gamer.hpp"
#include"othello.board.hpp"
#include"components.h"
namespace bw::othello {
	namespace components {
		class BoardBase;
	};
	struct aspect {
		dynamic_brd brd;
		color col;
		template<typename Archive>
		void serialize(Archive& ar) {
			ar(brd, col);
		}
	};
	using aspects = std::vector<aspect>;
	class game :public basic_game {
		friend class components::BoardBase;
	public:
		game() = default;
		game(basic_gamer_ptr g0, basic_gamer_ptr g1, int brd_size = default_size) :state(ready), brd(brd_size) {
			g[col0] = std::move(std::dynamic_pointer_cast<gamer>(g0));
			g[col1] = std::move(std::dynamic_pointer_cast<gamer>(g1));
			brd.initialize();
			state = ready;
		}
		string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
		}
		boost::cobalt::task<void> start(basic_gamer_ptr g0, basic_gamer_ptr g1) {
			if (g0 == nullptr || g1 == nullptr) {
				ui::msgbox("Invalid Gamer!");
				co_return;
			}
			g[col0] = std::dynamic_pointer_cast<gamer>(g0);
			g[col1] = std::dynamic_pointer_cast<gamer>(g1);
			if (g[col0]->col != op_col(g[col1]->col)) {
				ui::msgbox("Invalid color!");
				co_return;
			}
			state = ongoing;
			brd.initialize();
			color op = core::op_col(col);
			game_aspects.emplace_back(brd, none);//用来处理第一次悔棋情况

			while (true) {
				mvs[col].update(brd, col);
				if (mvs[col].size == 0) {
					mvs[op].update(brd, op);
					if (mvs[op].size == 0) {
						//对局结束
						//公布比分(由components完成)
						announce("End");
						state = ended;
						break;
					}
					else {
						//宣布pass
						col = op;
						op = op_col(col);
						announce("Pass");
					}
					continue;
				}
				move mv{ .mvtype = move::invalid,.msg = "None" };
				try {
					mv = co_await g[col]->getmove(brd);
				}
				catch (const std::exception& e) {
					ui::msgbox(gbk2utf8(e.what()));
					state = game::ended;
				}

				co_await g[col]->pass_move(mv);
				co_await g[op_col(col)]->pass_move(mv);

				switch (mv.mvtype) {
				case move::mv:
					brd.applymove(mv.pos, col);
					announce("flush");
					break;
				case move::regret:
					if (brd.count(col0) + brd.count(col1) < 6) {
						co_await g[col]->pass_msg("你现在还不能悔棋！");
						continue;
					}
					if (g[col]->gamertype == gamer::remote || g[op_col(col)]->gamertype == gamer::remote) {
						co_await g[col]->pass_msg("简易联机不支持悔棋！");
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
					announce("flush");
					wait_for_gamer(col);
					continue;
					break;
				case move::suspend:
					state = suspended;
					suspend();
					co_return;
					break;
				case move::quit:
					state = ended;
					if(screen_ptr)screen_ptr->ExitLoopClosure()();
					co_return;
					break;
				case move::str:
					continue;
					break;
				case move::invalid:
					ui::msgbox(std::format("Invalid move:pos:{},type:{},\nmsg:{}", mv.pos, mv.mvtype, mv.msg));
					if (screen_ptr)screen_ptr->ExitLoopClosure()();
					state = ended;
					co_return;
					break;
				default:
					ui::msgbox(std::format("move type id:{}", mv.mvtype));
					state = ended;
					std::unreachable();
				}
				game_aspects.emplace_back(brd, col);
				col = op;//change the setter
				op = op_col(col);
			}
		}
		bool valid_move(color c, const coord& crd) {
			return mvs[c].find(crd) != moves::npos;
		}
		void announce(std::string_view s) {
			if (screen_ptr)screen_ptr->PostEvent(ftxui::Event::Special(s.data()));
		}
		void refresh_screen() {
			if (screen_ptr)screen_ptr->PostEvent(ftxui::Event::Custom);
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
		void endgame() {
			if (state == ongoing) {
				state = ended;
				g[col]->cancel();
				g[col ^ 1]->cancel();
			}
		}
		virtual ~game() = default;
		ftxui::ScreenInteractive* screen_ptr = nullptr;
		enum { unready, ready, ongoing, ended, suspended };
		int state;
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
		std::map<color, std::string> col2str = { {col0, std::string("black")},{col1, std::string("white")} };
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