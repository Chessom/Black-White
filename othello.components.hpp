#pragma once
#include"stdafx.h"
#include"othello.game.hpp"
#include"othello.gamer.computer.hpp"
#include"othello.gamer.human.hpp"
#include"othello.gamer.remote.hpp"
#include"othello.globals.hpp"
#include<ftxui/dom/elements.hpp>
#include<ftxui/component/component.hpp>
#include<ftxui/component/screen_interactive.hpp>
#include<ftxui/screen/box.hpp>
namespace bw::othello::components {
	using namespace ftxui;
	/*class  PieceImpl :public ComponentBase {
	public:
		PieceImpl(const coord& Coord, dynamic_brd& board) :crd(Coord), brd(board) {};
		Element Render() override{
			const bool active = Active();
			const bool focused = Focused();
			const bool focused_or_hover = focused || mouse_hover_;

		}
	private:
		dynamic_brd& brd;
		bool mouse_hover_ = false;
		Box box_;
		coord crd;
	};
	
	Component Piece(int x, int y, dynamic_brd& brd) {
		return Make<PieceImpl>(coord{ x,y });
	}
	Component Cell(int x, int y, dynamic_brd) {
		return Renderer([]()->Element {

			});
	}*/
	class BoardBase : public ComponentBase {
	public:
		BoardBase(){
			col2str[col0] = L"●";
			col2str[col1] = L"○";
			col2str[none] = L"  ";
			brd.initialize();
		}
		BoardBase(int board_size) :brd(board_size) {
			col2str[col0] = L"●";
			col2str[col1] = L"○";
			col2str[none] = L"  ";
		}
		BoardBase(dynamic_brd& Board) :brd(Board) {
			col2str[col0] = L"●";
			col2str[col1] = L"○";
			col2str[none] = L"  ";
		}
		Element Render() override {
			Elements rows;
			color col;
			std::wstring firstr;
			firstr = L"╔";
			for (int i = 1; i < brd.size; i++)
				firstr += L"═══╦";
			firstr += L"═══╗ ";
			rows.push_back(text(firstr) | ftxui::color(Color::Black) | bgcolor(Color::Green));
			std::wstring rowstr1, rowstr2;
			Elements row;

			rowstr1 = L"╠";
			for (int j = 0; j < brd.size - 1; j++)
				rowstr1 += L"═══╬";
			rowstr1 += L"═══╣ ";

			rowstr2= L"╚";
			for (int j = 0; j < brd.size - 1; j++)
				rowstr2 += L"═══╩";
			rowstr2 += L"═══╝ ";

			for (int x = 0; x < brd.size; ++x) {
				row.clear();
				row.emplace_back(text(L"║") | ftxui::color(Color::Black) | bgcolor(Color::Green));

				for (int  y = 0; y < brd.size; ++y) {
					col = brd.getcol({ x,y });
					if (col == core::none) {
						row.emplace_back(text(game_ptr->valid_move(game_ptr->current_color(), { x,y }) ? L" • " : L"   ") | ftxui::color(Color::Red) | bgcolor(Color::Green));
					}
					else if (col==core::col0) {
						row.emplace_back(text(L" ●") | ftxui::color(Color::Black) | bgcolor(Color::Green));
					}
					else {
						row.emplace_back(text(L" ●") | ftxui::color(Color::White) | bgcolor(Color::Green));
					}
					row.emplace_back(text(L"║") | ftxui::color(Color::Black) | bgcolor(Color::Green));
				}
				row.emplace_back(text(" ") | bgcolor(Color::Green));
				rows.emplace_back(hbox(row));
				if (x != brd.size - 1) {
					rows.emplace_back(text(rowstr1) | ftxui::color(Color::Black) | bgcolor(Color::Green));
				}
				else {
					rows.emplace_back(text(rowstr2) | ftxui::color(Color::Black) | bgcolor(Color::Green));
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
		bool OnEvent(Event e) override {
			if (e.is_mouse()) {
				return OnMouseEvent(e);
			}
			if (e == Event::Special("flush")) {
				brd = game_ptr->current_board();
				dqptr->conv.notify_one();
				return true;
			}
			if (e == Event::Special("End")) {
				int points0 = brd.countpiece(col0), points1 = brd.countpiece(col1);
				if (points0 > points1) {
					msgbox("Black win!");
				}
				else if (points0 < points1) {
					msgbox("White win!");
				}
				else {
					msgbox("Draw!");
				}
				return true; 
			}
			if (e == Event::Special("Regret")) {
				dqptr->q.push_back({ -1,-2 });
				dqptr->conv.notify_one();
				return true;
			}
			if (e == Event::Special("EndLoop")) {
				dqptr->q.push_back({ -1,-1 });
				dqptr->conv.notify_one();
				return true;
			}
			if (e == Event::Special("Suspend")) {
				/*dqptr->q.push_back({ -1,-3 });
				dqptr->conv.notify_one();*/
				return true;
			}
			if (e == Event::Special("Save")) {

				return true;
			}
			return false;
		}
		bool OnMouseEvent(Event e) {
			if (box.Contain(e.mouse().x, e.mouse().y) && CaptureMouse(e)) {
				int dx = e.mouse().x - box.x_min;
				int dy = e.mouse().y - box.y_min;
				if (e.mouse().button == Mouse::Left &&
					e.mouse().motion == Mouse::Pressed) {
					TakeFocus();
					dqptr->q.push_back({ dy / 2,dx / 4 });
					dqptr->conv.notify_one();
					//做出move反应
					//MessageBoxA(NULL, fmt::format("click at x:{} y:{}", dx/4, dy/2).c_str(), "", 0);
					return true;
				}
			}
			return false;
		}
		
		bool Focusable() const override { return true; }
		std::map<int, std::wstring> col2str;
		mtxdq_ptr dqptr = nullptr;
		game* game_ptr = nullptr;
		dynamic_brd brd;
	private:
		bool mouse_hover = false;
		Box box;
	};
	using Board = std::shared_ptr<BoardBase>;
	class Game {
	public:
		inline std::wstring wcenter(const std::wstring& str, int width) {
			return fmt::format(L"{0:^{1}}", str, width);
		}
		bool GamePreparing() {
			using namespace ftxui;
			bool ret = true;
			ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
			int selected_2 = 0, selected_1 = 0, selected_3 = 0;


			Component layout = Container::Vertical({
				Container::Horizontal({Renderer([] {return ftxui::text("First Player:"); }) | center,Dropdown(&gamer_list,&selected_1) | align_right,}),
				Container::Horizontal({Renderer([] {return ftxui::text("Second Player:"); }) | center,Dropdown(&gamer_list,&selected_2) | align_right,}),
				Container::Horizontal({Renderer([] {return ftxui::text("Board size:"); }) | center,Dropdown(&size_list,&selected_3) | align_right}),
				Button(wcenter(L"开始", 8), screen.ExitLoopClosure(), ButtonOption::Animated()) | center,
				Button(wcenter(L"返回", 8), [&ret,&screen] { ret = false; screen.Exit(); }, ButtonOption::Animated()) | center
				});
			screen.Loop(Renderer(layout, [&layout, &ret] {return dbox(layout->Render() | center); }));
			switch (selected_1)
			{
			case 0:gptr[0] = std::make_shared<human_gamer>(col0); break;
			case 1:gptr[0] = std::make_shared<computer_gamer_random>(col0); break;
			case 2:gptr[0] = std::make_shared<computer_gamer_ai>(col0); break;
			case 3:gptr[0] = std::make_shared<computer_gamer>(col0); break;
			default:
				break;
			}
			switch (selected_2)
			{
			case 0:gptr[1] = std::make_shared<human_gamer>(col1); break;
			case 1:gptr[1] = std::make_shared<computer_gamer_random>(col1); break;
			case 2:gptr[1] = std::make_shared<computer_gamer_ai>(col1, 2); break;
			case 3:gptr[1] = std::make_shared<computer_gamer>(col1,3); break;
			default:
				break;
			}
			switch (selected_3) {
			case 0:board_size = 8; break;
			case 1:case 2:board_size = 2 + selected_3 * 2; break;
			case 3:case 4:case 5:case 6:
				board_size = 4 + 2 * selected_3;
				break;
			default:
				std::unreachable();
			}
			return ret;
		}
		void GamePageLocal() {
			ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
			Board brd_ptr = Make<BoardBase>(board_size);
			brd_ptr->dqptr = std::make_shared<mtxdeque>();
			brd_ptr->brd.initialize();
			gptr[0]->dqptr = brd_ptr->dqptr;
			gptr[1]->dqptr = brd_ptr->dqptr;
			game gm(game::ready, board_size);
			gm.screen_ptr = &screen;
			brd_ptr->game_ptr = &gm;
			Component layout =
				Container::Horizontal({
					Container::Vertical({
						Container::Horizontal({brd_ptr}) | center,
						Container::Horizontal({
							Button(wcenter(L"退出", 8), [&] {brd_ptr->TakeFocus(); screen.PostEvent(Event::Special("EndLoop")); if (gm.state() == game::ended) { screen.Exit(); } }, ButtonOption::Animated()) | center,
							Renderer([] {return separator(); }),
							Button(wcenter(L"悔棋", 8), [&] {brd_ptr->TakeFocus(); screen.PostEvent(Event::Special("Regret")); }, ButtonOption::Animated()) | center,
							Renderer([] {return separator(); }),
							Button(wcenter(L"暂停", 8), [&] {brd_ptr->TakeFocus(); screen.PostEvent(Event::Special("Suspend")); }, ButtonOption::Animated()) | center,
							Renderer([] {return separator(); }),
							Button(wcenter(L"保存", 8), [&] {brd_ptr->TakeFocus(); screen.PostEvent(Event::Special("Save")); }, ButtonOption::Animated()) | center
						}) | ftxui::border

					}),
					Container::Vertical({
						Renderer([&] {return text(std::format("Black-{}:{}",gptr[col0]->get_name(),brd_ptr->brd.countpiece(col0))) | center; }),
						Renderer([&] {return text(std::format("White-{}:{}",gptr[col1]->get_name(),brd_ptr->brd.countpiece(col1))) | center; }),
					}) | center
				});
			

			std::jthread j([this, &gm] { gm.start(gptr[0], gptr[1]); });

			screen.Loop(Renderer(layout, [&layout] {return dbox(layout->Render() | center); }));



			/*
			* 先根据给出的信息给出两个gamer的shared_ptr,用于实现多态
			* 对于不同的类型的computer gamer,可以根据Name或者ID识别
			* 只考虑local的对战
			* game里边保存有component(component本质是个shared_ptr)
			*
			*
			*
			*
			* 创建一个默认初始化的game（状态为）
			*
			*/
			/*
			if othello selected :
			1.show the button of humanVhuman, humanVcomputer, computerVcomputer...these default game settings
				if the default setting is selected
					then show the default difficulty and the first, second hand
					2.show the button of self - design game.
					if the self - design selected
						show the dropdown of gamer types, the first and the second hand gamer type.
						(computer gamer maybe diverse)
						3.then the game settings are ready, so we enter an new game page(a new function, defined by the othello.components.hpp)
						调用othello.components中的Game(settings)
						settings有先手、后手的信息，包括类型（人类，计算机，哪种算法，使用name辨别),
			*/
			/*
			* basic_gamer g0(...),g1(...);
			* bw::othello::game game;
			* game.start(g0,g1);
			* 
			* 
			* 
			* components buttons=...;
			*
			* auto
			* component gamepage
			* screen.loop(gamepage);
			*/
			
			/*
			* board on event:
			* if(e==Event::Special("move_ready")){
			*		game.resume();
			*
			* }
			*/
		}
		gamer_ptr gptr[2] = { nullptr,nullptr };
		int board_size = 8;
	};
}