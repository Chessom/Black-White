#include "ftxui/component/captured_mouse.hpp"     // for CapturedMouse
#include "ftxui/component/component.hpp"          // for Make, Input
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "tui/text_editor.hpp"  // for TextEditorOption
#include "ftxui/component/event.hpp"  // for Event, Event::ArrowDown, Event::ArrowLeft, Event::ArrowLeftCtrl, Event::ArrowRight, Event::ArrowRightCtrl, Event::ArrowUp, Event::Backspace, Event::Delete, Event::End, Event::Home, Event::Return
#include "ftxui/component/mouse.hpp"  // for Mouse, Mouse::Left, Mouse::Pressed
#include "ftxui/component/screen_interactive.hpp"  // for Component
#include "ftxui/dom/elements.hpp"  // for operator|, reflect, text, Element, xflex, hbox, Elements, frame, operator|=, vbox, focus, focusCursorBarBlinking, select
#include "ftxui/screen/box.hpp"    // for Box
#include "ftxui/screen/string.hpp"           // for string_width
#include "tui/third_party/string_internal.hpp"  // for GlyphNext, GlyphPrevious, WordBreakProperty, EatCodePoint, CodepointToWordBreakProperty, IsFullWidth, WordBreakProperty::ALetter, WordBreakProperty::CR, WordBreakProperty::Double_Quote, WordBreakProperty::Extend, WordBreakProperty::ExtendNumLet, WordBreakProperty::Format, WordBreakProperty::Hebrew_Letter, WordBreakProperty::Katakana, WordBreakProperty::LF, WordBreakProperty::MidLetter, WordBreakProperty::MidNum, WordBreakProperty::MidNumLet, WordBreakProperty::Newline, WordBreakProperty::Numeric, WordBreakProperty::Regional_Indicator, WordBreakProperty::Single_Quote, WordBreakProperty::WSegSpace, WordBreakProperty::ZWJ
#include "tui/third_party/util.hpp"             // for clamp
#include "ftxui/util/ref.hpp"                // for StringRef, Ref

namespace bw::components {
    TextEditorOption TextEditorOption::Default() {
        using namespace ftxui;
        TextEditorOption option;
        option.transform = [](TextEditorState state) {
            state.element |= borderEmpty;

            if (state.is_placeholder) {
                state.element |= dim;
            }

            return state.element;
            };
        option.line_renderer = [](const std::string& input, int line_num) {
            return text(input);
        };
        option.line_number_renderer = [](int line_number) {
            return text(std::to_string(line_number)) | dim;
        };
        return option;
    }

    namespace {

        std::vector<std::string> Split(const std::string& input) {
            std::vector<std::string> output;
            std::stringstream ss(input);
            std::string line;
            while (std::getline(ss, line)) {
                output.push_back(line);
            }
            if (input.back() == '\n') {
                output.emplace_back("");
            }
            return output;
        }

        size_t GlyphWidth(const std::string& input, size_t iter) {
            using namespace ftxui;
            uint32_t ucs = 0;
            if (!EatCodePoint(input, iter, &iter, &ucs)) {
                return 0;
            }
            if (IsFullWidth(ucs)) {
                return 2;
            }
            return 1;
        }

        bool IsWordCodePoint(uint32_t codepoint) {
            using namespace ftxui;
            switch (CodepointToWordBreakProperty(codepoint)) {
            case WordBreakProperty::ALetter:
            case WordBreakProperty::Hebrew_Letter:
            case WordBreakProperty::Katakana:
            case WordBreakProperty::Numeric:
                return true;

            case WordBreakProperty::CR:
            case WordBreakProperty::Double_Quote:
            case WordBreakProperty::LF:
            case WordBreakProperty::MidLetter:
            case WordBreakProperty::MidNum:
            case WordBreakProperty::MidNumLet:
            case WordBreakProperty::Newline:
            case WordBreakProperty::Single_Quote:
            case WordBreakProperty::WSegSpace:
                // Unexpected/Unsure
            case WordBreakProperty::Extend:
            case WordBreakProperty::ExtendNumLet:
            case WordBreakProperty::Format:
            case WordBreakProperty::Regional_Indicator:
            case WordBreakProperty::ZWJ:
                return false;
            }
            return false;  // NOT_REACHED();
        }

        bool IsWordCharacter(const std::string& input, size_t iter) {
            using namespace ftxui;
            uint32_t ucs = 0;
            if (!EatCodePoint(input, iter, &iter, &ucs)) {
                return false;
            }

            return IsWordCodePoint(ucs);
        }

