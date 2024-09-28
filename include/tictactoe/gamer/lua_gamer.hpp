#pragma once
#include"tui/bwlocale.hpp"
#include"gamer/basic_lua_gamer.hpp"
#include"tictactoe/gamer.hpp"
namespace bw::tictactoe {
	struct lua_gamer :tictactoe::gamer, plugin::basic_lua_gamer {
		lua_gamer() :basic_lua_gamer() { name = gettext("lua_tictactoe_gamer"); bind_class(); };
		lua_gamer(core::color Color, int ID = 0, const std::string& Name = "", int GamerType = external)
			:gamer(Color, ID, Name, GamerType), basic_lua_gamer() {
			bind_class();
		};
		lua_gamer(const basic_gamer& gamer_info) :gamer(gamer_info), basic_lua_gamer() { bind_class(); };
		virtual void bind_class() override {
			basic_lua_gamer::bind_class();

			sol::table bw_nmsp = lua["bw"];
			sol::table ttt_nmsp = bw_nmsp["tictactoe"].get_or_create<sol::table>();

			ttt_nmsp["default_brd_size"] = 8;
			ttt_nmsp["max_brd_size"] = 16;

			ttt_nmsp.new_usertype<board>(
				"board",
				sol::constructors<board(), board(int)>(),
				"initialize", &board::initialize,
				"get_col", &board::get_col,
				"set_col", &board::set_col,
				"apply_move", &board::apply_move,
				"check_move", &board::check_move,
				"check_winner", &board::check_winner,
				"count", sol::as_function(&board::cnt)
			);
			ttt_nmsp["npos"] = moves::npos;

			ttt_nmsp.new_usertype<moves>(
				"moves",
				sol::constructors<moves(), moves(dynamic_brd, color)>(),
				"update", &moves::update<dynamic_brd>,
				"empty", &moves::empty,
				"find", &moves::find,
				"get_crd", &moves::get_crd,
				"coords", &moves::coords
			);
		}
		void test() {

		}
		virtual boost::cobalt::task<move> get_move(board & brd, std::chrono::seconds limit = std::chrono::seconds(0)) {
			sol::protected_function getmv = lua["get_move"];
			auto res = getmv(brd, col);
			core::coord crd;
			if (res.valid()) {
				crd = res;
				
			}
			else {
				sol::error err = res;
				std::string what = err.what();
				throw std::runtime_error(what);
				spdlog::error(what);
			}
			co_return move{ .mvtype = move::mv,.crd = crd,.col = col };
		}
		virtual std::string get_name() {
			return name;
		};
		virtual boost::cobalt::task<void> pass_msg(std::string) {
			co_return;
		};
		virtual boost::cobalt::task<void> pass_move(move mv) {
			co_return;
		};
		virtual void cancel() {

		};
		virtual ~lua_gamer() = default;
	};
}