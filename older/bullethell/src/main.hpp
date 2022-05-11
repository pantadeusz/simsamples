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

class obstacle_c {
public:
    std::array<double, 2> position;
    std::array<double, 2> size;
    std::string texture;
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

    std::vector<obstacle_c> obstacles;


    std::chrono::milliseconds dt;

    std::vector<std::map<std::string, int>> keyboard_map;


};

#endif
