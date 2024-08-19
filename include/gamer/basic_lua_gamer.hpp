#pragma once
#include<sol/sol.hpp>
#include"basic_gamer.hpp"
namespace bw::plugin {
	struct basic_lua_gamer {
		using lua_state_ptr = std::shared_ptr<lua_State>;
		basic_lua_gamer() {
			lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::os, sol::lib::math, sol::lib::table);
		};
		virtual void bind_class() {
			using namespace core;
			
			sol::table bw_nmsp = lua["bw"].get_or_create<sol::table>();

			bw_nmsp.new_enum("directions", "R", 0, "RD", 1, "D", 2, "DL", 3, "L", 4, "LU", 5, "U", 6, "UR", 7);
			bw_nmsp.new_enum("color", "none", bw::core::none, "col0", bw::core::col0, "col1", bw::core::col1);
			bw_nmsp.new_usertype<coord>(
				"coord",
				sol::constructors<coord(), coord(int, int)>(),
				"x", &coord::x,
				"y", &coord::y,
				"to_next", &coord::to_next,
				"to_next_n", &coord::to_next_n,
				"next", sol::resolve<coord(const int&)>(&coord::next),
				"next_n", sol::resolve<coord(const int&, int)>(&coord::next),
				"clear",&coord::clear
			);
		};
		/*virtual void load_script_file(std::filesystem::path Script_path) {

		}*/
		virtual void load_script(std::string Script) {
			script = Script;
		}
		virtual ~basic_lua_gamer() = default;
		std::string script;
	protected:
		sol::state lua;
	};
}