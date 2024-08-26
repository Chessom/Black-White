#define SPDLOG_ACTIVE_LEVEL 0
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
		std::cerr << "Exception: " << e.what() << std::endl;
		std::cerr << "Initialize failed, please delete the configuration file and retry!\n";
		std::cerr << "初始化失败，请删除配置文件后重试！\n";
		std::cerr << std::stacktrace().current();
		auto c = std::getchar();
		return 0;
	}
	bw::tui tui;
	tui.start();
	return 0;
}