//Map handling
Map createMap(string seed)
{
    Map map = Map();

    map.map = AVL<Map_Tile>();

    map.SEED = hashMapSeed(seed);

    //Biome noisers
    for(int i = 0; i < BIOME_SIZE; i++)
    {
        OpenSimplexNoise::Noise* noise = new OpenSimplexNoise::Noise(map.SEED + i);
        map.biome_noisers[(Biome)i] = noise;
    }

    //Other noisers
    for(int i = 0; i < TILE_DECORATION_SIZE + TILE_TYPE_SIZE; i++)
    {
        OpenSimplexNoise::Noise* noise = new OpenSimplexNoise::Noise(map.SEED + BIOME_SIZE + i);
        map.other_noisers[i] = noise;
    }

    map.biomes_cleared[HOPE] = true;

    return map;
}

void destroyMap(Map &map)
{
    //Don't need to destroy AVL, it's automatic - doing it causes a segfault
    //map.map.~AVL();
    return;
}

long int hashMapSeed(string seed)
{
    long int hash = 0;
    for(int i = 0; i < seed.size(); i++)
    {
        hash = seed[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

//Map handling - High-level getTile
Map_Tile getMapTile(int x, int y, Game_Instance &game)
{
    Map_Tile tile;

    //Is it already created?
    tile = AVL_search(x, y, game);

    //YES - load it from map
    if(tile.notFound == false)
    {
        tile = updateOutdatedTile(tile, game);
        return tile;
    }

    //NO - create it
    tile = generateMapTile(x, y, game);
    tile = updateOutdatedTile(tile, game);
    AVL_insert(tile, game);
    return tile;
}

void updateMapTile(int x, int y, Map_Tile tile, Game_Instance &game)
{
    AVL_insert(tile, game);
    return;
}

//Map handling - Generation
Map_Tile generateMapTile(int x, int y, Game_Instance &game)
{   
    srand(game.map.SEED*(x+y*100));
    
    //Generates new tile and calls addMapTile
    Map_Tile tile = Map_Tile();
    
    //AVL Key
    tile.key = std::to_string(x) + "." + std::to_string(y);
    tile.notFound = false;

    //Coords
    tile.x = x;
    tile.y = y;

    //Blend info
    tile.blend_info = Blend_Info();

    //Biome
    tile.biome = setBiome(x, y, game);

    //Tile type
    tile.type = setTileType(x, y, game);

    //Tile decoration
    tile.decoration = setTileDecoration(tile, game);
    
    //Tile properties
    tile = setTileProperties(tile, game);

    //Tile color
    tile = setTileColor(tile, game);

    //Offset
    float smin = -game.params.TILE_SIZE / 8;
    float smax = game.params.TILE_SIZE / 8;
    tile.dec_offset_x = ((float)rand() / RAND_MAX) * (smax - smin) + smin;
    tile.dec_offset_y = ((float)rand() / RAND_MAX) * (smax - smin) + smin;

    return tile;
}

Biome setBiome(int x, int y, Game_Instance &game)
{
    Biome biome = (Biome)1;
    
    //Simplex noise
    float space = game.params.MAP_SPACE;        //How big are the biomes
    float chaos = game.params.MAP_CHAOS;        //How chaotic are the biomes spawn
 
    int num_biomes = (int)BIOME_SIZE;

    float* noise = new float[num_biomes];
    for(int i = 0; i < num_biomes; i++)
    {  
        double noise_x = (x / (space));
        double noise_y = (y / (space));
        
        OpenSimplexNoise::Noise noise_gen = *game.map.biome_noisers[(Biome)i];
        noise[i] = noise_gen.eval(noise_x, noise_y);
    }

    //Even out biomes
    //Avarage
    float biome_i_noise[num_biomes] = {0};
    int blend_batch = ceil(num_biomes - 1);
    for(int i = 0; i < num_biomes; i++) //Average of biome_i_noise
    {
        for(int j = 0; j < blend_batch; j++)
        {
            biome_i_noise[i] = biome_i_noise[i] + noise[(i + j) % num_biomes];
        }
    }

    //Add chaos
    //NOISE Rand
    for(int i = 0; i < num_biomes; i++) //Average of biome_i_noise  
    {
        float smax = 1;
        float smin = -1;
        biome_i_noise[i] = biome_i_noise[i] + (smin + rand() / (RAND_MAX / (smax - smin + 1) + 1)) * (chaos * 0.0001);
    }

    //Set biome
    //MAX noise
    for(int i = 0; i < num_biomes; i++) //Set biome to MAX of biome_i_noise
    {
        if(biome_i_noise[i] > biome_i_noise[biome]) biome = (Biome)i;
    }

    return biome;
}

Tile_Type setTileType(int x, int y, Game_Instance &game)
{
    //HEIGHTS and PROBS
    float rock_height = 0.55;
    float sand_height = 0.45;
    float ice_height = 0.65;
    float rock_rand_prob = 0.01;
    float sand_rand_prob = 0.00;
    float ice_rand_prob = 0.00;

    //
    Tile_Type type = (Tile_Type)0;

    float smin_n = -1;
    float smax_n = 1;

    float space = game.params.MAP_SPACE / 8;        //How big are the biomes

    //ROCK CLUSTERS
    OpenSimplexNoise::Noise noise_gen = *game.map.other_noisers[0];

    double noise_x = (x / (space));
    double noise_y = (y / (space));

    float noise = noise_gen.eval(noise_x, noise_y);

    if(noise > rock_height) 
    {
        type = ROCK;
        return type;
    }

    //SAND CLUSTERS
    OpenSimplexNoise::Noise noise_gen2 = *game.map.other_noisers[1];

    noise_x = (x / (space));
    noise_y = (y / (space));

    noise = noise_gen2.eval(noise_x, noise_y);

    if(noise > sand_height) 
    {
        type = SAND;
        return type;
    }

    //ICE CLUSTERS
    OpenSimplexNoise::Noise noise_gen3 = *game.map.other_noisers[2];

    noise_x = (x / (space));
    noise_y = (y / (space));

    noise = noise_gen3.eval(noise_x, noise_y);

    if(noise > ice_height) 
    {
        type = ICE;
        return type;
    }


    //RANDOM TILE TYPE
    const float DEFAULT_WEIGHT = 0;
    
    //Setting default probabilities for each tile type
    int num_tile_types = (int)TILE_TYPE_SIZE;
    float tile_type_weight[num_tile_types];
    for(int i = 0; i < num_tile_types; i++)
    {
        tile_type_weight[i] = DEFAULT_WEIGHT;
    }
    
    //Setting custom probabilities for each tile type
    tile_type_weight[ROCK] = rock_rand_prob;
    tile_type_weight[SAND] = sand_rand_prob;
    tile_type_weight[ICE] = ice_rand_prob;

    //Sum of weights
    float sum = 0;
    for(int i = 0; i < num_tile_types; i++)
    {
        sum = sum + tile_type_weight[i];
    }

    //Random number
    float smin = 0;
    float smax = sum;
    float random = smin + rand() / (RAND_MAX / (smax - smin + 1) + 1);

    //Set tile type
    float weight = 0;
    for(int i = 0; i < num_tile_types; i++)
    {
        weight = weight + tile_type_weight[i];
        if(random < weight)
        {
            type = (Tile_Type)i;
            break;
        }
    }

    return type;
}

Tile_Decoration setTileDecoration(Map_Tile tile, Game_Instance &game)
{
    //HEIGHTS and PROBS
    float tree_height = 0.5;
    float tall_grass_height = 0.4;
    float bush_height = 0.8;
    float bad_bush_height = 0.9;
    float tree_rand_prob = 0.01;
    float tall_grass_rand_prob = 0.1;
    float bush_rand_prob = 0.005;
    float bad_bush_rand_prob = 0.005;
    float star_rand_prob = game.params.STAR_RARITY;
    
    //
    float x = tile.x;
    float y = tile.y;

    Tile_Decoration decoration = (Tile_Decoration)0;

    float space = game.params.MAP_SPACE / 6;

    float smin_n = -1;
    float smax_n = 1;

    //GRASS TILE
    if(tile.type == GRASS) {

        //TREE CLUSTERS
        OpenSimplexNoise::Noise noise_gen = *game.map.other_noisers[2];

        double noise_x = (x / (space));
        double noise_y = (y / (space));

        float noise = noise_gen.eval(noise_x, noise_y);

        if(noise > tree_height)
        {
            decoration = TREE;
            return decoration;
        }
        
        //TALL GRASS CLUSTERS
        OpenSimplexNoise::Noise noise_gen2 = *game.map.other_noisers[3];

        noise_x = (x / (space));
        noise_y = (y / (space));

        noise = noise_gen2.eval(noise_x, noise_y);

        if(noise > tall_grass_height)
        {
            decoration = TALL_GRASS;
            return decoration;
        }

        //BUSH CLUSTERS
        OpenSimplexNoise::Noise noise_gen3 = *game.map.other_noisers[4];

        noise_x = (x / (space));
        noise_y = (y / (space));

        noise = noise_gen3.eval(noise_x, noise_y);

        if(noise > bush_height)
        {
            decoration = BUSH;
            return decoration;
        }

        //BAD BUSH CLUSTERS
        OpenSimplexNoise::Noise noise_gen4 = *game.map.other_noisers[5];

        noise_x = (x / (space));
        noise_y = (y / (space));

        noise = noise_gen4.eval(noise_x, noise_y);

        if(noise > bad_bush_height)
        {
            decoration = BAD_BUSH;
            return decoration;
        }

        //RANDOM DECORATION
        const float DEFAULT_WEIGHT = 0;
        
        //Setting default probabilities for each tile dec
        int num_tile_decs = (int)TILE_DECORATION_SIZE;
        float tile_dec_weight[num_tile_decs];
        for(int i = 0; i < num_tile_decs; i++)
        {
            tile_dec_weight[i] = DEFAULT_WEIGHT;
        }
        
        //Setting custom probabilities for each tile dec
        tile_dec_weight[STAR] = star_rand_prob;
        tile_dec_weight[TREE] = tree_rand_prob;
        tile_dec_weight[TALL_GRASS] = tall_grass_rand_prob;
        tile_dec_weight[BUSH] = bush_rand_prob;
        tile_dec_weight[BAD_BUSH] = bad_bush_rand_prob;

        //Sum of weights
        float sum = 0;
        for(int i = 0; i < num_tile_decs; i++)
        {
            sum = sum + tile_dec_weight[i];
        }

        //Random number
        float smin = 0;
        float smax = sum;
        float random = smin + rand() / (RAND_MAX / (smax - smin + 1) + 1);

        //Set tile dec
        float weight = 0;
        for(int i = 0; i < num_tile_decs; i++)
        {
            weight = weight + tile_dec_weight[i];
            if(random < weight)
            {
                decoration = (Tile_Decoration)i;
                break;
            }
        }

    }

    return decoration;
}

Map_Tile setTileProperties(Map_Tile tile, Game_Instance &game)
{
    if(tile.type == ROCK) tile.colision = true;
    if(tile.decoration == TREE) tile.colision = true;

    if(tile.decoration == BUSH) tile.speed_modifier = 0.8;
    if(tile.decoration == BAD_BUSH) tile.speed_modifier = 0.8;
    if(tile.type == SAND) tile.speed_modifier = 0.6;
    if(tile.type == ICE) tile.speed_modifier = 1.2;

    if(tile.decoration == BAD_BUSH) tile.damage_modifier = 2;
    
    return tile;
}

Map_Tile setTileColor(Map_Tile tile, Game_Instance &game)
{
    Biome biome = tile.biome;

    //Is color not set?
    if(tile.color[0] != 0 || tile.color[1] != 0 || tile.color[2] != 0) return tile;
    
    //Set color
    switch(biome)
    {

        case HOPE: //White
            tile.color[0] = 255;
            tile.color[1] = 255;
            tile.color[2] = 255;
            break;
        case LUST: //Red
            tile.color[0] = 255;
            tile.color[1] = 30;
            tile.color[2] = 100;
            break;
        case GRUTTONY: //Orange
            tile.color[0] = 200;
            tile.color[1] = 100;
            tile.color[2] = 30;
            break;
        case PRIDE: //Yellow
            tile.color[0] = 150;
            tile.color[1] = 150;
            tile.color[2] = 30;
            break;
        case SLOTH: //Green
            tile.color[0] = 100;
            tile.color[1] = 255;
            tile.color[2] = 100;
            break;
        case WRATH: //Blue
            tile.color[0] = 30;
            tile.color[1] = 30;
            tile.color[2] = 255;
            break;
        case ENVY: //Purple
            tile.color[0] = 150;
            tile.color[1] = 30;
            tile.color[2] = 255;
            break;
        case GREED: //Swamp like
            tile.color[0] = 1;
            tile.color[1] = 150;
            tile.color[2] = 1;
            break;
        default: //Black
            tile.color[0] = 30;
            tile.color[1] = 30;
            tile.color[2] = 30;
            break;
    }

    return tile;
}

Map_Tile updateTileColor(Map_Tile tile, Game_Instance &game)
{
    tile.color[0] = 0;
    tile.color[1] = 0;
    tile.color[2] = 0;
    tile = setTileColor(tile, game);
    tile.blend_info = Blend_Info();
    return tile;
}

//Map handling - AVL handling
Map_Tile AVL_search(int x, int y, Game_Instance &game)
{ //Searches for tile in MAP
    Map_Tile tile = Map_Tile();
    tile.key = std::to_string(x) + "." + std::to_string(y);

    return game.map.map.search(tile);
}

void AVL_insert(Map_Tile tile, Game_Instance &game)
{ //Adds map tile to MAP - And updates
    game.map.map.insert(tile);
    return;
}

//Map handling - etc
Map_Tile updateOutdatedTile(Map_Tile tile, Game_Instance &game)
{
    //INVALID BLOCKS
    if(tile.decoration == STAR && tile.biome == HOPE) tile.decoration = NONE;

    //STAR COLLECTED - Is biome cleared?
    if(game.map.biomes_cleared[tile.biome] == true && tile.biome != HOPE)
    {
        tile.biome = HOPE;
        if(tile.decoration == STAR) tile.decoration = NONE;
        tile.color[0] = 0;
        tile.color[1] = 0;
        tile.color[2] = 0;
        tile = setTileColor(tile, game);
    }

    //SPAWN PROTECTION
    if(sqrt(pow(tile.x - game.player.spawn_x, 2) + pow(tile.y - game.player.spawn_y, 2)) < game.params.SPAWN_PROTECTION_RANGE)
    {
        tile.type = GRASS;
        tile.decoration = NONE;
        tile.colision = false;
        tile.speed_modifier = 1;
        tile.damage_modifier = 0;
    }

    tile = setTileProperties(tile, game);

    return tile;
}
