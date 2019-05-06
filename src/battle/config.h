#ifndef BATTLE_CONFIG_H_INCLUDED
#define BATTLE_CONFIG_H_INCLUDED

#include <tuple>
#include <vector>
#include <string>

namespace battle {

struct Stats;
class Skill;

[[nodiscard]] std::tuple<Stats, std::vector<Skill>>
getEntityDetails(const std::string& kind, const std::string& type, int level);

}

#endif // BATTLE_CONFIG_H_INCLUDED
