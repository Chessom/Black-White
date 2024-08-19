#pragma once
#define PYBIND11_DETAILED_ERROR_MESSAGES
#include "core.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <pybind11/embed.h>
PYBIND11_EMBEDDED_MODULE(bw_embed, bw_m) {
    namespace py = pybind11;
    bw_m.doc() = "BlackWhite bindings with Python. This module include color, board and moves"; //module docstring
}
namespace bw::plugin {
	namespace py = pybind11;
	inline std::weak_ptr<py::scoped_interpreter> global_py_ptr;
	struct basic_python_gamer {
		basic_python_gamer() {
			if (global_py_ptr.expired()) {
				python = std::make_shared<py::scoped_interpreter>();
				global_py_ptr = python;
			}
			else {
				python = global_py_ptr.lock();
			}
		};
		virtual void bind_class() {
			if(global_config->py_use_embedded_module)
				do_bind_class();
		};
		virtual void load_script(std::string Script) {
			script = Script;
		}
		virtual ~basic_python_gamer() = default;
		std::string script;
	protected:
		std::shared_ptr<py::scoped_interpreter> python;
    private:
        void do_bind_class() {
			py::module bw_m = py::module::import("bw_embed");
			using namespace bw::core;
			py::class_<bw::core::coord>(bw_m, "coord")
				.def(py::init<>())
				.def(py::init<int, int>())
				.def("to_next", &coord::to_next)
				.def("to_next_n", &coord::to_next_n)
				.def("next", py::overload_cast<const int&>(&coord::next))
				.def("next_n", py::overload_cast<const int&, int>(&coord::next))
				.def("clear", &coord::clear)
				.def_readwrite("x", &coord::x)
				.def_readwrite("y", &coord::y)
				.def("__repr__", [](const coord& crd) {return std::format("{}", crd); })
				;
			bw_m.attr("none") = py::int_(int(bw::core::none));
			bw_m.attr("col0") = py::int_(int(bw::core::col0));
			bw_m.attr("col1") = py::int_(int(bw::core::col1));
			auto directions_m = bw_m.def_submodule("directions", "Directions enum");
			directions_m.attr("R") = py::int_(int(directions::R));
			directions_m.attr("RD") = py::int_(int(directions::RD));
			directions_m.attr("D") = py::int_(int(directions::D));
			directions_m.attr("DL") = py::int_(int(directions::DL));
			directions_m.attr("L") = py::int_(int(directions::L));
			directions_m.attr("LU") = py::int_(int(directions::LU));
			directions_m.attr("U") = py::int_(int(directions::U));
			directions_m.attr("UR") = py::int_(int(directions::UR));
        }
	};
}