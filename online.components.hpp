#pragma once
#include"stdafx.h"
#include"components.h"
#include"online.gamer.hpp"
namespace bw::online::components {
	//generate a scrollable chat window
	class ChatMsgComBase : public ftxui::ComponentBase {
	public:
		ChatMsgComBase(gamer& gmr) :g(gmr), selector_(std::max(gmr.chat_msg_queue.size() - 1, 0ull)) {}
		ftxui::Component MakeAvatarButton(const str_msg& msg) {
			using namespace ftxui;
			return Button(msg.name, [&, this] {
				selector_ = msg.id1;
				}, ButtonOption::Animated(Color::Orange1, Color::White, Color::Orange4, Color::White));
		}
		virtual ftxui::Element Render() override {
			using namespace ftxui;
			Elements es;
			for (int i = 0; i < g.chat_msg_queue.size(); ++i) {
				auto& msg = g.chat_msg_queue[i];
				auto but = MakeAvatarButton(msg);
				if (msg.id1 == g.id) {
					es.push_back(hbox(ui::autolines(msg.content) | borderRounded, but->Render()) | align_right);
				}
				else {
					es.push_back(hbox(ui::autolines(msg.content) | borderRounded, but->Render()));
				}
			}
			return vbox(es) | ftxui::vscroll_indicator;
		}
		virtual bool OnEvent(ftxui::Event e) override {
			if (e == ftxui::Event::ArrowUp) {
				selector_ = std::max(selector_ - 1, 0);
				return true;
			}
			else if (e == ftxui::Event::ArrowDown) {
				selector_ = std::min(selector_ + 1, int(g.chat_msg_queue.size() - 1));
				return true;
			}
			return false;
		}
		virtual bool Focusable() const override { return true; }
	private:
		int selector_;
		gamer& g;
	};
	
	/*class ChatMsgComBase :ftxui::ComponentBase {
	public:
		ChatMsgComBase(gamer& gmr) :g(gmr), selector_(std::max(gmr.chat_msg_queue.size() - 1, 0ull)) {}
		virtual ftxui::Element Render() override {

		}
		virtual bool OnEvent(ftxui::Event e) override {

		}
		virtual bool Focusable() const override { return true; }
	private:
		int selector_;
		gamer& g;
	};*/
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
			auto but = room_id != self.roomid ? MakeRoomInfoJoinButton(room_id) | center : MakeRoomInfoQuitButton() | center;
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
				}, ButtonOption::Animated(Color::Orange1, Color::White, Color::Orange4, Color::White));
		}
		ftxui::Component MakeChatMsg(const str_msg& msg) {
			using namespace ftxui;
			auto but = MakeAvatarButton(msg);
			auto temp = Renderer([this, but, msg] {
				if (msg.id1 == self.id) {
					return hbox(ui::autolines(msg.content) | borderRounded, but->Render()) | align_right;
				}
				else {
					return hbox(ui::autolines(msg.content) | borderRounded, but->Render());
				}
			});
			temp->Add(ui::Focusable());
			return temp;
			//return Button(msg.content, [] {});
		}
		void RefreshRoomChatMsgs() {
			if (RoomChatMsgs->ChildCount())
				RoomChatMsgs->DetachAllChildren();
			for (int i = 0; i < self.chat_msg_queue.size() - 1; ++i) {
				RoomChatMsgs->Add(MakeChatMsg(self.chat_msg_queue[i]));
			}
			RoomChatMsgs->Add(MakeChatMsg(self.chat_msg_queue.back()) | ftxui::focus);
			RoomChatMsgs->TakeFocus();
		}
		void AddRoomChatMsg() {
			using namespace ftxui;
			auto msg = self.chat_msg_queue.back();
			RoomChatMsgs->Add(MakeChatMsg(self.chat_msg_queue.back()) | ftxui::focus);
			RoomChatMsgs->TakeFocus();
		}
		ftxui::Component MakeStartGameButtons() {
			using namespace ftxui;
			return ftxui::Container::Vertical({
				Button("Refresh",[this] {RefreshRoomChatMsgs(); },ButtonOption::Animated(Color::Blue,Color::White,Color::BlueLight,Color::White)),
				Button("Othello",[] {},ButtonOption::Animated()),
				Button("TicTacToe",[] {},ButtonOption::Animated())
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
				auto CancelSubmitName = Button("Cancel"_l, [&]{
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
					screen.PostEvent(Event::Custom);
					}, ButtonOption::Animated(Color::GrayLight)) | center;
				/*END*/
				
				/*Bulletin*/
				auto Bulletin = Renderer([this] {
					Elements es;
					for (auto& s : self.notices) {
						es.push_back(paragraphAlignJustify(s));
					}
					return window(text("Notices"_l) | center, vbox(es) | vscroll_indicator | yframe | flex );
					});
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

				/*Room Page*/
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
					RoomChatMsgs->TakeFocus();
					}, ButtonOption::Animated(Color::Orange1));
				auto SendChatCom = ftxui::Container::Vertical({ 
					InputChatComp,
					Renderer(SendChatBut, [&] {
						return hbox(text("Press enter to line feed.") | center, SendChatBut->Render() | center);
					})
					});
				int ChatRightSize = 16;
				int SendChatSize = 10;
				////////////////////////////////////////////////////
				/*auto RoomPageLeft = ResizableSplitBottom(SendChatCom | size(HEIGHT, GREATER_THAN, 3), 
					RoomChatMsgs | vscroll_indicator | frame | flex | size(HEIGHT, GREATER_THAN, 15), &ChatLeftSize);*/
				////////////////////////////////////////////////////
				auto RoomPageLeft = RoomChatMsgs | vscroll_indicator | frame | size(HEIGHT, GREATER_THAN, 15);
				auto RoomPageRight = ResizableSplitBottom(SendChatCom, MakeStartGameButtons(), &SendChatSize);
				RoomChatPageCom = ResizableSplitRight(RoomPageRight, RoomPageLeft, &ChatRightSize)| CatchEvent([this](Event e) {
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
				Taskbar->Add(Maybe(MakeTaskBut(showRoomChatPage, "Chat"), &showRoomChatTaskBar));
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