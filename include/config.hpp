#pragma once
#include"stdafx.h"
#include"tui/bwlocale.hpp"
namespace bw {
	struct config {
		using path = std::filesystem::path;
		static config default_config() {
			return
				config
			{
				.locale_id = "zh_CN",
				.locale_path = R"(locale\)",
				.default_address = "localhost",
				.port = 22222,
				.first_run = true,
				.first_login = true,
				.default_name = boost::asio::ip::host_name()
			};
		}
		static constexpr std::string bin_config_path = R"(.\config.bin)";
		static constexpr std::string json_config_path = R"(.\config.json)";
		bool load_binary_file(const path& p) {
			using namespace std::filesystem;
			if (exists(p)) {
				std::fstream fin(p, std::ios::binary | std::ios::in);
				auto conf = struct_pack::deserialize<config>(fin);
				fin.close();
				if (conf) {
					*this = conf.value();
					return true;
				}
				else {
					*this = default_config();
					return false;
				}
			}
			else {
				return false;
			}
		}
		bool dump_binary_file(const path& p) const {
			using namespace std::filesystem;
			std::fstream fout;
			try {
				fout.open(p, std::ios::out | std::ios::binary);
				fout << struct_pack::serialize<std::string>(*this);
				fout.close();
				return true;
			}
			catch (const std::exception&) {
				fout.close();
				return false;
			}
		}
		std::string locale_id = "zh_CN";
		std::string locale_path = R"(locale\)";
		std::string default_address = "localhost";
		int port = 22222;
		bool first_run = true;
		bool first_login = true;
		std::string default_name;
		enum class type {
			json, binary
		};
		type config_t;
	};
	REFLECTION(config, locale_id, locale_path, default_address, port, first_run, first_login, default_name);
	using config_ptr = std::shared_ptr<config>;
}

