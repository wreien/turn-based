#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <locale>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <unordered_map>

#include "battle/battlesystem.h"
#include "battle/entity.h"
#include "battle/config.h"
#include "battle/npccontroller.h"
#include "battle/playercontroller.h"
#include "overload.h"

template <typename T, typename F>
T getInput(F is_valid, std::string_view errormsg = "Invalid input!\n> ") {
    // T should be default-constructable
    T value{};

    while (!(std::cin >> value) || !is_valid(value)) {
        // end quickly if EOF called
        if (std::cin.eof()) {
            std::cout << "\nGoodbye!" << std::endl;
            std::exit(0);
        }

        std::cout << errormsg;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // remove any extra input now
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return value;
}

template <typename T>
T getInput(std::string_view errormsg = "Invalid input!\n> ") {
    return getInput<T>([](auto){ return true; }, errormsg);
}

std::shared_ptr<battle::Entity> loadEntity(battle::EntityID id) {
    using namespace battle;
    std::string path = "./data/entity/" + id.kind + "." + id.type + ".entity";
    std::ifstream in { path };
    if (!in) throw std::invalid_argument("couldn't open '" + path + "'.");

    Stats stats;
    std::vector<Skill> skills;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty())
            continue;

        std::istringstream iss{ line };
        std::string stat;
        iss >> stat;

        if (stat.empty() || stat[0] == '#') continue; // ignore comments
        else if (stat == "max_hp") iss >> stats.max_hp;
        else if (stat == "max_mp") iss >> stats.max_mp;
        else if (stat == "max_tech") iss >> stats.max_tech;
        else if (stat == "p_atk") iss >> stats.p_atk;
        else if (stat == "p_def") iss >> stats.p_def;
        else if (stat == "m_atk") iss >> stats.m_atk;
        else if (stat == "m_def") iss >> stats.m_def;
        else if (stat == "skill") iss >> stats.skill;
        else if (stat == "evade") iss >> stats.evade;
        else if (stat == "speed") iss >> stats.speed;
        else if (stat == "ability") {
            std::string skill_name;
            std::getline(iss >> std::ws, skill_name);
            skills.emplace_back(skill_name);
        } else
            throw std::invalid_argument(id.name + ": unknown key '" + stat + "'.");
    }

    auto test_stat = [](auto stat, std::string name) {
        if (stat <= 0)
            throw std::invalid_argument("bad value for '" + name + "'.");
    };
    test_stat(stats.max_hp, "max_hp");
    test_stat(stats.max_mp, "max_mp");
    test_stat(stats.max_tech, "max_tech");
    test_stat(stats.p_atk, "p_atk");
    test_stat(stats.p_def, "p_def");
    test_stat(stats.m_atk, "m_atk");
    test_stat(stats.m_def, "m_def");
    test_stat(stats.skill, "skill");
    test_stat(stats.evade, "evade");
    test_stat(stats.speed, "speed");

    return std::make_shared<Entity>(id, 1, stats, std::move(skills));
}

auto generateTeams() {
    using battle::Team;

    std::unordered_map<std::string, int> seen_ids;

    auto gen_id = [](std::string&& kind, std::string&& type, int count, bool do_1) {
        auto name = kind + " " + type;
        if (do_1 || count > 1)
            name += " #" + std::to_string(count);
        return battle::EntityID { std::move(kind), std::move(type), std::move(name) };
    };

    // can't just use getInput here: should really fix that
    auto get_kind_type = [] {
        std::string kind;
        std::string type;
        while (true) {
            std::string line;
            std::cin >> std::ws;
            if (std::cin.eof()) {
                std::cout << "\nGoodbye!" << std::endl;
                std::exit(0);
            }
            std::getline(std::cin, line);
            std::istringstream iss { line };
            iss >> kind >> type;
            std::string path = "./data/entity/" + kind + "." + type + ".entity";
            std::ifstream in { path };
            if (!in) {
                std::cout << "Unknown entity [" << kind << ", " << type << "]. "
                          << "Try again: ";
            } else
                return std::make_pair(kind, type);
        }
    };

    std::vector<battle::EntityID> blue_ids;
    std::cout << "How many players?\n> ";
    int players = getInput<int>();

    for (int i = 0; i < players; i++) {
        std::cout << "Player #" << i + 1 << ": ";
        auto [kind, type] = get_kind_type();
        int count = ++seen_ids[kind + "\0" + type];
        blue_ids.push_back(gen_id(std::move(kind), std::move(type), count, false));
    }

    std::vector<battle::EntityID> red_ids;
    std::cout << "How many enemies?\n> ";
    int enemies = getInput<int>();

    for (int i = 0; i < enemies; i++) {
        std::cout << "Enemy #" << i + 1 << ": ";
        auto [kind, type] = get_kind_type();
        int count = ++seen_ids[kind + "\0" + type];
        red_ids.push_back(gen_id(std::move(kind), std::move(type), count, false));
    }

    // converts an EntityID to an Entity; NOTE destroys `id' (assume xvalue)
    auto to_entity = [&seen_ids,gen_id](Team team) {
        return [&seen_ids,gen_id,team](battle::EntityID& id) {
            // catch those ids we missed adding numbers to in the first insertion
            auto test_str = id.kind + "\0" + id.type;
            if (seen_ids[test_str] > 1) {
                id = gen_id(std::move(id.kind), std::move(id.type), 1, true);
                seen_ids[test_str] = 0; // but we only want to change the first one
            }

            // set controllers (if applicable)
            auto e = loadEntity(std::move(id));
            if (team == Team::Blue)
                e->assignController<battle::PlayerController>();

            return e;
        };
    };

    // Could reserve, but really don't need to
    std::vector<std::shared_ptr<battle::Entity>> blue_entities;
    std::vector<std::shared_ptr<battle::Entity>> red_entities;

    // TODO: C++20 ranges to get an idiomatic destructive transform (I hope)
    std::transform(std::begin(blue_ids), std::end(blue_ids),
                   std::back_inserter(blue_entities), to_entity(Team::Blue));
    std::transform(std::begin(red_ids), std::end(red_ids),
                   std::back_inserter(red_entities), to_entity(Team::Red));

    return std::make_pair(blue_entities, red_entities);
}

