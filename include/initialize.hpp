#pragma once
#include"stdafx.h"
#include"env.hpp"
#include"config.hpp"
#include"globals.hpp"
#include<spdlog/sinks/rotating_file_sink.h>
#include<stacktrace>
namespace bw {
	inline void initialize() {

		using namespace std::filesystem;
		try{
			auto logger = spdlog::rotating_logger_mt("mylogger", "log.txt", 5ll * 1024 * 1024, 3);
			spdlog::set_default_logger(logger);
			spdlog::set_level(spdlog::level::trace);
			spdlog::info("started.");
			spdlog::flush_every(std::chrono::seconds(2));
		}
		catch (const std::exception& e)
		{
			spdlog::error(e.what());
			std::cerr << "日志初始化错误\n";
			std::cerr << std::stacktrace().current();
			auto c = std::getchar();
			return;
		}

		try {
			bw::env::set_codepage(CP_UTF8);
			//bw::env::set_font(L"Consolas");
			bw::env::set_window_size(101, 45);
		}
		catch (const std::exception& e)
		{
			spdlog::error(e.what());
			std::cerr << "设置代码页错误\n";
			std::cerr << std::stacktrace().current();
			auto c = std::getchar();
			return;
		}
		try {
			bw::global_config = std::make_shared<bw::config>();
			if (exists(json_config_path)) {
				struct_json::from_json_file(*global_config, json_config_path);
			}
			else {
				*global_config = bw::config::default_config();
				std::string config_json;
				struct_json::to_json(*global_config, config_json);
				std::ofstream fout(json_config_path, std::ios::out);
				fout << json_format(config_json);
				fout.close();
			}

			locale_gen.add_messages_path(global_config->locale_path);
			locale_gen.add_messages_domain("zh_CN");
			locale_gen.add_messages_domain("en_US");

			// 设置全局 locale
			std::locale::global(locale_gen(global_config->locale_id + ".utf8"));
			std::cout.imbue(std::locale());
		}
		catch (const std::exception& e) {
			spdlog::error(e.what());
			*global_config = bw::config::default_config();
			std::string config_json;
			struct_json::to_json(*global_config, config_json);
			std::ofstream fout(json_config_path, std::ios::out);
			fout << config_json;
			fout.close();
			
			locale_gen.add_messages_path(global_config->locale_path);
			locale_gen.add_messages_domain("zh_CN");
			locale_gen.add_messages_domain("en_US");

			std::locale::global(locale_gen(global_config->locale_id + ".utf8"));
			std::cout.imbue(std::locale());
		}
		try
		{
			bw::initialize_globals();
		}
		catch (const std::exception& e)
		{
			spdlog::error(e.what());
			std::cerr << "初始化数据错误\n";
			std::cerr << std::stacktrace().current();
			auto c = std::getchar();
			return;
		}
	}
}