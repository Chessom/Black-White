#pragma once
#include"othello/gamer.hpp"
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
		virtual boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = std::chrono::seconds(0)) {
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
		std::string getmove_func_signature = "getmove";
		std::string script_filename = "othello.py";
	private:
		void do_bind_class() {
			namespace py = pybind11;
			py::gil_scoped_acquire acquire;
			py::module bw_m = py::module::import("bw_embed");
			using namespace bw::othello;
			auto othello_m = bw_m.def_submodule("othello", "submodule for othello game");
			py::class_<dynamic_brd>(othello_m, "board")
				.def(py::init<>())
				.def(py::init<int>())
				.def("in_board", &dynamic_brd::in_board)
				.def("initialize", &dynamic_brd::initialize)
				.def("getcol", &dynamic_brd::getcol)
				.def("setcol", &dynamic_brd::setcol)
				.def("applymove", &dynamic_brd::applymove)
				.def("count", &dynamic_brd::count)
				.def("resize", &dynamic_brd::resize)
				.def("brd_size", &dynamic_brd::brd_size)
				.def("clear", &dynamic_brd::resize)
				.def("__str__", [](const dynamic_brd& brd) {return std::vformat("{:sm<}", std::make_format_args(brd)); })
				.def("__repr__", [](const dynamic_brd& brd) {return std::vformat("{:n<}", std::make_format_args(brd)); })
				.def("__format__", [](const dynamic_brd& brd, const std::string& fmt_s) {return std::vformat("{0:{1}}", std::make_format_args(brd, fmt_s)); });

			py::class_<bitbrd>(othello_m, "bitbrd")
				.def(py::init<>())
				.def(py::init<const ull&, const ull&>())
				.def(py::init<const dynamic_brd&>())
				.def("in_board", &bitbrd::in_board)
				.def("initialize", &bitbrd::initialize)
				.def("getcol", &bitbrd::getcol)
				.def("setcol", &bitbrd::setcol)
				.def("applymove", py::overload_cast<const coord&, const color&>(&bitbrd::applymove))
				.def("applymove", py::overload_cast<const ull&, const color&>(&bitbrd::applymove))
				.def("getmoves", &bitbrd::getmoves)
				.def("to_dynamic", &bitbrd::to_dynamic)
				.def("count", &bitbrd::count)
				.def("countpiece", &bitbrd::countpiece)
				.def("brd_size", &bitbrd::brd_size)
				.def("clear", &bitbrd::clear)
				.def("__str__", [](const bitbrd& brd) {return std::vformat("{:sm<}", std::make_format_args(brd)); })
				.def("__repr__", [](const bitbrd& brd) {return std::vformat("{:n<}", std::make_format_args(brd)); })
				.def("__format__", [](const bitbrd& brd, const std::string& fmt_s) {return std::vformat("{0:{1}}", std::make_format_args(brd, fmt_s)); });

			py::class_<moves>(othello_m, "moves")
				.def(py::init<>())
				.def(py::init<const dynamic_brd&, const color&>())
				.def(py::init<const bitbrd&, const color&>())
				.def("update", &moves::update<dynamic_brd>)
				.def("update", &moves::update<bitbrd>)
				.def("push_back", &moves::push_back)
				.def("empty", &moves::empty)
				.def("find", &moves::find)
				.def_readwrite("coords", &moves::coords)
				.def_property_readonly("npos", [] {return moves::npos; })
				.def("__repr__", [](const moves& mvs) {return std::format("{}", mvs); });
			py::gil_scoped_release release;
		}
	};
	using python_gamer_ptr = std::shared_ptr<python_gamer>;
}