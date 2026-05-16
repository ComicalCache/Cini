--- @meta

--- Faces define RGB colors of Cells drawn to the Display.
--- @class Core.Face
--- @field fg Core.Rgb? Foreground color.
--- @field bg Core.Rgb? Background color.
--- @field uc Core.Rgb? Underline color.
--- @field bold boolean
--- @field italic boolean
--- @field underline boolean
--- @field squiggly boolean
--- @field strikethrough boolean
Core.Face = {}

--- @class Core.FaceOptions
--- @field fg Core.Rgb?
--- @field bg Core.Rgb?
--- @field uc Core.Rgb?
--- @field bold boolean?
--- @field italic boolean?
--- @field underline boolean?
--- @field squiggly boolean?
--- @field strikethrough boolean?
local FaceOptions = {}

--- @param options? Core.FaceOptions
--- @return Core.Face
function Core.Face(options) end

--- Clones this object.
--- @return Core.Face
function Core.Face:clone() end
