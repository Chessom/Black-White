#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"game.hpp"
#include"config.hpp"
#include"bwlocale.hpp"
#include"components.h"
#include"othello.components.hpp"
#include"online.components.hpp"
#include"tictactoe.components.hpp"
namespace bw {
	inline std::string censtr(const std::string& str, int width) {
		return std::format("{0:^{1}}", str, width);
	}
	auto hcon(ftxui::Components children) {
		return ftxui::Container::Horizontal(children);
	}
	auto vcon(ftxui::Components children) {
		return ftxui::Container::Vertical(children);
	}
	class cui {
	public:
		cui() {};
		cui(int X, int Y) :x_width(X), y_height(Y) {}
		void HomePage();
		void LocalGamePage();
		void OnlinePage();
		void SimpleOnlinePage();
		void OnlineGameHallPage();
		void SettingsPage();
		void HelpPage();
		void AboutPage();
		int x_width = 80, y_height = 40;
		std::vector<std::wstring> home_page_button_str{
			L"本地对战",L"联机",L"设置",L"帮助",L"关于",L"退出"
		};
	};
	void cui::HomePage() {
		using namespace ftxui;
		
		/*std::shared_ptr<ScreenInteractive> scr = Make<ScreenInteractive>();
		current_screen = scr;*/
		auto scr = ScreenInteractive::Fullscreen();
		Components buttons;
		auto header = Renderer([] {return text(L"黑白棋") | center; });
		buttons = {
			Button(censtr("本地对战", 6),[&,this]() {LocalGamePage(); },ButtonOption::Animated()) | center,
			Button(censtr("联机", 8), [&, this]() {OnlinePage(); }, ButtonOption::Animated()) | center,
			Button(censtr("设置", 8), [&, this]() {SettingsPage(); }, ButtonOption::Animated()) | center,
			Button(censtr("帮助", 8), [&, this]() {HelpPage(); }, ButtonOption::Animated()) | center,
			Button(censtr("关于", 8), [&, this]() {AboutPage(); }, ButtonOption::Animated()) | center,
			Button(censtr("退出", 8), scr.ExitLoopClosure(), ButtonOption::Animated()) | center
		};
		Components com;
		com.push_back(header);
		com.append_range(buttons);
		auto HBox = Container::Vertical(com) | center;

		Canvas c(90, 90);
		c.DrawPointCircleFilled(22, 20, 20);
		c.DrawPointCircle(67, 20, 20);
		c.DrawPointCircle(22, 65, 20);
		c.DrawPointCircleFilled(67, 65, 20);

		auto document = canvas(std::move(c)) | borderDouble | size(HEIGHT, EQUAL, 25);
		auto GameIcon = Renderer([&document] {return document; });
		auto ScreenSize = Renderer([&scr] {return text(std::format("X:{} Y:{}", scr.dimx(), scr.dimy())); });
		auto Page = hcon({ GameIcon,HBox });

		scr.Loop(Page | center | ui::EnableMessageBox());
	}
	void cui::LocalGamePage() {
		using namespace ftxui;
		ScreenInteractive scr = ScreenInteractive::Fullscreen();
		Components  buttons = { 
			Button("黑白棋",[&,this] {
				bw::othello::components::Game Game;
				if (Game.GamePreparing()) {
					Game.GamePageLocal();
				}
				},ButtonOption::Animated()) | center,
			Button("井字棋",[&,this] {
				bw::tictactoe::components::Game Game;
				if (Game.GamePreparing()) {
					Game.GamePageLocal();
				}
				},ButtonOption::Animated()) | center,
			Button("同化棋",[&,this] { 
				
				},ButtonOption::Animated()) | center,
			Button("五子棋",[&,this] { 
				
				},ButtonOption::Animated()) | center,
			Button(" 围棋 ",[&,this] { 
			
				},ButtonOption::Animated()) | center,

			/*Button(" Premsg ",[&,this] {
				ui::premsgbox();
				},ButtonOption::Animated()) | center,*/
			Button(" 退出 ",scr.ExitLoopClosure(),ButtonOption::Animated()) | center
		};
		auto vertial_buttons = vcon(buttons);
		
		scr.Loop(vertial_buttons | center | ui::EnableMessageBox());
		/*
		* show the game types:Othello,Ataxx,Gobang......
		* if the button clicked, then call the function to load another page.
		*
		if othello selected:
			1.show the button of humanVhuman,humanVcomputer,computerVcomputer...these default game settings
			if the default setting is selected
				then show the default difficulty and the first, second hand
			2.show the button of self-design game.
			if the self-design selected
				show the dropdown of gamer types,the first and the second hand gamer type.
				(computer gamer maybe diverse)
			3.then the game settings are ready,so we enter an new game page(a new function,defined by the othello.components.h)
			调用othello.components中的Game(settings)
				settings有先手、后手的信息，包括类型（人类、计算机，哪种算法，)
		if ataxx selected:
			......(alike)
		*/

	}
	void cui::OnlinePage() {
		using namespace ftxui;
		ScreenInteractive scr = ScreenInteractive::Fullscreen();
		Components  buttons = {
			Button(censtr("联机大厅",6),[&,this] {
				OnlineGameHallPage();
				},ButtonOption::Animated()) | center,
			Button(censtr("简易联机",6),[&,this] {
				ui::msgbox("These are in developing!");
				SimpleOnlinePage();
				},ButtonOption::Animated()) | center,
			Button(censtr("退出",8),scr.ExitLoopClosure(),ButtonOption::Animated()) | center
		};
		auto HBox = Container::Vertical(buttons);
		scr.Loop(HBox | ui::EnableMessageBox() | center);
	}
	void cui::SimpleOnlinePage() {
		using namespace ftxui;
		ui::msgbox("Developing.");
	}
	void cui::OnlineGameHallPage() {
		using namespace ftxui;
		bw::online::components::HallPages Pages;
		Pages.startHallPages();
	}
	void cui::SettingsPage() {
		using namespace ftxui;
		ui::msgbox("Developing.");
	}
	void cui::HelpPage() {
		using namespace ftxui;
		ui::msgbox("Developing.");
	}
	void cui::AboutPage() {
		using namespace ftxui;
		ui::msgbox("Developing.");
	}
}
