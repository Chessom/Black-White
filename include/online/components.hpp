#pragma once
#include"stdafx.h"
#include"signals.hpp"
#include"tui/components.hpp"
#include"online/user.hpp"
#include"othello/gamer/online.hpp"
#include"othello/components.hpp"
#include"tictactoe/gamer/online.hpp"
#include"tictactoe/components.hpp"
namespace bw::online::components {
	
	struct SideBar {
		SideBar(user_ptr G, ftxui::ScreenInteractive& scr) :self(G), screen(scr) {}
		ftxui::Component MakeEditNameWindow() {
			using namespace ftxui;

			auto SubmitNameBut = Button(gettext("Submit"), [this] {
				self->try_update_name(NameBuf.data());
				showEditNameWindow = false;
				screen.PostEvent(Event::Custom);
				}, ButtonOption::Animated(Color::Green, Color::White, Color::GreenLight, Color::White));
			auto CancelSubmitName = Button(gettext("Cancel"), [this] {
				showEditNameWindow = false;
				screen.PostEvent(Event::Custom);
				}, ButtonOption::Animated(Color::Red, Color::White, Color::RedLight, Color::White));
			auto EditNameInput = Input(&NameBuf, gettext("Enter your name"), InputOption::Spacious());
			auto EditName = Renderer(EditNameInput, [EditNameInput] {return hbox(text(gettext("Name: ")) | center, EditNameInput->Render() | center) | center; });
			auto EditNameInner = ftxui::Container::Vertical({
				EditName,
				ftxui::Container::Horizontal({SubmitNameBut,CancelSubmitName}) | center,
				Renderer([] {return text(gettext("Changes will take effect later")) | center; })
				});
			EditNameWindow = Window({
				.inner = EditNameInner,
				.title = gettext("Edit Name"),
				.left = 25,
				.top = 6,
				.width = 35,
				.height = 10,
				.resize_left = true,
				.resize_right = true,
				.resize_top = false,
				.resize_down = false,
				.render = &ui::AlwaysActiveRenderState,
				});
			return Maybe(EditNameWindow, &showEditNameWindow) | bgcolor(Color::Black);
		}
		ftxui::Component MakeSideBarComp() {
			using namespace ftxui;
			Component QuitServerBut = Button(gettext("Quit"), 
				[&, this] { self->stop_flag.test_and_set(); self->stop(); screen.Exit(); }
			, ButtonOption::Animated(Color::Red));
			/*Name show and edit*/
			NameBuf = self->name;
			auto ShowName = Renderer([this] {return text(std::format("{}: {}", gettext("Name"), self->name)); }) | center | borderRounded;
			auto ShowRoom = Renderer([this] {return text(std::format("{}:{}", gettext("Room ID"), self->crt_room_info.load().id > 0 ? std::to_string(self->crt_room_info.load().id) : gettext("None"))); }) | center | borderRounded;
			if (!EditNameWindow) throw std::runtime_error("Invalid Component!");
			auto EditBut = Button(gettext("Edit Name"), [this] {
				showEditNameWindow = true;
				EditNameWindow->TakeFocus();
				}, ButtonOption::Animated(Color::GrayLight)) | center;
			/*END*/

			/*Bulletin*/
			auto RefreshNoticesBut = Button(gettext("Refresh Notices"), [this] {self->get("notices"); }, ButtonOption::Animated(Color::Blue)) | center;
			auto BulletinComp = Renderer([this] {
				Elements es;
				for (auto& s : self->notices) {
					es.push_back(paragraphAlignJustify(s));
				}
				return window(text(gettext("Notices")) | center, vbox(es) | vscroll_indicator | yframe | flex);
				});
			auto Bulletin = Container::Vertical({ RefreshNoticesBut ,BulletinComp });
			/*END*/


			return ftxui::Container::Vertical({
				ShowName,
				Renderer([this] {return text(std::format("{} :{}",gettext("ID"),self->id)); }) | center | borderRounded,
				ftxui::Container::Horizontal({EditBut, QuitServerBut}) | center,
				Bulletin | center
			});
		}
		user_ptr self;
		ftxui::ScreenInteractive& screen;
		std::string NameBuf;
		bool showEditNameWindow = false;
		ftxui::Component EditNameWindow;
	};
	struct RoomInfoPages {
		RoomInfoPages(user_ptr G,ftxui::ScreenInteractive& scr) :self(G),screen(scr) {}
		
