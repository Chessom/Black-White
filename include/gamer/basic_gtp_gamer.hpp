#pragma once
#include<iostream>
#include<string>
namespace bw {
	//Go Text Protocol Agent
	struct basic_gtp_gamer {
		basic_gtp_gamer(std::istream& is, std::ostream& os) :in(is), out(os) {}
		void get_protocol_version() {
			out << "protocol_version" << std::endl;
		}
		void get_name() {

		}
		void get_program_version() {

		}
		void set_board_size(int size_n) {

		}
		void clear_board() {

		}
		void play() {

		}
		std::string handle_one_command() {
			std::string op, ret;
			in >> op;
			if (op == "protocol_version") {
				ret = ret_hd + std::to_string(protocol_ver) + "\n";
			}
			else if (op == "name") {
				ret = ret_hd + name + "\n";
			}
			else if (op == "boardsize") {
				in >> brd_sz;
				ret = ret_hd + "\n";
			}
			else if (op == "name") {
				ret = ret_hd + name;
			}
			else if (op == "name") {
				ret = ret_hd + name;
			}
			else if (op == "name") {
				ret = ret_hd + name;
			}
		}
		virtual ~basic_gtp_gamer() = default;
	protected:
		std::string ret_hd = "= ";
		std::string err_hd = "? ";
		std::istream& in;
		std::ostream& out;
		int protocol_ver = 0;
		std::string name = "Annonymous";
		int version = 0;
		int brd_sz = 0;
	};
}