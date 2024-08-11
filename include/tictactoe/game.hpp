#pragma once
#include"game.hpp"
#include"tui/components.hpp"
#include"tui/screen.hpp"
#include"tictactoe/gamer.hpp"
#include"tictactoe/board.hpp"

namespace bw::tictactoe {
	struct aspect {
		board brd;
		core::color col;
	};
	using aspects = std::vector<aspect>;
	struct game : basic_game, std::enable_shared_from_this<game> {
		game() = delete;
		game(basic_gamer_ptr g0, basic_gamer_ptr g1) :basic_game(ready) {
			using namespace core;
			
			g[col0] = std::move(std::dynamic_pointer_cast<gamer>(g0));
			g[col1] = std::move(std::dynamic_pointer_cast<gamer>(g1));
			if (g[col0] == nullptr || g[col1] == nullptr) {
				ui::msgbox(gettext("Invalid Gamer!"));
				throw std::invalid_argument("Invalid Gamer!");
			}
			if (g[col0]->col != op_col(g[col1]->col)) {
				ui::msgbox(gettext("Invalid color!"));
				throw std::invalid_argument("Invalid color!");
			}
			brd.initialize();
			st = ready;
		}

		boost::cobalt::task<void> start();
		void announce(std::string s) {
			if (screen_ptr)screen_ptr->PostEvent(ftxui::Event::Special(s.data()));
		}
		void refresh_screen() {
			if (screen_ptr)screen_ptr->PostEvent(ftxui::Event::Custom);
		}
		void wait_for_gamer(core::color c) {

			return;
		}
		board current_board() const  {
			return brd;
		}
		core::color current_color() const {
			return col;
		}
		gamer_ptr current_gamer() const {
			return g[col];
		}
		void end_game() {
			if (st == ongoing) {
				st = ended;
			}
			g[col]->cancel();
			g[col ^ 1]->cancel();
		}
		virtual ~game() = default;
		ftxui::ScreenInteractive* screen_ptr = nullptr;
		gamer_ptr g[2];
		board brd;
		core::color col = core::col0, op = core::col1;
		move mv;
		aspects game_aspects;
	};
	using game_ptr = std::shared_ptr<game>;
}
#pragma optimize("",off)
boost::cobalt::task<void> bw::tictactoe::game::start() {
	using namespace core;
	auto self = shared_from_this();

	st = ongoing;
	brd.initialize();
	game_aspects.emplace_back(brd, none);
	while (true) {
		auto winner = brd.check_winner();
		if (winner != core::none || brd.cnt == 9) {
			announce("End");
			co_return;
		}
		mv = co_await g[col]->getmove(brd);

		co_await g[col]->pass_move(mv);
		co_await g[op_col(col)]->pass_move(mv);

		switch (mv.mvtype) {
		case move::mv:
			brd.applymove(mv.crd, col);
			announce("Flush");
			break;
		case move::regret:
			if (brd.cnt < 2) {
				co_await g[col]->pass_msg("你现在还不能悔棋！");
				continue;
			}
			if (g[col]->gamertype == gamer::remote || g[op_col(col)]->gamertype == gamer::remote) {
				co_await g[col]->pass_msg("简易联机不支持悔棋！");
			}
			game_aspects.pop_back();
			game_aspects.pop_back();
			brd = game_aspects.back().brd;
			announce("Flush");
			continue;
			break;
		case move::suspend:
			st = suspended;
			suspend();
			co_return;
			break;
		case move::quit:
			st = ended;
			if (screen_ptr)screen_ptr->ExitLoopClosure()();
			co_return;
			break;
		case move::str:
			continue;
			break;
		case move::invalid:
			if (screen_ptr)screen_ptr->ExitLoopClosure()();
			st = ended;
			co_return;
			break;
		default:
			ui::msgbox(std::format("move type id:{}", mv.mvtype));
			st = ended;
			std::unreachable();
		}
		game_aspects.emplace_back(brd, col);
		col = op;//change the setter
		op = op_col(col);
	}
}
#pragma optimize("",on)