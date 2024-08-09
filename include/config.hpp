#pragma once
#include"stdafx.h"
#include"tui/bwlocale.hpp"
#include<boost/asio/ip/host_name.hpp>
namespace bw {
	struct config {
		using path = std::filesystem::path;
		static config default_config() {
			return 
			config
			{ 
				"zh_CN",
				"localhost",
				22222,
				true,
				true,
				boost::asio::ip::host_name() 
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
		bool load_json_file(const path& p) {
			using namespace std::filesystem;
			try
			{
				if (exists(p)) {
					struct_json::from_json_file(*this, p.string());
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
		bool dump_json_file(const path& p) const {
			using namespace std::filesystem;
			std::fstream fout;
			try {
				fout.open(p, std::ios::out);
				struct_json::to_json(*this, fout);
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
		std::string to_json() {
			std::string res;
			struct_json::to_json(*this, res);
			return res;
		}
		std::string locale_id;
		std::string default_address;
		int port = 22222;
		bool first_run = true;
		bool first_login = true;
		std::string default_name;
	};
	REFLECTION(config, locale_id, default_address, port, first_run, default_name);
	using config_ptr = std::shared_ptr<config>;
}

