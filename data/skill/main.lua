require "skill.base"
require "skill.func"

function skill.list.attack(level)
    return {
        desc = "A level-based attacking move.",

        power = 5,
        accuracy = 60,
        method = method.physical,

        perform = function(self, source, target)
            -- does damage equal to "skill_level * user_level"
            for _, entity in ipairs(target:getTeam()) do
                if skill.did_hit(self.accuracy, source, entity) then
                    entity:drainHP(math.random(1,10) + level * source.level)
                end
            end
        end
    }
end
