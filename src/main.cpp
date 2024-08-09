#include"stdafx.h"
#include"tui/ui.hpp"
#include"initialize.hpp"
#include"tools.hpp"
#include"config.hpp"
//#include"othello.ai.practise.hpp"
int main() {
	bw::initialize();
	bw::cui cui;
	cui.HomePage();
	/*std::string json;
	auto mv = bw::othello::move{ 0,{1,2},bw::core::col0 };
	struct_json::to_json(mv, json);
	std::print("{}", json);*/
	//bw::othello::ai::full_test();
	return 0;
}