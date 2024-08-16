#pragma once
#include"globals.hpp"
#include"online/signals.hpp"
#include"tictactoe/game.hpp"
#include"tictactoe/gamer/human.hpp"
#include"tictactoe/gamer/computer.hpp"
namespace bw::tictactoe::components {
	using namespace core;
	class BoardBase : public ftxui::ComponentBase {
	public:
		using context_ptr = std::shared_ptr<boost::asio::io_context>;
		BoardBase(context_ptr pctx, game_ptr gm_ptr) :brd(gm_ptr->brd), game_ptr(gm_ptr) {
			pmvdq = std::make_shared<timdq>(pctx);
			if (game_ptr->g[col0] == nullptr) {
				throw std::runtime_error("Invalid gamer[0]!");
			}
			if (game_ptr->g[col0]->is_human()) {
				std::dynamic_pointer_cast<human_gamer>(game_ptr->g[col0])->set_move_queue(pmvdq);
			}
			if (game_ptr->g[col1] == nullptr) {
				throw std::runtime_error("Invalid gamer[1]!");
			}
			if (game_ptr->g[col1]->is_human()) {
				std::dynamic_pointer_cast<human_gamer>(game_ptr->g[col1])->set_move_queue(pmvdq);
			}
		}
		virtual ftxui::Element Render() override {
			using namespace ftxui;
			brd = game_ptr->current_board();
			Elements rows;
			color col;
			std::string firstr;
			firstr = "╔";
			for (int i = 1; i < brd.size; i++)
				firstr += "═══╦";
			firstr += "═══╗ ";
			rows.push_back(text(firstr));
			std::string rowstr1, rowstr2;
			Elements row;

			rowstr1 = "╠";
			for (int j = 0; j < brd.size - 1; j++)
				rowstr1 += "═══╬";
			rowstr1 += "═══╣ ";

			rowstr2 = "╚";
			for (int j = 0; j < brd.size - 1; j++)
				rowstr2 += "═══╩";
			rowstr2 += "═══╝ ";

			for (int x = 0; x < brd.size; ++x) {
				row.clear();
				row.emplace_back(text("║"));

				for (int y = 0; y < brd.size; ++y) {
					col = brd.getcol({ x,y });
					if (col == core::none) {
						row.emplace_back(text("   "));
					}
					else if (col == core::col0) {
						row.emplace_back(text(" X ") | ftxui::color(Color::Red));
					}
					else {
						row.emplace_back(text(" O ") | ftxui::color(Color::Green));
					}
					row.emplace_back(text("║"));
				}
				rows.emplace_back(hbox(row));
				if (x != brd.size - 1) {
					rows.emplace_back(text(rowstr1));
				}
				else {
					rows.emplace_back(text(rowstr2));
				}
			}
			return vbox(std::move(rows)) | reflect(box);
		}
		virtual bool OnEvent(ftxui::Event e) override {
			using namespace ftxui;
			if (e.is_mouse()) {
				return OnMouseEvent(e);
			}
			return false;
		}
		bool OnMouseEvent(ftxui::Event e) {
			using namespace ftxui;
			if (box.Contain(e.mouse().x, e.mouse().y) && CaptureMouse(e)) {
				int dx = e.mouse().x - box.x_min;
				int dy = e.mouse().y - box.y_min;
				if (e.mouse().button == Mouse::Left &&
					e.mouse().motion == Mouse::Pressed) {
					TakeFocus();
					if (game_ptr->current_gamer()->is_human()) {
						pmvdq->q.push_back(move{ .mvtype = move::mv,.crd = coord{ dy / 2,dx / 4 } });
						pmvdq->tim.cancel_one();
					}
					return true;
				}
			}
			return false;
		}
		virtual bool Focusable() const override { return true; }
		void set_move_queue(timdq_ptr move_queue) {
			pmvdq = std::move(move_queue);
		}
		timdq_ptr pmvdq;
		std::shared_ptr<game> game_ptr = nullptr;
		tictactoe::board brd;
	private:
		bool mouse_hover = false;
		ftxui::Box box;
	};
	using Board = std::shared_ptr<BoardBase>;
	
