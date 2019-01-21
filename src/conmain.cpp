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

// TODO: do these actually work on Windows? This is so dodgy...
constexpr const char red_colour[]   = "\033[31m";
constexpr const char blue_colour[]  = "\033[36m";
constexpr const char bold_colour[]  = "\033[1m";
constexpr const char reset_colour[] = "\033[0m";

template <typename T, typename F>
T getInput(F is_valid, std::string_view errmsg = "\033[1;33mInvalid input!\n> \033[0m") {
    // T should be default-constructable
    T value{};

    while (!(std::cin >> value) || !is_valid(value)) {
        // end quickly if EOF called
        if (std::cin.eof()) {
            std::cout << "\nGoodbye!" << std::endl;
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

auto init() {
    using battle::Team;

    std::cout << bold_colour << "Welcome to the wonderful battle simulator!\n\n";

    int players = 0, enemies = 0;
    std::cout << bold_colour << "How many players?\n> " << reset_colour;
    players = getInput<int>();
    std::cout << bold_colour << "How many enemies?\n> " << reset_colour;
    enemies = getInput<int>();

    std::vector<std::shared_ptr<battle::Entity>> blue;
    for (int i = 0; i < players; ++i) {
        auto e = std::make_shared<battle::Entity>(
            std::string{blue_colour} + "default good #" +
                std::to_string(i + 1) + reset_colour, 1);
        e->assignController<battle::PlayerController>();
        blue.push_back(std::move(e));
    }

    std::vector<std::shared_ptr<battle::Entity>> red;
    for (int i = 0; i < enemies; ++i) {
        auto e = std::make_shared<battle::Entity>(
            std::string{red_colour} + "default evil #" +
                std::to_string(i + 1) + reset_colour, 1);
        e->assignController<battle::NPCController>();
        red.push_back(std::move(e));
    }

    return std::make_tuple(std::move(blue), std::move(red));
}

void drawEntity(const battle::Entity& entity) {
    const auto stats = entity.getStats();
    std::cout << "\"" << entity.getKind() << "\" level " << entity.getLevel() << " | "
              << "HP: " << entity.getHP() << "/" << stats.max_hp << " | "
              << "MP: " << entity.getMP() << "/" << stats.max_mp << " | "
              << "Tech: " << entity.getTech() << "/" << stats.max_tech << "\n";
}

void drawTeams(const battle::BattleSystem& system) {
    using battle::Team;
    std::cout << bold_colour << blue_colour;
    std::cout << "\nBlue team:\n" << std::string(60, '=') << reset_colour << "\n";
    for (auto entity : system.getTeam(Team::Blue))
        drawEntity(*entity);
    std::cout << bold_colour << red_colour;
    std::cout << "\nRed team:\n" << std::string(60, '=') << reset_colour << "\n";
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
        std::cout << entity.getKind() << " used " << s.skill->name
                  << " on " << s.target.getKind() << "!\n";
    }

    void operator()(battle::action::UntargetedSkill s) const noexcept {
        std::cout << entity.getKind() << " used " << s.skill->name << "!\n";
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
                std::cout << bold_colour;
                std::cout << "Choose skill (enter the number, 0 to defend):\n";
                for (auto&& skill : s)
                    std::cout << "  " << ++i << ". " << skill->name << "\n";
                std::cout << "> " << reset_colour;

                auto x = getInput<unsigned>([&](auto x){
                    return 0 <= x && x <= s.size();
                });
                return UntargetedSkill{ s[x - 1] };
            });
        choice.emplace_back('t', "S[t]ats", [&controller](){
            auto printStat = [](auto stat){ return std::string(stat, '*'); };
            auto& e = controller.getEntity();
            battle::Stats s = e.getStats();
            std::cout << bold_colour << "Stats for " << e.getKind() << ":\n";
            std::cout << reset_colour
                      << "HP:     " << s.max_hp << "/" << e.getHP() << "\n"
                      << "MP:     " << s.max_mp << "/" << e.getMP() << "\n"
                      << "Tech:   " << s.max_tech << "/" << e.getTech() << "\n"
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
        });

        std::cout << bold_colour << "\nWhat will " << entity.getKind() << " do?\n";
        for (auto&& [id, msg, fn] : choice)
            std::cout << " - " <<  msg << "\n";
        std::cout << "> " << reset_colour;

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

    std::cout << bold_colour << "Game over! Come back next time!\n" << std::flush;

    return 0;
}
