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
#include "conutil.h"
#include "overload.h"

// TODO: do these actually work on Windows? This is so dodgy...
constexpr const char red_colour[]    = "\033[31m";
constexpr const char yellow_colour[] = "\033[33m";
constexpr const char blue_colour[]   = "\033[36m";
constexpr const char bold_colour[]   = "\033[1m";
constexpr const char reset_colour[]  = "\033[0m";

template <typename T, typename F>
T getInput(F is_valid, std::string_view errmsg = "\033[1;33mInvalid input!\n> \033[0m") {
    // T should be default-constructable
    T value{};

    while (!(std::cin >> value) || !is_valid(value)) {
        // end quickly if EOF called
        if (std::cin.eof()) {
            std::cout << bold_colour << "\n\nGoodbye!" << std::endl;
            std::exit(0);
        }

        std::cout << errmsg;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // remove any extra input now
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return value;
}

template <typename T>
T getInput(std::string_view errmsg = "\033[1;33mInvalid input!\n> \033[0m") {
    return getInput<T>([](auto){ return true; }, errmsg);
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
                std::cout << bold_colour << "\nGoodbye!" << reset_colour << std::endl;
                std::exit(0);
            }
            std::getline(std::cin, line);
            std::istringstream iss { line };
            iss >> kind >> type;
            std::string path = "./data/entity/" + kind + "." + type + ".entity";
            std::ifstream in { path };
            if (!in) {
                std::cout << "Unknown entity [" << yellow_colour << kind 
                          << reset_colour << ", " << yellow_colour << type 
                          << reset_colour << "]. Try again: ";
            } else
                return std::make_pair(kind, type);
        }
    };

    std::vector<battle::EntityID> blue_ids;
    std::cout << bold_colour << "How many players?\n> " << reset_colour;
    int players = getInput<int>();

    for (int i = 0; i < players; i++) {
        std::cout << blue_colour << "Player #" << i + 1 << reset_colour << ": ";
        auto [kind, type] = get_kind_type();
        int count = ++seen_ids[kind + "\0" + type];
        blue_ids.push_back(gen_id(std::move(kind), std::move(type), count, false));
    }

    std::vector<battle::EntityID> red_ids;
    std::cout << bold_colour << "How many enemies?\n> " << reset_colour;
    int enemies = getInput<int>();

    for (int i = 0; i < enemies; i++) {
        std::cout << red_colour << "Enemy #" << i + 1 << reset_colour << ": ";
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

            // set colours
            switch (team) {
            case Team::Blue: id.name = blue_colour + id.name + reset_colour; break;
            case Team::Red:  id.name = red_colour  + id.name + reset_colour; break;
            }

            // set controllers (if applicable)
            auto e = loadEntity(std::move(id));
            if (team == Team::Blue)
                e->assignController<battle::PlayerController>();
            else
                e->assignController<battle::NPCController>();

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
    std::cout << bold_colour << blue_colour;
    std::cout << "\nBlue team:\n" << std::string(60, '=') << "\n" << reset_colour;
    for (auto entity : system.teamMembersOf(Team::Blue))
        drawEntity(*entity);
    std::cout << bold_colour << red_colour;
    std::cout << "\nRed team:\n" << std::string(60, '=') << "\n" << reset_colour;
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
            std::cout << bold_colour;
            std::cout << "Choose skill (enter the number, 0 to defend):\n";

            unsigned i = 0;
            for (auto&& skill : options.skills) {
                auto& details = skill->getDetails();
                std::cout << bold_colour << "  " << ++i << ". " << reset_colour
                          << yellow_colour << details.getName() << reset_colour;

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
            std::cout << bold_colour << "> " << reset_colour;

            auto skillchoice = getInput<unsigned>([&](auto x){
                return 0 <= x && x <= i;
            });

            if (skillchoice == 0)
                return Defend {};

            auto skill = options.skills[skillchoice - 1];

            // TODO: handle Spread::Self
            std::cout << bold_colour << "Choose target:\n";

            auto red_team = system.teamMembersOf(battle::Team::Red);
            auto blue_team = system.teamMembersOf(battle::Team::Blue);

            i = 0;
            std::cout << red_colour << bold_colour << "Red team:\n" << reset_colour;
            for (auto&& target : red_team) {
                if (!target->isDead()) {
                    std::cout << bold_colour << "  " << ++i << ". " << reset_colour;
                    drawEntity(*target);
                }
            }
            unsigned splitpoint = i;
            std::cout << blue_colour << bold_colour << "Blue Team:\n" << reset_colour;
            for (auto&& target : blue_team) {
                if (!target->isDead()) {
                    std::cout << bold_colour << "  " << ++i << ". " << reset_colour;
                    drawEntity(*target);
                }
            }
            std::cout << bold_colour << "> " << reset_colour;

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
        std::cout << bold_colour << "Stats for " << e.getID().name << ":\n";
        std::cout << reset_colour
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
            std::cout << bold_colour << "Applied status effects:\n" << reset_colour;
            for (auto&& se : effects) {
                std::cout << "  - " << se.getName();
                auto duration = se.getRemainingTurns();
                if (duration) {
                    std::cout << " (" << yellow_colour << *duration
                              << reset_colour << " turns remaining)";
                }
                std::cout << "\n";
            }
        }
        // TODO: resistances
        return UserChoice{ controller };
    });
    choice.emplace_back('q', "[Q]uit", []() -> battle::Action {
        std::cout << bold_colour << "\nGoodbye!\n";
        std::exit(0);
    });

    std::cout << bold_colour << "===\nWhat will "
              << controller.getEntity().getID().name << bold_colour
              << " do?\n" << reset_colour;

    for (auto&& [id, msg, fn] : choice)
        std::cout << " - " << msg << "\n";
    std::cout << bold_colour << "> " << reset_colour;

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
                      << yellow_colour << su.skill->getDetails().getName()
                      << reset_colour << " on " << su.target.getID().name << "!\n";
        },
        [](const Miss& m) {
            std::cout << m.entity.getID().name
                      << yellow_colour << " dodged " << reset_colour << "the attack!\n";
        },
        [](const Critical& c) {
            std::cout << c.entity.getID().name << " took a " 
                      << yellow_colour << "critical hit" << reset_colour << "!\n";
        },
        [](const PoolChanged& pc) {
            const auto name = pc.entity.getID().name;
            const auto diff = pc.new_value - pc.old_value;
            std::string poolname = to_string(pc.pool);
            if (diff < 0) {
                std::cout << name << " lost " << yellow_colour
                          << -diff << " " << poolname << reset_colour << "!\n";
            } else if (diff > 0) {
                std::cout << name << " restored " << yellow_colour
                          << diff << " " << poolname << reset_colour << "!\n";
            } else {
                std::cout << name << "'s " << poolname << " remained "
                          << yellow_colour << "unchanged" << reset_colour << ".\n";
            }
        },
        [](const StatusEffect& e) {
            const auto name = e.entity.getID().name;
            if (e.applied) {
                std::cout << name << " is now affected by " << yellow_colour
                          << e.effect << reset_colour << "!\n";
            } else {
                std::cout << name << "'s " << yellow_colour
                          << e.effect << reset_colour << " wore off.\n";
            }
        },
        [](const Defended& d) {
            std::cout << d.entity.getID().name << " is " << yellow_colour
                      << "defending" << reset_colour << "!\n";
        },
        [](const Fled& f) {
            std::cout << f.entity.getID().name << " attempted to flee";
            if (f.succeeded)
                std::cout << ", and " << yellow_colour
                          << "succeeded" << reset_colour << "!\n";
            else
                std::cout << "... but " << yellow_colour
                          << "failed" << reset_colour << ".\n";
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
    if (!enableVtEscapeCodes())
        return -1;

    std::cout << bold_colour
              << "Welcome to the wonderful battle simulator!\n\n" << reset_colour;

    auto system = std::make_from_tuple<battle::BattleSystem>(generateTeams());
    drawTeams(system);

    while (!system.isDone()) {
        battle::TurnInfo info = system.doTurn();
        for (const auto& m : info.messages)
            printMessage(m);
        if (info.need_user_input)
            handleUserChoice(*info.controller, system);
    }

    std::cout << bold_colour << "Game over! Come back next time!\n";
    std::cout << reset_colour << std::flush;

    return 0;
}
