if skill == nil then skill = {} end

-- Skill helper functions

-- determine if a skill hit the target
-- params:
--   s = the skill being used
--   source = user of the skill
--   target = who the skill is aimed at
-- returns:
--   true if hit, false otherwise
function skill.did_hit(s, source, target)
    local hit_chance = s.accuracy + source:stats().skill - target:stats().evade
    local random_var = random(100)

    local success = (hit_chance >= random_var)
    if not success then
        log(message.miss(target))
    end
    return success
end

-- calculate raw damage for a skill
-- params:
--   s = the skill being used
--   source = user of the skill
--   target = who the skill is aimed at
function skill.raw_damage(s, source, target)
    local attack
    local defense

    if s.method == method.physical then
        attack = source:stats().p_atk
        defense = target:stats().p_def
    elseif s.method == method.magical then
        attack = source:stats().m_atk
        defense = target:stats().m_def
    else
        error("raw_damage: skill.method must be physical or magical")
    end

    local variance = randf(0.8, 1.2)
    local raw = variance * (s.power / 100) * (4 * attack - 2 * defense)
    return math.max(raw, 0)  -- ensure positive damage at this stage
end


-- generate a 'perform' function that is often correct;
-- no support for any perks at the moment, but good for prototyping
-- can use this as an (overcomplicated) base for new specialised skills
function skill.default_perform(s, source, target)
    -- get who we are actually attacking
    -- is it just the direct target, or is this an AOE move?
    local target_list = { target }
    if s.spread == spread.aoe or s.spread == spread.semiaoe then
        target_list = target:getTeam()
    elseif s.spread == spread.field then
        error('unimplemented!')
    end

    -- loop over every target and see what happens
    for _, entity in ipairs(target_list) do
        -- test if we actually hit them
        if skill.did_hit(s, source, entity) then
            local raw = skill.raw_damage(s, source, entity)
            local mod = 1 -- TODO

            -- if this is semiaoe and they weren't our original target,
            -- we do a 70% modifier
            if s.spread == spread.semiaoe and entity == target then
                mod = mod * 0.7
            end

            -- deal the damage we intended to do
            entity:drainHP(mod * raw)
        end
    end
end
