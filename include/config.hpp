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
				.default_address = "localhost",
				.port = 22222,
				.first_run = true,
				.first_login = true,
				.default_name = boost::asio::ip::host_name()
			};
		}
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
		std::string locale_id;
		std::string default_address;
		int port = 22222;
		bool first_run = true;
		bool first_login = true;
		std::string default_name;
	};
	REFLECTION(config, locale_id, default_address, port, first_run, first_login, default_name);
	using config_ptr = std::shared_ptr<config>;
	/*inline bool load_json_file(const config& conf, const std::filesystem::path& p) {
		using namespace std::filesystem;
		try
		{
			if (exists(p)) {
				struct_json::from_json_file(conf, p.string());
			}
			else {
				std::cout << gettext("config.json not exist") << std::endl;
				std::system("pause");
				return false;
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			std::system("pause");
			return false;
		}

	}
	inline bool dump_json_file(const config& conf, const std::filesystem::path& p) {
		using namespace std::filesystem;
		std::fstream fout;
		try {
			fout.open(p, std::ios::out);
			std::string buf;
			struct_json::to_json(conf, buf);
			fout << buf;
			fout.close();
			return true;
		}
		catch (const std::exception& e) {
			fout.close();
			std::cout << e.what() << std::endl;
			std::system("pause");
			return false;
		}
	}
	inline std::string to_json(const config& conf) {
		std::string res;
		struct_json::to_json(conf, res);
		return res;
	}*/
}

