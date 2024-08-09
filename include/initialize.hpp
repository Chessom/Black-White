#pragma once
#include"stdafx.h"
#include"env.hpp"
#include"config.hpp"
#include"globals.hpp"
#include<spdlog/sinks/rotating_file_sink.h>
namespace bw {
	inline void initialize() {
		auto logger = spdlog::rotating_logger_mt("mylogger", "log.txt", 5ll * 1024 * 1024, 3);
		spdlog::set_default_logger(logger);

		bw::env::set_codepage(CP_UTF8);
		bw::env::set_font(boost::nowide::widen("Consolas"));
		bw::env::set_window_size(101, 45);

		bw::global_config = std::make_shared<bw::config>();
		if (!global_config->load_binary_file(".\\config.bin")) {
			*global_config = bw::config::default_config();
		}

		boost::locale::generator gen;
		gen.add_messages_path(R"(locale\)");
		gen.add_messages_domain(global_config->locale_id);

		// 设置全局 locale
		std::locale::global(gen(global_config->locale_id + ".utf8"));
		std::cout.imbue(std::locale());

		bw::initialize_globals();
	}
}