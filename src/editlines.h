#ifndef PROCASM_EDITLINES_H
#define PROCASM_EDITLINES_H
#include <string>
#include <vector>
#include <cstdint>

/**
 * Struct for storing and comparing a row-collum position in the text.
 **/
struct TextPosition {
    int64_t row, col;

    bool operator<(const TextPosition& other) const;
    bool operator>(const TextPosition& other) const;
    bool operator<=(const TextPosition& other) const;
    bool operator>=(const TextPosition& other) const;
    bool operator==(const TextPosition& other) const;
    bool operator!=(const TextPosition& other) const;
};

/**
 * Struct representing a singe edit action performend on the text.
 **/
struct EditAction {
    // Position of fist character inserted by this action.
    TextPosition start {};
    // Position after last character inserted by this action.
    TextPosition end {};

    // The text deleted by this action.
    std::string text {};
    // Boolean indicating that this action is grouped together with
    // the following action in a stack.
    bool chain = false;
};

/**
 * A finite size stack of actions.
 **/
class EditStack {
public:
    /**
     * Return the number of elements in the stack.
     **/
    unsigned size() const;

    /**
     * Removes all elements from the stack.
     **/
    void clear();

    /**
     * Push an action to the stack, potentialy removing the oldest one. 
     *
     * @param action: the action to add 
     **/
    void push(const EditAction& action);

    /**
     * Peek at the top of the stack. Undefined behaviour if stack is empty.
     *
     * @return the top element of the stack.
     **/
    EditAction& top();

    /**
     * Pop the top element of the stack. Undefined behaviour if stack is empty.
     *
     * @return the previous top element of the stack.
     */
    EditAction pop();
private:
    unsigned pos {0}, data_size {0};
    std::vector<EditAction> data {};
};

/**
 * Enum with various edit action types.
 * NONE, UNDO and REDO have special meaning.
 **/
enum class EditType {
    NONE, UNDO, REDO, WRITE, DELETE, BACKSPACE, INSERT
};

/**
 * Class maintaining a list of editable lines.
 **/
class EditLines {
public:
    /**
     * EditLines constuctor.
     *
     * @param max_rows the max amount of rows allowed, or -1 for MAX_INT.
     * @param max_cols the max amount of collums allowed, or -1 for MAX_INT.
     * @param change_callback callback called whenever a change is made.
     * @param aux extra data to pass the to the callback.
     **/
    EditLines(int64_t max_rows, int64_t max_cols, void (*change_callback)(TextPosition start, TextPosition end, int64_t removed, void*), void* aux);

    /**
     * Moves the given position to the left by off, wrapping at line ends.
     * Does nothing if this is not possible.
     *
     * @param pos the TextPosition object to move.
     * @param off the number of steps to move.
     * @returns true if a move was done, false otherwise.
     **/
    bool move_left(TextPosition &pos, int64_t off) const;

    /**
     * Moves the given position to the right by off, wrapping at line ends.
     * Does nothing if this is not possible.
     *
     * @param pos the TextPosition object to move.
     * @param off the number of steps to move.
     * @returns true if a move was done, false otherwise.
     **/
    bool move_right(TextPosition &pos, int64_t off) const;

    /**
     * Returns true if any text is selected.
     *
     * @returns true if any text is selected, false otherwise.
     **/
    bool has_selection() const;

    /**
     * Moves the cursor to given position. If select is true, also moves
     * the selection. Otherwise, the selection is cleared.
     * Clears the last edit action.
     * If pos is out of bounds, it will be clamped within bounds.
     *
     * @param pos the new cursor position.
     * @param select true if selection should be made.
     **/
    void move_cursor(TextPosition pos, bool select);

    /**
     * Same as move_cursor, except does not clear last edit action.
     *
     * @param pos the new cursor position.
     * @param select true if selection should be made.
     **/
    void set_cursor(TextPosition pos, bool select);

    /**
     * Clears the current selection.
     **/
    void clear_selection();

    /**
     * Inserts a string, deleting any current selection.
     * Pushes the performend action onto the undo or redo stack.
     * If type is EditType::NONE, this action will not be chained.
     * EditType::UNDO and EditType::REDO must come from undo and redo stack.
     * Use undo_action instead.
     * All other EditType values will chain with the last action if it was of
     * the same type, meaning one undo unos both actions.
     *
     * @param str the string to insert.
     * @param type the type of edit.
     **/
    bool insert_str(const std::string& str, EditType type = EditType::NONE);

    /**
     * Gets the current cursor position. Will be the same as at least one of
     * get_selection_start and get_selection_end.
     *
     * @return the position of the cursor.
     **/
    const TextPosition& get_cursor_pos() const;

    /**
     * Gets the current selection start. Equal to get_selection_end when no
     * selection exists.
     *
     * @return the start of the current selection.
     **/
    const TextPosition& get_selection_start() const;

    /**
     * Gets the current selection end. Equal to get_selection_start when no
     * selection exists. This position is not part of the selection.
     *
     * @return the end of the current selection.
     **/
    const TextPosition& get_selection_end() const;

    /**
     * Sets the current to [start-end).
     * cursor_at_end indicates if the cursor should be placed at end or start.
     *
     * @param start the start of the selection.
     * @param end the end of the selection, >= start
     * @param cursor_at_end if true, cursor is placed at end.
     **/
    void set_selection(TextPosition start, TextPosition end, bool cursor_at_end);

    /**
     * Gets the current content of all lines.
     *
     * @return the current state of the lines.
     **/
    const std::vector<std::string> &get_lines() const;

    /**
     * Extracts a string from the selection. Line breaks become '\n'.
     *
     * @return the text contained in the selection.
     **/
    std::string extract_selection() const;

    /**
     * Clears the last action, causing the next action not to chain.
     **/
    void clear_action();

    /**
     * Undos one chain of actions, either from undo stack or redo stack.
     * Undo pushes action to redo stack. All other actions (except) redo
     * clears the redo stack, and pushes to undo stack.
     * Does nothing if stack is empty.
     *
     * @param redo true if redo stack should be used instead of undo stack.
     **/
    void undo_action(bool redo);

    /**
     * Returns the number of lines, equal to get_lines().size().
     *
     * @return the number of lines.
     **/
    int64_t line_count() const;

    /**
     * Returns the number of character in line row, equal to
     * get_lines()[row].size()
     *
     * @param row the row to get the size of.
     * @return the size of the row.
     **/
    int64_t line_size(int64_t row) const;

    /**
     * Returns the character at position pos.
     *
     * @param the position to check.
     * @return the character at pos, or '\n' if at line end.
     **/
    char char_at_pos(TextPosition pos) const;

    /**
     * Clears both the undo and redo stack.
     **/
    void clear_undo_stack();
private:
    void delete_region(TextPosition start, TextPosition end);

    std::string extract_region(TextPosition start, TextPosition end) const;

    bool insert_region(const std::string &str, TextPosition start, TextPosition end, EditAction& action);

    EditType edit_action {EditType::NONE};

    EditStack undo_stack {};
    EditStack redo_stack {};

    TextPosition cursor_pos {};

    TextPosition selection_start {};
    TextPosition selection_end {};
    TextPosition selection_base {};

    std::vector<std::string> lines {};

    void (*change_callback)(TextPosition start, TextPosition end, int64_t removed, void*);
    void* aux_data;

    std::size_t max_rows, max_cols;
};

#endif // PROCASM_EDITLINES_H
