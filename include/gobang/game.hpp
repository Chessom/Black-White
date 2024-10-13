#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"game.hpp"
#include"gobang/moves.hpp"
#include"gobang/gamer.hpp"
#include"gobang/board.hpp"
#include"tui/components.hpp"
#include"tui/ftxui_screen.hpp"
#include"online/signals.hpp"
namespace bw::gobang {
	using namespace core;
	namespace components {
		class BoardBase;
	};
	struct aspect {
		board brd;
		core::color col = core::col0;//setter color
	};
	using aspects = std::vector<aspect>;
	class game :public basic_game, public std::enable_shared_from_this<game> {
		friend class components::BoardBase;
	public:
		game(basic_gamer_ptr g0, basic_gamer_ptr g1, bw::components::ftxui_screen_ptr Screen = nullptr) :basic_game(ready) {
			g[col0] = std::move(std::dynamic_pointer_cast<gamer>(g0));
			g[col1] = std::move(std::dynamic_pointer_cast<gamer>(g1));
			if (g[col0] == nullptr || g[col1] == nullptr) {
				ui::msgbox(gettext("Invalid Gamer!"));
				throw std::invalid_argument("Invalid Gamer!");
			}
			if (g[col0]->col != op_col(g[col1]->col)) {
				ui::msgbox(gettext("Invalid color!"));
				throw std::invalid_argument("Invalid Gamer color!");
			}

			brd.initialize();
			st = ready;
		}
		boost::cobalt::task<void> start() override {
			auto self = shared_from_this();
			begin_time = std::chrono::system_clock::now();
			st = ongoing;
			brd.initialize();
			color op = core::op_col(col);
			game_aspects.emplace_back(brd, none);//用来处理第一次悔棋情况

			while (true) {
				mvs[col].update(brd, col);
				if (mvs[col].size == 0) {
					mvs[op].update(brd, op);
					if (mvs[op].size == 0) {
						//announce("End");
						end_sig();
						st = ended;
						end_game();
						goto end_label;
					}
					else {
						pass_sig();
						col = op;
						op = op_col(col);
					}
					continue;
				}
				move mv{ .mvtype = move::invalid,.msg = "None" };
				try {
					mv = co_await g[col]->get_move(brd);

					co_await g[op_col(col)]->pass_move(mv);
				}
				catch (const std::exception& e) {
					ui::msgbox(gbk2utf8(e.what()));
					st = game::ended;
					mv.mvtype = move::quit;
				}

				switch (mv.mvtype) {
				case move::mv:
					brd.apply_move(mv.pos, col);
					//screen->post_event("Flush");
					flush_sig();
					break;
				case move::regret:
					if (brd.count(col0) + brd.count(col1) < 6) {
						co_await g[col]->pass_msg(gettext("You can't regret it now!"));
						continue;
					}
					if (cannot_regret()) {
						co_await g[col]->pass_msg(gettext("Regretting is not supported now!"));
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
					}
					flush_sig();
					wait_for_gamer(col);
					continue;
					break;
				case move::suspend:
					st = suspended;
					suspend();
					goto end_label;
					break;
				case move::quit:
					st = ended;
					goto end_label;
					break;
				case move::str:
					continue;
					break;
				case move::invalid:
					ui::msgbox(std::format("Invalid move:pos:{},type:{},\nmsg:{}", mv.pos, mv.mvtype, mv.msg));
					st = ended;
					goto end_label;
					break;
				default:
					ui::msgbox(std::format("{}:{}", gettext("move type id"), mv.mvtype));
					st = ended;
					break;
				}
				game_aspects.emplace_back(brd, col);
				col = op;//change the setter
				op = op_col(col);
			}
		end_label:
			end_time = std::chrono::system_clock::now();
			co_return;
		}
		bool valid_move(color c, const coord& crd) {
			return mvs[c].find(crd) != moves::npos;
		}
		void wait_for_gamer(color c) {

			return;
		}
		board current_board() {
			return brd;
		}
		color current_color() const {
			return col;
		}
		gamer_ptr current_gamer() const {
			return g[col];
		}
		void end_game() override {
			g[col]->cancel();
			g[col ^ 1]->cancel();
			bw::online::signals::end_game(st == ended ? game_msg::ok : game_msg::gamer_quit_or_disconnect);
			st = ended;
		}
		bool cannot_regret() {
			return
				g[col]->gamertype == gamer::online ||
				g[op_col(col)]->gamertype == gamer::online ||
				g[col]->gamertype == gamer::remote ||
				g[op_col(col)]->gamertype == gamer::remote;
		}
		std::string to_string() const {
			return struct_pack::serialize<std::string>(game_aspects);
		}
		void from_string(const std::string& buffer) {
			auto ex = struct_pack::deserialize<aspects>(buffer);
			if (ex.has_value()) {
				game_aspects = ex.value();
			}
			else {
				throw std::runtime_error(gettext("Load game failed!"));
			}
		}
		virtual ~game() {
			spdlog::trace("Othello game Destructor");
		};
		std::chrono::system_clock::time_point begin_time, end_time;
	protected:
		void suspend() {

		}
		gamer_ptr g[2] = { nullptr,nullptr };
		board brd;
		color col = col0;//current color
		moves mvs[2];
		aspects game_aspects;
	};
	using game_ptr = std::shared_ptr<game>;
}