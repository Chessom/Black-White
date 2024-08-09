#pragma once
#include<lua.hpp>
#include"gamer.hpp"
namespace bw::plugin {
	struct lua_gamer :virtual bw::basic_gamer {
		
		lua_gamer() { L = luaL_newstate(); };
		
	private:
		std::string script;
		lua_State* L;
	};
}