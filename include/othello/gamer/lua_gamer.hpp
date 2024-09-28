#pragma once
#include"tui/bwlocale.hpp"
#include"gamer/basic_lua_gamer.hpp"
#include"othello/gamer.hpp"
namespace bw::othello {
	struct lua_gamer :othello::gamer, plugin::basic_lua_gamer {
		lua_gamer() :basic_lua_gamer() { name = gettext("lua_othello_gamer"); bind_class(); detailed_gamer_type = detailed_type::lua_script_gamer; };
		lua_gamer(core::color Color, int ID = 0, const std::string& Name = "", int GamerType = external)
			:gamer(Color, ID, Name, GamerType), basic_lua_gamer() {
			bind_class();
			detailed_gamer_type = detailed_type::lua_script_gamer;
			if (!script.empty()) {
				lua.safe_script(script);
			}
		};
		lua_gamer(const basic_gamer& gamer_info) :gamer(gamer_info), basic_lua_gamer() { 
			bind_class(); 
			detailed_gamer_type = detailed_type::lua_script_gamer;
		};
		virtual void bind_class() override{
			basic_lua_gamer::bind_class();

			sol::table bw_nmsp = lua["bw"];
			sol::table othello_nmsp = bw_nmsp["othello"].get_or_create<sol::table>();

			othello_nmsp["default_brd_size"] = 8;
			othello_nmsp["max_brd_size"] = 16;

			othello_nmsp.new_usertype<dynamic_brd>(
				"board",
				sol::constructors<dynamic_brd(), dynamic_brd(int)>(),
				"in_board",&dynamic_brd::in_board,
				"initialize",&dynamic_brd::initialize,
				"get_col", &dynamic_brd::get_col,
				"set_col", &dynamic_brd::set_col,
				"apply_move", &dynamic_brd::apply_move,
				"count", &dynamic_brd::count,
				"resize", &dynamic_brd::resize,
				"brd_size", &dynamic_brd::brd_size,
				"clear", &dynamic_brd::clear
			);
			othello_nmsp["npos"] = moves::npos;
			
			othello_nmsp.new_usertype<moves>(
				"moves",
				sol::constructors<moves(), moves(dynamic_brd, color)>(),
				"update", &moves::update<dynamic_brd>,
				"empty", &moves::empty,
				"find", &moves::find,
				"get_crd", &moves::get_crd,
				"coords", &moves::coords
			);
		}
		std::string default_script_path() {
			return global_config->default_script_directory + script_filename;
		}
		void test() {
			lua.safe_script(script);
			sol::protected_function getmv = lua[getmove_func_signature];
			dynamic_brd brd;
			brd.initialize();
			auto res = getmv(brd, col0);
			coord crd;
			if (res.valid()) {
				crd = res;
			}
			else {
				sol::error err = res;
				throw std::runtime_error(err.what());
			}
			mvs.update(brd, col0);
			if (mvs.find(crd) == moves::npos) {
				throw std::runtime_error(std::format("{}:{}", gettext("Invalid move returned by the script"), crd));
			}
			is_good = true;
		}
		virtual boost::cobalt::task<move> get_move(dynamic_brd& brd, std::chrono::seconds limit = std::chrono::seconds(0)) {
			auto resfunc = lua[getmove_func_signature];
			sol::protected_function getmv;
			if (resfunc.valid()) {
				getmv = resfunc;
			}
			else {
				sol::error err = resfunc;
				co_return move{ .mvtype = move::invalid,.msg = err.what() };
			}
			
			auto res = getmv(brd, col);

			coord crd;
			if (res.valid()) {
				crd = res;
			}
			else {
				sol::error err = res;
				std::string what = err.what();
				spdlog::error(what);
				co_return move{ .mvtype = move::invalid,.msg = what };
			}
			co_return move{ .mvtype = move::mv,.pos = crd,.c = col };
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
		virtual bool good()const override { return is_good; }
		virtual void reset() override {}
		virtual ~lua_gamer() = default;
		bool is_good = false;
		std::string getmove_func_signature = "get_move";
		std::string script_filename = "othello.lua";
	};
	using lua_gamer_ptr = std::shared_ptr<lua_gamer>;
}