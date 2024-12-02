enum Mob_Type
{
    RUNNER,
    SHOOTER,
    FLYER,

    MOB_TYPE_SIZE
};

enum Mob_Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Mob 
{
    string key;

    Mob_Type type = (Mob_Type)0;
    int health = 100;
    
    bool colision = true;
    bool is_alert = false;

    float x = 0;
    float y = 0;

    float x_speed = 0;
    float y_speed = 0;

    Mob_Direction last_dir = (Mob_Direction)0;

    int cooldown = 0;
};

struct Next_Tile
{
    float x = 0;
    float y = 0;

    bool reachable = false;
    bool keep_position = false;

    float cost = 0;
    float heuristic = 0;

    int path = 0; //This is the reference to the next tile in the List

};