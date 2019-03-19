#!/usr/bin/env lua

require "skills/skill"
require "skills/perk"

-- Create some skills

Skills:add{
    name = "attack",
    max_level = 2,

    power = 10,
    accuracy = 100,
    method = "physical",

    perform = function (self, source, target)
        local mod = self:modifier(source, target)
        local damage = self:baseDamage(source, target)
        target:drainHP(mod * damage)
    end,
}


Skills:add{
    name = "heal",

    mp_cost = 5,
    tech_cost = 1,

    power = 8,
    method = "magical",

    perform = function (self, source, target)
        local health = self.attr.power * (source.stats.m_atk + source.stats.m_def)
        target:restoreHP(health)
    end,
}


-- Print some details

a = Skills:get "heal"
a:addPerk("+hp")

b = Skills:get "heal"
b.mp_cost = b.mp_cost - 1
b.power = b.power + 2

c = Skills:get "attack"
c:levelUp()

d = Skills:get "attack"
d:addPerk("+power")
d:addPerk("-acc")

print("Name", "Level", "MP", "Power", "Perks")
print(string.rep("=======", 5, "|"))
function printDetails(skill)
    print(skill.name, skill.level, skill.mp_cost, skill.power, table.unpack(skill.perks))
end

printDetails(a)
printDetails(b)
printDetails(c)
printDetails(d)
