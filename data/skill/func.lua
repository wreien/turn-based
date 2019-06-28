if skill == nil then skill = {} end

-- Skill helper functions

-- determine if a skill hit the target
-- params:
--   s = the skill being used
--   source = user of the skill
--   target = who the skill is aimed at
--   (optional) crit_difficulty = how hard to score a crit; higher is harder
-- returns:
--   0 if miss, 1 if hit, 2 if critical
function skill.did_hit(s, source, target, crit_difficulty)
    crit_difficulty = crit_difficulty or 6  -- set default value

    -- percentage chance of scoring a hit
    local hit_chance = s.accuracy + source.stats.skill - target.stats.evade

    -- function to see if we score a hit
    local did_score_hit = function(chance) return chance >= random(100) end

    -- if we don't score a hit, then we missed
    if not did_score_hit(hit_chance) then
        log(message.miss(target))
        return 0
    end

    -- test if we scored a critical hit
    if did_score_hit(hit_chance / crit_difficulty) then
        log(message.critical(target))
        return 2
    end

    -- just a normal hit
    return 1
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
        attack = source.stats.p_atk
        defense = target.stats.p_def
    elseif s.method == method.magical then
        attack = source.stats.m_atk
        defense = target.stats.m_def
    else
        error("raw_damage: skill.method must be physical or magical")
    end

    local variance = randf(0.8, 1.2)
    local raw = variance * (s.power / 100) * (4 * attack - 2 * defense)
    return math.max(raw, 0)  -- ensure positive damage at this stage
end

-- gets the resistance to the skill for an entity
-- params:
--   s = the skill to test the resistance for
--   entity = the entity to test the resistance of
-- returns a value suitable for use as a modifer
--   i.e, 20% resistance results in a return value of 0.8
function skill.resistance(s, entity)
    -- TODO: correctly calculate secondary elements
    local resist = entity.stats:resists(s.element)
    return -resist / 100 + 1
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
        local result = skill.did_hit(s, source, entity)
        if result ~= 0 then
            local raw = skill.raw_damage(s, source, entity)
            local mod = skill.resistance(s, entity)

            -- if we scored a crit, do double damage
            if result == 2 then
                mod = mod * 2
            end

            -- if this is semiaoe and they weren't our original target,
            -- we do a 70% modifier
            if s.spread == spread.semiaoe and entity == target then
                mod = mod * 0.7
            end

            -- deal the damage we intended to do
            entity:drainHealth(mod * raw)
        end
    end
end
