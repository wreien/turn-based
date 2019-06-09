require "skill.base"
require "skill.func"

function skill.list.attack(level)
    return {
        desc = "A basic attacking move.",
        max_level = 5,

        power = 50 + 5 * (level - 1),
        accuracy = 70 + 5 * (level - 1),
        method = method.physical,

        -- just use the bog-standard damage calculations
        perform = skill.default_perform
    }
end
