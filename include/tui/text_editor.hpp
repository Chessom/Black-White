#pragma once
#include<ftxui/component/component_options.hpp>
namespace bw::components {
	struct TextEditorState {
		ftxui::Element element;
		bool hovered;         /// < Whether the TextEditor is hovered by the mouse.
		bool focused;         /// < Whether the TextEditor is focused by the user.
		bool is_placeholder;  /// < Whether the TextEditor is empty and displaying the
		/// < placeholder.
	};

	/// @brief Option for the TextEditor component.
	/// @ingroup component
	struct TextEditorOption {
		// A set of predefined styles:

		/// @brief Create the default TextEditor style:
		static TextEditorOption Default();

		/// The content of the TextEditor.
		ftxui::StringRef content = "";

		/// The content of the TextEditor when it's empty.
		ftxui::StringRef placeholder = "";

		/// The content of the string inserted when Tab is pressed.
		ftxui::StringRef tab_string = "    ";

		// Style:
		std::function<ftxui::Element(TextEditorState)> transform;
		std::function<ftxui::Element(const std::string&, int)> line_renderer;
		std::function<ftxui::Element(int)> line_number_renderer;

		/// Called when the content changes.
		std::function<void()> on_change = [] {};
		/// Called when the user presses enter.
		std::function<void()> on_enter = [] {};

		// The char position of the cursor:
		ftxui::Ref<int> cursor_position = 0;
		ftxui::Ref<bool> insert = true;     ///< Insert or overtype character mode.
	};

	ftxui::Component TextEditor(TextEditorOption options = {});
	ftxui::Component TextEditor(ftxui::StringRef content, TextEditorOption options = {});
	ftxui::Component TextEditor(ftxui::StringRef content,
		ftxui::StringRef placeholder,
		TextEditorOption options = {});
}