require "skill.base"
require "skill.func"

function skill.list.attack(level)
    assert(level == 1)
    return {
        desc = "A basic attacking move.",

        power = 50,
        accuracy = 60,
        method = method.physical,

        perform = function(self, source, target)
            -- does damage equal to "skill_level * user_level"
            for _, entity in ipairs(target:getTeam()) do
                if skill.did_hit(self.accuracy, source, entity) then
                    local raw = skill.raw_damage(self, source, entity)
                    entity:drainHP(raw)
                end
            end
        end
    }
end
