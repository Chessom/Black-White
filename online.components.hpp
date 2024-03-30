#pragma once
#include"stdafx.h"
#include"components.h"
#include"online.gamer.hpp"
namespace bw::online::components {
	class HallPages {
	public:
		ftxui::Component Taskbar = ftxui::Container::Horizontal({});
		int taskbarsize = 3;

		ftxui::Component SideCom, Windows, RoomInfoEntries;
		bool showRoomInfoPage = true;

		ftxui::Component RoomChatPageCom, RoomChatMsgs = ftxui::Container::Vertical({});
		bool showRoomChatPage = false;
		bool showRoomChatTaskBar = false;
		int selectedPlayerID = 0;

		ftxui::Component GamePageCom;
		bool showRoomGamePage = false;
		bool showRoomGameTaskBar = false;

		gamer self;
		ftxui::Component MakeRoomInfoJoinButton(int room_id) {
			using namespace ftxui;
			return ftxui::Button("Join", [this, room_id] {
				if (self.infostate == gamer::updated && self.hall.infostate == hall_info::updated) {
					if (self.roomid != 0) {
						ui::msgbox("You are already in a room.");
					}
					else {
						self.try_join_room(room_id);
					}
				}
				else {
					ui::msgbox("Please refresh the information and retry.");
				}
				}, ButtonOption::Animated(Color::Green, Color::White, Color::GreenLight, Color::White));
		}
		ftxui::Component MakeRoomInfoQuitButton() {
			using namespace ftxui;
			return ftxui::Button("Quit", [this] {
				if (self.infostate == gamer::updated && self.hall.infostate == hall_info::updated) {
					self.try_leave();
				}
				else {
					ui::msgbox("Please refresh the information and retry.");
				}
				}, ButtonOption::Animated(Color::Red, Color::White, Color::RedLight, Color::White));
		}
		ftxui::Component MakeRoomInfoEntry(int room_id) {
			using namespace ftxui;
			auto but = (room_id != self.roomid ? MakeRoomInfoJoinButton(room_id) : MakeRoomInfoQuitButton()) | center;
			return Renderer(but, [but, room_id, this] {
				auto& rinfo = self.hall.rooms[room_id];
				return vbox(hbox(
					text(std::to_string(rinfo.id)) | center | size(WIDTH, GREATER_THAN, 4),
					separator(),
					text(rinfo.name) | center | size(WIDTH, GREATER_THAN, 8),
					separator(),
					text(std::to_string(rinfo.gamersize)) | center | size(WIDTH, GREATER_THAN, 5),
					separator(),
					text(state_str[rinfo.state].data()) | center | size(WIDTH, GREATER_THAN, 7),
					separator(),
					but->Render() | center | size(WIDTH, GREATER_THAN, 11)
				), separator());
				});
		}
		void RefreshRoomInfoEntries() {
			if (RoomInfoEntries->ChildCount())
				RoomInfoEntries->DetachAllChildren();
			for (int i = 1; i < self.hall.rooms.size(); ++i) {
				RoomInfoEntries->Add(MakeRoomInfoEntry(i));
			}
			RoomInfoEntries->TakeFocus();
		}
		ftxui::Component MakeTaskBut(bool& showflag, std::string button_name) {
			using namespace ftxui;
			return Button(button_name, [&showflag] {
				showflag = showflag ? false : true;
				}, ButtonOption::Animated());
		}




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
				if (msg.id1 == self.id) {
					return hbox(ui::autolines(msg.content) | borderRounded, but->Render()) | align_right;
				}
				else {
					return hbox(but->Render(), ui::autolines(msg.content) | borderRounded);
				}
				});
			//temp->Add(ui::Focusable());
			return temp | vcenter;
			//return Button(msg.content, [] {});
		}
		void RefreshRoomChatMsgs() {
			if (RoomChatMsgs->ChildCount())
				RoomChatMsgs->DetachAllChildren();
			self.chat_mutex.acquire();
			auto msgq = self.chat_msg_queue;
			self.chat_mutex.release();
			for (int i = 0; i < msgq.size() - 1; ++i) {
				RoomChatMsgs->Add(MakeChatMsg(msgq[i]));
			}
			RoomChatMsgs->Add(MakeChatMsg(msgq.back())/* | ftxui::focus*/);
			RoomChatMsgs->TakeFocus();
		}
		void AddRoomChatMsg() {
			using namespace ftxui;
			self.chat_mutex.acquire();
			auto msg = self.chat_msg_queue.back();
			self.chat_mutex.release();
			RoomChatMsgs->Add(MakeChatMsg(msg)/* | ftxui::focus*/);
			RoomChatMsgs->TakeFocus();
		}
		ftxui::Component MakeStartGameButtons() {
			using namespace ftxui;
			return ftxui::Container::Vertical({
				Button("Refresh",[this] {RefreshRoomChatMsgs(); },ButtonOption::Animated(Color::Blue,Color::White,Color::BlueLight,Color::White)),
				Button("Othello",[this] {
					using namespace othello;
					move mv{.mvtype=move::preparaed,.c=col0};
					//gm_ptr=std::make_shared<othello::game>();
					/*
					* 1. add the component to the window
					* 2. spawn the task coroutine to run in the executor
					* ----Q:how to make the channel connected to the user?
					* ----A:we register the channel in the handle_msg function of the user?
					* ----we only forward the raw move string(json format) to the gamer, 
					* ----and the gamer will handle the move string on their own.(or perhaps we can use the std::any?)
					* 
					* 
					* 3.
					* the false code:
					* GamePageCom = ftxui::Container::Vertical({});
					* GamePageCom->DetachAllChildren();
					* GamePageCom->Add(game_type::components::GamePageComp());
					* 
					*/
					},ButtonOption::Animated()),
				Button("TicTacToe",[this] {
					using namespace tictactoe;
					move mv{ .mvtype = move::preparaed,.col = col0 };
					
					},ButtonOption::Animated())
				});
		}
		
		



		void startHallPages(){
			using namespace ftxui;
			{//连接服务器
				auto screen = ftxui::ScreenInteractive::Fullscreen();
				std::string PortString = "22222";
				Component enter = ftxui::Container::Vertical({
					ftxui::Container::Horizontal({
					Renderer([] {
						return vbox(text("Address:"_l) | border,text("Port:"_l) | border);
						}),
					ftxui::Container::Vertical({
						Input(&self.hall.address, "IP or Domain Name"_l) | size(WIDTH,GREATER_THAN,16) | border,
						Input(&PortString, "22222") | size(WIDTH,GREATER_THAN,16) | border
						}),
					}) | center,
					ftxui::Container::Horizontal({
					Button("Connect"_l,[&] {
						if (self.connect(self.hall.address, std::stoi(PortString))) {
							screen.Exit();
							self.hall.port = std::stoi(PortString);
						}
						},ButtonOption::Animated(Color::Green,Color::White,Color::GreenLight,Color::White)),
					Button("Quit"_l,[&] {
						screen.Exit();
						},ButtonOption::Animated(Color::Red,Color::White,Color::RedLight,Color::White)),
					}) | center,
					}) | ui::EnableMessageBox();
				screen.Loop(enter | center);
			}
			if (self.connected()) {
				self.start();
			}
			else {
				return;
			}
			if (global_config->first_login) {
				std::string loginName = global_config->default_name;
				bool setDefault = true;
				auto screen = ftxui::ScreenInteractive::Fullscreen();
				auto inputCom = Input(&loginName, "Enter your name to login"_l, InputOption::Spacious()) | border | center;
				auto loginCom = ftxui::Container::Vertical({
					Renderer(inputCom,[&inputCom] {return hbox(text("You name:"_l) | center,inputCom->Render() | center); }) | center,
					Checkbox("Set this name as default"_l,&setDefault) | center,
					Button("Login"_l,screen.ExitLoopClosure(),ButtonOption::Animated(Color::Green,Color::White,Color::Green4,Color::White)) | center
				}) | ui::EnableMessageBox();
				screen.Loop(loginCom | center);
				if (setDefault) {
					global_config->first_login = false;
					global_config->default_name = loginName;
					global_config->dump_binary_file("config.bin");
				}
				self.name = loginName;
			}
			{
				self.name = global_config->default_name;
				self.login();
			}
			{
				auto screen = ftxui::ScreenInteractive::Fullscreen();
				self.screen_ptr = &screen;
				Windows = ftxui::Container::Stacked({});
				int sizeleft = 20;

				Component QuitServerBut = Button("Quit", [&, this] {self.stop(); screen.Exit(); }, ButtonOption::Animated(Color::Red));

				/*Name show and edit*/
				std::string NameBuf = self.name;
				bool showEditNameWindow = false;
				auto ShowName = Renderer([&] {return text(std::format("Name :{}", self.name)); }) | center | borderRounded;
				auto ShowRoom = Renderer([&] {return text(std::format("Room ID:{}", self.roomid > 0 ? std::to_string(self.roomid) : "None")); }) | center | borderRounded;
				auto SubmitNameBut = Button("Submit"_l, [&] {
					self.try_update_name(NameBuf.data());
					showEditNameWindow = false;
					screen.PostEvent(Event::Custom);
					}, ButtonOption::Animated(Color::Green, Color::White, Color::GreenLight, Color::White));
				auto CancelSubmitName = Button("Cancel"_l, [&] {
					showEditNameWindow = false;
					screen.PostEvent(Event::Custom);
					}, ButtonOption::Animated(Color::Red, Color::White, Color::RedLight, Color::White));
				auto EditNameInput = Input(&NameBuf, "Enter your name"_l, InputOption::Spacious());
				auto EditName = Renderer(EditNameInput, [&] {return hbox(text("Name: "_l) | center, EditNameInput->Render() | center) | center; });
				auto EditNameInner = ftxui::Container::Vertical({
					EditName,
					ftxui::Container::Horizontal({SubmitNameBut,CancelSubmitName}) | center,
					Renderer([] {return text("Changes will take effect later"_l) | center; })
					});
				auto EditNameWindow = Window({
					.inner = EditNameInner,
					.title = "Edit Name"_l,
					.left = 25,
					.top = 6,
					.width = 35,
					.height = 10,
					.resize_left = true,
					.resize_right = true,
					.resize_top = false,
					.resize_down = false,
					});
				Windows->Add(Maybe(EditNameWindow, &showEditNameWindow));
				auto EditBut = Button("Edit Name"_l, [&] {
					showEditNameWindow = true;
					EditNameWindow->TakeFocus();
					screen.PostEvent(Event::Custom);
					}, ButtonOption::Animated(Color::GrayLight)) | center;
				/*END*/

				/*Bulletin*/
				auto RefreshNoticesBut = Button("Refresh Notices", [this] {self.get("notices"); }, ButtonOption::Animated(Color::Blue)) | center;
				auto BulletinComp = Renderer([this] {
					Elements es;
					for (auto& s : self.notices) {
						es.push_back(paragraphAlignJustify(s));
					}
					return window(text("Notices"_l) | center, vbox(es) | vscroll_indicator | yframe | flex);
					});
				auto Bulletin = Container::Vertical({ RefreshNoticesBut ,BulletinComp });
				/*END*/

				/*Room info*/
				int RoomInfoHeight = 38;
				int RoomInfoWidth = 44;
				auto RoomInfosTitle = Renderer([] {
					auto style = ftxui::color(Color::BlueLight) | center;
					return hbox(
						text("ID") | style | size(WIDTH, GREATER_THAN, 4),
						separator(),
						text("Name") | style | size(WIDTH, GREATER_THAN, 8),
						separator(),
						text("Num") | style | size(WIDTH, GREATER_THAN, 5),
						separator(),
						text("State") | style | size(WIDTH, GREATER_THAN, 7),
						separator(),
						text("Operation") | style | size(WIDTH, GREATER_THAN, 11)
					) | center;
					});
				RoomInfoEntries = ftxui::Container::Vertical({});
				RefreshRoomInfoEntries();
				auto RefreshRoomInfoBut = Button("Refresh", [this] {
					self.get("room_info");
					self.infostate = gamer::outdated;
					self.hall.infostate = hall_info::outdated;
					RefreshRoomInfoEntries();
					}, ButtonOption::Animated(Color::Blue, Color::White, Color::BlueLight, Color::White)) | center;
				auto HideRoomInfoBut = Button("Hide", [this, &screen] { showRoomInfoPage = false; }, ButtonOption::Animated(Color::GrayLight)) | center;
				Component RoomInfoInner = ftxui::Container::Vertical({
					Renderer([&] {return text("Operations take effect after refreshing.") | center; }),
					ftxui::Container::Horizontal({RefreshRoomInfoBut,HideRoomInfoBut}) | center,
					RoomInfosTitle | center | border,
					RoomInfoEntries | vscroll_indicator | frame | flex | border | center
					}) | CatchEvent([this](Event e) {
						if (e == Event::Special("RefreshRoomInfo")) {
							RefreshRoomInfoEntries();
							if (self.roomid <= 0) {
								showRoomChatPage = false;
								showRoomChatTaskBar = false;
							}
							else {
								showRoomChatPage = true;
								showRoomChatTaskBar = true;
							}
							return true;
						}
						return false;
						});
				auto RoomInfoWindow = Window({
					.inner = RoomInfoInner,
					.title = "Room Information"_l,
					.left = 25,
					.top = 2,
					.width = &RoomInfoWidth,
					.height = &RoomInfoHeight,
					.resize_left = false,
					.resize_right = false,
					});
				Windows->Add(Maybe(RoomInfoWindow, &showRoomInfoPage));
				Taskbar->Add(Button("Rooms", [&] {
					showRoomInfoPage = showRoomInfoPage ? false : true;
					//screen.PostEvent(Event::Custom);
					}, ButtonOption::Animated()));
				/*END*/

				/*Room Chat Page*/
				std::string chatmsg;
				auto InputChatComp = Input(&chatmsg, "Enter your message here"_l, InputOption::Spacious());
				auto SendChatBut = Button("Send"_l, [this, &chatmsg] {
					if (self.roomid > 0) {
						self.broadcast(chatmsg);
					}
					else {
						showRoomChatPage = false;
						showRoomChatTaskBar = false;
					}
					}, ButtonOption::Animated(Color::Orange1));
				auto SendChatCom = ftxui::Container::Vertical({ 
					InputChatComp,
					Renderer(SendChatBut, [&] {
						return hbox(text("Press enter to line feed."_l) | center, SendChatBut->Render() | center);
					})
					});
				int SendChatSize = 7;
				RoomChatPageCom = ResizableSplitBottom(SendChatCom, RoomChatMsgs | vscroll_indicator | frame | flex, &SendChatSize) | CatchEvent([this](Event e) {
					if (e == Event::Special("AddChat"_l)) {
						AddRoomChatMsg();
						return true;
					}
					else if (e == Event::Special("RefreshChat"_l)) {
						RefreshRoomChatMsgs();
						return true;
					}
					return false;
					});
				auto RoomChatWindow = Window({
					.inner = RoomChatPageCom,
					.title = "Chat"_l,
					.left = 71,
					.top = 4,
					.width = 80,
					.height = 30,
					});
				Windows->Add(Maybe(RoomChatWindow, &showRoomChatPage));
				Taskbar->Add(Maybe(MakeTaskBut(showRoomChatPage, "Chat"_l), &showRoomChatTaskBar));
				/*END*/

				/*Room Game Page*/

				/*END*/
				
				SideCom = ftxui::Container::Vertical({
					ShowName,
					Renderer([&] {return text(std::format("ID :{}",self.id)); }) | center | borderRounded,
					ftxui::Container::Horizontal({EditBut, QuitServerBut}) | center,
					Bulletin | center
					});
				auto SideRight = ResizableSplitBottom(Taskbar | center, Windows, &taskbarsize);
				screen.Loop(ResizableSplitLeft(SideCom, SideRight, &sizeleft) | ui::EnableMessageBox());
			}
		};
	
	};
}