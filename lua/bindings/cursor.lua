--- @meta

---  @class Core.Cursor
---  @field row integer Physical row in the Document.
--- @field col integer Physical column in the Document.
Core.Cursor = {}

--- Moves the Cursor up one row.
--- @param doc Core.Document
--- @param n integer amount of rows
function Core.Cursor:up(doc, n) end

--- Moves the Cursor down one row.
--- @param doc Core.Document
--- @param n integer amount of rows
function Core.Cursor:down(doc, n) end

--- Moves the Cursor left by one utf8 character.
--- @param doc Core.Document
--- @param n integer amount of characters
function Core.Cursor:left(doc, n) end

--- Moves the Cursor right by one utf8 character.
--- @param doc Core.Document
--- @param n integer amount of characters
function Core.Cursor:right(doc, n) end

--- Moves the Cursor to the next utf8 character.
--- @param doc Core.Document
--- @return boolean success
function Core.Cursor:step_forward(doc) end

--- Moves the Cursor to the previous utf8 character.
--- @param doc Core.Document
--- @return boolean success
function Core.Cursor:step_backward(doc) end

--- Returns the utf8 code-point of the next utf8 character.
--- @param doc Core.Document
--- @return integer code_point
function Core.Cursor:peek_forward(doc) end

--- Returns the utf8 code-point of the previous utf8 character.
--- @param doc Core.Document
--- @return integer code_point
function Core.Cursor:peek_backward(doc) end

--- Moves the Cursor to specific point in the Document.
--- @param doc Core.Document
--- @param point integer
function Core.Cursor:move_to(doc, point) end

--- Returns the point the Cursor in the Document is pointing at.
--- @param doc Core.Document
--- @return integer point
function Core.Cursor:point(doc) end

-- Predefined movement functions for performance.

function Core.Cursor:_jump_to_beginning_of_line(doc) end

function Core.Cursor:_jump_to_end_of_line(doc) end

function Core.Cursor:_jump_to_beginning_of_file(doc) end

function Core.Cursor:_jump_to_end_of_file(doc) end

function Core.Cursor:_next_word(doc, n) end

function Core.Cursor:_next_word_end(doc, n) end

function Core.Cursor:_prev_word(doc, n) end

function Core.Cursor:_prev_word_end(doc, n) end

function Core.Cursor:_next_whitespace(doc, n) end

function Core.Cursor:_prev_whitespace(doc, n) end

function Core.Cursor:_next_empty_line(doc, n) end

function Core.Cursor:_prev_empty_line(doc, n) end

function Core.Cursor:_jump_to_matching_opposite(doc) end
