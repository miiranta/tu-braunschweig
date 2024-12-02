enum Player_Direction
{
    P_UP,
    P_DOWN,
    P_LEFT,
    P_RIGHT
};

struct Effect
{
    int TTL = 0;
};

struct Player 
{
    int health = 100;

    List<Star> stars_collected;
    std::map<string, Effect> effects;

    float x = 0;
    float y = 0;

    float x_speed = 0;
    float y_speed = 0;

    Player_Direction direction_d = P_DOWN;
    float direction = 0;
    float minimap_direction = 0;

    float knockback_correction = 1;

    float draw_alpha = 1;

    int spawn_x = 0;
    int spawn_y = 0;
};
