#pragma once
#include"globals.hpp"
#include"tictactoe.game.hpp"
#include"tictactoe.gamer.human.hpp"
#include"tictactoe.gamer.computer.hpp"
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
			else if (e == Event::Special("Flush")) {
				brd = game_ptr->current_board();
				pmvdq->tim.cancel_one();
				game_ptr->refresh_screen();
				return true;
			}
			else if (e == Event::Special("End")) {
				auto winner = brd.check_winner();
				if (winner != core::none) {
					ui::msgbox(std::format("{} Win!", game_ptr->g[winner]->name));
				}
				else if (brd.cnt == 9) {
					ui::msgbox("Draw!");
				}
				game_ptr->screen_ptr->Exit();
				return true;
			}
			else if (e == Event::Special("Regret")) {
				pmvdq->q.push_back({ .mvtype = move::regret });
				pmvdq->tim.cancel_one();
				return true;
			}
			else if (e == Event::Special("EndLoop")) {
				game_ptr->endgame();
				return true;
			}
			else if (e == Event::Special("Suspend")) {
				ui::msgbox("Function in Developing");
				return true;
			}
			else if (e == Event::Special("Save")) {
				ui::msgbox("Function in Developing");
				return true;
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
	
	class Game {
	public:
		Game() { pctx = std::make_shared<boost::asio::io_context>(); }
		inline std::string censtr(const std::string& str, int width) {
			return std::format("{0:^{1}}", str, width);
		}
		inline string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
		}
		bool GamePreparing() {
			using namespace ftxui;
			bool ret = true;
			ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
			int selected_2 = 0, selected_1 = 0, selected_3 = 2;
			ui::auto_close_modal _f;

			Component layout = Container::Vertical({
				Container::Horizontal({Renderer([] {return ftxui::text("First Player:"); }) | center,Dropdown(&gamer_list,&selected_1) | align_right,}),
				Container::Horizontal({Renderer([] {return ftxui::text("Second Player:"); }) | center,Dropdown(&gamer_list,&selected_2) | align_right,}),
				Button(censtr("开始", 8), screen.ExitLoopClosure(), ButtonOption::Animated()) | center,
				Button(censtr("返回", 8), [&ret,&screen] { ret = false; screen.Exit(); }, ButtonOption::Animated()) | center
				}) | ui::EnableMessageBox();
			screen.Loop(Renderer(layout, [&layout, &ret] {return dbox(layout->Render() | center); }));
			if (!ret)
				return ret;
			switch (selected_1)
			{
			case 0:gptr[col0] = std::make_shared<human_gamer>(col0); break;
			case 1:gptr[col0] = std::make_shared<computer_gamer>(col0); break;
				break;
			default:
				break;
			}
			switch (selected_2)
			{
			case 0:gptr[col1] = std::make_shared<human_gamer>(col1); break;
			case 1:gptr[col1] = std::make_shared<computer_gamer>(col1); break;
			default:
				break;
			}
			return ret;
		}
		void GamePageLocal() {
			using namespace ftxui;

			ui::auto_close_modal _f;
			ScreenInteractive screen = ScreenInteractive::Fullscreen();
			std::shared_ptr<game> gm = std::make_shared<game>(gptr[core::col0], gptr[core::col1]);
			gm->screen_ptr = &screen;
			Board brd_ptr = Make<BoardBase>(pctx, gm);

			Component buttons = Container::Vertical({
				Button(censtr("退出", 8), [&] {screen.PostEvent(Event::Special("EndLoop")); if (gm->state() == game::ended) { screen.Exit(); } }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr("悔棋", 8), [&] {screen.PostEvent(Event::Special("Regret")); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr("暂停", 8), [&] {screen.PostEvent(Event::Special("Suspend")); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr("保存", 8), [&] {screen.PostEvent(Event::Special("Save")); }, ButtonOption::Animated()) | center,
				}) | ftxui::border;
			Component brd = Container::Horizontal({ brd_ptr }) | center;
			Component layout = Container::Vertical({
				Container::Horizontal({
					buttons,
					brd,
				}),
				Renderer([&] {return text(std::format("Current player:{}",gptr[gm->current_color()]->name)) | center; }),
				});
			std::jthread j([this, gm, &screen] {
				try
				{
					boost::cobalt::spawn(*pctx, gm->start(gptr[0], gptr[1]), boost::asio::detached);
					pctx->run();
				}
				catch (const std::exception& e)
				{
					ui::msgbox(gbk2utf8(e.what()));
					screen.Exit();
				}
				});
			screen.Loop(layout | center | ui::EnableMessageBox());
			pctx->stop();
		}

		gamer_ptr gptr[2] = { nullptr,nullptr };
		using context_ptr = std::shared_ptr<boost::asio::io_context>;
		std::shared_ptr<boost::asio::io_context> pctx;
	};
}