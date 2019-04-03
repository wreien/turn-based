require "skill.base"

function skill.list.attack(level)
    return {
        desc = "A level-based attacking move.",

        power = 5,
        method = method.physical,

        perform = function(self, source, target)
            -- does damage equal to "skill_level * user_level"
            target:drainHP(level * source:getLevel())
        end
    }
end
