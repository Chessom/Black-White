#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"tictactoe.moves.hpp"
#include"tictactoe.gamer.hpp"
#include"tictactoe.components.hpp"
namespace bw::tictactoe {
	using namespace std::chrono_literals;
	class computer_gamer :public gamer {
	public:
		computer_gamer() :gamer() { gamertype = basic_gamer::computer; }
		computer_gamer(core::color Color, int ID = 0, const std::string& Name = "basic_computer_gamer", int GamerType = basic_gamer::computer)
			:gamer(Color, ID, Name, computer) {
		};
		virtual boost::cobalt::task<move> getmove(board& brd, std::chrono::seconds limit = 0s) { co_return{ .mvtype = invalid }; };
		string get_name() { return name; };
		boost::cobalt::task<void> pass_msg(std::string_view message)override { co_return; };
		boost::cobalt::task<void> pass_move(move mv) { co_return; };
		virtual void cancel() override {};
		moves mvs;
	};
	class computer_gamer_random :public computer_gamer {
	public:
		computer_gamer_random() : computer_gamer() {}
		computer_gamer_random(core::color Color, int ID = 1, const std::string& Name = "computer_gamer_random")
			:computer_gamer(Color, ID, Name, basic_gamer::computer) {};
		virtual boost::cobalt::task<move> getmove(board& brd, std::chrono::seconds limit = 0s) override {
			mvs.update(brd, col);
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distrib(0, mvs.size - 1);
			co_return move{ .mvtype = move::mv, .crd = mvs.coords[distrib(gen)],.col = col };
		}
		virtual ~computer_gamer_random() = default;
	};
	
}