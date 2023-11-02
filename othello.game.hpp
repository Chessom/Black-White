#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"game.hpp"
#include"othello.moves.hpp"
#include"othello.gamer.hpp"
#include"othello.board.hpp"
#include"ftxui/component/screen_interactive.hpp"
#include<Windows.h>
void msgbox(std::string_view s) {
	MessageBoxA(NULL, s.data(), "", 0);
}
namespace bw::othello {
	
	using namespace core;
	struct aspect {
		dynamic_brd brd;
		color col;
		template<typename Archive>
		void serialize(Archive& ar) {
			ar(brd, col);
		}
	};
	using aspects = std::vector<aspect>;
	class game :public bw::basic_game {
		friend class BoardBase;
	public:
		game() = default;
		game(int state, int brd_size = default_size) :basic_game(state), brd(brd_size) {};
		game(basic_gamer_ptr g0, basic_gamer_ptr g1, int brd_size = default_size) :basic_game(ready), brd(brd_size) {
			g[col0] = std::move(std::dynamic_pointer_cast<gamer>(g0));
			g[col1] = std::move(std::dynamic_pointer_cast<gamer>(g1));
			brd.initialize();
			state() = ready;
		}
		virtual void start(basic_gamer_ptr g0, basic_gamer_ptr g1) override {
			if (g0 == nullptr || g1 == nullptr) {
				throw std::runtime_error("Invalid Gamer!");
			}
			g[col0] = std::dynamic_pointer_cast<gamer>(g0);
			g[col1] = std::dynamic_pointer_cast<gamer>(g1);
			state() = ongoing;
			brd.initialize();
			color op = core::op_col(col);

			game_aspects.emplace_back(brd, none);//用来处理第一次悔棋情况

			while (true) {
				mvs[col].update(brd, col);
				//检查对局
				if (mvs[col].size == 0) {
					mvs[op].update(brd, op);
					if (mvs[op].size == 0) {
						//对局结束
						//公布比分(由components完成)
						announce("End");
						state() = ended;
						break;
					}
					else {
						//宣布pass
						col = op;
						op = op_col(col);
						announce("Pass");
					}
					continue;
				}
				//检查对局完毕，开始getmove*****************TO BE CHANGED
				
				move mv = g[col]->getmove(brd);

				std::string str;
				switch (mv.mvtype) {
				case move::mv:
					brd.applymove(mv.pos, col);
					announce("flush");
					wait_for_gamer(col);
					break;
				case move::regret:
					if (brd.count(col0) + brd.count(col1) < 6) {
						msgbox("你现在还不能悔棋！");
						continue;
					}
					if (col == game_aspects.back().col) {//和上次的是同一个人
						game_aspects.pop_back();
						brd = game_aspects.back().brd;
						//msgbox("Regret type:1");
					}
					else {
						game_aspects.pop_back();
						game_aspects.pop_back();
						brd = game_aspects.back().brd;
						//msgbox("Regret type:2");
					}
					announce("flush");
					wait_for_gamer(col);
					continue;//是while循环的continue;
					break;
				case move::suspend:
					suspend();
					return;
					break;
				case move::quit:
					screen_ptr->ExitLoopClosure()();
					return;
					break;
				case move::invalid:
					throw std::runtime_error("Invalid move!");
					return;
					break;
				default:
					msgbox(std::format("move type id:{}", mv.mvtype));
					std::unreachable();
				}
				game_aspects.emplace_back(brd, col);
				col = op;//change the setter
				op = op_col(col);
			}
		}
		virtual void load(archive arch) override {
			arch.dump(game_aspects);
			auto& asp = game_aspects.back();
			brd = asp.brd;
			col = asp.col;
		}
		bool valid_move(color c, const coord& crd) {
			return mvs[c].find(crd) != moves::npos;
		}
		void announce(std::string_view s) 
		void wait_for_gamer(color c) {
			std::unique_lock<std::mutex> uqmtx(g[c]->dqptr->mtx);
			g[c]->dqptr->conv.wait(uqmtx);
			return;
		}
		dynamic_brd current_board() {
			return brd;
		}
		color current_color() const {
			return col;
		}
		virtual ~game() = default;
		ftxui::ScreenInteractive* screen_ptr = nullptr;
	protected:
		virtual void suspend() override {
			state() = suspended;
			archive arch;
			try
			{
				arch.load(brd, col, mvs[0], mvs[1], game_aspects);
				
			}
			catch (const std::exception& e)
			{
				msgbox(std::format("Error:{},happened in load.", e.what()));
			}
			try
			{
				arch.dumpfile(R"(C:\Users\Vemy\Desktop\a.bin)");
			}
			catch (const std::exception& e)
			{
				msgbox(std::format("Error:{},happened in dumpfile.", e.what()));
			}
		}
		gamer_ptr g[2] = { nullptr,nullptr };
		dynamic_brd brd;
		color col = col0;//current color
		moves mvs[2];
		aspects game_aspects;
	};
	using game_ptr = std::shared_ptr<game>;
	template<typename Board>
	class light_human_game {
	public:
		light_human_game() = default;
		light_human_game(int size) {
			if constexpr (std::is_same<Board, dynamic_brd>::value) {
				brd.resize(size);
			}
		}
		void start() {
			brd.initialize();
			color op = 0;
			while (true) {
				
				op = op_col(col);
				std::println("{:sm<}", brd);
				mvs[col].update(brd, col);
				if (mvs[col].size == 0) {
					mvs[op].update(brd, op);
					if (mvs[op].size == 0) {
						std::println("Game end.");
						std::print("Black:{}\nWhite:{}\n", brd.countpiece(col0), brd.countpiece(col1));
						break;
					}
					std::println("{} pass.", col2str[col]);
					col = op;
					op = op_col(col);
					continue;
				}
				coord mv;
				std::string str;
				while (true) {
					std::println("Moves:{:c}", mvs[col]);
					std::print("Input {} move:", col2str[col]);
					std::cin >> str;
					for (auto& c : str) {
						c = std::toupper(c);
					}
					if (!is_valid_movestr(str) || mvs[col].find(fromstr(str)) == moves::npos) {
						std::println("Invalid move.");
						continue;
					}
					else {
						mv = fromstr(str);
						break;
					}
				}
				brd.applymove(mv, col);
				col = op;
			}
		}

	private:
		Board brd;
		moves mvs[2];
		color col = col0;
		std::map<color, std::string> col2str = { {col0, std::string("black")},{col1, std::string("white")} };
		coord fromstr(const std::string& s) {
			return coord(s[1] - '1', s[0] - 'A');
		}
		bool is_valid_movestr(const std::string& s) {
			return s[0] >= 'A' && s[0] <= 'Z' && s[1] >= '0' && s[1] <= '9';
		}
	};

	template<typename Board>
	int quick_self_play(int times) {
		return 0;
	}
}