		ftxui::Component MakeWindow(bool& showRoomChatPage, bool& showRoomChatTaskBar) {
			using namespace ftxui;
			auto CurrentRoomInfo = Renderer([this, &showRoomChatPage, &showRoomChatTaskBar] {
				auto rinfo = self->crt_room_info.load();
				bool in_any_room = rinfo.id != 0;
				if (!in_any_room) {
					showRoomChatPage = false;
					showRoomChatTaskBar = false;
				}
				auto style = ftxui::color(Color::BlueLight) | center;
				return
					hbox(
						vbox(
							emptyElement() | flex,
							text(gettext("Current Room:")) | borderRounded
						) | center,
						vbox(
							hbox(
								text(gettext("ID")) | style | size(WIDTH, GREATER_THAN, 4),
								separator(),
								text(gettext("Name")) | style | size(WIDTH, GREATER_THAN, 8),
								separator(),
								text(gettext("Num")) | style | size(WIDTH, GREATER_THAN, 5),
								separator(),
								text(gettext("State")) | style | size(WIDTH, GREATER_THAN, 7)
							),
							separator(),
							hbox(
								text(in_any_room ? std::to_string(rinfo.id) : gettext("None")) | center | size(WIDTH, GREATER_THAN, 4),
								separator(),
								text(in_any_room ? rinfo.name : gettext("None")) | center | size(WIDTH, GREATER_THAN, 8),
								separator(),
								text(in_any_room ? std::to_string(rinfo.usersize) : gettext("None")) | center | size(WIDTH, GREATER_THAN, 5),
								separator(),
								text(in_any_room ? state_str[rinfo.state].data() : gettext("None")) | center | size(WIDTH, GREATER_THAN, 7)
							)
						) | border | center
					);
				});
			auto SearchResult = Renderer([this] {
				auto rinfo = self->search_rinfo_res.load();
				return
					hbox(
						text(gettext("Result:")) | borderRounded | center,
						hbox(
							text(std::to_string(rinfo.id)) | center | size(WIDTH, GREATER_THAN, 4),
							separator(),
							text(rinfo.name) | center | size(WIDTH, GREATER_THAN, 8),
							separator(),
							text(std::to_string(rinfo.usersize)) | center | size(WIDTH, GREATER_THAN, 5),
							separator(),
							text(state_str[rinfo.state].data()) | center | size(WIDTH, GREATER_THAN, 7)
						) | border | center
					) | center;
				});
			auto OperationPannel = Container::Horizontal({
				ui::TextComp(gettext("Room ID:")) | center,
				Input(&search_str,gettext("Enter Room ID")) | border | 
				CatchEvent([&](Event event) {
					return event.is_character() && !std::isdigit(event.character()[0]);
				})|CatchEvent([&](Event event) {
					return event.is_character() && search_str.size() > 5;
				}),
				Button(gettext("Search"), [this] {
					if(!search_str.empty())
						self->get("search_room_info",{std::stoi(search_str)});
				}, ButtonOption::Animated(Color::Blue, Color::White)) | center,
				Button(gettext("Join"), [this] {
					if (self->infostate == user::latest) {
						if (self->in_room()) {
							ui::msgbox(gettext("You are already in a room."));
						}
						else {
							self->infostate = user::outdated;
							self->try_join_room(std::stoi(search_str));
						}
					}
					else {
						ui::msgbox(gettext("Please wait for the information update to be completed."));
					}
				}, ButtonOption::Animated(Color::Green, Color::White)) | center,
				Button(gettext("Quit"),[this,&showRoomChatPage,&showRoomChatTaskBar] {
					if (self->infostate == user::latest && self->in_room()) {
						self->infostate = user::outdated;
						self->try_leave();
						showRoomChatPage = false;
						showRoomChatTaskBar = false;
					}
					else {
						ui::msgbox(gettext("Please wait for the information update to be completed."));
					}
				}, ButtonOption::Animated(Color::Red, Color::White)) | center,
				Button(gettext("Refresh"), [this] {
					self->infostate = user::outdated;
					self->get("current_room_info");
				}, ButtonOption::Animated(Color::Blue, Color::White)) | center,
				Button(gettext("Hide"),[this] {showRoomInfoPage = false; },ButtonOption::Animated()) | center
				});
			auto RoomInfoInner = Container::Vertical({
				CurrentRoomInfo | center,
				SearchResult | center,
				OperationPannel | center,
				});
			auto RoomInfoWindow = Window({
				.inner = RoomInfoInner,
				.title = gettext("Room Information"),
				.left = &RoomInfoX,
				.top = &RoomInfoY,
				.width = &RoomInfoWidth,
				.height = &RoomInfoHeight,
				.render = &ui::AlwaysActiveRenderState,
				});
			return Maybe(RoomInfoWindow, &showRoomInfoPage) | bgcolor(Color::Black);
		}
		user_ptr self;
		int RoomInfoHeight = 13;
		int RoomInfoWidth = 62;
		int RoomInfoX = 2;
		int RoomInfoY = 1;
		int search_room_id = 1;
		std::string search_str;
		ftxui::ScreenInteractive& screen;
		ftxui::Component RoomInfoEntries;
		bool showRoomInfoPage = true;
	};
	struct RoomChatPages {
		RoomChatPages(user_ptr G) :self(G) {};
		ftxui::Component MakeAvatarButton(const str_msg& msg) {
			using namespace ftxui;
			return Button(msg.name, [&, this] {
				selectedPlayerID = msg.id1;
				}, ButtonOption::Border());
		}
		ftxui::Component MakeChatMsg(const str_msg& msg) {
			using namespace ftxui;
			auto but = MakeAvatarButton(msg);
			auto temp = Renderer(but, [this, but, msg] {
				if (msg.id1 == self->id) {
					return hbox(ui::autolines(msg.content) | borderRounded, but->Render()) | align_right;
				}
				else {
					return hbox(but->Render(), ui::autolines(msg.content) | borderRounded);
				}
				});
			return temp | vcenter;
		}
		ftxui::Component MakeWindow() {
			using namespace ftxui;
			auto InputChatComp = Input(&chatmsg, gettext("Enter your message here"), InputOption::Spacious());
			auto NextlineBut = Button(gettext("Next line"), [InputChatComp] {
				InputChatComp->OnEvent(Event::Return);
				InputChatComp->TakeFocus();
				}, ButtonOption::Animated(Color::Orange1));
			auto SendChatCom = 
				ftxui::Container::Vertical({
					InputChatComp | flex ,
					Renderer(NextlineBut, [NextlineBut] {
						return hbox(text(gettext("Press enter to send")) ,filler(), NextlineBut->Render() | vcenter);
					})
				}) | flex | ftxui::CatchEvent(
					[this](Event e) {
						if (e == Event::Return) {
							if (self->in_room()) {
								if (chatmsg != "") {
									self->broadcast(chatmsg);
									chatmsg = "";
								}
							}
							else {
								show_content = false;
								showRoomChatTaskBar = false;
							}
							return true;
						}
						return false;
					});
			auto temp_ = ResizableSplitBottom(
				SendChatCom, 
				RoomChatMsgs | Renderer([this] (Element e){
						if (RoomChatMsgs->ChildCount() == 0)
							return emptyElement(); 
						else {
							return e;
						}
					}) | vscroll_indicator | frame | flex,
				&SendChatSize);
			content = temp_ | CatchEvent([this, temp_](Event e) {
				if (e == Event::Special("AddChat")) {
					AddRoomChatMsg();
					return true;
				}
				else if (e == Event::Special("RefreshChat")) {
					RefreshRoomChatMsgs();
					return true;
				}
				else {
					return temp_->OnEvent(e);
				}
			});
			auto RoomChatWindow = Window({
				.inner = content,
				.title = gettext("Chat"),
				.left = &RoomChatPageX,
				.top = &RoomChatPageY,
				.width = &RoomChatPageWidth,
				.height = &RoomChatPageHeight,
				.render = &ui::AlwaysActiveRenderState,
				});
			self->show_room_chats.connect([this] {showRoomChatTaskBar = true; show_content = true; });
			return Maybe(RoomChatWindow, &show_content) | bgcolor(Color::Black);
		}
		void RefreshRoomChatMsgs() {
			if (RoomChatMsgs->ChildCount())
				RoomChatMsgs->DetachAllChildren();
			self->chat_mutex.acquire();
			auto msgq = self->chat_msg_queue;
			self->chat_mutex.release();
			if (!msgq.empty()) {
				for (int i = 0; i < msgq.size() - 1; ++i) {
					RoomChatMsgs->Add(MakeChatMsg(msgq[i]));
				}
				ftxui::Component LastMessage = MakeChatMsg(msgq.back());
				RoomChatMsgs->Add(LastMessage);
				LastMessage->TakeFocus();
			}
		}
		void AddRoomChatMsg() {
			using namespace ftxui;
			self->chat_mutex.acquire();
			auto msg = self->chat_msg_queue.back();
			self->chat_mutex.release();
			auto NewChatMsg = MakeChatMsg(msg);
			RoomChatMsgs->Add(NewChatMsg);
			NewChatMsg->TakeFocus();
		}
		user_ptr self;
		int SendChatSize = 7;
		int RoomChatPageX = 85;
		int RoomChatPageY = 1;
		int RoomChatPageWidth = 73;
		int RoomChatPageHeight = 25;
		std::string chatmsg;
		ftxui::Component content, RoomChatMsgs = ftxui::Container::Vertical({});
		bool show_content = false;
		bool showRoomChatTaskBar = false;
		int selectedPlayerID = 0;
	};
	struct GamePage {
		GamePage(user_ptr G, ftxui::ScreenInteractive& scr) :self(G), screen(scr) {
			GamePageCom = ftxui::Container::Vertical({});
		}
		ftxui::Component MakeGamePreparePages() {
			
			tab_entries = {
				gettext("Othello"),
				gettext("TicTacToe"),
			};
			auto tab_selection = ftxui::Menu(&tab_entries, &game_index, ftxui::MenuOption::HorizontalAnimated());
			auto tab_content = ftxui::Container::Tab({}, &game_index);


			//othello
			auto othello_Game = std::make_shared<othello::components::Game>(self->get_executor());
			Games.push_back(othello_Game);
			tab_content->Add(othello_Game->OnlinePrepareCom(self));

			//tictactoe
			auto tictactoe_Game = std::make_shared<tictactoe::components::Game>(self->get_executor());
			Games.push_back(tictactoe_Game);
			tab_content->Add(tictactoe_Game->OnlinePrepareCom(self));
			//END
			auto exit_button = ftxui::Button(
				gettext("Hide"), [this] { showGameWindow = false; }, ftxui::ButtonOption::Animated());
			auto watch_button = ftxui::Button(
				gettext("Watch"), [this] {
					if (self->infostate == user::outdated) {
						ui::msgbox(gettext("Please wait for the information update to be completed."));
						return;
					}
					if (self->in_room()) {
						if (self->state == user_st::free) {
							self->deliver(wrap(
								game_msg{
									.type = game_msg::watch,
									.id = self->id,
								},
								msg_t::game
							));
							self->infostate = user::outdated;
						}
						else {
							ui::msgbox(gettext("You can only watch games in an \"Free\" state"));
						}
					}
				}, ftxui::ButtonOption::Animated(ftxui::Color::Blue)
			);
			auto clear_button = ftxui::Button(
				gettext("Clear"), [this] {
					if (self->infostate == user::outdated) {
						ui::msgbox(gettext("Please wait for the information update to be completed."));
						return;
					}
					if (self->in_room()) {
						if (self->state == user_st::free) {
							GamePageCom->DetachAllChildren();
							self->Game_ptr = nullptr;
							self->gm_ptr = nullptr;
						}
						else {
							ui::msgbox(gettext("You can only clear the board in an \"Free\" state"));
						}
					}
				}
			, ftxui::ButtonOption::Animated(ftxui::Color::CyanLight));
			auto main_container =
				ftxui::Container::Vertical({
					tab_selection | ftxui::center,
					tab_content | ftxui::center,
					ftxui::Container::Horizontal({exit_button , watch_button  }) | ftxui::center ,
					clear_button | ftxui::center,
					ftxui::Renderer(
					[] {
						return ftxui::paragraphAlignJustify(gettext(
						"When you are in the \"Watching\" state,"
						"if there is a game going on,"
						"you can watch the game from the sidelines")) | ftxui::border | ftxui::center;
					}),
				});
			return main_container;
		}
		ftxui::Component MakeGamePage() {
			using namespace ftxui;
			signals::start_game.connect([this]() {
				GamePageCom->DetachAllChildren();
				self->gm_ptr = self->Game_ptr->generate_game(screen);
				GamePageCom->Add(self->Game_ptr->OnlineGamePage(screen, self->gm_ptr));
				boost::cobalt::spawn(*self->get_executor(), self->gm_ptr->start(), boost::asio::detached);
				screen.PostEvent(Event::Custom);
			});
			signals::end_game.connect([this](int code) {
				if (self->state == user_st::gaming) {
					self->deliver(wrap(
						game_msg{
							.type = game_msg::end,
							.id = code,
						},
						msg_t::game
						));
					if (code == game_msg::gamer_quit_or_disconnect) {
						ui::msgbox(gettext("The game ends."));
					}
					else if (code == game_msg::server_end_game) {
						ui::msgbox(gettext("Server end the game."));
					}
					self->state = user_st::free;
				}
				else if (self->state == user_st::watching) {
					if (code == game_msg::gamer_quit_or_disconnect) {
						ui::msgbox(gettext("The game ends."));
					}
					else if (code == game_msg::server_end_game) {
						ui::msgbox(gettext("Server end the game."));
					}
					self->state = user_st::free;
				}
			});
			self->prepare_watch_game.connect([this](int game_id) {
				switch (game_id)
				{
				case core::gameid::othello:
					self->Game_ptr = Games[0];
					break;
				case core::gameid::tictactoe:
					self->Game_ptr = Games[1];
					break;
				default:
					break;
				}
			});
			return GamePageCom | Renderer(
				[this](Element e){
					if (GamePageCom->ChildCount() == 0) {
						return text(gettext("No ongoing games")) | center;
					}
					else {
						return e;
					}
				});
		}
		ftxui::Component MakeWindow() {
			using namespace ftxui;
			auto _st_str = gettext("State");
			std::vector<std::string> state_str_map = { gettext("Matching"),gettext("Gaming"),gettext("Free"),gettext("Watching") };
			Component Content = ResizableSplitLeft(
				Container::Vertical({
					MakeGamePreparePages() | hcenter,
					Renderer([this, _st_str, state_str_map] {return text(std::format("{}: {}",_st_str,state_str_map[self->state])) | center | border; }),
				}),
				MakeGamePage() | center,
				&RightSize
			);
			return Maybe(Window({
				.inner = Content,
				.title = gettext("Game"),
				.left = &GamePageX,
				.top = &GamePageY,
				.width = &GamePageWidth,
				.height = &GamePageHeight,
				.render = &ui::AlwaysActiveRenderState,
				}), &showGameWindow) | bgcolor(Color::Black);
		}
		user_ptr self;
		int game_index = 0;
		int GamePageX = 2;
		int GamePageY = 15;
		int GamePageWidth = 80;
		int GamePageHeight = 30;
		int RightSize = 18;
		ftxui::ScreenInteractive& screen;
		ftxui::Component GamePageCom, GamePreparePage;
		bool showGameWindow = true;
		bool showGamePage = false;
		std::vector<basic_Game_ptr> Games;
		std::vector<std::string> tab_entries;
	};
	class HallPages {
	public:
		HallPages() :
			GamePageCom(self,screen),
			RoomInfoPagesCom(self, screen),
			RoomChatPagesCom(self),
			SideBarComp(self, screen),
			self(std::make_shared<user>())
		{}

		ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
		user_ptr self;