void drawEntity(const battle::Entity& entity) {
    constexpr auto HP   = battle::Pool::HP;
    constexpr auto MP   = battle::Pool::MP;
    constexpr auto Tech = battle::Pool::Tech;

    std::cout << "\"" << entity.getID().name << "\" "
              << "level " << entity.getLevel() << " | "
              << "HP: " << entity.get<HP>() << "/" << entity.getMax<HP>() << " | "
              << "MP: " << entity.get<MP>() << "/" << entity.getMax<MP>() << " | "
              << "Tech: " << entity.get<Tech>() << "/" << entity.getMax<Tech>() << "\n";
}

void drawTeams(const battle::BattleSystem& system) {
    using battle::Team;
    std::cout << "\nBlue team:\n" << std::string(60, '=') << "\n";
    for (auto entity : system.teamMembersOf(Team::Blue))
        drawEntity(*entity);
    std::cout << "\nRed team:\n" << std::string(60, '=') << "\n";
    for (auto entity : system.teamMembersOf(Team::Red))
        drawEntity(*entity);
    std::cout << "\n";
}

// TODO: would probably split up the options placing
// also make this simpler to loop/re-enter, etc.
void handleUserChoice(battle::PlayerController& controller,
                      const battle::BattleSystem& system)
{
    using namespace battle::action;

    std::vector<
        std::tuple<
            char,
            std::string,
            std::function<battle::Action()>
        >
    > choice;

    auto options = controller.options();

    if (options.defend)
        choice.emplace_back('d', "[D]efend", [](){ return Defend{}; });
    if (options.flee)
        choice.emplace_back('f', "[F]lee", [](){ return Flee{}; });
    if (!options.skills.empty())
        choice.emplace_back('s', "[S]kill", [&]() -> battle::Action {
            std::cout << "Choose skill (enter the number, 0 to defend):\n";

            unsigned i = 0;
            for (auto&& skill : options.skills) {
                auto& details = skill->getDetails();
                std::cout << "  " << ++i << ". " << details.getName();

                std::cout << " | elementid = "
                          << static_cast<int>(details.getElement());

                if (auto power = details.getPower())
                    std::cout << " | power = " << *power;
                if (auto accuracy = details.getAccuracy())
                    std::cout << " | accuracy = " << *accuracy;

                {
                    std::cout << " | method = ";
                    using Method = battle::SkillMethod;
                    switch (details.getMethod()) {
                    case Method::Physical: std::cout << "physical"; break;
                    case Method::Magical:  std::cout << "magical";  break;
                    case Method::Mixed:    std::cout << "mixed";    break;
                    case Method::None:     std::cout << "status";   break;
                    }
                }

                std::cout << " | cost = ";
                bool has_cost = false;
                if (auto hpc = details.getHPCost(); hpc) {
                    has_cost = true;
                    std::cout << *hpc << " HP";
                }
                if (auto mpc = details.getMPCost(); mpc) {
                    if (has_cost)
                        std::cout << " + ";
                    has_cost = true;
                    std::cout << *mpc << " MP";
                }
                if (auto tpc = details.getTechCost(); tpc) {
                    if (has_cost)
                        std::cout << " + ";
                    has_cost = true;
                    std::cout << *tpc << " TP";
                }
                if (!has_cost)
                    std::cout << "none";

                std::cout << "\n";
            }
            std::cout << "> ";

            auto skillchoice = getInput<unsigned>([&](auto x){
                return 0 <= x && x <= i;
            });

            if (skillchoice == 0)
                return Defend {};

            auto skill = options.skills[skillchoice - 1];

            // TODO: handle Spread::Self
            std::cout << "Choose target:\n";

            auto red_team = system.teamMembersOf(battle::Team::Red);
            auto blue_team = system.teamMembersOf(battle::Team::Blue);

            i = 0;
            std::cout << "Red team:\n";
            for (auto&& target : red_team) {
                if (!target->isDead()) {
                    std::cout << "  " << ++i << ". ";
                    drawEntity(*target);
                }
            }
            unsigned splitpoint = i;
            std::cout << "Blue Team:\n";
            for (auto&& target : blue_team) {
                if (!target->isDead()) {
                    std::cout << "  " << ++i << ". ";
                    drawEntity(*target);
                }
            }
            std::cout << "> ";

            auto targetchoice = getInput<unsigned>([&](auto x){
                return 1 <= x && x <= i;
            });
            auto target = (targetchoice <= splitpoint) ?
                    red_team[targetchoice - 1]
                  : blue_team[targetchoice - splitpoint - 1];

            return Skill{ skill, *target };
        });
    choice.emplace_back('i', "[I]nfo", [&controller](){
        using P = battle::Pool;
        const auto printStat = [](auto stat) {
            return std::string(static_cast<unsigned>(stat), '*');
        };

        auto& e = controller.getEntity();
        battle::Stats s = e.getStats();
        std::cout << "Stats for " << e.getID().name << ":\n";
        std::cout
            << "  - HP:     " << e.get<P::HP>() << "/" << e.getMax<P::HP>() << "\n"
            << "  - MP:     " << e.get<P::MP>() << "/" << e.getMax<P::MP>() << "\n"
            << "  - Tech:   " << e.get<P::Tech>() << "/" << e.getMax<P::Tech>() << "\n"
            << "  - P. atk: " << printStat(s.p_atk) << "\n"
            << "  - P. def: " << printStat(s.p_def) << "\n"
            << "  - M. atk: " << printStat(s.m_atk) << "\n"
            << "  - M. def: " << printStat(s.m_def) << "\n"
            << "  - Skill:  " << printStat(s.skill) << "\n"
            << "  - Evade:  " << printStat(s.evade) << "\n"
            << "  - Speed:  " << printStat(s.speed) << "\n";
        auto& effects = e.getAppliedStatusEffects();
        if (!effects.empty()) {
            std::cout << "Applied status effects:\n";
            for (auto&& se : effects) {
                std::cout << "  - " << se.getName();
                auto duration = se.getRemainingTurns();
                if (duration)
                    std::cout << " (" << *duration << " turns remaining)";
                std::cout << "\n";
            }
        }
        // TODO: resistances
        return UserChoice{ controller };
    });
    choice.emplace_back('q', "[Q]uit", []() -> battle::Action {
        std::cout << "Goodbye!\n";
        std::exit(0);
    });

    std::cout << "===\nWhat will " << controller.getEntity().getID().name << " do?\n";
    for (auto&& [id, msg, fn] : choice)
        std::cout << " - " <<  msg << "\n";
    std::cout << "> ";

    auto c = getInput<char>([&](auto c){
        for (auto&& [id, msg, fn] : choice)
            if (std::tolower(c, std::locale{}) == id) return true;
        return false;
    });
    for (auto&& [id, msg, fn] : choice)
        if (std::tolower(c, std::locale{}) == id)
            controller.choose(fn());
}

