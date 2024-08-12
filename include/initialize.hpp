#pragma once
#include"stdafx.h"
#include"env.hpp"
#include"config.hpp"
#include"globals.hpp"
#include<spdlog/sinks/rotating_file_sink.h>
namespace bw {
	inline void initialize() {

		using namespace std::filesystem;
		auto logger = spdlog::rotating_logger_mt("mylogger", "log.txt", 5ll * 1024 * 1024, 3);
		spdlog::set_default_logger(logger);

		bw::env::set_codepage(CP_UTF8);
		bw::env::set_font(boost::nowide::widen("Consolas"));
		bw::env::set_window_size(101, 45);

		bw::global_config = std::make_shared<bw::config>();

		try {
			if (exists(config::json_config_path)) {
				struct_json::from_json_file(*global_config, config::json_config_path);
				global_config->config_t = config::type::json;
			}
			else if (exists(config::bin_config_path)) {
				global_config->load_binary_file(config::bin_config_path);
				global_config->config_t = config::type::binary;
			}
			else {
				*global_config = bw::config::default_config();
				global_config->config_t = config::type::json;
				std::string config_json;
				struct_json::to_json(*global_config, config_json);
				std::ofstream fout(config::json_config_path, std::ios::out);
				fout << config_json;
				fout.close();
			}
			boost::locale::generator gen;
			gen.add_messages_path(global_config->locale_path);
			gen.add_messages_domain(global_config->locale_id);

			// 设置全局 locale
			std::locale::global(gen(global_config->locale_id + ".utf8"));
			std::cout.imbue(std::locale());
		}
		catch (const std::exception& e) {
			spdlog::error(e.what());
			*global_config = bw::config::default_config();
			global_config->config_t = config::type::json;
			std::string config_json;
			struct_json::to_json(*global_config, config_json);
			std::ofstream fout(config::json_config_path, std::ios::out);
			fout << config_json;
			fout.close();


			
			locale_gen.add_messages_path(global_config->locale_path);
			locale_gen.add_messages_domain("zh_CN");
			locale_gen.add_messages_domain("en_US");

			std::locale::global(locale_gen(global_config->locale_id + ".utf8"));
			std::cout.imbue(std::locale());
		}
		bw::initialize_globals();
	}
}