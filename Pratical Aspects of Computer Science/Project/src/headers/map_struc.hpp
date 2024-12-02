struct Blend_Info
{
    int color[3] = {0, 0, 0};
};

enum Biome
{
    HOPE,
    LUST,
    GRUTTONY,
    PRIDE,
    SLOTH,
    WRATH,
    GREED,
    ENVY,

    BIOME_SIZE
};

enum Tile_Type
{
    GRASS,
    ROCK,
    SAND,
    ICE,

    TILE_TYPE_SIZE
};

enum Tile_Decoration
{
    NONE,
    TREE,
    TALL_GRASS,
    BUSH,
    BAD_BUSH,
    STAR,

    TILE_DECORATION_SIZE
};

struct Star
{
    string key;
};

struct Map_Tile
{
    string key; 
    bool notFound = false;

    long long int x;
    long long int y;

    Tile_Type type = GRASS;
    Tile_Decoration decoration = NONE;
    Biome biome = HOPE;

    int dec_offset_x = 0;
    int dec_offset_y = 0;

    int color[3] = {0, 0, 0};
    Blend_Info blend_info;

    float speed_modifier = 1; 
    int damage_modifier = 0;
    bool colision = false;

    bool is_concluded = false;

};

struct Map 
{
    AVL<Map_Tile> map;
    long int SEED;

    //Biome noisers
    std::map<Biome, OpenSimplexNoise::Noise*> biome_noisers;
    std::map<int, OpenSimplexNoise::Noise*> other_noisers;

    bool biomes_cleared[BIOME_SIZE] = {false};
};
