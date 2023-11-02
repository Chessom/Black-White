#pragma once
#include"stdafx.h"
#include<ylt/struct_json/json_reader.h>
#include<ylt/struct_json/json_writer.h>
namespace bw {
	struct config {
		//config() = default;
		//config(const path& p) {
		//	load(p);
		//}
		//void load(const path& p) {
		//	if (!exists(p)) {
		//		throw std::runtime_error("Configuration file not exists.");
		//	}
		//	else {
		//		std::ifstream fin(p, std::ios::in | std::ios::binary);
		//		cereal::JSONInputArchive ar(fin);
		//		ar(ARGS);
		//	}
		//}
		//void dump(const std::filesystem::path& p) {
		//	std::ofstream fout;
		//	fout.open(p, std::ios::out | std::ios::binary);
		//	cereal::JSONOutputArchive ar(fout);
		//	ar(ARGS);
		//}
		//template<typename Archive>
		//void serialize(Archive& ar) {
		//	ar(ARGS);
		//}
		////data
		std::string ip;
		int port = 8888;
	};
	REFLECTION(config, ip, port)
}

