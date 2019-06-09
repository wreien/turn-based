if skill == nil then skill = {} end

-- Skill base functionality
skill.list = {}

-- make sure we're getting proper skills
local skilllist_mt = {}
setmetatable(skill.list, skilllist_mt)

-- the skill 's' with name 'name' can have a value 'value'; if so, it's type 'valtype'
local function optional_value_type(s, name, value, valtype)
    local t = type(s[value])
    assert(t == valtype or t == "nil", "'" .. name .. "." .. value ..
           "' must be of type '" .. valtype .. "'; got '" .. t .. "'.")
end

-- the skill 's' with name 'name' needs a value 'value' of type 'valtype'
local function require_value_type(s, name, value, valtype)
    assert(s[value], "'" .. name .. "' must provide '" .. value .. "'.")
    optional_value_type(s, name, value, valtype)
end

local function optional_value_enum(s, name, value, enum)
    local r = s[value]
    if r == nil then
        return
    end
    -- this is ugly, but it's because enums are read-only and hence don't
    -- actually contain their members for real; we look at the index instead.
    for _, v in pairs(getmetatable(enum).__index) do
        if r == v then return end
    end
    error("'" .. name .. '.' .. value .. "' must be a member of '" .. value .. "'.")
end

-- TODO: better way of warning? Can we link with stdout / provide a C++ function?
local function warn_fractional(s, name, value)
    local x = s[value]
    if x ~= nil then
        local _, frac = math.modf(s[value])
        assert(frac == 0, "'" .. name .. '.' .. value .. "' should be a whole number.")
    end
end

local function prepare(func)
    return function(...)
        -- get result
        local ret = func(...)

        -- set default values
        ret.max_level = ret.max_level or 1
        ret.method = ret.method or method.none
        ret.spread = ret.spread or spread.single
        ret.element = ret.element or element.neutral

        -- make read only and return it
        return setmetatable({}, {
            __index = ret,
            __newindex = function()
                error("table is read only!")
            end,
            __metatable = false
        })
    end
end

skilllist_mt.__newindex = function (table, key, value)
    if type(value) ~= "function" then
        error("'skill.list." .. key .. "' must be a function, got " .. type(value))
    end

    local s = value(1)  -- just test for level 1

    if type(s) ~= "table" then
        error("'skill.list." .. key .. "()" .. "' must return a table, got " .. type(s))
    end

    require_value_type(s, key, "desc", "string")
    require_value_type(s, key, "perform", "function")

    optional_value_type(s, key, "max_level", "number")

    optional_value_type(s, key, "hp_cost", "number")
    optional_value_type(s, key, "mp_cost", "number")
    optional_value_type(s, key, "tech_cost", "number")

    optional_value_type(s, key, "power", "number")
    optional_value_type(s, key, "accuracy", "number")

    optional_value_enum(s, key, "method", method)
    optional_value_enum(s, key, "spread", spread)
    optional_value_enum(s, key, "element", element)

    warn_fractional(s, key, "max_level")
    warn_fractional(s, key, "hp_cost")
    warn_fractional(s, key, "mp_cost")
    warn_fractional(s, key, "tech_cost")
    warn_fractional(s, key, "power")
    warn_fractional(s, key, "accuracy")

    -- can't do a direct assignment here; we'll just call __newindex again!
    rawset(table, key, prepare(value))
end


return skill
