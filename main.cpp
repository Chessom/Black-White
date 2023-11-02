#include"stdafx.h"
#include"ui.hpp"
#include"initialize.hpp"
#include"tools.hpp"
#include"config.hpp"
int main() {
	bw::initialize();
	bw::cui cui;
	cui.HomePage();
	//bw::tools::print_empty_brd();
	return 0;
}