//Animation handling
void animationHandler(string animation, Animate_Params anima, Game_Instance &game)
{
    if(animation == "star_collection")
    {
        std::thread thread(animateStarCollection, anima, std::ref(game));
        game.animation_running["star_collection"] = true;
        thread.detach();
    }

    else if(animation == "blindness")
    {
        std::thread thread(animateBlindness, anima, std::ref(game));
        game.animation_running["blindness"] = true;
        thread.detach();
    }

    else if(animation == "invincibility")
    {
        std::thread thread(animateInvincibility, anima, std::ref(game));
        game.animation_running["invincibility"] = true;
        thread.detach();
    }

    else if(animation == "start_count")
    {
        std::thread thread(animateStartCount, anima, std::ref(game));
        game.animation_running["start_count"] = true;
        thread.detach();
    }

    else if(animation == "menu_fade")
    {
        std::thread thread(animateMenuFade, anima, std::ref(game));
        game.animation_running["sprite"] = true;
        thread.detach();
    }

    return;
}

void animateStarCollection(Animate_Params anima, Game_Instance &game)
{
    int x = anima.x;
    int y = anima.y;
    int ms_per_step = anima.ms_per_step;

    //Initial biome
    Biome current_biome = getMapTile(x, y, game).biome;

    //Update x, y tile
    Map_Tile tile = getMapTile(x, y, game);
    
    tile.biome = HOPE;
    if(tile.decoration == STAR) tile.decoration = NONE;
    tile = updateTileColor(tile, game);
    tile.blend_info.color[0] = 255;
    tile.blend_info.color[1] = 255;
    tile.blend_info.color[2] = 255;
    updateMapTile(x, y, tile, game);

    //Transform biome to HOPE step by step
    int px, py, radius;
    for(int k = 0; k < game.params.RENDER_DISTANCE; k++)
    {
        radius = k;
        int last_px = 0, last_py = 0;
        for(int i = 0; i < ceil(10*radius*M_PI); i++)
        {
            px = round(cos(i) * radius + x);
            py = round(sin(i) * radius + y);

            if(px == last_px && py == last_py) continue;

            Map_Tile tile = getMapTile(px, py, game);
            if(tile.biome != current_biome) continue;

            tile.biome = HOPE;
            if(tile.decoration == STAR) tile.decoration = NONE;
            tile = updateTileColor(tile, game);

            updateMapTile(px, py, tile, game);
        }

        //Sleep thread
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_per_step));

    }

    //Rerun to update colors
    for(int k = 0; k < game.params.RENDER_DISTANCE; k++)
    {
        radius = k;
        int last_px = 0, last_py = 0;
        for(int i = 0; i < ceil(10*radius*M_PI); i++)
        {
            px = round(cos(i) * radius + x);
            py = round(sin(i) * radius + y);

            if(px == last_px && py == last_py) continue;

            Map_Tile tile = getMapTile(px, py, game);
            if(tile.biome != HOPE) continue;

            tile = updateTileColor(tile, game);

            updateMapTile(px, py, tile, game);
        }

        //Sleep thread
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_per_step/2));

    }

    //Biome cleared
    game.map.biomes_cleared[current_biome] = true;

    //KILL THREAD
    game.animation_running["star_collection"] = false;
    return;
     
}

void animateBlindness(Animate_Params anima, Game_Instance &game)
{
    for(int i = game.visibility_distance; i > 2; i--)
    {
        //Reduce visibility
        game.visibility_distance = i;

        //Sleep thread
        std::this_thread::sleep_for(std::chrono::milliseconds(anima.ms_per_step));

    }

    //Sleep thread
    while(game.player.effects["blindness"].TTL > 0) std::this_thread::sleep_for(std::chrono::milliseconds(anima.ms_per_step));

    for(int i = game.visibility_distance; i <= game.params.RENDER_DISTANCE; i++)
    {
        //Increase visibility
        game.visibility_distance = i;
        
        //Sleep thread
        std::this_thread::sleep_for(std::chrono::milliseconds(anima.ms_per_step * 2));
    }

    //KILL THREAD
    game.animation_running["blindness"] = false;
    return;
}

void animateInvincibility(Animate_Params anima, Game_Instance &game)
{
    //Sleep thread
    while(game.player.effects["invincibility"].TTL > 0) 
    {
        game.player.draw_alpha = 0.5;
        std::this_thread::sleep_for(std::chrono::milliseconds(anima.ms_per_step));
        game.player.draw_alpha = 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(anima.ms_per_step));
    }
    
    //KILL THREAD
    game.animation_running["invincibility"] = false;
    return;
}

void animateStartCount(Animate_Params anima, Game_Instance &game)
{
    int ms_till_start = 2000;

    //Sleep 3s
    game.game_paused = true;
    float alpha_step = (1 - game.game_view_alpha) / ms_till_start;

    for(int i = ms_till_start; i > 0; i--)
    {
        game.start_count = floor(i/1000) + 1;
        game.game_view_alpha += alpha_step;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    game.start_count = 0;
    game.game_view_alpha = 1;
    game.game_paused = false;

    //Kill thread
    game.animation_running["start_count"] = false;
    return;
}

void animateMenuFade(Animate_Params anima, Game_Instance &game)
{
    int steps = 100;
    float alpha_step = 1 / (float)(steps);
    float TTL_per_step = anima.TTL / steps;

    game.menu_view_alpha = 1;
    for(int i = steps/2; i > 0; i--)
    {
        game.menu_view_alpha -= alpha_step*2;
        std::this_thread::sleep_for(std::chrono::milliseconds((int)((TTL_per_step / game.params.TICKS_PER_SECOND) * 1000)));
    }
    for(int i = steps/2; i > 0; i--)
    {
        game.menu_view_alpha += alpha_step*2;
        std::this_thread::sleep_for(std::chrono::milliseconds((int)((TTL_per_step / game.params.TICKS_PER_SECOND) * 1000)));
    }
    game.menu_view_alpha = 1;

    //Kill thread
    game.animation_running["menu_fade"] = false;
    return;
}


void createSpriteAnimation(Animate_Params_Sprite anima, Game_Instance &game)
{
    //Is it already running?
    if(game.animation_running_sprite.search(anima) != -1) return;

    //Add animation to list
    anima.starting_ms = game.last_draw_time / 1000;
    game.animation_running_sprite.insert(anima, game.animation_running_sprite.size());
}

void updateSpriteAnimation(Animate_Params_Sprite anima, Game_Instance &game)
{
    //Is it already running?
    int index = game.animation_running_sprite.search(anima);
    if(index == -1)
    {
        createSpriteAnimation(anima, game);
        return;
    }

    //Destroy and create
    destroySpriteAnimation(anima, game);
    game.animation_running_sprite.insert(anima, game.animation_running_sprite.size());
}

void destroySpriteAnimation(Animate_Params_Sprite anima, Game_Instance &game)
{
    //Is it already running?
    int index = game.animation_running_sprite.search(anima);
    if(index == -1) return;

    //Remove animation from list
    Animate_Params_Sprite dummy;
    game.animation_running_sprite.remove(dummy, index);
}
