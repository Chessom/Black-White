#pragma once
#define PYBIND11_DETAILED_ERROR_MESSAGES
#include "core.hpp"
#include "core_bind.hpp"
#include <pybind11/embed.h>
PYBIND11_EMBEDDED_MODULE(bw_embed, bw_m) {
    namespace py = pybind11;
    bw_m.doc() = "BlackWhite bindings with Python. This module include color, board and moves";
}
namespace bw::plugin {
	namespace py = pybind11;
	inline std::weak_ptr<py::scoped_interpreter> global_py_ptr;
	inline std::weak_ptr<py::gil_scoped_release> global_gil_ptr;
	inline bool basic_class_binded = false;
	struct basic_python_gamer {
		basic_python_gamer() {
			//if (global_py_ptr.expired()) {
			//	python = std::make_shared<py::scoped_interpreter>();
			//	global_py_ptr = python;
			//	spdlog::trace("start Python Interpreter at thread id: {}", std::this_thread::get_id()._Get_underlying_id());
			//}
			//else {
			//	python = global_py_ptr.lock();
			//}
			if (!Py_IsInitialized()) {
				py::initialize_interpreter();
			}
			if (global_gil_ptr.expired()) {
				gil_release = std::make_shared<py::gil_scoped_release>();
				global_gil_ptr = gil_release;
			}
			else {
				gil_release = global_gil_ptr.lock();
			}
		};
		virtual void bind_class() {
			if (global_config->py_use_embedded_module && !basic_class_binded) {
				do_bind_class();
				basic_class_binded = true;
			}
		};
		virtual void load_script(std::string Script) {
			script = Script;
		}
		virtual ~basic_python_gamer() {
			spdlog::trace("Basic Python Gamer Destructor at thread id: {}", std::this_thread::get_id()._Get_underlying_id());
		};
		std::string script;
	protected:
		std::shared_ptr<py::scoped_interpreter> python = nullptr;
		std::shared_ptr<py::gil_scoped_release> gil_release = nullptr;
    private:
        void do_bind_class() {
			py::gil_scoped_acquire acquire;
			py::module bw_m = py::module::import("bw_embed");
			bw::core::python_bind::bind_core_classes(bw_m);
			py::gil_scoped_release release;
        }
	};
}