        // An input box. The user can type text into it.
        class TextEditorBase : public ftxui::ComponentBase, public TextEditorOption {
        public:
            // NOLINTNEXTLINE
            TextEditorBase(TextEditorOption option) : TextEditorOption(std::move(option)) {}

        protected:
            // Component implementation:
            ftxui::Element Render() {
                using namespace ftxui;
                const bool is_focused = Focused();
                const auto focused =
                    (is_focused || hovered_) ? focusCursorBarBlinking : ftxui::select;

                auto transform_func =
                    transform ? transform : TextEditorOption::Default().transform;

                // placeholder.
                if (content->empty()) {
                    auto element = text(placeholder()) | xflex | frame;
                    if (is_focused) {
                        element |= focus;
                    }
                    return hbox(
                        line_number_renderer(1) | borderEmpty | yframe,
                        transform_func({
                               std::move(element), hovered_, is_focused,
                               true  // placeholder
                        }) |
                        reflect(box_)
                    );
                }

                Elements elements, line_numbers;
                const std::vector<std::string> lines = Split(*content);

                cursor_position() = util::clamp(cursor_position(), 0, (int)content->size());

                // Find the line and index of the cursor.
                int cursor_line = 0;
                int cursor_char_index = cursor_position();
                for (const auto& line : lines) {
                    if (cursor_char_index <= (int)line.size()) {
                        break;
                    }

                    cursor_char_index -= line.size() + 1;
                    cursor_line++;
                }

                if (lines.empty()) {
                    elements.push_back(text("") | focused);
                    line_numbers.push_back(line_number_renderer(1));
                }

                elements.reserve(lines.size());
                for (size_t i = 0; i < lines.size(); ++i) {
                    const std::string& line = lines[i];

                    // This is not the cursor line.
                    if (int(i) != cursor_line) {
                        elements.push_back(Text(line, i));
                        continue;
                    }

                    // The cursor is at the end of the line.
                    if (cursor_char_index >= (int)line.size()) {
                        elements.push_back(hbox({
                                               Text(line,i),
                                               text(" ") | focused | reflect(cursor_box_),
                            }) |
                            xflex);
                        continue;
                    }

                    // The cursor is on this line.
                    const int glyph_start = cursor_char_index;
                    const int glyph_end = GlyphNext(line, glyph_start);
                    const std::string part_before_cursor = line.substr(0, glyph_start);
                    const std::string part_at_cursor =
                        line.substr(glyph_start, glyph_end - glyph_start);
                    const std::string part_after_cursor = line.substr(glyph_end);
                    auto element = hbox({
                                       Text(part_before_cursor, i) ,
                                       Text(part_at_cursor, i) | focused | reflect(cursor_box_),
                                       Text(part_after_cursor, i),
                        }) |
                        xflex;
                    elements.push_back(element);
                }

                auto element = vbox(std::move(elements)) | frame | vscroll_indicator;

                
                for (int i = 1; i <= lines.size(); ++i) {
                    if (cursor_line + 1 == i) {
                        line_numbers.push_back(line_number_renderer(i) | align_right | focus);
                    }
                    else {
                        line_numbers.push_back(line_number_renderer(i) | align_right);
                    }
                }

                return hbox(
                    vbox(line_numbers) | yframe | borderEmpty,
                    transform_func({
                           std::move(element), hovered_, is_focused,
                           false  // placeholder
                    }) |
                    xflex | reflect(box_)
                );
            }

            ftxui::Element Text(const std::string& input, int line_num) {
                return line_renderer(input, line_num);
            }

            bool HandleBackspace() {
                using namespace ftxui;
                if (cursor_position() == 0) {
                    return false;
                }
                const size_t start = GlyphPrevious(content(), cursor_position());
                const size_t end = cursor_position();
                content->erase(start, end - start);
                cursor_position() = start;
                on_change();
                return true;
            }

            bool HandleTab() {
                return HandleCharacter(tab_string());
            }

            bool DeleteImpl() {
                using namespace ftxui;
                if (cursor_position() == (int)content->size()) {
                    return false;
                }
                const size_t start = cursor_position();
                const size_t end = GlyphNext(content(), cursor_position());
                content->erase(start, end - start);
                return true;
            }

            bool HandleDelete() {
                if (DeleteImpl()) {
                    on_change();
                    return true;
                }
                return false;
            }

