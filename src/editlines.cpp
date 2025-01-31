#include "editlines.h"
#include <sstream>
#include <climits>
#include <algorithm>
#include "config.h"

bool TextPosition::operator<(const TextPosition &other) const {
    if (other.row == row) {
        return col < other.col;
    }
    return row < other.row;
}
bool TextPosition::operator>(const TextPosition &other) const {
    return (other < *this);
}
bool TextPosition::operator<=(const TextPosition &other) const {
    return !(other < *this);
}
bool TextPosition::operator>=(const TextPosition &other) const {
    return !(*this < other);
}
bool TextPosition::operator==(const TextPosition &other) const {
    return col == other.col && row == other.row;
}
bool TextPosition::operator!=(const TextPosition &other) const {
    return !(*this == other);
}

unsigned EditStack::size() const {
    return data_size;
}

void EditStack::clear() {
    data_size = 0;
    pos = 0;
}

void EditStack::push(const EditAction &action) {
    if (data_size < BOX_UNDO_BUFFER_SIZE) {
        ++data_size;
    }
    if (pos == data.size()) {
        data.push_back(action);
    } else {
        data[pos] = action;
    }
    pos = (pos + 1) % BOX_UNDO_BUFFER_SIZE;
}

EditAction& EditStack::top() {
    return data[(pos + BOX_UNDO_BUFFER_SIZE - 1) % BOX_UNDO_BUFFER_SIZE];
}

EditAction EditStack::pop() {
    --data_size;
    EditAction res = std::move(top());
    pos = (pos + BOX_UNDO_BUFFER_SIZE - 1) % BOX_UNDO_BUFFER_SIZE;
    return res;
}

EditLines::EditLines(int64_t max_rows, int64_t max_cols, void (*change_callback)(TextPosition start, TextPosition end, int64_t removed, void*), void* aux) : lines(1, ""),
                                                   max_rows{static_cast<std::size_t>(max_rows == -1 ? INT64_MAX : max_rows)},
                                                   max_cols{static_cast<std::size_t>(max_cols == -1 ? INT64_MAX : max_cols)},
                                                   change_callback{change_callback},
                                                   aux_data{aux}{}

int64_t EditLines::line_size(int64_t row) const {
    return static_cast<int64_t>(lines[row].size());
}

void EditLines::delete_region(TextPosition start, TextPosition end) {
    if (start >= end) {
        return;
    }
    std::string first = lines[start.row];
    first.erase(start.col);
    std::string last = lines[end.row];
    last.erase(0, end.col);
    lines[start.row] = first + last;
    for (int64_t ix = 0; ix < lines.size() - (end.row + 1); ++ix) {
        lines[start.row + ix + 1] = lines[end.row + 1 + ix];
    }
    int64_t rem_lines = static_cast<int64_t>(lines.size()) - (end.row - start.row);
    lines.resize(rem_lines);
}

bool EditLines::insert_region(const std::string &str, TextPosition start, TextPosition end, EditAction &action) {
    int64_t new_rows = static_cast<int64_t>(std::count(str.begin(), str.end(), '\n'));
    if (lines.size() + new_rows - (end.row - start.row) > max_rows) {
        // Would add too many lines
        return false;
    }
    if (new_rows == 0) {
        std::string old = extract_region(start, end);
        bool split_delete = false;
        if (start.row == end.row) {
            if (str.size() + start.col + line_size(end.row) - end.col > max_cols) {
                // The first line would become too long
                return false;
            }
            delete_region(start, end);
        } else if (str.size() + start.col > max_cols) {
            // The first line would become too long
            return false;
        }  else if (str.size() + start.col + line_size(end.row) - end.col > max_cols) {
            delete_region(start, {start.row, line_size(start.row)});
            delete_region({start.row + 1, 0}, end);
            split_delete = true;
        } else {
            delete_region(start, end);
        }
        TextPosition start_pos = start;
        std::string s = lines[start.row];
        s.insert(start.col, str);
        lines[start.row] = s;
        start.col += static_cast<int64_t>(str.size());
        if (split_delete) {
            action = {start_pos, {start.row + 1, 0}, old};
        } else {
            action = {start_pos, start, old};
        }
    } else {
        size_t ix = str.find('\n');
        std::string first = str.substr(0, ix);
        if (start.col + first.size() > max_cols) {
            return false;
        }
        size_t last_ix = str.rfind('\n');
        std::string last = str.substr(last_ix + 1);
        if (line_size(end.row) - end.col + last.size() > max_cols) {
            return false;
        }
        std::string rem = str.substr(ix + 1, last_ix - ix);
        size_t offset = 0;
        for (int64_t i = 0; i < new_rows - 1; ++i) {
            size_t pos = rem.find('\n', offset);
            if (pos - offset > max_cols) {
                return false;
            }
            offset = pos + 1;
        }
        std::string old = extract_region(start, end);
        delete_region(start, end);
        std::string s = lines[start.row];
        std::string end_str = s.substr(start.col);
        s.erase(start.col);
        lines[start.row] = (s + first);
        offset = 0;
        for (int64_t i = 1; i <= new_rows; ++i) {
            lines.insert(lines.begin() + start.row + i, "");
        }
        for (int64_t i = 1; i < new_rows; ++i) {
            size_t pos = rem.find('\n', offset);
            lines[start.row + i] = (rem.substr(offset, pos - offset));
            offset = pos + 1;
        }
        TextPosition start_pos = start;
        lines[start.row + new_rows] = (last + end_str);
        start.row += new_rows;
        start.col = static_cast<int64_t>(last.size());
        action = {start_pos, start, old};
    }
    return true;
}

