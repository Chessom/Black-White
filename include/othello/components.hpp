#pragma once
#include"stdafx.h"
#include"othello/game.hpp"
#include"othello/gamer/computer.hpp"
#include"othello/gamer/human.hpp"
#include"othello/gamer/remote.hpp"
#include"othello/gamer/online.hpp"
#include"globals.hpp"
#include"tui/components.hpp"
#include"tui/ftxui_screen.hpp"
#include"grid-container.hpp"
#include"online/signals.hpp"
#include"basic_Game.hpp"
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
					col = brd.getcol({ x,y });
					if (col == core::none) {
						row.emplace_back(text(game_ptr->valid_move(game_ptr->current_color(), { x,y }) ? " • " : "   ")
							| ftxui::color(Color::Red) | bgcolor(Color::Green));
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
			/*firstr.clear();
			for (int i = 0; i < brd.size; ++i) {
				firstr += L"  " + std::to_wstring(i) + L" ";
			}
			rows.push_back(text(firstr));
			Element colnum;
			firstr.clear();
			for (int i = 0; i < brd.size; ++i) {
				firstr += (L" " + to_wstring(i));
			}*/
			return vbox(std::move(rows)) | reflect(box);
		}
		bool OnEvent(ftxui::Event e) override {
			using namespace ftxui;
			if (e.is_mouse()) {
				return OnMouseEvent(e);
			}
			else if (e == Event::Special("Flush")) {
				brd = game_ptr->current_board();
				pmvdq->tim.cancel_one();
				game_ptr->screen->refresh();
				return true;
			}
			else if (e == Event::Special("End")) {
				int points0 = brd.countpiece(col0), points1 = brd.countpiece(col1);
				if (points0 > points1) {
					ui::msgbox(gettext("Black win!"));
				}
				else if (points0 < points1) {
					ui::msgbox(gettext("White win!"));
				}
				else {
					ui::msgbox(gettext("Draw!"));
				}
				return true;
			}
			else if (e == Event::Special("Regret")) {
				pmvdq->q.push_back({ .mvtype = move::regret });
				pmvdq->tim.cancel_one();
				return true;
			}
			else if (e == Event::Special("EndLoop")) {
				game_ptr->end_game();
				return true;
			}
			else if (e == Event::Special("Suspend")) {
				/*dqptr->q.push_back({ -1,-3 });
				dqptr->conv.notify_one();*/
				ui::msgbox(gettext("Function in Developing"));
				return true;
			}
			else if (e == Event::Special("Save")) {
				ui::msgbox(gettext("Function in Developing"));
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
	protected:
		bool mouse_hover = false;
		ftxui::Box box;
	};
	using Board = std::shared_ptr<BoardBase>;
	class Game :public basic_Game, public std::enable_shared_from_this<Game> {
	public:
		Game() { pctx = std::make_shared<boost::asio::io_context>(); }
		Game(std::shared_ptr<boost::asio::io_context> context_ptr) :basic_Game(context_ptr) {};
		inline std::string censtr(const std::string& str, int width) {
			return std::format("{0:^{1}}", str, width);
		}
		inline std::string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
		}
		ftxui::Component AnotherRoundButton() {
			using namespace ftxui;
			return Button(gettext("Another round"), [this] {

			}, ButtonOption::Animated());
		}
		ftxui::Component GamePreparingPage(ftxui::ScreenInteractive& screen, bool& ret, int& s1, int& s2, int& s3) {
			using namespace ftxui;
			return Container::Vertical({
				Container::Horizontal({Renderer([] {return ftxui::text(gettext("First Player:")); }) | center,Dropdown(&gamer_list,&s1) | align_right,}),
				Container::Horizontal({Renderer([] {return ftxui::text(gettext("Second Player:")); }) | center,Dropdown(&gamer_list,&s2) | align_right,}),
				Container::Horizontal({Renderer([] {return ftxui::text(gettext("Board size:")); }) | center,Dropdown(&size_list,&s3) | align_right}),
				Button(censtr(gettext("Start"), 8), screen.ExitLoopClosure(), ButtonOption::Animated()) | center,
				Button(censtr(gettext("Back"), 8), [&ret,&screen] { ret = false; screen.Exit(); }, ButtonOption::Animated()) | center
				}) | ui::EnableMessageBox();
		}
		virtual ftxui::Component OnlinePrepareCom(bw::online::basic_user_ptr uptr) override {
			using namespace ftxui;
			auto layout = Container::Vertical({
				ui::TextComp(gettext("Othello")) | center,
				Container::Horizontal({
					ui::TextComp(gettext("Board Size: ")) | center,
					Input(&s_size,"   ") | underlined,
				}) | center,
				Button(gettext("Match"),[this, uptr]
				{
					static boost::asio::steady_timer tim(*pctx);
					static std::atomic_flag flag;
					auto othello_Game = shared_from_this();
					board_size = std::stoi(s_size);
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
				},ButtonOption::Animated()) | center
				});
			return layout;
		}
		virtual ftxui::Component OnlineGamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) override {
			assert(gptr[col0] != nullptr && gptr[col1] != nullptr);
			using namespace ftxui;
			game_ptr gm = std::dynamic_pointer_cast<game>(gm_ptr);

			Board brd_ptr = Make<BoardBase>(pctx, gm);

			Component buttons = Container::Vertical({
				Button(censtr(gettext("Quit"), 8), [gm] {gm->end_game(); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr(gettext("Regret"), 8), [&screen] {screen.PostEvent(Event::Special("Regret")); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr(gettext("Suspend"), 8), [&screen] {screen.PostEvent(Event::Special("Suspend")); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr(gettext("Save"), 8), [&screen] {screen.PostEvent(Event::Special("Save")); }, ButtonOption::Animated()) | center,
				}) | ftxui::border;
			Component brd = Container::Horizontal({ brd_ptr }) | center;
			Component layout = Container::Vertical({
				Container::Horizontal({
					buttons | center,
					brd | center,
				}),
				Container::Vertical({
					Renderer([=,this] { return text(std::format("{}:{}",gettext("Current player"),gptr[gm->current_color()]->get_name())) | center; }),
					Renderer([=,this] {return text(std::format("{}-{}:{}",gettext("Black"),gptr[col0]->get_name(),brd_ptr->brd.countpiece(col0))) | center; }),
					Renderer([=,this] {return text(std::format("{}-{}:{}",gettext("White"),gptr[col1]->get_name(),brd_ptr->brd.countpiece(col1))) | center; }),
					Renderer([=,this] {return text(std::format("{}-{} {}:{}",gettext("Black"),gptr[col0]->get_name(),gettext("state"),
					gptr[col0]->is_remote() ?
						(std::dynamic_pointer_cast<remote_tcp_gamer>(gptr[col0])->connected() ? gettext("Good") : gettext("Disconnetced"))
						: gettext("Good"))) | center; }),
					Renderer([&] {return text(std::format("{}-{} {}:{}",gettext("White"),gptr[col1]->get_name(),gettext("state"),
					gptr[col1]->is_remote() ?
						(std::dynamic_pointer_cast<remote_tcp_gamer>(gptr[col1])->connected() ? gettext("Good") : gettext("Disconnetced"))
						: gettext("Good"))) | center; }),
				}) | center | border
				});
			return layout;
		}
		virtual ftxui::Component GamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) override {
			assert(gptr[col0] != nullptr && gptr[col1] != nullptr);
			using namespace ftxui;
			game_ptr gm = std::dynamic_pointer_cast<game>(gm_ptr);

			Board brd_ptr = Make<BoardBase>(pctx, gm);

			Component buttons = Container::Vertical({
				Button(censtr(gettext("Quit"), 8), [=, &screen] {gm->end_game(); if (!pctx->stopped()) { pctx->stop(); } screen.Exit(); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr(gettext("Regret"), 8), [&screen] {screen.PostEvent(Event::Special("Regret")); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr(gettext("Suspend"), 8), [&screen] {screen.PostEvent(Event::Special("Suspend")); }, ButtonOption::Animated()) | center,
				Renderer([] {return separator(); }),
				Button(censtr(gettext("Save"), 8), [&screen] {screen.PostEvent(Event::Special("Save")); }, ButtonOption::Animated()) | center,
				}) | ftxui::border;
			Component brd = Container::Horizontal({ brd_ptr }) | center;
			Component layout = Container::Vertical({
				Container::Horizontal({
					buttons,
					brd,
				}),
				Container::Vertical({
					Renderer([=,this] { return text(std::format("{}:{}",gettext("Current player"),gptr[gm->current_color()]->get_name())) | center; }),
					Renderer([=,this] {return text(std::format("{}-{}:{}",gettext("Black"),gptr[col0]->get_name(),brd_ptr->brd.countpiece(col0))) | center; }),
					Renderer([=,this] {return text(std::format("{}-{}:{}",gettext("White"),gptr[col1]->get_name(),brd_ptr->brd.countpiece(col1))) | center; }),
					Renderer([=,this] {return text(std::format("{}-{} {}:{}",gettext("Black"),gptr[col0]->get_name(),gettext("state"),
					gptr[col0]->is_remote() ?
						(std::dynamic_pointer_cast<remote_tcp_gamer>(gptr[col0])->connected() ? gettext("Good") : gettext("Disconnetced"))
						: gettext("Good"))) | center; }),
					Renderer([&] {return text(std::format("{}-{} {}:{}",gettext("White"),gptr[col1]->get_name(),gettext("state"),
					gptr[col1]->is_remote() ?
						(std::dynamic_pointer_cast<remote_tcp_gamer>(gptr[col1])->connected() ? gettext("Good") : gettext("Disconnetced"))
						: gettext("Good"))) | center; }),
				}) | center | border
				});
			return layout;
		}
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
				ret = computer_gamer_from_id(info.id, info.col);
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
			gm->screen_ptr = &screen;
			return gm;
		}
		bool GamePreparing() {
			using namespace ftxui;
			bool ret = true;
			ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
			int selected_2 = 0, selected_1 = 0, selected_3 = 2;
			ui::auto_close_modal _f;

			Component layout = GamePreparingPage(screen, ret, selected_1, selected_2, selected_3);
			screen.Loop(Renderer(layout, [&layout, &ret] {return dbox(layout->Render() | center); }));
			if (!ret)
				return ret;
			board_size = stoi(size_list[selected_3]);
			switch (selected_1)
			{
			case 0:gptr[col0] = std::make_shared<human_gamer>(col0); break;
			case 1:gptr[col0] = std::make_shared<computer_gamer_random>(col0); break;
			case 2:gptr[col0] = std::make_shared<computer_gamer_ai>(col0); break;
			case 3:gptr[col0] = std::make_shared<computer_gamer_ai>(col0); break;
				break;
			default:
				break;
			}
			switch (selected_2)
			{
			case 0:gptr[col1] = std::make_shared<human_gamer>(col1); break;
			case 1:gptr[col1] = std::make_shared<computer_gamer_random>(col1); break;
			case 2:gptr[col1] = std::make_shared<computer_gamer_ai>(col1); break;
			case 3:gptr[col1] = std::make_shared<computer_gamer_ai>(col1); break;
			default:
				break;
			}
			if (selected_1 == 4) {
				ret = RemoteGamerPrepare(col0);
				if (!ret)return false;
			}
			if (selected_2 == 4) {
				ret = RemoteGamerPrepare(col1);
				if (!ret)return false;
			}
			return ret;
		}
		bool RemoteGamerPrepare(color col) {

			ui::auto_close_modal _f;
			using namespace ftxui;
			ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

			std::string IP = "localhost", port = "22222";
			float prog = 0;
			bool ret = false;

			std::shared_ptr<remote_tcp_gamer> remote_ptr = Make<remote_tcp_gamer>(pctx, col);
			remote_ptr->socket_ptr = std::make_shared<boost::asio::ip::tcp::socket>(*pctx);
			vector<std::string> vec = { gettext("Actively Connect"),gettext("Wait for Connection") };
			int selected_mode = 0;
			auto toggle = Toggle(&vec, &selected_mode);
			bool running = false;

			auto active_connect = Container::Vertical({
				Container::Horizontal({
				Renderer([] {
					return vbox(text(gettext("Address:")) | border,text(gettext("Port:")) | border);
					}),
				Container::Vertical({
					Input(&IP, "localhost") | size(WIDTH,GREATER_THAN,16) | border,
					Input(&port, "22222") | size(WIDTH,GREATER_THAN,16) | border
					}),
				}) | center,
				Container::Horizontal({
				Button(gettext("Connect"),[&] {
					if (running) {
						ui::msgbox(gettext("Accepting Now"));
						ret = false;
						return;
					}
					try {
						if (remote_ptr->connected()) {
							remote_ptr->socket_ptr->close();
						}
						remote_ptr->connect(IP, std::stoi(port));
						screen.PostEvent(Event::Custom);
						prog = 1.0;
						ret = true;
					}
					catch (const std::exception& e) {
						ui::msgbox(boost::locale::conv::to_utf<char>(e.what(),"gbk"));
						ret = false;
					}
					},ButtonOption::Animated())
				}) | center ,
				Renderer([&ret] {return text(fmt::format("{}:[{}]",gettext("State"),ret ? gettext("Connected") : gettext("Disconnected"))) | center | border; })
				});

			std::string port_listen = "22222", remote_port = gettext("None"), remote_IP = gettext("None");
			auto runningtim = boost::asio::deadline_timer(*(remote_ptr->context_ptr));
			auto co_ac = [&]()->boost::cobalt::task<void> {
				running = true;
				boost::system::error_code ec;
				*(remote_ptr->socket_ptr) = std::move(co_await remote_ptr->acceptor_ptr->async_accept(boost::asio::redirect_error(boost::cobalt::use_op, ec)));
				if (ec) {
					ret = false;
					running = false;
					runningtim.cancel();
					screen.Exit();
					co_return;
				}

				auto ep = remote_ptr->socket_ptr->remote_endpoint();
				remote_IP = ep.address().to_string();
				remote_port = std::to_string(ep.port());
				ret = true;
				running = false;
				screen.PostEvent(Event::Custom);
				runningtim.cancel();
				co_return;
				};

			auto co_running = [&]()->boost::cobalt::task<void> {
				runningtim.expires_from_now(boost::posix_time::hours(24));
				boost::system::error_code ec;
				co_await runningtim.async_wait(boost::asio::redirect_error(boost::cobalt::use_op, ec));
				co_return;
				};
			auto wait_for_connect = Container::Vertical({
				Container::Horizontal({
						Renderer([] {return text(gettext("Listening port:")) | border; }),
						Input(&port_listen, "22222") | size(WIDTH,GREATER_THAN,16) | border
					}) | center,
				Renderer([&] {
					return
					hbox(
						vbox(
							text(gettext("remote Address:")) | center | border,
							text(gettext("remote Port:")) | center | border
						),
						vbox(
							text(remote_IP) | size(WIDTH, GREATER_THAN, 16) | center | border,
							text(remote_port) | size(WIDTH, GREATER_THAN, 16) | center | border
						)
					) | center;
				}),
				Renderer([] {return separator(); }),
				Container::Horizontal({
					Button(gettext("Start Accept"),[&] {
						if (!running) {
							try
							{
								boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), std::stoi(port_listen));
								remote_ptr->acceptor_ptr = std::make_shared<boost::asio::ip::tcp::acceptor>(*remote_ptr->context_ptr, ep);
								boost::cobalt::spawn(*(remote_ptr->context_ptr), co_ac(), boost::asio::detached);
								/*remote_ptr->thread_ptr = std::make_shared<std::jthread>([remote_ptr] {
										if (!remote_ptr->context_ptr->stopped()) {
											remote_ptr->context_ptr->stop();
										}
										remote_ptr->context_ptr->reset();
										remote_ptr->context_ptr->run();
									});*/
							}
							catch (const std::exception& e)
							{
								ui::msgbox(gbk2utf8(e.what()));
							}
						}
						},ButtonOption::Animated()),
					Button(gettext("Stop Accept"),[&] {
						if (running) {
							remote_ptr->context_ptr->post([&] {remote_ptr->acceptor_ptr->cancel(); });
							ret = false;
							screen.Exit();
						}
						else {
							ui::msgbox(gettext("Not Accepting"));
						}
						},ButtonOption::Animated()),
					}) | center,
				Renderer([&] {return text(fmt::format("{}:[{}]",gettext("State"),running ? gettext("Accepting") : (ret ? gettext("Connected") : gettext("Stopped")))) | center | border; })
				});

			auto tab = Container::Tab({ active_connect,wait_for_connect }, &selected_mode);
			auto container = Container::Vertical({
				toggle,
				tab,
				});
			auto layout = Container::Vertical({
				Renderer(container, [&, this] {
				return vbox({
						   toggle->Render(),
						   separator(),
						   tab->Render(),
					}) |
					border;
				}),
				Container::Horizontal({
					Button(gettext(" Quit "),[&] {
						if (running) {
							remote_ptr->context_ptr->post([&] {remote_ptr->acceptor_ptr->cancel(); });
						}
						ret = false;
						screen.Exit();
						return false;
					},ButtonOption::Animated()),
					Button(gettext("  OK  "),[&] {
						if (running) {
							ui::msgbox(gettext("Accepting Now"));
							ret = false;
							return;
						}
						if (remote_ptr->connected() && ret) {
							running = false;
							screen.Exit();
						}
						else {
							ui::msgbox(gettext("Not Connected"));
						}
					},ButtonOption::Animated())
				}) | center | border,
				Renderer([] {return text(gettext("WARNING:")) | ftxui::color(Color::Red); }) | center,
				Renderer([] {return ui::autolines(gettext(
					"The purpose of this mode is to provide users\n"
					"with a customized interface for the game.\n"
					"If you want to go online, please go to \n"
					"\"Home->Online\"\n"
					"For more information about this mode, please go to\n"
					"\"Home->More\"")
					, center | ftxui::color(Color::BlueLight)); }) | center
				});


			boost::cobalt::spawn(*remote_ptr->context_ptr, co_running(), boost::asio::detached);
			remote_ptr->thread_ptr = std::make_shared<std::jthread>([remote_ptr] {remote_ptr->context_ptr->run(); });

			screen.Loop(layout | center | ui::EnableMessageBox());

			if (remote_ptr->acceptor_ptr) {
				remote_ptr->acceptor_ptr->close();
			}
			runningtim.cancel();
			if (!pctx->stopped()) {
				pctx->stop();
			}
			remote_ptr->thread_ptr->join();
			pctx->restart();
			assert(!pctx->stopped());
			if (remote_ptr->connected()) {
				remote_ptr->start();
				gptr[col] = remote_ptr;
				return true;
			}
			else {
				return false;
			}

			
		}
		void GamePageLocal() {
			using namespace ftxui;

			ui::auto_close_modal _f;
			ScreenInteractive screen = ScreenInteractive::Fullscreen();

			std::shared_ptr<game> gm = 
				std::make_shared<game>(gptr[col0], gptr[col1], board_size, std::make_shared<bw::components::ftxui_screen>(&screen));
			gm->screen_ptr = &screen;
			Component GamePageComponent = GamePage(screen, gm) | center | ui::EnableMessageBox();
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
			screen.Loop(GamePageComponent);
			pctx->stop();
		}
		void reset() {
			gptr[0] = nullptr;
			gptr[1] = nullptr;
			board_size = 8;
			s_size = "8";
		}
		
		void set_board_size(int size) override {
			board_size = size;
		}
		gamer_ptr gptr[2] = { nullptr,nullptr };
		int board_size = 8;
	protected:
		std::string s_size = "8";
	};
	using othello_Game_ptr = std::shared_ptr<Game>;
}
//std::string temp_name = gptr[op_col(col)]->name, opname = "Unknown";
			//std::atomic_flag started, opstarted;
			//std::vector<std::string> colorVec = { "Black","White" };
			//std::vector<std::string> colorVecOp = { "Black","White","Unknown" };
			//int selectedCol = op_col(col);
			//int selectedSize = 0;
			//std::atomic_int opcol = 2;
			//std::atomic_int opsize = 0;
			//auto move_handler = [&](const move& mv) {
			//	if (mv.mvtype == move::str) {
			//		opcol = mv.c;
			//		opname = mv.msg;
			//		opsize = mv.pos.x;
			//		screen.PostEvent(Event::Custom);
			//		return;
			//	}
			//	if (mv.mvtype == move::start) {
			//		opstarted.test_and_set();
			//		if (started.test()) {
			//			screen.Exit();
			//			remote_ptr->post([remote_ptr] {remote_ptr->clear_handler(); });
			//		}
			//	}
			//	};
			//remote_ptr->register_handle_move(std::make_shared<remote_tcp_gamer::move_handler>(std::move(move_handler)));
			//auto confirmpage = Container::Vertical({
			//	Container::Horizontal({
			//		Container::Vertical({
			//			Renderer([&] {return text("You") | center | ftxui::size(ftxui::WIDTH,ftxui::GREATER_THAN,8); }) | ftxui::size(ftxui::HEIGHT,ftxui::GREATER_THAN,3),
			//			Renderer([] {return separator(); }),
			//			Renderer([&] {return text("Opponent") | center | ftxui::size(ftxui::WIDTH,ftxui::GREATER_THAN,8); }) | ftxui::size(ftxui::HEIGHT,ftxui::GREATER_THAN,3),
			//		}),
			//		Renderer([] {return separator(); }),
			//		Container::Vertical({
			//			Input(&temp_name,gptr[op_col(col)]->name) | center | ftxui::size(ftxui::HEIGHT,ftxui::GREATER_THAN,3),
			//			Renderer([] {return separator(); }),
			//			Renderer([&opname] {return text(opname) | center; }) | ftxui::size(ftxui::HEIGHT,ftxui::GREATER_THAN,3),
			//		}),
			//		Renderer([] {return separator(); }),
			//		Container::Vertical({
			//			Dropdown(&colorVec,&selectedCol) | frame | ftxui::size(ftxui::HEIGHT,ftxui::LESS_THAN,3),
			//			Renderer([] {return separator(); }),
			//			Renderer([&] {return text(colorVecOp[opcol]) | center; }) | ftxui::size(ftxui::HEIGHT,ftxui::GREATER_THAN,3),
			//		}),
			//		Renderer([] {return separator(); }),
			//		Container::Vertical({
			//			Dropdown(&size_list,&selectedSize) | frame | ftxui::size(ftxui::HEIGHT,ftxui::LESS_THAN,3),
			//			Renderer([] {return separator(); }),
			//			Renderer([&] {return text(to_string(opsize)) | center; }) | ftxui::size(ftxui::HEIGHT,ftxui::GREATER_THAN,3),
			//		}),
			//	}) | border,
			//	Container::Horizontal({
			//		Button("Send",[&] {
			//			if (!opstarted.test())
			//				remote_ptr->pass_move({.mvtype = move::str,.pos = {stoi(size_list[selectedSize]),0},.c = selectedCol,.msg = temp_name });
			//		},ButtonOption::Animated()),
			//		Button("Start",[&] {
			//			if (!opstarted.test())
			//				remote_ptr->pass_move({.mvtype = move::str,.pos = {stoi(size_list[selectedSize]),0},.c = selectedCol,.msg = temp_name });
			//			if (opcol == op_col(selectedCol) && opsize == stoi(size_list[selectedSize])) {
			//				started.test_and_set();
			//				remote_ptr->pass_move({.mvtype = move::start });
			//				if (opstarted.test()) {
			//					ret = true;
			//					screen.Exit();
			//				}
			//				else {
			//					ui::msgbox("Wait for your opponent to start...");
			//				}
			//			}
			//			else {
			//				started.clear();
			//				if (opcol == 2 || opsize == 0) {
			//					ui::msgbox("Wait for your opponent and retry...");
			//				}
			//				else {
			//					ui::msgbox("Opponent same color or different size!");
			//				}
			//			}
			//		},ButtonOption::Animated())
			//	})
			//	});
			//screen.Loop(Renderer(confirmpage, [&confirmpage] {return dbox(confirmpage->Render() | center); }) | ui::EnableMessageBox());
			//board_size = opsize;
			//if (remote_ptr->connected()) {
			//	gptr[col ^ 1]->name = temp_name;
			//	gptr[col ^ 1]->col = op_col(opcol);
			//	remote_ptr->name = opname;
			//	remote_ptr->col = opcol;
			//	gptr[col] = std::static_pointer_cast<gamer>(remote_ptr);
			//	if (op_col(col) != selectedCol) {
			//		std::swap(gptr[col], gptr[col ^ 1]);
			//		//std::swap(gptr[col]->col, gptr[col ^ 1]->col);
			//	}
			//}
			//else {
			//	return false;
			//}
			//return ret;