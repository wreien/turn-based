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
    for (auto entity : system.getTeam(Team::Blue))
        drawEntity(*entity);
    std::cout << "\nRed team:\n" << std::string(60, '=') << "\n";
    for (auto entity : system.getTeam(Team::Red))
        drawEntity(*entity);
    std::cout << "\n";
}

struct TurnDrawer {
    TurnDrawer(const battle::Entity& entity) : entity{ entity } {}
    const battle::Entity& entity;

    void operator()(battle::action::Defend) const noexcept {
        std::cout << entity.getKind() << " is defending!\n";
    }

    void operator()(battle::action::Flee) const noexcept {
        std::cout << entity.getKind() << " attempted to flee!\n";
        // TODO actually test something here
        std::cout << entity.getKind() << " couldn't run!\n";
    }

    void operator()(battle::action::TargetedSkill s) const noexcept {
        std::cout << entity.getKind() << " used " << s.skill->getName()
                  << " on " << s.target.getKind() << "!\n";
    }

    void operator()(battle::action::UntargetedSkill s) const noexcept {
        std::cout << entity.getKind() << " used " << s.skill->getName() << "!\n";
    }

    void operator()(battle::action::UserChoice u) const noexcept {
        using namespace battle::action;

        std::vector<
            std::tuple<
                char,
                std::string,
                std::function<battle::Action()>
            >
        > choice;

        auto& controller = u.controller;
        auto options = controller.options();

        if (options.defend)
            choice.emplace_back('d', "[D]efend", [](){ return Defend{}; });
        if (options.flee)
            choice.emplace_back('f', "[F]lee", [](){ return Flee{}; });
        if (!options.skills.empty())
            choice.emplace_back('s', "[S]kill", [&s = options.skills](){
                int i = 0;
                std::cout << "Choose skill (enter the number, 0 to defend):\n";
                for (auto&& skill : s)
                    std::cout << "  " << ++i << ". " << skill->getName() << "\n";
                std::cout << "> ";

                auto x = getInput<unsigned>([&](auto x){
                    return 0 <= x && x <= s.size();
                });
                return UntargetedSkill{ s[x - 1] };
            });
        choice.emplace_back('t', "S[t]ats", [&controller](){
            using P = battle::Pool;
            auto printStat = [](auto stat){ return std::string(stat, '*'); };
            auto& e = controller.getEntity();
            battle::Stats s = e.getStats();
            std::cout << "Stats for " << e.getKind() << ":\n";
            std::cout << "HP:     " << s.max_hp << "/" << e.get<P::HP>() << "\n"
                      << "MP:     " << s.max_mp << "/" << e.get<P::MP>() << "\n"
                      << "Tech:   " << s.max_tech << "/" << e.get<P::Tech>() << "\n"
                      << "P. atk: " << printStat(s.p_atk) << "\n"
                      << "P. def: " << printStat(s.p_def) << "\n"
                      << "M. atk: " << printStat(s.m_atk) << "\n"
                      << "M. def: " << printStat(s.m_def) << "\n"
                      << "Skill:  " << printStat(s.skill) << "\n"
                      << "Evade:  " << printStat(s.evade) << "\n"
                      << "Speed:  " << printStat(s.speed) << "\n";
            // TODO: resistances
            return UserChoice{ controller };
        });
        choice.emplace_back('q', "[Q]uit", []() -> battle::Action {
            std::cout << "Goodbye!\n";
            std::exit(0);
            return {};
        });

        std::cout << "===\nWhat will " << entity.getKind() << " do?\n";
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
};

int main() {
    auto [blue, red] = init();
    battle::BattleSystem system(blue, red);
    drawTeams(system);

    while (!system.isDone()) {
        battle::TurnInfo info = system.doTurn();
        std::visit(TurnDrawer{ info.entity }, info.action);
    }

    std::cout << "Game over! Come back next time!\n" << std::flush;

    return 0;
}
