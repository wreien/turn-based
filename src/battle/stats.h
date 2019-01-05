#ifndef BATTLE_STATS_H_INCLUDED
#define BATTLE_STATS_H_INCLUDED


namespace battle {

// the relevant stats for an entity
struct Stats {
    // housekeeping
    int level;
    int exp_to_next;

    // pools for health, magic, and skills respectively
    int max_hp;
    int cur_hp;

    int max_mp;
    int cur_mp;

    int max_tech;
    int cur_tech;

    // modifiers for physical and magical damage
    int p_atk;
    int p_def;

    int m_atk;
    int m_def;

    // hit chance
    double skill;
    double evade;

    // turn order
    int speed;

    // TODO: base resistances
    // TODO: status effects
};

}


#endif // BATTLE_STATS_H_INCLUDED
