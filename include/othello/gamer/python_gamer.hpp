#pragma once
#include"othello/gamer.hpp"
#include"othello_bind.hpp"
#include"gamer/basic_python_gamer.hpp"
namespace bw::othello {
	inline bool py_othello_class_binded = false;
	struct python_gamer :othello::gamer, plugin::basic_python_gamer {
		python_gamer() :basic_python_gamer(), gamer() {
			name = gettext("python_othello_gamer"); 
			bind_class(); 
			detailed_gamer_type = detailed_type::python_script_gamer; 
		};
		python_gamer(core::color Color, int ID = 0, const std::string& Name = gettext("python_othello_gamer"), int GamerType = external)
			:gamer(Color, ID, Name, GamerType), basic_python_gamer() {
			bind_class();
			detailed_gamer_type = detailed_type::python_script_gamer;
		};
		python_gamer(const basic_gamer& gamer_info) :gamer(gamer_info), basic_python_gamer() {
			bind_class();
			detailed_gamer_type = detailed_type::python_script_gamer;
		};
		virtual void bind_class() override {
            namespace py = pybind11;
			try {
				basic_python_gamer::bind_class();
				if (global_config->py_use_embedded_module && !py_othello_class_binded) {
					do_bind_class();
					py_othello_class_binded = true;
				}
			}
			catch (const py::error_already_set& e) {
				ui::msgbox(e.what());
			}
			catch (const std::exception& e) {
				ui::msgbox(e.what());
			}
		}
		std::string default_script_path() const {
			return global_config->default_script_directory + script_filename;
		}
		void test() {
            namespace py = pybind11;
			py::exec(script);
			py::function getmv_py = py::globals()[getmove_func_signature.data()];
			dynamic_brd brd;
			brd.initialize();
			auto res = getmv_py(brd, color(col0));
			coord crd = res.cast<coord>();
			mvs.update(brd, col0);
			if (mvs.find(crd) == moves::npos) {
				throw std::runtime_error(std::format("{}:{}", gettext("Invalid move returned by the script"), crd));
			}
			is_good = true;
		}
		virtual boost::cobalt::task<move> get_move(dynamic_brd& brd, std::chrono::seconds limit = std::chrono::seconds(0)) {
			namespace py = pybind11;
			try {
				coord crd;
				py::gil_scoped_acquire acquire;
				py::function getmv_py = py::globals()[getmove_func_signature.data()];
				auto res = getmv_py(brd, col);
				crd = res.cast<coord>();
				co_return move{ .mvtype = move::mv,.pos = crd,.c = col };
			}
			catch (const py::error_already_set& e) {
				ui::msgbox(e.what());
				co_return move{ .mvtype = move::invalid,.msg = e.what() };
			}
		}
		virtual std::string get_name() {
			return name;
		};
		virtual boost::cobalt::task<void> pass_msg(std::string) {
			co_return;
		};
		virtual boost::cobalt::task<void> pass_move(move mv) {
			co_return;
		};
		virtual void cancel() {};
		virtual bool good()const override { return is_good; }
		virtual void reset() override {}
		virtual ~python_gamer() {
			spdlog::trace("Python Gamer Destructor");
		};
		bool is_good = false;
		std::string getmove_func_signature = "get_move";
		std::string script_filename = "othello.py";
	private:
		void do_bind_class() {
			namespace py = pybind11;
			py::gil_scoped_acquire acquire;
			py::module bw_m = py::module::import("bw_embed");
			using namespace bw::othello;
			auto othello_m = bw_m.def_submodule("othello", "submodule for othello game");
			bw::othello::python_bind::bind_othello_all(othello_m);
			py::gil_scoped_release release;
		}
	};
	using python_gamer_ptr = std::shared_ptr<python_gamer>;
}