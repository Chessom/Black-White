#pragma once
#include"stdafx.h"
#include"env.hpp"
#include"config.hpp"
#include"globals.hpp"
#include<spdlog/sinks/rotating_file_sink.h>
namespace bw {
	void initialize() {
		//std::setlocale(LC_ALL, "");
		bw::env::set_codepage(CP_UTF8);
		bw::env::set_font(boost::nowide::widen("Consolas"));
		bw::env::set_window_size(101, 45);

		bw::global_config = std::make_shared<bw::config>();
		if (!global_config->load_binary_file(".\\config.bin")) {
			*global_config = bw::config::default_config();
		}
		auto logger = spdlog::rotating_logger_mt("mylogger", "log.txt", 5 * 1024 * 1024, 3);
		spdlog::set_default_logger(logger);
	}
}