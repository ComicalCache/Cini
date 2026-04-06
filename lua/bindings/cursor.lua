--- @meta

--- A Cursor is the virtual index into a Document. It manages the byte offsets into the Document as well as a preferred
--- column to keep the Cursor in one column while scrolling.
--- @class Core.Cursor
--- @field row integer Physical row in the Document.
--- @field col integer Physical column in the Document.
Core.Cursor = {}

--- Moves the Cursor up one row.
--- @param view Core.DocumentView
--- @param n integer amount of rows
function Core.Cursor:up(view, n) end

--- Moves the Cursor down one row.
--- @param view Core.DocumentView
--- @param n integer amount of rows
function Core.Cursor:down(view, n) end

--- Moves the Cursor left by one utf8 character.
--- @param view Core.DocumentView
--- @param n integer amount of characters
function Core.Cursor:left(view, n) end

--- Moves the Cursor right by one utf8 character.
--- @param view Core.DocumentView
--- @param n integer amount of characters
function Core.Cursor:right(view, n) end

--- Moves the Cursor to the next utf8 character.
--- @param view Core.DocumentView
--- @return boolean success
function Core.Cursor:step_forward(view) end

--- Moves the Cursor to the previous utf8 character.
--- @param view Core.DocumentView
--- @return boolean success
function Core.Cursor:step_backward(view) end

--- Returns the utf8 code-point of the next utf8 character.
--- @param view Core.DocumentView
--- @return integer|nil code_point
function Core.Cursor:peek_forward(view) end

--- Returns the utf8 code-point of the previous utf8 character.
--- @param view Core.DocumentView
--- @return integer|nil code_point
function Core.Cursor:peek_backward(view) end

--- Moves the Cursor to specific point in the Document.
--- @param view Core.DocumentView
--- @param point integer
function Core.Cursor:move_to(view, point) end

--- Returns the point the Cursor in the Document is pointing at.
--- @param view Core.DocumentView
--- @return integer point
function Core.Cursor:point(view) end

-- Predefined movement functions for performance.

function Core.Cursor:_jump_to_beginning_of_line(view) end

function Core.Cursor:_jump_to_end_of_line(view) end

function Core.Cursor:_jump_to_beginning_of_file(view) end

function Core.Cursor:_jump_to_end_of_file(view) end

function Core.Cursor:_next_word(view, n) end

function Core.Cursor:_next_word_end(view, n) end

function Core.Cursor:_prev_word(view, n) end

function Core.Cursor:_prev_word_end(view, n) end

function Core.Cursor:_next_whitespace(view, n) end

function Core.Cursor:_prev_whitespace(view, n) end

function Core.Cursor:_next_empty_line(view, n) end

function Core.Cursor:_prev_empty_line(view, n) end

function Core.Cursor:_jump_to_matching_opposite(view) end
