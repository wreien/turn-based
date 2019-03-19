-- Perk boilerplate

Perk = {}
Perk.__index = Perk

function Perk:new(p)
    p = p or {}
    setmetatable(p, self)
end

-- change the skill's stats appropriately
-- (feel free to add some bonus data to the skill in table `skill.perkdata[self]`
function Perk:apply(skill, perkdata)
end

-- revert the changes to the skill's stats
-- (feel free to clean up the perkdata, but I don't think it should matter)
function Perk:unapply(skill, perkdata)
end





StatAddPerk = {
    options = {
        hp_cost = 0,
        mp_cost = 0,
        tech_cost = 0,
        power = 0,
        accuracy = 0,
    }
}
setmetatable(StatAddPerk.options, { __index = StatAddPerk.options })

function StatAddPerk:apply(skill, data)
    for k, v in pairs(self.options) do
        skill[k] = skill[k] + v
    end
end

function StatAddPerk:unapply(skill, data)
    for k, v in pairs(self.options) do
        skill[k] = skill[k] - v
    end
end

function StatAddPerk:new(stats)
    local p = {}
    setmetatable(p, { __index = StatAddPerk })

    for k, v in pairs(stats) do
        if not options[k] then
            error('"' .. k .. '" is not a valid stat to add a value to.')
        end
        options[k] = v
    end

    return p
end

