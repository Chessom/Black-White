#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"othello.gamer.hpp"
#include"othello.components.hpp"
#include"othello.ai.hpp"
namespace bw::othello {
	using namespace std::chrono_literals;
	class computer_gamer :public gamer {
	public:
		computer_gamer() :gamer() { gamertype = basic_gamer::computer; }
		computer_gamer(core::color Color, int ID = 0, const std::string& Name = "basic_computer_gamer", int GamerType = basic_gamer::computer)
			:gamer(Color, ID, Name, GamerType) {
		};
		[[nodiscard]] virtual move getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) {
			mvs.update(brd, col);
			move mv;
			mv.mvtype = invalid;
			return mv;
		}
		string get_name() { return name; };
		void passmsg(std::string_view message) {};
	};

	class computer_gamer_random :public bw::othello::computer_gamer {
	public:
		computer_gamer_random() : computer_gamer() {}
		computer_gamer_random(core::color Color, int ID = 1, const std::string& Name = "computer_gamer_random", int GamerType = basic_gamer::computer)
			:computer_gamer(Color, ID, Name, GamerType) {};
		[[nodiscard]] move getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) override {
			mvs.update(brd, col);
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distrib(0, mvs.size - 1);
			return move{ .mvtype = move::mv, .pos = mvs.coords[distrib(gen)],.c = col };
		}
		virtual ~computer_gamer_random() = default;
	};
	class computer_gamer_ai :public bw::othello::computer_gamer {
	public:
		computer_gamer_ai() : computer_gamer() {}
		computer_gamer_ai(core::color Color, int ID = 2, const std::string& Name = "computer_gamer_ai", int GamerType = basic_gamer::computer)
			:computer_gamer(Color, ID, Name, GamerType) {};

		[[nodiscard]] move getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) override {
			mvs.update(brd, col);
			move mv;
			int points = -9999;
			dynamic_brd board = brd;
			for (int i = 0; i < mvs.size; ++i) {
				board.applymove(mvs.coords[i], col);
				auto mark = e.simple_evaluate_dynamic(board, col);
				if (mark > points) {
					points = mark;
					mv = { move::mv, mvs.coords[i], col };
				}
			}
			return mv;
		}
		virtual ~computer_gamer_ai() = default;
	private:
		ai::evaluator e;
	};
}