void printMessage(const battle::Message& m) {
    using namespace battle::message;
    std::visit(overload{
        [](const SkillUsed& su) {
            std::cout << su.source.getID().name << " used "
                      << su.skill->getDetails().getName() << " on "
                      << su.target.getID().name << "!\n";
        },
        [](const PoolChanged& pc) {
            const auto name = pc.entity.getID().name;
            const auto diff = pc.new_value - pc.old_value;
            std::string poolname = (pc.pool == battle::Pool::HP)
                ? "HP" : ((pc.pool == battle::Pool::MP) ? "MP" : "Tech");
            if (diff < 0) {
                std::cout << name << " lost "
                          << -diff << " " << poolname << "!\n";
            } else if (diff > 0) {
                std::cout << name << " restored "
                          << diff << " " << poolname << "!\n";
            } else {
                std::cout << name << "'s "
                          << poolname << " remained unchanged.\n";
            }
        },
        [](const StatusEffect& e) {
            const auto name = e.entity.getID().name;
            if (e.applied) {
                std::cout << name << " is now affected by "
                          << e.effect << "!\n";
            } else {
                std::cout << name << "'s "
                          << e.effect << " wore off.\n";
            }
        },
        [](const Defended& d) {
            std::cout << d.entity.getID().name << " is defending!\n";
        },
        [](const Fled& f) {
            std::cout << f.entity.getID().name << " attempted to flee";
            if (f.succeeded)
                std::cout << ", and succeeded!\n";
            else
                std::cout << "... but failed.";
        },
        [](const Died& d) {
            std::cout << d.entity.getID().name << " died!\n";
        },
        [](const Notification& n) {
            std::cout << n.message << "\n";
        }
    }, m);
}

int main() {
    std::cout << "Welcome to the wonderful battle simulator!\n\n";

    auto system = std::make_from_tuple<battle::BattleSystem>(generateTeams());
    drawTeams(system);

    while (!system.isDone()) {
        battle::TurnInfo info = system.doTurn();
        for (const auto& m : info.messages)
            printMessage(m);
        if (info.need_user_input)
            handleUserChoice(*info.controller, system);
    }

    std::cout << "Game over! Come back next time!\n" << std::flush;

    return 0;
}