            bool HandleArrowLeft() {
                using namespace ftxui;
                if (cursor_position() == 0) {
                    return false;
                }

                cursor_position() = GlyphPrevious(content(), cursor_position());
                return true;
            }

            bool HandleArrowRight() {
                using namespace ftxui;
                if (cursor_position() == (int)content->size()) {
                    return false;
                }

                cursor_position() = GlyphNext(content(), cursor_position());
                return true;
            }

            size_t CursorColumn() {
                using namespace ftxui;
                size_t iter = cursor_position();
                int width = 0;
                while (true) {
                    if (iter == 0) {
                        break;
                    }
                    iter = GlyphPrevious(content(), iter);
                    if (content()[iter] == '\n') {
                        break;
                    }
                    width += GlyphWidth(content(), iter);
                }
                return width;
            }

            // Move the cursor `columns` on the right, if possible.
            void MoveCursorColumn(int columns) {
                using namespace ftxui;
                while (columns > 0) {
                    if (cursor_position() == (int)content().size() ||
                        content()[cursor_position()] == '\n') {
                        return;
                    }

                    columns -= GlyphWidth(content(), cursor_position());
                    cursor_position() = GlyphNext(content(), cursor_position());
                }
            }

            bool HandleArrowUp() {
                using namespace ftxui;
                if (cursor_position() == 0) {
                    return false;
                }

                const size_t columns = CursorColumn();

                // Move cursor at the beginning of 2 lines above.
                while (true) {
                    if (cursor_position() == 0) {
                        return true;
                    }
                    const size_t previous = GlyphPrevious(content(), cursor_position());
                    if (content()[previous] == '\n') {
                        break;
                    }
                    cursor_position() = previous;
                }
                cursor_position() = GlyphPrevious(content(), cursor_position());
                while (true) {
                    if (cursor_position() == 0) {
                        break;
                    }
                    const size_t previous = GlyphPrevious(content(), cursor_position());
                    if (content()[previous] == '\n') {
                        break;
                    }
                    cursor_position() = previous;
                }

                MoveCursorColumn(columns);
                return true;
            }

            bool HandleArrowDown() {
                using namespace ftxui;
                if (cursor_position() == (int)content->size()) {
                    return false;
                }

                const size_t columns = CursorColumn();

                // Move cursor at the beginning of the next line
                while (true) {
                    if (content()[cursor_position()] == '\n') {
                        break;
                    }
                    cursor_position() = GlyphNext(content(), cursor_position());
                    if (cursor_position() == (int)content().size()) {
                        return true;
                    }
                }
                cursor_position() = GlyphNext(content(), cursor_position());

                MoveCursorColumn(columns);
                return true;
            }

            bool HandleHome() {
                cursor_position() = 0;
                return true;
            }

            bool HandleEnd() {
                cursor_position() = content->size();
                return true;
            }

            bool HandleReturn() {
                HandleCharacter("\n");
                on_enter();
                return true;
            }

            bool HandleCharacter(const std::string& character) {
                if (!insert() && cursor_position() < (int)content->size() &&
                    content()[cursor_position()] != '\n') {
                    DeleteImpl();
                }
                content->insert(cursor_position(), character);
                cursor_position() += character.size();
                on_change();
                return true;
            }

            bool OnEvent(ftxui::Event event) override {
                using namespace ftxui;
                cursor_position() = util::clamp(cursor_position(), 0, (int)content->size());

                if (event == Event::Return) {
                    return HandleReturn();
                }
                if (event.is_character()) {
                    return HandleCharacter(event.character());
                }
                if (event.is_mouse()) {
                    return HandleMouse(event);
                }
                if (event == Event::Tab) {
                    return HandleTab();
                }
                if (event == Event::Backspace) {
                    return HandleBackspace();
                }
                if (event == Event::Delete) {
                    return HandleDelete();
                }
                if (event == Event::ArrowLeft) {
                    return HandleArrowLeft();
                }
                if (event == Event::ArrowRight) {
                    return HandleArrowRight();
                }
                if (event == Event::ArrowUp) {
                    return HandleArrowUp();
                }
                if (event == Event::ArrowDown) {
                    return HandleArrowDown();
                }
                if (event == Event::Home) {
                    return HandleHome();
                }
                if (event == Event::End) {
                    return HandleEnd();
                }
                if (event == Event::ArrowLeftCtrl) {
                    return HandleLeftCtrl();
                }
                if (event == Event::ArrowRightCtrl) {
                    return HandleRightCtrl();
                }
                if (event == Event::Insert) {
                    return HandleInsert();
                }
                return false;
            }