bool EditLines::move_left(TextPosition &pos, int64_t off) const {
    TextPosition old = pos;
    while (pos.col - off < 0) {
        if (pos.row == 0) {
            pos = old;
            return false;
        }
        off -= 1 + pos.col;
        --pos.row;
        pos.col = line_size(pos.row);
    }
    pos.col -= off;
    return true;
}

bool EditLines::move_right(TextPosition &pos, int64_t off) const {
    TextPosition old = pos;
    while (pos.col + off > line_size(pos.row)) {
        if (pos.row == lines.size() - 1) {
            pos = old;
            return false;
        }
        off -= 1 + line_size(pos.row) - pos.col;
        ++pos.row;
        pos.col = 0;
    }
    pos.col += off;
    return true;
}

bool EditLines::has_selection() const {
    return selection_start != selection_end;
}

void EditLines::clear_selection() {
    selection_start = cursor_pos;
    selection_end = cursor_pos;
    selection_base = cursor_pos;
}

void EditLines::set_cursor(TextPosition pos, bool select) {
    if (pos.row < 0) {
        pos.row = 0;
        pos.col = 0;
    } else if (pos.row >= lines.size()) {
        pos.row = static_cast<int64_t>(lines.size() - 1);
        pos.col = line_size(pos.row);
    } else if (pos.col > line_size(pos.row)) {
        pos.col = line_size(pos.row);
    }
    if (select) {
        if (!(pos >= selection_base)) {
            selection_start = pos;
            selection_end = selection_base;
        } else {
            selection_start = selection_base;
            selection_end = pos;
        }
        cursor_pos = pos;
    } else {
        cursor_pos = pos;
        clear_selection();
    }
}

void EditLines::move_cursor(TextPosition pos, bool select) {
    set_cursor(pos, select);
    clear_action();
}

const TextPosition &EditLines::get_cursor_pos() const {
    return cursor_pos;
}

const TextPosition &EditLines::get_selection_start() const {
    return selection_start;
}

const TextPosition &EditLines::get_selection_end() const {
    return selection_end;
}

const std::vector<std::string> &EditLines::get_lines() const {
    return lines;
}

void EditLines::set_selection(TextPosition start, TextPosition end, bool cursor_at_end) {
    if (end < start) {
        end = start;
    }
    if (cursor_at_end) {
        move_cursor(start, false);
        move_cursor(end, true);
    } else {
        move_cursor(end, false);
        move_cursor(start, true);
    }
}

void EditLines::clear_undo_stack() {
    undo_stack.clear();
    redo_stack.clear();
}

bool EditLines::insert_str(const std::string &str, EditType edit) {
    EditAction action;
    if (!insert_region(str, selection_start, selection_end, action)) {
        return false;
    }
    cursor_pos = action.end;
    clear_selection();

    if (edit != EditType::NONE && edit == edit_action) {
        action.chain = true;
    }
    if (edit == EditType::UNDO) {
        redo_stack.push(action);
    } else if (edit == EditType::REDO) {
        undo_stack.push(action);
    } else if (edit != edit_action || edit == EditType::NONE) {
        redo_stack.clear();
        undo_stack.push(action);
    } else {
        redo_stack.clear();
        if (undo_stack.size() == 0) {
            undo_stack.push(action);
        } else {
            if (undo_stack.top().end == action.start) {
                undo_stack.top().end = action.end;
                undo_stack.top().text += action.text;
            } else {
                undo_stack.push(action);
            }
        }
    }
    edit_action = edit;
    if (change_callback != nullptr) {
        change_callback(action.start, action.end, static_cast<int64_t>(action.text.size()), aux_data);
    }
    return true;
}