	class Game :public basic_Game, public std::enable_shared_from_this<Game> {
	public:
		Game() { pctx = std::make_shared<boost::asio::io_context>(); }
		Game(std::shared_ptr<boost::asio::io_context> context_ptr) :basic_Game(context_ptr) {};
		ftxui::Component AnotherRoundButton(game_ptr gm) {
			using namespace ftxui;
			return Button(gettext("Another round"), [gm, this] {
				another_round = true;
				gm->end_game();
				if (!pctx->stopped()) { pctx->stop(); }
				ftxui::ScreenInteractive::Active()->Exit();
				}, ButtonOption::Animated());
		}
		virtual ftxui::Component GamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) override;
		virtual ftxui::Component OnlinePrepareCom(bw::online::basic_user_ptr uptr) override;
		virtual ftxui::Component OnlineGamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr);
		virtual void join(basic_gamer_ptr gp) override {
			assert(gp != nullptr);
			gptr[gp->col] = std::dynamic_pointer_cast<gamer>(gp);
		}
		virtual basic_gamer_ptr gamer_from_info(basic_gamer_info info) override {
			basic_gamer_ptr ret;
			switch (info.gamertype)
			{
			case gamer::human:
				ret = std::make_shared<tictactoe::human_gamer>(info);
				break;
			case gamer::computer:
				ret = std::make_shared<tictactoe::computer_gamer_random>(info);
				break;
			case gamer::online:
				ret = std::make_shared<tictactoe::online_gamer>(info);
				break;
			default:
				break;
			}
			return ret;
		}
		virtual basic_game_ptr generate_game(ftxui::ScreenInteractive& screen) override {
			std::shared_ptr<game> gm = std::make_shared<game>(gptr[core::col0], gptr[core::col1]);
			return gm;
		}
		void set_board_size(int size) override {}
		bool GamePreparing();
		void GamePageLocal();

		gamer_ptr gptr[2] = { nullptr,nullptr };
		using context_ptr = std::shared_ptr<boost::asio::io_context>;

	};
	using ttt_Game_ptr = std::shared_ptr<Game>;

	ftxui::Component Game::GamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) {
		using namespace ftxui;
		assert(gptr[col0] != nullptr && gptr[col1] != nullptr);
		std::shared_ptr<game> gm = std::dynamic_pointer_cast<game>(gm_ptr);
		Board brd_ptr = Make<BoardBase>(pctx, gm);

		gm->regret_sig.connect([brd_ptr] {
			brd_ptr->pmvdq->q.push_back({ .mvtype = move::regret });
			brd_ptr->pmvdq->tim.cancel_one();
			});
		gm->end_sig.connect([brd_ptr, gm, this] {
			auto winner = gm->brd.check_winner();
			if (winner != core::none) {
				ui::msgbox(std::format("{} {}", gm->g[winner]->name, gettext("Win!")), { ui::MakeOKButton(),ui::TextComp(" "),AnotherRoundButton(gm) });
			}
			else if (gm->brd.cnt == 9) {
				ui::msgbox(gettext("Draw!"), { ui::MakeOKButton(), ui::TextComp(" "), AnotherRoundButton(gm) });
			}
			});
		gm->flush_sig.connect([brd_ptr, gm, &screen] {
			screen.Post([brd_ptr, gm] { brd_ptr->brd = gm->current_board(); });
			});
		gm->save_sig.connect([] {ui::msgbox(gettext("Function in Developing")); });
		gm->suspend_sig.connect([] {ui::msgbox(gettext("Function in Developing")); });

		Component buttons = Container::Vertical({
			Button(censtr(gettext("Quit"), 8), [this,gm,&screen] {gm->end_game(); if (!pctx->stopped()) { pctx->stop(); } screen.Exit(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Regret"), 8), [gm] {gm->regret_sig(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Suspend"), 8), [gm] {gm->suspend_sig(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Save"), 8), [gm] {gm->save_sig(); }, ButtonOption::Animated()) | center,
			}) | ftxui::border;
		Component brd = brd_ptr | center;
		return
			Container::Vertical({
				Container::Horizontal({
					buttons,
					brd,
				}),
				Renderer([this, gm] { return text(std::format("{}:{}",gettext("Current player"),gptr[gm->current_color()]->name)) | center; }),
				});
	}
	
	ftxui::Component Game::OnlinePrepareCom(bw::online::basic_user_ptr uptr) {
		using namespace ftxui;
		auto layout = Container::Vertical({
			ui::TextComp(gettext("TicTacToe")),
			Button(gettext("Match"),[this, uptr]
			{
				static boost::asio::steady_timer tim(*pctx);
				static std::atomic_flag flag;
				auto tictactoe_Game = shared_from_this();
				if (!flag.test()) {
					if (uptr->state != online::user_st::gaming) {
						uptr->Game_ptr = tictactoe_Game;
						basic_gamer_info info(core::none, uptr->id, uptr->name, basic_gamer::online, core::gameid::tictactoe);
						std::string infostr;
						struct_json::to_json(info, infostr);
						uptr->deliver(wrap(
							game_msg{
								.type = game_msg::prepare,
								.id = uptr->id,
								.movestr = infostr,
								.board = std::format("{} {}", "tictactoe","3"),
							},
							msg_t::game
							));
						uptr->state = online::user_st::prepared;
					}
					else {
						ui::msgbox(gettext("Cannot match during game!"));
					}
					flag.test_and_set();
					tim.expires_after(5s);
					tim.async_wait([](boost::system::error_code ec) {
						flag.clear();
					});
				}
				else {
					ui::msgbox(gettext("The operation is too fast, please match later."));
				}
			},ButtonOption::Animated()) | center
			});
		return layout;
	}
	
	ftxui::Component Game::OnlineGamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) {
		using namespace ftxui;
		assert(gptr[col0] != nullptr && gptr[col1] != nullptr);
		std::shared_ptr<game> gm = std::dynamic_pointer_cast<game>(gm_ptr);
		Board brd_ptr = Make<BoardBase>(pctx, gm);

		gm->regret_sig.connect([brd_ptr] {
			brd_ptr->pmvdq->q.push_back({ .mvtype = move::regret });
			brd_ptr->pmvdq->tim.cancel_one();
			});
		gm->end_sig.connect([brd_ptr, gm] {
			auto winner = gm->brd.check_winner();
			if (winner != core::none) {
				ui::msgbox(std::format("{} {}", gm->g[winner]->name, gettext("Win!")));
			}
			else {
				ui::msgbox(gettext("Draw!"));
			}
			});
		gm->flush_sig.connect([brd_ptr, gm, &screen] {
			screen.Post([brd_ptr, gm] { brd_ptr->brd = gm->current_board(); });
			});
		gm->save_sig.connect([] {ui::msgbox(gettext("Function in Developing")); });
		gm->suspend_sig.connect([] {ui::msgbox(gettext("Function in Developing")); });

		Component buttons = Container::Vertical({
			Button(censtr(gettext("Quit"), 8), [gm] {gm->end_game(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Regret"), 8), [gm] {gm->regret_sig(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Suspend"), 8), [gm] {gm->suspend_sig(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Save"), 8), [gm] {gm->save_sig(); }, ButtonOption::Animated()) | center,
			}) | ftxui::border;
		Component brd = brd_ptr | center;
		return
			Container::Vertical({
				Container::Horizontal({
					buttons | center,
					brd | center,
				}),
				Renderer([this, gm] { return text(std::format("{}:{}",gettext("Current player"),gptr[gm->current_color()]->name)) | center; }),
				});
	}
	
	bool Game::GamePreparing() {
		using namespace ftxui;
		bool ret = true;
		ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
		int selected_2 = 0, selected_1 = 0, selected_3 = 2;
		ui::auto_close_modal _f;

		Component layout = Container::Vertical({
			Container::Horizontal({Renderer([] {return ftxui::text(gettext("First Player:")); }) | center,Dropdown(&gamer_list,&selected_1) | align_right,}),
			Container::Horizontal({Renderer([] {return ftxui::text(gettext("Second Player:")); }) | center,Dropdown(&gamer_list,&selected_2) | align_right,}),
			Button(censtr(gettext("Start"), 8), screen.ExitLoopClosure(), ButtonOption::Animated()) | center,
			Button(censtr(gettext("Back"), 8), [&ret,&screen] { ret = false; screen.Exit(); }, ButtonOption::Animated()) | center
			}) | ui::EnableMessageBox();
		screen.Loop(Renderer(layout, [&layout, &ret] {return dbox(layout->Render() | center); }));
		if (!ret)
			return ret;
		switch (selected_1)
		{
		case 0:gptr[col0] = std::make_shared<human_gamer>(col0); break;
		case 1:gptr[col0] = std::make_shared<computer_gamer_random>(col0); break;
		default:
			break;
		}
		switch (selected_2)
		{
		case 0:gptr[col1] = std::make_shared<human_gamer>(col1); break;
		case 1:gptr[col1] = std::make_shared<computer_gamer_random>(col1); break;
		default:
			break;
		}
		return ret;
	}
	
	void Game::GamePageLocal() {
		using namespace ftxui;
		another_round = false;
		ui::auto_close_modal _f;
		ScreenInteractive screen = ScreenInteractive::Fullscreen();
		std::shared_ptr<game> gm = std::make_shared<game>(gptr[core::col0], gptr[core::col1]);
		auto GamePageComp = GamePage(screen, gm) | center | ui::EnableMessageBox();
		std::jthread j([this, gm, &screen] {
			try
			{
				boost::cobalt::spawn(*pctx, gm->start(), boost::asio::detached);
				pctx->run();
			}
			catch (const std::exception& e)
			{
				ui::msgbox(gbk2utf8(e.what()));
				screen.Exit();
			}
			});
		screen.Loop(GamePageComp);
		if (!pctx->stopped())
			pctx->stop();
		pctx->restart();
	}
}