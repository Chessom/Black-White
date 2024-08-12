#include"stdafx.h"
#include"tui/ui.hpp"
#include"initialize.hpp"
#include"tools.hpp"
#include"config.hpp"
int main() {
	try {
		bw::initialize();
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what();
		std::cerr << "Initialize failed, please delete the configuration file and retry!\n";
		std::cerr << "初始化失败，请删除配置文件后重试！\n";
		auto c = std::getchar();
		return 0;
	}
	bw::cui cui;
	cui.HomePage();
	return 0;
}