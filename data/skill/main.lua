require "skill.base"
require "skill.perk"

Skills:add{
    name = "attack",
    desc = "Perform a basic attack.",

    power = 5,
    method = method.physical,

    perform = function(self, source, target)
        local mod = self:modifier(source, target)
        local damage = self:baseDamage(source, target)
        target:drainHP(math.ceil(mod * damage))
    end
}