std::string EditLines::extract_region(TextPosition start,
                                      TextPosition end) const {
    if (start == end) {
        return "";
    }
    if (start.row == end.row) {
        return lines[start.row].substr(start.col,
                                                  end.col - start.col);
    } else {
        std::stringstream res{};
        res << lines[start.row].substr(start.col);
        for (int64_t row = start.row + 1; row < end.row; ++row) {
            res << '\n' << lines[row];
        }
        res << '\n' << lines[end.row].substr(0, end.col);
        return res.str();
    }
}
std::string EditLines::extract_selection() const {
    return extract_region(selection_start, selection_end);
}
void EditLines::clear_action() {
    edit_action = EditType::NONE;
}

void EditLines::undo_action(bool redo) {
    EditStack& stack = redo ? redo_stack : undo_stack;
    EditType edit = redo ? EditType::REDO : EditType::UNDO;
    if (stack.size() == 0) {
        return;
    }
    EditAction action;
    do {
        action = stack.pop();
        selection_start = action.start;
        selection_end = action.end;
        cursor_pos = selection_start;
        insert_str(action.text, edit);
    } while (action.chain);
    edit_action = EditType::NONE;
}

int64_t EditLines::line_count() const {
    return static_cast<int64_t>(lines.size());
}

char EditLines::char_at_pos(TextPosition pos) const {
    if (pos.col == line_size(pos.row)) {
        return '\n';
    }
    return lines[pos.row].at(pos.col);
}


#ifdef EDITLINE_TESTS
#include <iostream>

std::ostream& operator<<(std::ostream &os, TextPosition pos) {
    os << '(' << pos.row << ',' << pos.col << ')';
    return os;
}

template<class T1, class T2>
int assert_eq(T1 a, T2 b, int ix, int line) {
    if (a != b) {
        std::cout << "Test " << ix << " failed: " << a << " != " << b << " : row " << line << std::endl;
        return 0;
    }
    std::cout << "Test " << ix << " passed" << std::endl;
    return 1;
}

static int test_ix = 0;
static int passed_tests = 0;
#define ASSERT_EQ(a, b) if (assert_eq(a, b, test_ix++, __LINE__)) { passed_tests += 1; } else { goto end; }


int main() {
    EditLines lines{-1, 20, nullptr, nullptr};
    ASSERT_EQ(lines.get_lines().size(), 1)
    ASSERT_EQ(lines.get_lines()[0].size(), 0)
    ASSERT_EQ(lines.get_cursor_pos(), (TextPosition{0, 0}))

    ASSERT_EQ(lines.insert_str("This is a test"), true)
    ASSERT_EQ(lines.get_lines().size(), 1)
    ASSERT_EQ(lines.get_lines()[0], "This is a test")
    ASSERT_EQ(lines.get_cursor_pos(), (TextPosition{0, 14}))
    ASSERT_EQ(lines.get_selection_start(), lines.get_cursor_pos())
    ASSERT_EQ(lines.get_selection_end(), lines.get_cursor_pos())
    ASSERT_EQ(lines.has_selection(), false)

    ASSERT_EQ(lines.insert_str("123456789101112131415"), false)
    ASSERT_EQ(lines.get_lines().size(), 1)
    ASSERT_EQ(lines.get_lines()[0].size(), 14)
    ASSERT_EQ(lines.get_cursor_pos(), (TextPosition{0, 14}))
    ASSERT_EQ(lines.get_selection_start(), lines.get_cursor_pos())
    ASSERT_EQ(lines.get_selection_end(), lines.get_cursor_pos())
    ASSERT_EQ(lines.has_selection(), false)

    ASSERT_EQ(lines.insert_str("123456"), true)
    ASSERT_EQ(lines.get_lines().size(), 1)
    ASSERT_EQ(lines.get_lines()[0].size(), 20)
    ASSERT_EQ(lines.get_cursor_pos(), (TextPosition{0, 20}))
    ASSERT_EQ(lines.has_selection(), false)
    ASSERT_EQ(lines.get_lines()[0], "This is a test123456")

    ASSERT_EQ(lines.insert_str("\nLine 2"), true)
    ASSERT_EQ(lines.get_lines().size(), 2)
    ASSERT_EQ(lines.get_lines()[0], "This is a test123456")
    ASSERT_EQ(lines.get_lines()[1], "Line 2")
    ASSERT_EQ(lines.get_cursor_pos(), (TextPosition{1, 6}))
    ASSERT_EQ(lines.get_selection_start(), lines.get_cursor_pos())
    ASSERT_EQ(lines.get_selection_end(), lines.get_cursor_pos())
    ASSERT_EQ(lines.has_selection(), false)

end:
std::cout << passed_tests << " / " << test_ix << " tests passed" << std::endl;
}
#endif