		ftxui::Component Taskbar = ftxui::Container::Horizontal({});
		int taskbarsize = 3;
		
		ftxui::Component Windows;

		SideBar SideBarComp;
		
		RoomInfoPages RoomInfoPagesCom;

		RoomChatPages RoomChatPagesCom;

		GamePage GamePageCom;

		ftxui::Component MakeTaskBut(bool& showflag, std::string button_name) {
			using namespace ftxui;
			return Button(button_name, [&showflag] {
				showflag = showflag ? false : true;
				}, ButtonOption::Animated());
		}

		void startHallPages();
		void ConnectServer() {
			using namespace ftxui;
			auto screen = ftxui::ScreenInteractive::Fullscreen();
			std::string PortString = "22222";
			Component enter = ftxui::Container::Vertical({
				ftxui::Container::Horizontal({
				Renderer([] {
					return vbox(text(gettext("Address:")) | border,text(gettext("Port:")) | border);
					}),
				ftxui::Container::Vertical({
					Input(&self->hall.address, gettext("IP or Domain Name")) | size(WIDTH,GREATER_THAN,16) | border,
					Input(&PortString, "22222") | size(WIDTH,GREATER_THAN,16) | border
					}),
				}) | center,
				ftxui::Container::Horizontal({
				Button(gettext("Connect"),[&screen,&PortString,s = self] {
					if (s->connect(s->hall.address, std::stoi(PortString))) {
						screen.Exit();
						s->hall.port = std::stoi(PortString);
					}
					},ButtonOption::Animated(Color::Green,Color::White,Color::GreenLight,Color::White)),
				Button(gettext("Quit"),screen.ExitLoopClosure(),ButtonOption::Animated(Color::Red,Color::White,Color::RedLight,Color::White)),
				}) | center,
				}) | ui::EnableMessageBox();
			screen.Loop(enter | center);
		};
		bool LoginServer() {
			using namespace ftxui;
			using namespace std::chrono_literals;
			/*std::condition_variable cond;
			std::mutex m;
			bool flag = false;
			boost::signals2::scoped_connection c(self->read_msg.connect(
				[&cond, &flag](const message& msg) {
					if (msg.type == msg_t::control) {
						control_msg con_m;
						struct_json::from_json(con_m, msg.jsonstr);
						if (con_m.type == control_msg::create && con_m.target_type == control_msg::g) {
							cond.notify_all();
							flag = true;
						}
					}
				})
			);*/
			if (global_config->first_login) {
				std::string loginName = global_config->default_name;
				bool setDefault = true;
				auto screen = ftxui::ScreenInteractive::Fullscreen();
				auto inputCom = Input(&loginName, gettext("Enter your name to login"), InputOption::Spacious()) | border | center;
				auto loginCom = ftxui::Container::Vertical({
					Renderer(inputCom,[&inputCom] {return hbox(text(gettext("You name:")) | center,inputCom->Render() | center); }) | center,
					Checkbox(gettext("Set this name as default"),&setDefault) | center,
					Button(gettext("Login"),screen.ExitLoopClosure(),ButtonOption::Animated(Color::Green,Color::White,Color::Green4,Color::White)) | center
					}) | ui::EnableMessageBox();
				screen.Loop(loginCom | center);
				if (setDefault) {
					global_config->first_login = false;
					global_config->default_name = loginName;
					std::string config_json;
					struct_json::to_json(*global_config, config_json);
					std::ofstream fout(json_config_path, std::ios::out);
					fout << json_format(config_json);
					fout.close();
				}
				self->name = loginName;
			}
			
			
			self->name = global_config->default_name;
			self->login();

			//if (flag == false) {
			//	boost::system::error_code ec;
			//	std::unique_lock lock(m);
			//	cond.wait_for(lock, 5s);
			//}
			return true;
		}
		~HallPages() = default;
	};
	void HallPages::startHallPages() {
		signals::start_game.disconnect_all_slots();
		signals::end_game.disconnect_all_slots();
		using namespace ftxui;
		self->scr_ptr = std::make_shared<bw::components::ftxui_screen>(&screen);
		ConnectServer();
		if (self->connected()) {
			self->start();
		}
		else {
			self->stop();
			return;
		}
		if (!LoginServer()) {
			ui::msgbox(gettext("Failed to login"), [this] {screen.Exit(); });
			screen.Loop(Renderer([] {return emptyElement(); }) | ui::EnableMessageBox());
			return;
		}
		
		Windows = ftxui::Container::Stacked({});

		/*Room info*/
		Windows->Add(RoomInfoPagesCom.MakeWindow(RoomChatPagesCom.show_content, RoomChatPagesCom.showRoomChatTaskBar));
		Taskbar->Add(MakeTaskBut(RoomInfoPagesCom.showRoomInfoPage, gettext("Room Info")));
		/*END*/

		/*Room Chat Page*/
		Windows->Add(RoomChatPagesCom.MakeWindow());
		Taskbar->Add(Maybe(MakeTaskBut(RoomChatPagesCom.show_content, gettext("Chat")), &RoomChatPagesCom.showRoomChatTaskBar));
		/*END*/

		/*Room Game Page*/
		Windows->Add(GamePageCom.MakeWindow());
		Taskbar->Add(MakeTaskBut(GamePageCom.showGameWindow, gettext("Game")));
		/*END*/

		int sizeleft = 20;
		Windows->Add(SideBarComp.MakeEditNameWindow());
		auto SideRight = ResizableSplitBottom(Taskbar | center, Windows, &taskbarsize);
		screen.Loop(ResizableSplitLeft(SideBarComp.MakeSideBarComp(), SideRight, &sizeleft) | ui::EnableMessageBox());

	};
}

