#include "kecs/bridge.hpp"

struct health
{
    int h_x, h_y;
};

struct position
{
    float p_x, p_y;
};

int main()
{
    spdlog::set_level(spdlog::level::debug);

    kecs::bridge kecs{};

    auto entity1 = kecs.create<position>({50, 80});
    auto entity2 = kecs.create<position, health>({500, 400}, {100, 90});
    auto entity3 = kecs.create<health>({2, 4});

    spdlog::debug("created component");

    for(auto [id, pos, health]: kecs.view<kecs::And<position, health>>())
    {
        spdlog::info("row: entity {} position({}, {}) health({}, {})", id, pos.p_x, pos.p_y, health.h_x, health.h_x);
    }

    kecs.add_component<position>(entity3, {100, 100});

    for(auto [id, pos, health]: kecs.view<kecs::And<position, health>>())
    {
        spdlog::info("row: entity {} position({}, {}) health({}, {})", id, pos.p_x, pos.p_y, health.h_x, health.h_y);
    }

    spdlog::debug("reached end");

    return 0;
}