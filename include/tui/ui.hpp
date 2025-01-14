#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"game.hpp"
#include"config.hpp"
#include"bwlocale.hpp"
#include"tui/components.hpp"
#include"tui/settings_page.hpp"
#include"tui/move_server_page.hpp"
#include"othello/components.hpp"
#include"online/components.hpp"
#include"tictactoe/components.hpp"
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
	class tui {
	public:
		tui() {};
		tui(int X, int Y) :x_width(X), y_height(Y) {}
		void start();
		//void HomePage();
		ftxui::Component LogoPage();
		ftxui::Component LocalGamePage();
		ftxui::Component OnlinePage();
		void MoveServerPage();
		void OnlineGameHallPage();
		ftxui::Component SettingsPage();
		ftxui::Component HelpPage();
		ftxui::Component AboutPage();
		ftxui::Component ExitPage();
		ftxui::Component InDeveloping() {
			return ui::TextComp(gettext("Developing.")) | ftxui::center;
		}
		int x_width = 80, y_height = 40;
	};
	void tui::start() {
		using namespace ftxui;

		auto scr = ScreenInteractive::Fullscreen();

		int selector = 0;
		std::vector<std::string> titles = {
			gettext("Home"),
			gettext("Local battles"),
			gettext("Online"),
			gettext("Settings"),
			gettext("Help"),
			gettext("More"),
			gettext("Quit")
		};
		auto option = MenuOption::HorizontalAnimated();
		option.underline.SetAnimation(std::chrono::milliseconds(500), animation::easing::ElasticOut);
		option.entries_option.transform = [](EntryState state) {
			Element e = text(state.label) | hcenter | flex;
			if (state.active && state.focused)
				e = e | bold;
			if (!state.focused && !state.active)
				e = e | dim;
			return e;
			};
		option.underline.color_inactive = Color::White;
		option.underline.color_active = Color::GreenLight;
		auto PagesMenu = Menu(titles, &selector, option);
		auto Tabs = Container::Tab({
			LogoPage(),
			LocalGamePage(),
			OnlinePage(),
			SettingsPage(),
			HelpPage(),
			AboutPage(),
			ExitPage()
		}, &selector);
		auto MainPage = Container::Vertical({
			PagesMenu,
			Tabs | flex | ui::EnableMessageBox()
			});
		scr.Loop(MainPage);
	}
	ftxui::Component tui::LogoPage() {
		using namespace ftxui;
		Canvas c(90, 90);
		c.DrawPointCircleFilled(22, 20, 20);
		c.DrawPointCircle(67, 20, 20);
		c.DrawPointCircle(22, 65, 20);
		c.DrawPointCircleFilled(67, 65, 20);

		auto document = canvas(std::move(c)) | borderDouble | size(HEIGHT, EQUAL, 25);
		auto GameIcon = Renderer([document] {return document; });
		return Container::Vertical({
			Renderer([] {return text(gettext("Black White")) | bold | borderRounded | hcenter; }),
			GameIcon | hcenter
			}) | center;
	}
	ftxui::Component tui::LocalGamePage() {
		using namespace ftxui;
		Components buttons = { 
			Button(gettext("Othello"),[&,this] {
				bw::othello::components::othello_Game_ptr Game = std::make_shared<bw::othello::components::Game>();
				if (Game->GameSetting()) {
					do {
						Game->GamePageLocal();
					} while (Game->another_round);
				}
				},ButtonOption::Animated()) | center,
			Button(gettext("TicTacToe"),[&,this] {
				bw::tictactoe::components::Game Game;
				if (Game.GameSetting()) {
					do {
						Game.GamePageLocal();
					} while (Game.another_round);
				}
				},ButtonOption::Animated()) | center,
			Button(gettext("Ataxx"),[&,this] { 
				
				},ButtonOption::Animated()) | center,
			Button(gettext("Gobang"),[&,this] { 
				
				},ButtonOption::Animated()) | center,
			Button(gettext(" GO "),[&,this] { 
			
				},ButtonOption::Animated()) | center,
		};
		auto vertial_buttons = vcon(buttons);
		return vertial_buttons | center | ui::EnableMessageBox();
	}
	ftxui::Component tui::OnlinePage() {
		using namespace ftxui;
		Components  buttons = {
			Button(censtr(gettext("Online Hall"),6),[&,this] {
				OnlineGameHallPage();
				},ButtonOption::Animated()) | center,
			Button(censtr(gettext("Run as move server"),6),[&,this] {
				MoveServerPage();
				},ButtonOption::Animated()) | center,
		};
		auto HBox = Container::Vertical(buttons);
		return HBox | ui::EnableMessageBox() | center;
	}
	void tui::MoveServerPage() {
		using namespace ftxui;
		ScreenInteractive screen = ScreenInteractive::Fullscreen();
		auto cmp = bw::components::MoveServerComp();
		screen.Loop(cmp | center | ui::EnableMessageBox());
	}
	void tui::OnlineGameHallPage() {
		using namespace ftxui;
		bw::online::components::HallPages Pages;
		Pages.startHallPages();
	}
	ftxui::Component tui::SettingsPage() {
		using namespace ftxui;
		return bw::components::SettingsPage();
	}
	ftxui::Component tui::HelpPage() {
		using namespace ftxui;
		ScreenInteractive screen = ScreenInteractive::Fullscreen();
		Component HelpContent = Renderer([] {
			return vbox(
				hbox(text(gettext("GitHub url of this project :")), hyperlink(R"(https://github.com/Chessom/Black-White)", text(gettext("Click to open")) | blink) | underlined), 
				paragraphAlignJustify(
					gettext("This is a TUI(CUI) game set program, you can interact with the program using a mouse.")
				)

			);
		});
		return HelpContent;
	}
	ftxui::Component tui::AboutPage() {
		using namespace ftxui;
		return InDeveloping();
	}
	ftxui::Component tui::ExitPage() {
		using namespace ftxui;
		return Button(gettext("Quit"), [] {ftxui::ScreenInteractive::Active()->Exit(); }, ButtonOption::Animated(ftxui::Color::Red)) | center;
	}
}
/*void tui::HomePage() {
		using namespace ftxui;
		auto scr = ScreenInteractive::Fullscreen();
		Components buttons;
		auto header = Renderer([] {return text(gettext("Black White")) | center; });
		buttons = {
			Button(censtr(gettext("Local battles"), 6),[&,this]() {LocalGamePage(); },ButtonOption::Animated()) | center,
			Button(censtr(gettext("Online"), 8), [&, this]() {OnlinePage(); }, ButtonOption::Animated()) | center,
			Button(censtr(gettext("Settings"), 8), [&, this]() {SettingsPage(); }, ButtonOption::Animated()) | center,
			Button(censtr(gettext("Help"), 8), [&, this]() {HelpPage(); }, ButtonOption::Animated()) | center,
			Button(censtr(gettext("More"), 8), [&, this]() {AboutPage(); }, ButtonOption::Animated()) | center,
			Button(censtr(gettext("Quit"), 8), scr.ExitLoopClosure(), ButtonOption::Animated()) | center
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
		auto GameIcon = Renderer([document] {return document; });
		auto ScreenSize = Renderer([&scr] {return text(std::format("X:{} Y:{}", scr.dimx(), scr.dimy())); });
		auto Page = hcon({ GameIcon,HBox });

		scr.Loop(Page | center | ui::EnableMessageBox());
	}*/