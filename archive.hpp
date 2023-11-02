#pragma once
#include"stdafx.h"
namespace bw {
	class archive {
	public:
		enum { portable_bin, bin /*, json, xml*/ };
		archive() :ss(std::ios::binary) {};
		archive(int arch_type) :ss(std::ios::binary), type(arch_type) {};
		archive(const archive& ar) {
			type = ar.type;
			ss.str(ar.ss.str());
		}
		archive(archive&& ar) noexcept {
			type = ar.type;
			ss.str(ar.ss.str());
		}
		archive& operator=(const archive& ar) {
			type = ar.type;
			ss.str(ar.ss.str());
		}
		template<typename... Args>
		void load(Args&&... args) {
			ss.clear();
			ss.str("");
			if (type == portable_bin) {
				cereal::PortableBinaryOutputArchive ar(ss);
				ar(std::forward<Args>(args)...);
			}
			if (type == bin) {
				cereal::BinaryOutputArchive ar(ss);
				ar(std::forward<Args>(args)...);
			}
			return;
		}
		template<typename... Args>
		void dump(Args&&... args) {
			if (type == portable_bin) {
				cereal::PortableBinaryOutputArchive ar(ss);
				ar(std::forward<Args>(args)...);
			}
			if (type == bin) {
				cereal::BinaryOutputArchive ar(ss);
				ar(std::forward<Args>(args)...);
			}
			return;
		}
		void loadfile(const std::filesystem::path& p) {
			if (!std::filesystem::exists(p)) {
				throw std::runtime_error(std::format("File: {} not exist.", p.string()));
			}
			std::ifstream fin(p, std::ios::in | std::ios::binary);
			ss << fin.rdbuf();
			fin.close();
		}
		void dumpfile(const std::filesystem::path& p) {
			std::ofstream fout(p, std::ios::out | std::ios::binary);
			fout << ss.rdbuf();
			fout.close();
		}
		std::string to_str() {
			return ss.str();
		}
		int type = portable_bin;
		std::stringstream ss;
	};
	template<typename T>
	std::string to_binstr(const T& obj, int arch_type = archive::portable_bin) {
		std::stringstream ss(std::ios::binary);
		if (arch_type == archive::portable_bin) {
			cereal::PortableBinaryOutputArchive ar(ss);
			ar(obj);
		}
		else if (arch_type == archive::bin) {
			cereal::BinaryOutputArchive ar(ss);
			ar(obj);
		}
		return ss.str();
	}
	template<typename T>
	T from_binstr(const std::string& s, int arch_type = archive::portable_bin) {
		std::stringstream ss(std::ios::binary);
		ss.str(s);
		T obj{};
		if (arch_type == archive::portable_bin) {
			cereal::PortableBinaryInputArchive ar(ss);
			ar(obj);
		}
		else if (arch_type == archive::bin) {
			cereal::BinaryInputArchive ar(ss);
			ar(obj);
		}
		else {
			throw std::runtime_error("Invalid archive type.");
		}
		return obj;
	}
}