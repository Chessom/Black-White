#pragma once
#include<sol/sol.hpp>
#include"gamer.hpp"
namespace bw::plugin {
	inline void lua_state_deleter(lua_State* L) {
		lua_close(L);
	}

	struct basic_lua_gamer {
		using lua_state_ptr = std::shared_ptr<lua_State>;
		basic_lua_gamer() { 
			lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::table);
		};
		virtual void bind_class() {
			using namespace core;
			
			sol::table bw_nmsp = lua["bw"].get_or_create<sol::table>();

			bw_nmsp.new_enum("directions", "R", 0, "RD", 1, "D", 2, "DL", 3, "L", 4, "LU", 5, "U", 6, "UR", 7);
			bw_nmsp.new_enum("color", "none", -1, "col0", 0, "col1", 1);
			bw_nmsp.new_usertype<coord>(
				"coord",
				sol::constructors<coord(),coord(int, int)>(),
				"x",&coord::x,
				"y",&coord::y,
				"to_next",&coord::to_next,
				"to_next_n",&coord::to_next_n,
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