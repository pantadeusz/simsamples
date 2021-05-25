#ifndef ___MAIN_CLASS_FOR_BULLETHELL_HPP__
#define ___MAIN_CLASS_FOR_BULLETHELL_HPP__

class physical_c
{
public:
    std::array<double, 2> position;
    std::array<double, 2> velocity;
    std::array<double, 2> acceleration;
    double friction;

    /**
     * this function updates in place
     * */
    void update(double dt_f)
    {
        using namespace tp::operators;
        // apply friction:
        auto new_acceleration = acceleration - velocity * length(velocity) * friction;
        auto new_velocity = velocity + new_acceleration * dt_f;
        auto new_position = position + new_velocity * dt_f + new_acceleration * dt_f * dt_f * 0.5;
        position = new_position;
        velocity = new_velocity;
        acceleration = new_acceleration;
    }
};

class bullet_c : public physical_c
{
public:
    std::string type;
};

class emitter_c : public physical_c
{
public:
    double emit_to_emit; ///< time to emit
    double emit_delay;   ///< time between bullet emission
};

class player_c : public physical_c
{
public:
    std::map<std::string, int> intentions;

    double health;
    double points;

    player_c(std::array<double, 2> position_ = {10, 10}, std::array<double, 2> velocity_ = {0, 0}, std::array<double, 2> acceleration_ = {0, 0}, double friction_ = 0.03)
    {
        position = position_;
        velocity = velocity_;
        acceleration = acceleration_;
        friction = friction_;

        health = 100;
        points = 0;
    }

    /**
 * applies and clears intentions
 * */
    void apply_intent()
    {
        acceleration = {0, 30};
        if (intentions.count("right")) acceleration[0] += 100;
        if (intentions.count("left")) acceleration[0] += -100;
        if (intentions.count("up")) acceleration[1] += -100;
        if (intentions.count("down")) acceleration[1] += +100;
    }

    bool is_safe_place()
    {
        return ((position[0] < 4.0) && (position[1] > 30) && (position[0] > 0.0) && (position[1] < 33));
    }
};

class game_c
{
public:
    std::shared_ptr<SDL_Window> window_p;
    std::shared_ptr<SDL_Renderer> renderer_p;
    std::map<std::string, std::shared_ptr<SDL_Texture>> textures;
    std::vector<player_c> players;
    std::vector<bullet_c> bullets;
    std::vector<emitter_c> emitters;


    std::chrono::milliseconds dt;

    std::vector<std::map<std::string, int>> keyboard_map;
};

game_c initialize_all()
{
    game_c game;
    /// SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    /// WINDOW
    game.window_p = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow("Better Worms", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, 640, 360, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        [](auto* window) { SDL_DestroyWindow(window); });

    game.renderer_p = std::shared_ptr<SDL_Renderer>(
        SDL_CreateRenderer(game.window_p.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        [](auto* renderer) {
            SDL_DestroyRenderer(renderer);
        });

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderSetLogicalSize(game.renderer_p.get(), 640, 360);

    /// MEDIA
    for (int i = 0; i < 3; i++) {
        game.textures["player[" + std::to_string(i) + "]"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), (std::string("data/player") + std::to_string(i) + ".png").c_str()),
            [](auto* tex) { SDL_DestroyTexture(tex); });
    }
    for (int i = 0; i < 1; i++) {
        game.textures["bullet[" + std::to_string(i) + "]"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), (std::string("data/bullet") + std::to_string(i) + ".png").c_str()),
            [](auto* tex) { SDL_DestroyTexture(tex); });
    }
    for (int i = 0; i < 1; i++) {
        game.textures["emitter[" + std::to_string(i) + "]"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), (std::string("data/emitter") + std::to_string(i) + ".png").c_str()),
            [](auto* tex) { SDL_DestroyTexture(tex); });
    }
    game.textures["font_10"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/oqls65n.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    game.textures["font_10_red"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/oqls65n_red.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    game.textures["font_10_blue"] = std::shared_ptr<SDL_Texture>(IMG_LoadTexture(game.renderer_p.get(), "data/oqls65n_blue.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });
    /// PLAYERS
    game.players.push_back(player_c({0.2, 10}));
    game.players.push_back(player_c({1.5, 10}));

    /// EMITTERS

    emitter_c emitter;
    emitter.position = {60, 15};
    emitter.friction = 0;
    emitter.acceleration = {0, 0};
    emitter.velocity = {0, 0};
    emitter.emit_delay = 1;
    emitter.emit_to_emit = 5;
    game.emitters.push_back(emitter);

    /// physics details
    game.dt = std::chrono::milliseconds(15);

    /// keyboard mapping
    // player 0
    game.keyboard_map.push_back({{"right", SDL_SCANCODE_RIGHT},
        {"left", SDL_SCANCODE_LEFT},
        {"up", SDL_SCANCODE_UP},
        {"down", SDL_SCANCODE_DOWN}});
    // player 1
    game.keyboard_map.push_back({{"right", SDL_SCANCODE_D},
        {"left", SDL_SCANCODE_A},
        {"up", SDL_SCANCODE_W},
        {"down", SDL_SCANCODE_S}});
    // player 2


    return game;
}

#endif