            bool HandleLeftCtrl() {
                using namespace ftxui;
                if (cursor_position() == 0) {
                    return false;
                }

                // Move left, as long as left it not a word.
                while (cursor_position()) {
                    const size_t previous = GlyphPrevious(content(), cursor_position());
                    if (IsWordCharacter(content(), previous)) {
                        break;
                    }
                    cursor_position() = previous;
                }
                // Move left, as long as left is a word character:
                while (cursor_position()) {
                    const size_t previous = GlyphPrevious(content(), cursor_position());
                    if (!IsWordCharacter(content(), previous)) {
                        break;
                    }
                    cursor_position() = previous;
                }
                return true;
            }

            bool HandleRightCtrl() {
                using namespace ftxui;
                if (cursor_position() == (int)content().size()) {
                    return false;
                }

                // Move right, until entering a word.
                while (cursor_position() < (int)content().size()) {
                    cursor_position() = GlyphNext(content(), cursor_position());
                    if (IsWordCharacter(content(), cursor_position())) {
                        break;
                    }
                }
                // Move right, as long as right is a word character:
                while (cursor_position() < (int)content().size()) {
                    const size_t next = GlyphNext(content(), cursor_position());
                    if (!IsWordCharacter(content(), cursor_position())) {
                        break;
                    }
                    cursor_position() = next;
                }

                return true;
            }

            bool HandleMouse(ftxui::Event event) {
                using namespace ftxui;
                hovered_ = box_.Contain(event.mouse().x,  //
                    event.mouse().y) &&
                    CaptureMouse(event);
                if (!hovered_) {
                    return false;
                }

                if (event.mouse().button != Mouse::Left) {
                    return false;
                }
                if (event.mouse().motion != Mouse::Pressed) {
                    return false;
                }

                TakeFocus();

                if (content->empty()) {
                    cursor_position() = 0;
                    return true;
                }

                // Find the line and index of the cursor.
                std::vector<std::string> lines = Split(*content);
                int cursor_line = 0;
                int cursor_char_index = cursor_position();
                for (const auto& line : lines) {
                    if (cursor_char_index <= (int)line.size()) {
                        break;
                    }

                    cursor_char_index -= line.size() + 1;
                    cursor_line++;
                }
                const int cursor_column =
                    string_width(lines[cursor_line].substr(0, cursor_char_index));

                int new_cursor_column = cursor_column + event.mouse().x - cursor_box_.x_min;
                int new_cursor_line = cursor_line + event.mouse().y - cursor_box_.y_min;

                // Fix the new cursor position:
                new_cursor_line = std::max(std::min(new_cursor_line, (int)lines.size()), 0);

                const std::string empty_string;
                const std::string& line = new_cursor_line < (int)lines.size()
                    ? lines[new_cursor_line]
                    : empty_string;
                new_cursor_column = util::clamp(new_cursor_column, 0, string_width(line));

                if (new_cursor_column == cursor_column &&  //
                    new_cursor_line == cursor_line) {
                    return false;
                }

                // Convert back the new_cursor_{line,column} toward cursor_position:
                cursor_position() = 0;
                for (int i = 0; i < new_cursor_line; ++i) {
                    cursor_position() += lines[i].size() + 1;
                }
                while (new_cursor_column > 0) {
                    new_cursor_column -= GlyphWidth(content(), cursor_position());
                    cursor_position() = GlyphNext(content(), cursor_position());
                }

                on_change();
                return true;
            }

            bool HandleInsert() {
                insert() = !insert();
                return true;
            }

            bool Focusable() const final { return true; }

            bool hovered_ = false;

            ftxui::Box box_;
            ftxui::Box cursor_box_;
        };

    }
    ftxui::Component TextEditor(TextEditorOption option) {
        return ftxui::Make<TextEditorBase>(std::move(option));
    }
    ftxui::Component TextEditor(ftxui::StringRef content, TextEditorOption option) {
        option.content = std::move(content);
        return ftxui::Make<TextEditorBase>(std::move(option));
    }
    ftxui::Component TextEditor(ftxui::StringRef content, ftxui::StringRef placeholder, TextEditorOption option) {
        option.content = std::move(content);
        option.placeholder = std::move(placeholder);
        return ftxui::Make<TextEditorBase>(std::move(option));
    }

}  // namespace ftxui
