#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"othello.gamer.hpp"
#include<windows.h>
namespace bw::othello {
	void msgbox(std::string_view s) {
		MessageBoxA(NULL, s.data(), "", 0);
	}
	using namespace std::chrono_literals;
	class human_gamer :public gamer {
	public:
		human_gamer() :gamer() { gamertype = basic_gamer::human; }
		human_gamer(core::color Color, int ID = 0, const std::string& Name = "Human", int GamerType = basic_gamer::human)
			:gamer(Color, ID, Name, GamerType) {
		};
		[[nodiscard]] move getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) {
			mvs.update(brd, col);
			while (true) {
				std::unique_lock<std::mutex> uqmtx(dqptr->mtx);
				dqptr->conv.wait(uqmtx);
				while (!dqptr->q.empty()) {
					auto pos = dqptr->q.front();
					dqptr->q.pop_front();
					if (mvs.find(pos) != moves::npos){
						return move{move::mv,pos,col};
					}
					else if (pos == coord{-1, -1}) {
						return move{ move::quit };
					}
					else if (pos == coord{ -1,-2 }) {
						return move{ move::regret };
					}
					else if (pos == coord{ -1,-3 }) {
						return move{ move::suspend };
					}
				}
			}
		}
		string get_name() { return name; }
		void passmsg(std::string_view message) {}
	};
}