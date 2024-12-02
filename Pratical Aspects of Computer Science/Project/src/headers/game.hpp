enum Difficulty
{
    EASY,
    NORMAL,
    HARD,
    OH_LORD,

    Difficulty_COUNT
};

struct Game_Params
{
    string TITLE = "STARt";

    string MAP_SEED_STR = "0";

    int TICKS_PER_SECOND = 512;

    Difficulty DIFFICULTY = EASY;

    int TILE_SIZE = 120;
    int PLAYER_SIZE = 100;
    int MOB_SIZE = 110;
    int PROJECTILE_SIZE = 5;
    int MINIMAP_SIZE = 350;

    int SPAWN_PROTECTION_RANGE = 5;

    int MOB_COUNT_MAX = 60; //Per player, in render distance
    int MOB_SPAWN_PROTECTION_RANGE = 10;
    float MOB_SPEED_MAX = 1;
    float MOB_BOUNCE = 1;     //How much a mob bounces off walls, From 0.1 to infinite
    int MOB_ACTION_RANGE = 8;
    int MOB_ACTION_RANGE_IF_ALERT = 16;
    int ASTAR_SEARCH_FACTOR = 1;  //WARNING - This is computationally expensive

    int SHOOTER_RANGE = 4;
    float SHOOTER_SHOT_COOLDOWN = 128;
    int RUNNER_DAMAGE = 5;
    float RUNNER_KNOCKBACK = 1;
    int FLYER_DAMAGE = 15;
    float FLYER_KNOCKBACK = 1;

    int PROJECTILE_SPEED = 1;
    int PROJECTILE_TTL = 2048;
    int PROJECTILE_DAMAGE = 10;
    float PROJECTILE_KNOCKBACK = 1;

    float TILE_KNOCKBACK = 2;

    float PLAYER_SPEED_MAX = 1;
    float PLAYER_DEACCERATION = 2;
    float PLAYER_ACCELERATION = 1;
    float PLAYER_BONCE = 1;   //How much player bounces off walls, From 0.1 to infinite
    
    float DRAW_SCALE_MAX = 1;
    float DRAW_SCALE_MIN = 0.5;
    float ZOOM_SENS = 2;

    float RENDER_DISTANCE = 40;
    float MINIMAP_RENDER_DISTANCE = 30;

    float MAP_SPACE = 80; //80 to 100 is fine
    float MAP_CHAOS = 100; //40 to 60 is fine

    float BIOME_COLOR_ALPHA = 0.9;      //From 0 to 1
    int BIOME_COLOR_BLEND = 1;        //WARNING - This is computationally expensive
    float BIOME_COLOR_BLEND_RANDOM_UPDATES = 0.005; //How many random updates to do each draw (%)

    float STAR_RARITY = 0.0005; //From 0 to 1
};

struct Game_Instance 
{
    Game_Params params;

    Draw_Resources draw_resources;
    
    Map map;
    Player player;

    List<Mob> mobs_around;

    List<Projectile> projectiles;

    std::map<string, bool> animation_running;
    List<Animate_Params_Sprite> animation_running_sprite;

    bool key_pressed[256] = {false};
    bool key_clicked[256] = {false};
    float scroll_delta = 0;
    float scroll_speed = 0;
    float mouse_x = 0;
    float mouse_y = 0;
    bool mouse_pressed[2] = {false};
    bool mouse_clicked[2] = {false};

    float view_offset_x = 0;
    float view_offset_y = 0;
    float vis_a_x1 = 0, vis_a_y1 = 0, vis_a_x2 = 0, vis_a_y2 = 0; //Visible area
    float draw_scale = 1;
    float edge_dist = 0;
    float visibility_distance = 0;
    float necessary_distance = 0;

    long long int tick = 0;
    int past_tick_diff = 0;
    long long int run_start_tick = 0;
    long long int last_draw_time = 1;
    float fps = 0;
    int slow_fps = 0;

    bool game_paused = false;
    
    long long int time_winEvents = 0, time_player = 0, time_mobs = 0, time_projectiles = 0, time_gameEvents = 0, time_zoom = 0, time_view = 0, time_draw = 0, time_draw_last = 0;

    int current_menu = 0;

    bool window_resized = false;
    bool window_center_player = false;
    std::map<std::string, sf::View> views;
    List<std::string> view_names;

    float game_view_alpha = 0.3;
    float menu_view_alpha = 1;

    long long int current_highscore = 0;

    int start_count = 3;

    std::map<std::string, bool> music_playing;
    int music_volume = 100;
    float music_volume_f = 0.6;
    int walk_sound_counter = 0;

    bool remake_instance = false;   

    //TESTING
    bool debug = false;
    List<Test_Line> test_lines;

};

Game_Instance createGameInstance(Game_Params params);
void remakeGameInstance(Game_Instance &game, Game_Params params);
void newGame(Game_Instance &game);

Game_Params setDifficultyConfig(Game_Params params, Difficulty diff);

void loadHighscore(Game_Instance &game);
void setHighscore(Game_Instance &game, long int highscore);