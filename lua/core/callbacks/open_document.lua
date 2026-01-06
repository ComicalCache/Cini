local M = {}

function M.callback(doc)
    if doc.path and doc.path:find("%.[ch]pp$") then
        doc:set_major_mode("cpp")
    else
        doc:set_major_mode("text")
    end
end

return M
