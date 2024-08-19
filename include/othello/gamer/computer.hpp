#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"othello/gamer.hpp"
#include"tui/components.hpp"
#include"othello/ai.hpp"
namespace bw::othello {
	using namespace std::chrono_literals;
	class computer_gamer :public gamer {
	public:
		computer_gamer() :gamer() { gamertype = basic_gamer::computer; }
		computer_gamer(core::color Color, int ID = -1, const std::string& Name = "basic_computer_gamer", int GamerType = basic_gamer::computer)
			:gamer(Color, ID, Name, computer){
		};
		virtual boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) { co_return { .mvtype = invalid }; };
		string get_name() { return name; };
		boost::cobalt::task<void> pass_msg(std::string message) override { co_return; };
		boost::cobalt::task<void> pass_move(move mv) { co_return; };
		virtual void cancel() override {};
	};
	class computer_gamer_random :public bw::othello::computer_gamer {
	public:
		computer_gamer_random(core::color Color, int ID = 0, const std::string& Name = gettext("computer_gamer_random"))
			:computer_gamer(Color, ID, Name, basic_gamer::computer) {
			detailed_gamer_type = detailed_type::computer_random;
		};
		virtual boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) override {
			mvs.update(brd, col);
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distrib(0, mvs.size - 1);
			co_return move{ .mvtype = move::mv, .pos = mvs.coords[distrib(gen)],.c = col };
		}
		virtual bool good()const override { return true; }
		virtual ~computer_gamer_random() = default;
	};
	class computer_gamer_ai :public bw::othello::computer_gamer {
	public:
		computer_gamer_ai(core::color Color, int ID = 1, const std::string& Name = gettext("computer_gamer_ai"), int GamerType = basic_gamer::computer)
			:computer_gamer(Color, ID, Name, GamerType) {
			detailed_gamer_type = detailed_type::computer_ai;
		};
		virtual boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) override {
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
			co_return mv;
		}
		virtual bool good()const override { return true; }
		virtual ~computer_gamer_ai() = default;
	private:
		ai::evaluator e;
	};
	inline basic_gamer_ptr computer_gamer_from_id(int ID, color c) {
		switch (ID) {
		case 0:
			return std::make_shared<computer_gamer_random>(c);
		case 1:
			return std::make_shared<computer_gamer_ai>(c);
		default:
			break;
		}
		return nullptr;
	}
}