#pragma once
#include"stdafx.h"
#include"othello/game.hpp"
#include"othello/gamer/human.hpp"
#include"othello/gamer/online.hpp"
#include"basic_Game.hpp"
#include"tui/components.hpp"
#include"tui/ftxui_screen.hpp"
#include"tui/game/gamer_prepare.hpp"
#include"othello/gamer_prepare.hpp"
#include"online/signals.hpp"
namespace bw::othello::components {
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
			brd.initialize();
		}
		ftxui::Element Render() override {
			using namespace ftxui;
			brd = game_ptr->current_board();
			Elements rows;
			color col;
			std::string firstr;
			firstr = "╔";
			for (int i = 1; i < brd.size; i++)
				firstr += "═══╦";
			firstr += "═══╗ ";
			auto style = ftxui::color(Color::Black) | bgcolor(Color::Green);
			rows.push_back(text(firstr) | style);
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
				row.emplace_back(text("║") | style);

				for (int y = 0; y < brd.size; ++y) {
					col = brd.get_col({ x,y });
					if (col == core::none) {
						if (game_ptr->current_gamer()->is_human() || game_ptr->current_gamer()->gamertype == gamer::online) {
							mvs.update(brd, game_ptr->current_color());
							row.emplace_back(text(mvs.find({ x,y }) != moves::npos ? " • " : "   ")
								| ftxui::color(Color::Red) | bgcolor(Color::Green));
						}
						else {
							row.emplace_back(text("   ")| ftxui::color(Color::Red) | bgcolor(Color::Green));
						}
					}
					else if (col == core::col0) {
						row.emplace_back(text(" ● ") | style);
					}
					else {
						row.emplace_back(text(" ● ") | ftxui::color(Color::White) | bgcolor(Color::Green));
					}
					row.emplace_back(text("║") | style);
				}
				row.emplace_back(text(" ") | bgcolor(Color::Green));
				rows.emplace_back(hbox(row));
				if (x != brd.size - 1) {
					rows.emplace_back(text(rowstr1) | style);
				}
				else {
					rows.emplace_back(text(rowstr2) | style);
				}
			}
			return vbox(std::move(rows)) | reflect(box);
		}
		bool OnEvent(ftxui::Event e) override {
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
						pmvdq->q.push_back(move{ .mvtype = move::mv,.pos = coord{ dy / 2,dx / 4 } });
						pmvdq->tim.cancel_one();
					}
					return true;
				}
			}
			return false;
		}
		bool Focusable() const override { return true; }
		void set_move_queue(timdq_ptr move_queue) {
			pmvdq = std::move(move_queue);
		}
		timdq_ptr pmvdq;
		std::shared_ptr<game> game_ptr = nullptr;
		dynamic_brd brd;
		moves mvs;
	protected:
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
		ftxui::Component GameSettingPage(ftxui::ScreenInteractive& screen, bool& ret, bool& advanced, ftxui::Component Tabs, int&);
		ftxui::Component GameAdvancedSettingPage(ftxui::Component, int&, bool&);
		virtual ftxui::Component OnlinePrepareCom(bw::online::basic_user_ptr uptr) override;
		virtual ftxui::Component OnlineGamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) override;
		virtual ftxui::Component GamePage(ftxui::ScreenInteractive& screen) override;
		virtual void join(basic_gamer_ptr gp) override {
			assert(gp != nullptr);
			gptr[gp->col] = std::dynamic_pointer_cast<gamer>(gp);
		}
		virtual basic_gamer_ptr gamer_from_info(basic_gamer_info info) override {
			basic_gamer_ptr ret;
			switch (info.gamertype)
			{
			case gamer::human:
				ret = std::make_shared<othello::human_gamer>(info);
				break;
			case gamer::computer:
				ret = std::make_shared<othello::computer_gamer_random>(info.col);
				break;
			case gamer::online:
				ret = std::make_shared<othello::online_gamer>(info);
				break;
			case gamer::remote:
				ret = std::make_shared<othello::remote_tcp_gamer>(pctx, info.col);
				break;
			default:
				break;
			}
			return ret;
		}
		virtual basic_game_ptr generate_game(ftxui::ScreenInteractive& screen) override {
			auto gm = std::make_shared<game>(gptr[col0], gptr[col1], board_size, std::make_shared<bw::components::ftxui_screen>(&screen));
			return gm;
		}
		bool GameSetting();
		void GamePageLocal();
		void reset() {
			gptr[0] = nullptr;
			gptr[1] = nullptr;
			board_size = 8;
			s_size = "8";
			pctx->restart();
		}
		void set_board_size(int size) override {
			board_size = size;
		}
		gamer_ptr gptr[2] = { nullptr,nullptr };
		game_ptr gm = nullptr;
		int board_size = 8;
		~Game() {
			spdlog::trace("Othello Game Destructor");
			spdlog::trace("pctx use_count:{} game object use_count:{}", pctx.use_count(), gm.use_count());
		}
	protected:
		std::string s_size = "8";
	};
	using othello_Game_ptr = std::shared_ptr<Game>;
	
	ftxui::Component Game::OnlinePrepareCom(bw::online::basic_user_ptr uptr)  {
		using namespace ftxui;
		auto layout = Container::Vertical({
			ui::TextComp(gettext("Othello")) | center,
			Container::Horizontal({
				ui::TextComp(gettext("Board Size: ")) | center,
				Input(&s_size,"  ") | underlined
				| CatchEvent([this](Event event) {
					return event.is_character() && !std::isdigit(event.character()[0]);
				}) | CatchEvent([&](Event event) {
				return event.is_character() && s_size.size() > 2;
			}),
			}) | center,
			Button(gettext("Match"),[this, uptr]
			{
				static boost::asio::steady_timer tim(*pctx);
				static std::atomic_flag flag;
				auto othello_Game = shared_from_this();
				board_size = std::stoi(s_size);
				if (board_size % 2 == 1) {
					ui::msgbox(gettext("The size of the othello board must be even."));
					return;
				}
				if (!flag.test()) {
					if (uptr->state != online::user_st::gaming) {
						uptr->Game_ptr = othello_Game;
						basic_gamer_info info(core::none, uptr->id, uptr->name, basic_gamer::online, core::gameid::othello);
						std::string infostr;
						struct_json::to_json(info, infostr);
						uptr->deliver(wrap(
							game_msg{
								.type = game_msg::prepare,
								.id = uptr->id,
								.movestr = infostr,
								.board = std::format("{} {}", "othello", othello_Game->board_size),
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
			},ButtonOption::Animated(Color::Green)) | center
			});
		return layout;
	}
	
	ftxui::Component Game::OnlineGamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) {
		assert(gptr[col0] != nullptr && gptr[col1] != nullptr);
		using namespace ftxui;
		gm = std::dynamic_pointer_cast<game>(gm_ptr);

		Board brd_ptr = Make<BoardBase>(pctx, gm);
		std::weak_ptr<BoardBase> weak_brd = brd_ptr;//防止循环引用brd_ptr->gm,gm->some_signal->brd_ptr
		gm->regret_sig.connect([weak_brd] {
			auto brd = weak_brd.lock();
			if (brd != nullptr) {
				brd->pmvdq->q.push_back({ .mvtype = move::regret });
				brd->pmvdq->tim.cancel_one();
			}
			});
		gm->end_sig.connect([this] {
			auto crt_brd = gm->current_board();
			int points0 = crt_brd.countpiece(col0), points1 = crt_brd.countpiece(col1);
			if (points0 > points1) {
				ui::msgbox(gettext("Black win!"));
			}
			else if (points0 < points1) {
				ui::msgbox(gettext("White win!"));
			}
			else {
				ui::msgbox(gettext("Draw!"));
			}
			});
		gm->flush_sig.connect([this, &brd = brd_ptr->brd, &screen] {
			screen.Post([this, &brd] { brd = gm->current_board(); });
			screen.PostEvent(Event::Custom);
			});
		gm->save_sig.connect([this] {
			auto str = gm->to_string();
			std::ofstream fout;
			if (!std::filesystem::exists(global_config->saved_games_path)) {
				std::filesystem::create_directories(global_config->saved_games_path);
			}
			fout.open(std::format("{}{}-{}.bin", global_config->saved_games_path, "othello", std::chrono::system_clock::now()), std::ios::binary | std::ios::out);
			if (fout) {
				fout << str;
			}
			else {
				ui::msgbox(gettext("Save game failed! Failed to create archive file."));
			}
			fout.close();
		});
		gm->suspend_sig.connect([] {ui::msgbox(gettext("Function in Developing")); });

		Component buttons = Container::Vertical({
			Button(censtr(gettext("Quit"), 8), [this] {gm->end_game(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Regret"), 8), [this] {gm->regret_sig(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Suspend"), 8), [this] {gm->suspend_sig(); } , ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Save"), 8), [this] {gm->save_sig(); } , ButtonOption::Animated()) | center,
			}) | ftxui::border;
		Component brd = Container::Horizontal({ brd_ptr }) | center;
		Component layout = Container::Vertical({
			Container::Horizontal({
				buttons | center,
				brd | center,
			}),
			Container::Vertical({
				Renderer([=,this] { return text(std::format("{}:{}",gettext("Current player"),gptr[gm->current_color()]->get_name())) | center; }),
				Renderer([=,this] { return text(std::format("{}-{}:{}",gettext("Black"),gptr[col0]->get_name(),brd_ptr->brd.countpiece(col0))) | center; }),
				Renderer([=,this] { return text(std::format("{}-{}:{}",gettext("White"),gptr[col1]->get_name(),brd_ptr->brd.countpiece(col1))) | center; }),
				Renderer([this] {
					return text(std::format("{}-{} {}:{}",
						gettext("Black"),
						gptr[col0]->get_name(),
						gettext("state"),
						gptr[col0]->good() ? gettext("Good") : gettext("Bad")
						)) | center;
					}),
				Renderer([this] {
					return text(std::format("{}-{} {}:{}",
						gettext("White"),
						gptr[col1]->get_name(),
						gettext("state"),
						gptr[col1]->good() ? gettext("Good") : gettext("Bad")
						)) | center;
					}),
			}) | center | border
			});
		return layout;
	}

	ftxui::Component Game::GameSettingPage(ftxui::ScreenInteractive& screen, bool& ret, bool& advanced, ftxui::Component Tabs, int& selector) {
		using namespace ftxui;
		std::vector<std::shared_ptr<int>> s = { std::make_shared<int>(0), std::make_shared<int>(0) };
		std::shared_ptr<int> s3 = std::make_shared<int>(2);
		auto start_func = [this, &screen, &ret, &advanced, Tabs, &selector, s, s3] {
			board_size = stoi(size_list[*s3]);
			bool adv[col1 - col0 + 1] = {};
			for (color col = col0; col <= col1; ++col) {
				switch (*s[col])
				{
				case std::to_underlying(detailed_type::human):
					gptr[col] = std::make_shared<human_gamer>(col);
					Tabs->Add(bw::components::HumanGamerSetting(gptr[col]));
					break;
				case std::to_underlying(detailed_type::computer_random):
					gptr[col] = std::make_shared<computer_gamer_random>(col);
					Tabs->Add(bw::components::GamerSetting<computer_gamer_random>(gptr[col]));
					break;
				case std::to_underlying(detailed_type::computer_ai):
					gptr[col] = std::make_shared<computer_gamer_ai>(col, board_size);
					Tabs->Add(bw::components::GamerSetting<computer_gamer_ai>(gptr[col]));
					adv[col] = true;
					break;
				case std::to_underlying(detailed_type::remote_tcp_gamer):
					gptr[col] = std::make_shared<remote_tcp_gamer>(pctx, col);
					Tabs->Add(bw::components::GamerSetting<remote_tcp_gamer>(gptr[col]));
					adv[col] = true;
					break;
				case std::to_underlying(detailed_type::lua_script_gamer):
					gptr[col] = std::make_shared<lua_gamer>(col);
					Tabs->Add(bw::components::GamerSetting<lua_gamer>(gptr[col]));
					adv[col] = true;
					break;
				case std::to_underlying(detailed_type::python_script_gamer):
					gptr[col] = std::make_shared<python_gamer>(col);
					Tabs->Add(bw::components::GamerSetting<python_gamer>(gptr[col]));
					adv[col] = true;
					break;
				default:
					break;
				}
			}
			advanced = adv[col0] || adv[col1];
			if (!adv[col0] && adv[col1]) {
				selector = col1;
			}
			else {
				selector = col0;
			}
			screen.Exit();
			};
		return Container::Vertical({
			Container::Horizontal({Renderer([s1 = s[col0]] {return ftxui::text(gettext("First Player:")); }) | center ,Dropdown(&gamer_list,s[col0].get()) | center ,}) | center,
			Container::Horizontal({Renderer([s2 = s[col1]] {return ftxui::text(gettext("Second Player:")); }) | center ,Dropdown(&gamer_list,s[col1].get()) | center ,}) | center ,
			Container::Horizontal({Renderer([s3] {return ftxui::text(gettext("Board size:")); }) | center ,Dropdown(&size_list,s3.get()) | center }) | center ,
			Container::Horizontal({
				Button(censtr(gettext("Start"), 8), start_func, ButtonOption::Animated(Color::GreenLight)) | center,
				ui::TextComp(" "),
				Button(gettext("Advanced"),[start_func, &advanced] {advanced = true; start_func(); },ButtonOption::Animated(Color::BlueLight)) | center,
				ui::TextComp(" "),
				Button(censtr(gettext("Back"), 8), [&ret,&screen] { ret = false; screen.Exit(); }, ButtonOption::Animated(Color::Red)) | center
				}) | center,
			}) | ui::EnableMessageBox();
	}

	ftxui::Component Game::GameAdvancedSettingPage(ftxui::Component Tabs, int& selector, bool& ret) {
		using namespace ftxui;
		std::vector<std::string> titles = {
				gettext("First Player"), gettext("Second Player")
		};
		auto option = MenuOption::HorizontalAnimated();
		option.underline.SetAnimation(std::chrono::milliseconds(1000),
			animation::easing::ElasticOut);
		option.entries_option.transform = [](EntryState state) {
			Element e = text(state.label) | hcenter | flex;
			if (state.active && state.focused)
				e = e | bold;
			if (!state.focused && !state.active)
				e = e | dim;
			return e;
			};
		option.underline.color_inactive = Color::White;
		option.underline.color_active = Color::BlueLight;
		auto player_menu = Menu(titles, &selector, option);
		auto MainPage = Container::Vertical({
			player_menu,
			Tabs | flex
			});
		auto Buttons = Container::Vertical({
			Button(gettext("Start"),[this] {
				if (!gptr[col0]->good()) {
					ui::msgbox(gettext("The setting information of the first gamer is incomplete or incorrect."));
				}
				else if (!gptr[col1]->good()) {
					ui::msgbox(gettext("The setting information of the second gamer is incomplete or incorrect."));
				}
				else {
					ftxui::ScreenInteractive::Active()->Exit();
				}
				},ButtonOption::Animated(Color::Green)) | center,
			Button(gettext("Back"),[&ret] {ftxui::ScreenInteractive::Active()->Exit(); ret = false; },ButtonOption::Animated(Color::Red)) | center
			}) | center;
		return ResizableSplit(
			ResizableSplitOption{
				.main = Buttons,
				.back = MainPage,
				.direction = Direction::Right,
				.main_size = 10,
				.separator_func = separatorDouble
			}
		);
	}
	
	ftxui::Component Game::GamePage(ftxui::ScreenInteractive& screen) {
		assert(gptr[col0] != nullptr && gptr[col1] != nullptr);
		using namespace ftxui;

		Board brd_ptr = Make<BoardBase>(pctx, gm);
		std::weak_ptr<BoardBase> weak_brd;
		gm->regret_sig.connect([weak_brd] {
			auto brd = weak_brd.lock();
			if (brd != nullptr) {
				brd->pmvdq->q.push_back({ .mvtype = move::regret });
				brd->pmvdq->tim.cancel_one();
			}
		});
		gm->end_sig.connect([this] {
			auto crt_brd = gm->current_board();
			int points0 = crt_brd.countpiece(col0), points1 = crt_brd.countpiece(col1);
			if (points0 > points1) {
				ui::msgbox(gettext("Black win!"), { ui::MakeOKButton(),ui::TextComp(" "),AnotherRoundButton(gm) });
			}
			else if (points0 < points1) {
				ui::msgbox(gettext("White win!"), { ui::MakeOKButton(),ui::TextComp(" "), AnotherRoundButton(gm) });
			}
			else {
				ui::msgbox(gettext("Draw!"), { ui::MakeOKButton(),ui::TextComp(" "), AnotherRoundButton(gm) });
			}
			});
		gm->flush_sig.connect([this, &brd=brd_ptr->brd, &screen] {
			screen.Post([this, &brd] { brd = gm->current_board(); });
			screen.PostEvent(Event::Custom);
			});
		gm->save_sig.connect([this] {
			auto str = gm->to_string();
			std::ofstream fout;
			if (!std::filesystem::exists(global_config->saved_games_path + "othello\\")) {
				std::filesystem::create_directories(global_config->saved_games_path + "othello\\");
			}
			auto file_name = std::format(R"(.\{}{}\{:%F--%H-%M-%S}.bin)", global_config->saved_games_path, "othello", gm->begin_time);
			fout.open(file_name, std::ios::binary | std::ios::out);
			if (fout.is_open()) {
				fout << str;
				ui::msgbox(gettext("Save game successfully"));
			}
			else {
				ui::msgbox(gettext("Save game failed! Failed to create archive file."));
			}
			fout.close(); 
			});
		gm->suspend_sig.connect([] {ui::msgbox(gettext("Function in Developing")); });

		Component buttons = Container::Vertical({
			Button(censtr(gettext("Quit"), 8),[this, &screen] { gm->end_game(); if (!pctx->stopped()) { pctx->stop(); } screen.Exit(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Regret"), 8), [this] {gm->regret_sig(); }, ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Suspend"), 8), [this] {gm->suspend_sig(); } , ButtonOption::Animated()) | center,
			Renderer([] {return separator(); }),
			Button(censtr(gettext("Save"), 8), [this] {gm->save_sig(); } , ButtonOption::Animated()) | center,
			}) | ftxui::border;
		Component brd = Container::Horizontal({ brd_ptr }) | center;
		Component layout = Container::Vertical({
			Container::Horizontal({
				buttons,
				brd,
			}),
			Container::Vertical({
				Renderer([this] { return text(std::format("{}:{}",gettext("Current player"),gptr[gm->current_color()]->get_name())) | center; }),
				Renderer([brd_ptr,this] {return text(std::format("{}-{}:{}",gettext("Black"),gptr[col0]->get_name(),brd_ptr->brd.countpiece(col0))) | center; }),
				Renderer([brd_ptr,this] {return text(std::format("{}-{}:{}",gettext("White"),gptr[col1]->get_name(),brd_ptr->brd.countpiece(col1))) | center; }),
				Renderer([this] {
					return text(std::format("{}-{} {}:{}",
					gettext("Black"),
					gptr[col0]->get_name(),
					gettext("state"),
					gptr[col0]->good() ? gettext("Good") : gettext("Bad")
					)) | center; 
					}),
				Renderer([this] {
					return text(std::format("{}-{} {}:{}",
					gettext("White"),
					gptr[col1]->get_name(),
					gettext("state"),
					gptr[col1]->good() ? gettext("Good") : gettext("Bad")
					)) | center; 
					}),
			}) | center | border
			});
		return layout;
	}

	bool Game::GameSetting() {
		using namespace ftxui;
		bool ret = true, advanced = false;
		ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
		ui::auto_close_modal _f;
		int selector = 0;
		Component Tabs = Container::Tab({}, &selector);;
		Component layout = GameSettingPage(screen, ret, advanced, Tabs, selector);
		screen.Loop(Renderer(layout, [&layout, &ret] {return dbox(layout->Render() | center); }));
		if (!ret)
			return false;
		if (advanced) {
			if (gptr[col0]->detailed_gamer_type == detailed_type::python_script_gamer
				&& gptr[col1]->detailed_gamer_type == detailed_type::python_script_gamer)
				ui::msgbox(gettext(
R"(Due to the implementation of Python, 
there will only be one interpreter instance 
during program execution, so please change 
the signatures of the "get_move" function for 
both players, otherwise one of them 
will be overwritten by the other.)"));
			screen.Loop(GameAdvancedSettingPage(Tabs, selector, ret) | ui::EnableMessageBox());
		}
		return ret;
	}
	
	void Game::GamePageLocal() {
		using namespace ftxui;

		another_round = false;
		ui::auto_close_modal _f;
		ScreenInteractive screen = ScreenInteractive::Fullscreen();

		gm = std::make_shared<game>(gptr[col0], gptr[col1], board_size, std::make_shared<bw::components::ftxui_screen>(&screen));
		Component GamePageComponent = GamePage(screen) | center | ui::EnableMessageBox();
		std::jthread j([this, &screen] {
			try
			{
				boost::cobalt::spawn(*pctx, gm->start(), boost::asio::detached);
				pctx->run();
				spdlog::trace("pctx use_count:{} game object use_count:{}", pctx.use_count(), gm.use_count());
			}
			catch (const std::exception& e)
			{
				ui::msgbox(gbk2utf8(e.what()));
				screen.Exit();
			}
			});
		screen.Loop(GamePageComponent);
		pctx->stop();
		pctx->restart();
		if (another_round) {
			gptr[col0]->reset();
			gptr[col1]->reset();
		}
	}
}