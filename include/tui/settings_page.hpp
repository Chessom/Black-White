#pragma once
#include"config.hpp"
#include"globals.hpp"
#include"components.hpp"
#include"utility.hpp"
namespace bw::components {
	struct SettingsPageImpl {
		SettingsPageImpl() {
			conf = *bw::global_config;
			origin_conf = conf;
		}

		std::vector<std::string> language_entries = {
				gettext("Chinese"),
				gettext("English"),
		};

		std::vector<std::string> on_off_entries = {
			gettext("ON"),
			gettext("OFF"),
		};

		int language_selected = 0;

		int py_embedded_module_selected = 0;

		std::string port_string = std::to_string(global_config->port);

		void save_handler() {
			if (language_selected == 0) {
				conf.locale_id = "zh_CN";
			}
			else if (language_selected == 1) {
				conf.locale_id = "en_US";
			}
			conf.port = std::stoi(port_string);
			if (py_embedded_module_selected == 0) {
				conf.py_use_embedded_module = true;
			}
			else {
				conf.py_use_embedded_module = false;
			}
			origin_conf = *bw::global_config;
			*bw::global_config = conf;
		}

		void reset_handler() {
			conf = origin_conf;
			port_string = std::to_string(conf.port);
			if (conf.locale_id == "zh_CN") {
				language_selected = 0;
			}
			else if (conf.locale_id == "en_US") {
				language_selected = 1;
			}
			py_embedded_module_selected = int(conf.py_use_embedded_module);
		}

		void start() {
			using namespace ftxui;
			auto screen = ScreenInteractive::Fullscreen();
			screen.Loop(Page());
		}

		ftxui::Component Page() {
			using namespace ftxui;
            
            Component language_toggle = Toggle(&language_entries, &language_selected);

            Component name_input = Input(&conf.default_name, gettext("Enter the name")) | underlined;

            Component address_input = Input(&conf.default_address, gettext("Enter the address")) | underlined;

            
            Component port_number_input = Input(&port_string, gettext("Enter the port number")) | underlined | CatchEvent([&](Event event) {return event.is_character() && !std::isdigit(event.character()[0]);}) | CatchEvent([&](Event event) {return event.is_character() && port_string.size() > 5;});

			Component locale_path_input = Input(&conf.locale_path, gettext("Path of the language files")) | underlined;

			Component py_use_embedded_module_toggle = Toggle(&on_off_entries, &py_embedded_module_selected);

			auto ConfirmButton = Button(gettext("Apply"),
				[this] {save_handler(); },
                ButtonOption::Animated(Color::GreenLight)
            );

			auto SaveButton = Button(gettext("Save to local"),
				[this] {
					save_handler();
					std::string config_json;
					struct_json::to_json(conf, config_json);
					std::ofstream fout(json_config_path, std::ios::out);
					fout << json_format(config_json);
					fout.close();
				},
				ButtonOption::Animated(Color::Blue)
			);
			auto CancelButton = Button(gettext("Cancel"),
				[this] {reset_handler();},
				ButtonOption::Animated(Color::YellowLight)
			);
            
            auto container = Container::Vertical({
                language_toggle,
                name_input,
                address_input,
                port_number_input,
				locale_path_input,
				py_use_embedded_module_toggle,
                });

			auto renderer = Renderer(container, 
				[this, language_toggle, name_input, address_input, port_number_input, locale_path_input, py_use_embedded_module_toggle] {
				return vbox({
					hbox(text("· " + gettext("Language (Changes take effect after restart): ")), filler(),language_toggle->Render()) | xflex ,
					hbox(text("· " + gettext("Default display name: ")),filler(),name_input->Render() | align_right) | xflex,
					hbox(text("· " + gettext("Default server address: ")),filler(),address_input->Render() | align_right) | xflex,
					hbox(text("· " + gettext("Default port number: ")),filler(),port_number_input->Render() | align_right) | xflex,
					hbox(text("· " + gettext("Locale files path: ")),filler(),locale_path_input->Render() | align_right) | xflex,
					hbox(text("· " + gettext("Python script use embedded module: ")), filler(),py_use_embedded_module_toggle->Render()) | xflex ,
					});
				}) | xflex | size(WIDTH, LESS_THAN, 80) | border;
			
			auto layout = Container::Vertical({
				renderer | center,
				Renderer([] {return filler(); }),
				Container::Horizontal({
					ConfirmButton | vcenter ,
					ui::TextComp("  "),
					SaveButton | vcenter ,
					ui::TextComp("  "),
					CancelButton | vcenter,
					}) | center,
				ui::TextComp(gettext(
					"\"Apply\" means that changes will only \n"
					"take effect until exiting the program, \n"
					"while \"Save to local\" means that \n"
					"changes will be written to the configuration file \n"
					"and will take effect at a later startup."
				),center | color(Color::BlueLight)) | center
			});

			return layout;
		}
		bw::config conf;
		bw::config origin_conf;
	};
	inline ftxui::Component SettingsPage(){
		struct Impl :ftxui::ComponentBase{
			Impl() {
				Add(impl.Page());
			}
			SettingsPageImpl impl;
		};
		return ftxui::Make<Impl>();
	}
}