#pragma once
#include"othello.components.hpp"
namespace bw::online::simple {
	/*class GamePreparing {
		void GamePage() {
			using namespace ftxui;
			auto scr = ScreenInteractive::Fullscreen();
			Components  buttons = {
				Button(L"黑白棋",[&,this] {
					bw::othello::components::Game Game;
					if (Game.GamePreparing()) {
						Game.GamePageLocal();
					}
					},ButtonOption::Animated()) | center,
				Button(L"同化棋",[&,this] {

					},ButtonOption::Animated()) | center,
				Button(L"五子棋",[&,this] {

					},ButtonOption::Animated()) | center,
				Button(L" 围棋 ",[&,this] {

					},ButtonOption::Animated()) | center,
				Button(L" 退出 ",scr.ExitLoopClosure(),ButtonOption::Animated()) | center
			};
			auto vertial_buttons = vcon(buttons);

			auto HBox = Renderer(vertial_buttons, [&vertial_buttons] {return dbox(vertial_buttons->Render() | center); });
			scr.Loop(HBox);
			

		}
	};*/
}