//ftxui::Component MakeRoomInfoJoinButton(int room_id) {
		//	using namespace ftxui;
		//	return ftxui::Button(gettext("Join"), [this, room_id] {
		//		if (self->infostate == user::latest && self->hall.infostate == hall_info::latest) {
		//			if (self->in_room()) {
		//				ui::msgbox(gettext("You are already in a room."));
		//			}
		//			else {
		//				self->try_join_room(room_id);
		//			}
		//		}
		//		else {
		//			ui::msgbox(gettext("Please refresh the information and retry."));
		//		}
		//		}, ButtonOption::Animated(Color::Green, Color::White, Color::GreenLight, Color::White));
		//}
		//ftxui::Component MakeRoomInfoQuitButton() {
		//	using namespace ftxui;
		//	return ftxui::Button(gettext("Quit"), [this] {
		//		if (self->infostate == user::latest && self->hall.infostate == hall_info::latest) {
		//			self->try_leave();
		//		}
		//		else {
		//			ui::msgbox(gettext("Please refresh the information and retry."));
		//		}
		//		}, ButtonOption::Animated(Color::Red, Color::White, Color::RedLight, Color::White));
		//}
		//ftxui::Component MakeRoomInfoEntry(int room_id) {
		//	using namespace ftxui;
		//	auto but = (room_id != self->roomid ? MakeRoomInfoJoinButton(room_id) : MakeRoomInfoQuitButton()) | center;
		//	return Renderer(but, [but, room_id, this] {
		//		auto& rinfo = self->hall.rooms.at(room_id);
		//		return vbox(hbox(
		//			text(std::to_string(rinfo.id)) | center | size(WIDTH, GREATER_THAN, 4),
		//			separator(),
		//			text(rinfo.name) | center | size(WIDTH, GREATER_THAN, 8),
		//			separator(),
		//			text(std::to_string(rinfo.gamersize)) | center | size(WIDTH, GREATER_THAN, 5),
		//			separator(),
		//			text(state_str[rinfo.state].data()) | center | size(WIDTH, GREATER_THAN, 7),
		//			separator(),
		//			but->Render() | center | size(WIDTH, GREATER_THAN, 11)
		//		), separator());
		//		});
		//}
		//void RefreshRoomInfoEntries() {
		//	if (RoomInfoEntries->ChildCount())
		//		RoomInfoEntries->DetachAllChildren();
		//	for (int i = 1; i < self->hall.rooms.size(); ++i) {
		//		RoomInfoEntries->Add(MakeRoomInfoEntry(i));
		//	}
		//	RoomInfoEntries->TakeFocus();
		//}
		//ftxui::Component MakeWindow(bool& showRoomChatPage, bool& showRoomChatTaskBar) {
		//	using namespace ftxui;
		//	auto RoomInfosTitle = Renderer([] {
		//		auto style = ftxui::color(Color::BlueLight) | center;
		//		return hbox(
		//			text(gettext("ID")) | style | size(WIDTH, GREATER_THAN, 4),
		//			separator(),
		//			text(gettext("Name")) | style | size(WIDTH, GREATER_THAN, 8),
		//			separator(),
		//			text(gettext("Num")) | style | size(WIDTH, GREATER_THAN, 5),
		//			separator(),
		//			text(gettext("State")) | style | size(WIDTH, GREATER_THAN, 7),
		//			separator(),
		//			text(gettext("Operation")) | style | size(WIDTH, GREATER_THAN, 11)
		//		) | center;
		//		});
		//	RoomInfoEntries = ftxui::Container::Vertical({});
		//	RefreshRoomInfoEntries();
		//	auto RefreshRoomInfoBut = Button(gettext("Refresh"), [this] {
		//		self->get("room_info");
		//		self->infostate = user::outdated;
		//		self->hall.infostate = hall_info::outdated;
		//		RefreshRoomInfoEntries();
		//		}, ButtonOption::Animated(Color::Blue, Color::White, Color::BlueLight, Color::White)) | center;
		//	auto HideRoomInfoBut = Button(gettext("Hide"), [this] { showRoomInfoPage = false; }, ButtonOption::Animated(Color::GrayLight)) | center;
		//	Component RoomInfoInner = ftxui::Container::Vertical({
		//		Renderer([] {return text(gettext("Operations take effect after refreshing.")) | center; }),
		//		ftxui::Container::Horizontal({RefreshRoomInfoBut,HideRoomInfoBut}) | center,
		//		RoomInfosTitle | center | border,
		//		RoomInfoEntries /*| Renderer([](Element e) {return e; })*/ | vscroll_indicator | frame | flex | border | center
		//		}) | CatchEvent([this, &showRoomChatPage, &showRoomChatTaskBar](Event e) {
		//			if (e == Event::Special("RefreshRoomInfo")) {
		//				RefreshRoomInfoEntries();
		//				if (self->roomid <= 0) {
		//					showRoomChatPage = false;
		//					showRoomChatTaskBar = false;
		//				}
		//				else {
		//					showRoomChatPage = true;
		//					showRoomChatTaskBar = true;
		//				}
		//				return true;
		//			}
		//			else {
		//				return RoomInfoEntries->OnEvent(e);
		//			}
		//		});
		//	auto RoomInfoWindow = Window({
		//		.inner = RoomInfoInner,
		//		.title = gettext("Room Information"),
		//		.left = 25,
		//		.top = 2,
		//		.width = &RoomInfoWidth,
		//		.height = &RoomInfoHeight,
		//		.resize_left = false,
		//		.resize_right = false,
		//		.render = &ui::AlwaysActiveRenderState,
		//		});
		//	return Maybe(RoomInfoWindow, &showRoomInfoPage);
		//}