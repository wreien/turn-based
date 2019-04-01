-- Skill base functionality

SkillBase = {
    description = "",
    level = 1,
    max_level = 1,
    perks = {},
}
SkillBase.__index = SkillBase

function SkillBase:new(s)
    s = s or {}
    setmetatable(s, self)
    return s
end

-- perform a level up
function SkillBase:levelUp()
    if self.level < self.max_level then
        self.level = self.level + 1

        -- remove perks temporarily
        -- makes sure that level ups aren't affected by current perks
        for i, p in ipairs(self.perks) do
            p:unapply(self)
        end

        -- update my level
        self:updateLevel()

        -- reapply perks
        for i, p in ipairs(self.perks) do
            p:apply(self)
        end
    end
end

-- override to change stats upon levelling up
function SkillBase:updateLevel()
end




-- adds a perk to the skill
-- `perk' is the name of a perk given in `perks'
function SkillBase:addPerk(perk)
    p = self.perks[perk]
    if not p then
        error('"' .. perk .. '" is not a valid perk for skill "' .. self.name .. '"')
    end
    self.applied_perks[perk] = {}  -- can store bonus data here
    p:apply(self, self.applied_perks[perk])
end

function SkillBase:hasPerk(perk)
    return self.applied_perks[perk] ~= nil
end

function SkillBase:removePerk(perk)
    if not SkillBase:hasPerk(perk) then
        error("cannot remove a perk not applied")
    end
    p = self.perks[perk]
    p:unapply(self, self.applied_perks[perk])
    self.applied_perks[perk] = nil
end



-- Standard damage calculator
-- Should override/ignore if using an unusual method
function SkillBase:baseDamage(source, target)
    local attack = 0
    local defense = 0

    if self.method == method.physical then
        attack = source.stats.p_atk
        defense = target.stats.p_def
    elseif self.method == method.magical then
        attack = source.stats.m_atk
        defense = target.stats.m_def
    end

    return self.power * attack * 2 / (1 + defense)
end

function SkillBase:modifier(source, target)
    -- local mod = target.stats:getResistance(self.element)
    -- hook into perks/buffs
    return 1
end



-- perform this skill from source on target with team target_team
function SkillBase:perform(souce, target, target_team)
    -- default skill: does nothing (except the cost, of course)
    error("Should not call this!");
    return nil
end




-- Manage skill types

Skills = { skills = {} }

-- define a new skill type
function Skills:add(data)
    if not data.name then
        error("must provide a skill name")
    end
    if not data.desc then
        error("must provide a skill description (\"\" is fine though)")
    end
    -- TODO other validation checking

    local s = SkillBase:new(data)
    s.__index = s

    self.skills[data.name] = s
end

-- construct a new instance of a given skill
function Skills:get(name)
    local skill = self.skills[name]
    if not skill then
        error("unknown skill '" .. name .. "'")
    end
    return skill:new{ applied_perks = {} }
end
