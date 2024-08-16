#pragma once
#include"stdafx.h"
#include"tui/bwlocale.hpp"
namespace bw {
	inline const std::string json_config_path = R"(.\config.json)";
	struct config {
		using path = std::filesystem::path;
		config() {
			default_name = boost::asio::ip::host_name();
		}
		static config default_config() {
			return config{};
		}
		std::string locale_id = "zh_CN";
		std::string locale_path = R"(locale\)";
		std::string default_script_directory = R"(.\)";
		std::string default_address = "localhost";
		int port = 22222;
		bool first_run = true;
		bool first_login = true;
		std::string default_name;
		
	};
	REFLECTION(config, locale_id, locale_path, default_script_directory, default_address, port, first_run, first_login, default_name);
	using config_ptr = std::shared_ptr<config>;
}

