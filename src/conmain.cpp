#include <cstdlib>
#include <functional>
#include <iostream>
#include <locale>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>

#include "battle/battlesystem.h"
#include "battle/entity.h"
#include "battle/npccontroller.h"
#include "battle/playercontroller.h"
#include "util.h"

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

auto init() {
    using battle::Team;

    std::cout << "Welcome to the wonderful battle simulator!\n\n";

    int players = 0, enemies = 0;
    std::cout << "How many players?\n> ";
    players = getInput<int>();
    std::cout << "How many enemies?\n> ";
    enemies = getInput<int>();

    std::vector<std::shared_ptr<battle::Entity>> blue;
    for (int i = 0; i < players; ++i) {
        auto e = std::make_shared<battle::Entity>(
                "default good #" + std::to_string(i + 1), 1);
        e->assignController<battle::PlayerController>();
        blue.push_back(std::move(e));
    }

    std::vector<std::shared_ptr<battle::Entity>> red;
    for (int i = 0; i < enemies; ++i) {
        auto e = std::make_shared<battle::Entity>(
                "default evil #" + std::to_string(i + 1), 1);
        e->assignController<battle::NPCController>();
        red.push_back(std::move(e));
    }

    return std::make_tuple(std::move(blue), std::move(red));
}

void drawEntity(const battle::Entity& entity) {
    constexpr auto HP   = battle::Pool::HP;
    constexpr auto MP   = battle::Pool::MP;
    constexpr auto Tech = battle::Pool::Tech;

    std::cout << "\"" << entity.getKind() << "\" level " << entity.getLevel() << " | "
              << "HP: " << entity.get<HP>() << "/" << entity.getMax<HP>() << " | "
              << "MP: " << entity.get<MP>() << "/" << entity.getMax<MP>() << " | "
              << "Tech: " << entity.get<Tech>() << "/" << entity.getMax<Tech>() << "\n";
}

void drawTeams(const battle::BattleSystem& system) {
    using battle::Team;
    std::cout << "\nBlue team:\n" << std::string(60, '=') << "\n";
    for (auto entity : system.getEntities(Team::Blue))
        drawEntity(*entity);
    std::cout << "\nRed team:\n" << std::string(60, '=') << "\n";
    for (auto entity : system.getEntities(Team::Red))
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

            int i = 0;
            for (auto&& skill : options.skills) {
                std::cout << "  " << ++i << ". " << skill->getName();

                std::cout << " | elementid = "
                          << static_cast<int>(skill->getElement());

                if (auto power = skill->getPower())
                    std::cout << " | power = " << *power;
                if (auto accuracy = skill->getAccuracy())
                    std::cout << " | accuracy = " << *accuracy;

                {
                    std::cout << " | method = ";
                    using Method = battle::Skill::Method;
                    switch (skill->getMethod()) {
                    case Method::Physical: std::cout << "physical"; break;
                    case Method::Magical:  std::cout << "magical";  break;
                    case Method::Mixed:    std::cout << "mixed";    break;
                    case Method::None:     std::cout << "status";   break;
                    }
                }

                std::cout << " | cost = ";
                bool has_cost = false;
                if (auto hpc = skill->getHPCost(); hpc) {
                    has_cost = true;
                    std::cout << *hpc << " HP";
                }
                if (auto mpc = skill->getMPCost(); mpc) {
                    if (has_cost)
                        std::cout << " + ";
                    has_cost = true;
                    std::cout << *mpc << " MP";
                }
                if (auto tpc = skill->getTechCost(); tpc) {
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
                return 0 <= x && x <= options.skills.size();
            });

            if (skillchoice == 0)
                return Defend {};

            auto skill = options.skills[skillchoice - 1];

            // TODO: handle Spread::Self
            std::cout << "Choose target:\n";

            auto red_team = system.getEntities(battle::Team::Red);
            auto blue_team = system.getEntities(battle::Team::Blue);

            i = 0;
            std::cout << "Red team:\n";
            for (auto&& target : red_team) {
                std::cout << "  " << ++i << ". ";
                drawEntity(*target);
            }
            std::cout << "Blue Team:\n";
            for (auto&& target : blue_team) {
                std::cout << "  " << ++i << ". ";
                drawEntity(*target);
            }
            std::cout << "> ";

            auto targetchoice = getInput<unsigned>([&](auto x){
                return 0 < x && x <= red_team.size() + blue_team.size();
            });
            auto target = (targetchoice <= red_team.size()) ?
                    red_team[targetchoice - 1]
                  : blue_team[targetchoice - red_team.size() - 1];

            return Skill{ skill, *target };
        });
    choice.emplace_back('i', "[I]nfo", [&controller](){
        using P = battle::Pool;
        auto printStat = [](auto stat){ return std::string(stat, '*'); };
        auto& e = controller.getEntity();
        battle::Stats s = e.getStats();
        std::cout << "Stats for " << e.getKind() << ":\n";
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
        return {};
    });

    std::cout << "===\nWhat will " << controller.getEntity().getKind() << " do?\n";
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
            std::cout << su.source.getKind() << " used "
                      << su.skill->getName() << " on "
                      << su.target.getKind() << "!\n";
        },
        [](const PoolChanged& pc) {
            auto diff = pc.new_value - pc.old_value;
            std::string poolname = (pc.pool == battle::Pool::HP)
                ? "HP" : ((pc.pool == battle::Pool::MP) ? "MP" : "Tech");
            if (diff < 0) {
                std::cout << pc.entity.getKind() << " lost "
                          << -diff << " " << poolname << "!\n";
            } else if (diff > 0) {
                std::cout << pc.entity.getKind() << " restored "
                          << diff << " " << poolname << "!\n";
            } else {
                std::cout << pc.entity.getKind() << "'s "
                          << poolname << " remained unchanged.\n";
            }
        },
        [](const StatusEffect& e) {
            if (e.applied) {
                std::cout << e.entity.getKind() << " is now affected by "
                          << e.effect << "!\n";
            } else {
                std::cout << e.entity.getKind() << "'s "
                          << e.effect << " wore off.\n";
            }
        },
        [](const Defended& d) {
            std::cout << d.entity.getKind() << " is defending!\n";
        },
        [](const Fled& f) {
            std::cout << f.entity.getKind() << " attempted to flee";
            if (f.succeeded)
                std::cout << ", and succeeded!\n";
            else
                std::cout << "... but failed.";
        },
        [](const Notification& n) {
            std::cout << n.message << "\n";
        }
    }, m);
}

int main() {
    auto [blue, red] = init();
    battle::BattleSystem system(blue, red);
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
