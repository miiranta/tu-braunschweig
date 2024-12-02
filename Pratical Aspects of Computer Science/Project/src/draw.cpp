//Draw handling
void drawHandler(sf::RenderWindow &window, Game_Instance &game)
{
    //Clear
    window.clear();

    //Iterate views
    for(int i = 0; i < game.view_names.size(); i++)
    {
        string key;
        game.view_names.retrieve(key, i);
        sf::View view = game.views[key];

        //Set view
        window.setView(view);

        //GAME
        if(key == "game")
        {
            //Player dist to center of the screen
            float p_dist_x = abs(game.player.x * game.params.TILE_SIZE - game.view_offset_x);
            float p_dist_y = abs(game.player.y * game.params.TILE_SIZE - game.view_offset_y);
            float p_dist_max = sqrt(pow(p_dist_x, 2) + pow(p_dist_y, 2));

            //Evaluate necessary distance
            game.necessary_distance = sqrt(pow(window.getSize().x,2) + pow(window.getSize().y,2));
            game.necessary_distance = ceil(((game.necessary_distance + p_dist_max)/(game.params.TILE_SIZE * 2) + 2) * game.draw_scale) + 1;

            drawMap(window, game);
            drawPlayer(window, game);
            drawMobs(window, game);
            drawProjectiles(window, game);
            drawSprites(window, game);
            drawMapDecoration(window, game);

            if(game.debug)
            {
                drawTestLines(window, game);
            }

            //Alpha
            sf::RectangleShape rect(sf::Vector2f(window.getSize().x * game.draw_scale, window.getSize().y * game.draw_scale));
            rect.setPosition(game.vis_a_x1, game.vis_a_y1);
            rect.setFillColor(sf::Color(0, 0, 0, 255 * (1-game.game_view_alpha)));
            window.draw(rect);
        }

        //GAME_OVERLAY
        else if(key == "game_overlay" && game.game_paused == false)
        {
            drawStats(window, game);
            drawMinimap(window, game);
            
            if(game.debug)
            {
                drawDebugStats(window, game);
            }
        }

        else if(key == "game_overlay" && game.game_paused == true)
        {
            drawStartCount(window, game);
        }

        //MENU
        else if(key == "menu")
        {
            drawMenu(window, game);

            //Alpha
            sf::RectangleShape rect(sf::Vector2f(window.getSize().x, window.getSize().y));
            rect.setPosition(0, 0);
            rect.setFillColor(sf::Color(0, 0, 0, 255 * (1-game.menu_view_alpha)));
            window.draw(rect);
        }

    }
   
    //Display
    window.display();
}

void drawSprites(sf::RenderWindow &window, Game_Instance &game)
{
    //Animations to be destroyed
    std::vector<std::string> to_destroy;

    //Iterate List of animations List<Animate_Params_Sprite> animation_running_sprite;
    for(int i = 0; i < game.animation_running_sprite.size(); i++)
    {
        Animate_Params_Sprite anima;
        game.animation_running_sprite.retrieve(anima, i);

        //Verify resources
        sf::Sprite sprite = game.draw_resources.sprites[anima.sprite];
        if(sprite.getTexture() == NULL) continue;

        //Calculate vars
        int text_height = sprite.getTexture()->getSize().y;
        int text_width = sprite.getTexture()->getSize().x;
        int frame_height = text_height / anima.num_frames;
        int max_frames = anima.num_frames;
        long long int current_ms = game.last_draw_time / 1000 - anima.starting_ms;
        
        //Calculate frame
        int frame = (current_ms / anima.ms_per_step) % max_frames;
        
        //Update start delay
        if(anima.ms_start_delay > 0)
        {
            anima.ms_start_delay -= (current_ms - anima.last_frame_ms);
            anima.starting_ms = game.last_draw_time / 1000;
            game.animation_running_sprite.replace(anima, i);
            continue;
        }

        //Update self destruct
        if(anima.self_destruct_after_iterations > 0 && frame == 0 && anima.last_frame != frame) anima.self_destruct_after_iterations--;
        if(anima.self_destruct_after_iterations == 0)
        {
            to_destroy.push_back(anima.key);
            continue;
        }

        //Set window of frame
        sprite.setTextureRect(sf::IntRect(0, frame * frame_height, text_width, frame_height));

        //Set scale
        float scale_ratio = (game.params.TILE_SIZE / text_width) * anima.scale;
        sprite.scale(scale_ratio, scale_ratio);

        //Set position
        sprite.setPosition(anima.x * game.params.TILE_SIZE, anima.y * game.params.TILE_SIZE);

        //Set alpha and blend
        float r = anima.r_blend;
        float g = anima.g_blend;
        float b = anima.b_blend;
        sprite.setColor(sf::Color(r, g, b, 255 * anima.alpha));

        //Update animation
        anima.last_frame = frame;
        anima.last_frame_ms = current_ms;
        game.animation_running_sprite.replace(anima, i);

        //Can draw?
        if(sqrt(pow(anima.x - game.player.x, 2) + pow(anima.y - game.player.y, 2)) > game.necessary_distance) continue;
        if(sqrt(pow(anima.x - game.player.x, 2) + pow(anima.y - game.player.y, 2)) > game.visibility_distance + 0.5) continue;

        //Draw
        window.draw(sprite);

    }

    //Destroy animations
    for(int i = 0; i < to_destroy.size(); i++)
    {
        Animate_Params_Sprite anima = {key: to_destroy[i].c_str()};
        destroySpriteAnimation(anima, game);
    }

}

void drawMap(sf::RenderWindow &window, Game_Instance &game)
{
    //Render distance
    const float player_to_tile_ratio = (float)game.params.PLAYER_SIZE / (float)game.params.TILE_SIZE;
    const int dist_to_check = std::max({ceil(player_to_tile_ratio) + 1, game.params.RENDER_DISTANCE});

    //X, Y from the middle of the player
    float player_x = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    float player_y = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;

    //Tile under player
    int player_tile_x = floor(player_x / game.params.TILE_SIZE);
    int player_tile_y = floor(player_y / game.params.TILE_SIZE);

    for(int i = -dist_to_check; i < dist_to_check + 1; i++)
    {
        for(int j = -dist_to_check; j < dist_to_check + 1; j++)
        {
            //Circle render
            if(floor(sqrt(i * i + j * j)) > game.necessary_distance) continue;
            if(floor(sqrt(i * i + j * j)) > game.visibility_distance) continue;
            
            int tile_x = i + player_tile_x;
            int tile_y = j + player_tile_y;

            Map_Tile tile = getMapTile(tile_x, tile_y, game);
            drawTile(tile, window, game);
        }
    }

}

void drawMapDecoration(sf::RenderWindow &window, Game_Instance &game)
{
    //Render distance
    const float player_to_tile_ratio = (float)game.params.PLAYER_SIZE / (float)game.params.TILE_SIZE;
    const int dist_to_check = std::max({ceil(player_to_tile_ratio) + 1, game.params.RENDER_DISTANCE});

    //X, Y from the middle of the player
    float player_x = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    float player_y = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;

    //Tile under player
    int player_tile_x = floor(player_x / game.params.TILE_SIZE);
    int player_tile_y = floor(player_y / game.params.TILE_SIZE);

    for(int i = -dist_to_check; i < dist_to_check + 1; i++)
    {
        for(int j = -dist_to_check; j < dist_to_check + 1; j++)
        {
            //Circle render
            if(floor(sqrt(i * i + j * j)) > game.necessary_distance) continue;
            if(floor(sqrt(i * i + j * j)) > game.visibility_distance) continue;
            
            int tile_x = i + player_tile_x;
            int tile_y = j + player_tile_y;

            Map_Tile tile = getMapTile(tile_x, tile_y, game);
            drawTileDecoration(tile, window, game);
        }
    }

}

void drawTile(Map_Tile tile, sf::RenderWindow &window, Game_Instance &game)
{
    //Draw tile
    sf::RectangleShape rect(sf::Vector2f(game.params.TILE_SIZE + 1, game.params.TILE_SIZE + 1));
    rect.setPosition(tile.x * game.params.TILE_SIZE, tile.y * game.params.TILE_SIZE);
    
    //Texture
    if(tile.type == GRASS) rect.setTexture(&game.draw_resources.textures["grass_tile"]);
    else if(tile.type == ROCK) rect.setTexture(&game.draw_resources.textures["rock_tile"]);
    else if(tile.type == SAND) rect.setTexture(&game.draw_resources.textures["sand_tile"]);
    else if(tile.type == ICE) rect.setTexture(&game.draw_resources.textures["ice_tile"]);

    //Is biome blend calculated?
    if(tile.blend_info.color[0] == 0 && tile.blend_info.color[1] == 0 && tile.blend_info.color[2] == 0)
    {
        tile.blend_info = getBiomeBlend(tile, game);
        updateMapTile(tile.x, tile.y, tile, game);
    }

    //Biome blend random updates
    if(((double)rand() / RAND_MAX) < game.params.BIOME_COLOR_BLEND_RANDOM_UPDATES)
    {
        tile.blend_info = getBiomeBlend(tile, game);
        updateMapTile(tile.x, tile.y, tile, game);
    }

    //Color alpha
    int color1_norm = std::min(255, (int)(tile.blend_info.color[0] + (255 * (1-game.params.BIOME_COLOR_ALPHA))));
    int color2_norm = std::min(255, (int)(tile.blend_info.color[1] + (255 * (1-game.params.BIOME_COLOR_ALPHA))));
    int color3_norm = std::min(255, (int)(tile.blend_info.color[2] + (255 * (1-game.params.BIOME_COLOR_ALPHA))));
    rect.setFillColor(sf::Color(color1_norm, color2_norm, color3_norm, 255));

    //Draw
    window.draw(rect);
}

void drawTileDecoration(Map_Tile tile, sf::RenderWindow &window, Game_Instance &game)
{
    if(tile.decoration == NONE) return;

    //Draw tile decoration
    sf::RectangleShape rect(sf::Vector2f(game.params.TILE_SIZE, game.params.TILE_SIZE));
    rect.setPosition(tile.x * game.params.TILE_SIZE, tile.y * game.params.TILE_SIZE);

    if(tile.decoration == TALL_GRASS) 
    {
        rect.setTexture(&game.draw_resources.textures["tall_grass_dec"]);
    }
    
    else if(tile.decoration == BUSH)
    {
        rect.setTexture(&game.draw_resources.textures["bush_dec"]);
    }
    
    else if(tile.decoration == BAD_BUSH) 
    {
        rect.setTexture(&game.draw_resources.textures["bad_bush_dec"]); 
    }
    
    else if(tile.decoration == TREE)
    {
        rect.setSize(sf::Vector2f(game.params.TILE_SIZE * 2, game.params.TILE_SIZE * 4));
        rect.setTexture(&game.draw_resources.textures["tree_dec"]);

        //Move texture up
        rect.move(-game.params.TILE_SIZE * 0.5, -game.params.TILE_SIZE * 3);
    }

    else if(tile.decoration == STAR) 
    {
        Animate_Params_Sprite anima = {
                key: tile.key + "_star",
                sprite: "star",
                x: (float)tile.x,
                y: (float)tile.y,
                num_frames: 2,
                ms_per_step: 150,
                scale: 1,
                self_destruct_after_iterations: 1,
                ms_start_delay: 0
        };
        createSpriteAnimation(anima, game);
        return; //Important, so the rect is not drawn
    }

    //Color alpha
    int color1_norm = std::min(255, (int)(tile.blend_info.color[0] + (255 * (1-game.params.BIOME_COLOR_ALPHA))));
    int color2_norm = std::min(255, (int)(tile.blend_info.color[1] + (255 * (1-game.params.BIOME_COLOR_ALPHA))));
    int color3_norm = std::min(255, (int)(tile.blend_info.color[2] + (255 * (1-game.params.BIOME_COLOR_ALPHA))));
    rect.setFillColor(sf::Color(color1_norm, color2_norm, color3_norm, 255));

    //Draw
    rect.move(tile.dec_offset_x, tile.dec_offset_y);
    window.draw(rect);
}

void drawPlayer(sf::RenderWindow &window, Game_Instance &game)
{
    float px = game.player.x;
    float py = game.player.y;

    //Total speed
    float x_speed_norm = game.player.x_speed;
    float y_speed_norm = game.player.y_speed;
    if(abs(x_speed_norm) < 0.002) x_speed_norm = 0;
    if(abs(y_speed_norm) < 0.002) y_speed_norm = 0;

    float total_speed = sqrt(pow(x_speed_norm, 2) + pow(y_speed_norm, 2));

    if(total_speed > 0.0000001)
    {
        //Direction
        Player_Direction dir = P_DOWN;
        int deg = atan2(y_speed_norm, x_speed_norm) * 180 / M_PI - 90;

        if(deg > 45 && deg <= 135) dir = P_LEFT;
        if(deg <= -45 && deg > -135) dir = P_RIGHT;
        if(deg > 135 || deg <= -135) dir = P_UP;
        if(deg > -45 && deg <= 45) dir = P_DOWN;

        game.player.direction_d = dir;

    }
    
    string sprite = "player_front";
    int num_frames = 5;
    switch (game.player.direction_d)
    {
        case P_UP:
            sprite = "player_back";
            break;
        case P_DOWN:
            sprite = "player_front";
            break;
        case P_LEFT:
            sprite = "player_left";
            num_frames = 6;
            break;
        case P_RIGHT:
            sprite = "player_right";
            num_frames = 6;
            break;
    }

    int ms_per_step = 600000;
    if(total_speed > 0.0001) ms_per_step = 50;
    else if(total_speed > 0.1) ms_per_step = 30;
    else if(total_speed > 1) ms_per_step = 15;

    //Color blend
    Map_Tile tile = getMapTile(px, py, game);
    float r_blend = tile.blend_info.color[0];
    float g_blend = tile.blend_info.color[1];
    float b_blend = tile.blend_info.color[2];

    //Sprite
    Animate_Params_Sprite anima = {
                key: "anima_player",
                sprite: sprite,
                alpha: game.player.draw_alpha,
                r_blend: r_blend,
                g_blend: g_blend,
                b_blend: b_blend,
                x: (float)px,
                y: (float)py,
                num_frames: num_frames,
                ms_per_step: ms_per_step,
                
                starting_ms: 0,   
                last_frame: 0,
                last_frame_ms: 0,
                
                scale: ((float)game.params.PLAYER_SIZE / (float)game.params.TILE_SIZE),
        };
    updateSpriteAnimation(anima, game);

    //rect.setFillColor(sf::Color(0, 0, 0, game.player.draw_alpha * 255));

}

void drawMobs(sf::RenderWindow &window, Game_Instance &game)
{
    int mobs_amount = game.mobs_around.size();

    for(int i = 0; i < mobs_amount; i++)
    {

        Mob mob;
        game.mobs_around.retrieve(mob, i);

        //is MOB in visiblity distance?
        if(sqrt(pow(mob.x - game.player.x, 2) + pow(mob.y - game.player.y, 2)) > game.necessary_distance) continue;
        if(sqrt(pow(mob.x - game.player.x, 2) + pow(mob.y - game.player.y, 2)) > game.visibility_distance + 0.5) continue;

        float px = mob.x * game.params.TILE_SIZE;
        float py = mob.y * game.params.TILE_SIZE;

        //Mob color blend
        Map_Tile tile = getMapTile(mob.x, mob.y, game);
        float r_blend = tile.blend_info.color[0];
        float g_blend = tile.blend_info.color[1];
        float b_blend = tile.blend_info.color[2];

        //Direction
        Mob_Direction dir = DOWN;

        //Total speed
        float total_speed = sqrt(pow(game.player.x_speed, 2) + pow(game.player.y_speed, 2));

        if(total_speed > 0.00001)
        {
            //What is mob direction? Degrees based on speedx and speedy
            int deg = atan2(mob.y_speed, mob.x_speed) * 180 / M_PI - 90;

            if(mob.type == SHOOTER)
            {
                deg = atan2(game.player.y - mob.y, game.player.x - mob.x) * 180 / M_PI - 90;
            }

            if(deg > 45 && deg <= 135) dir = LEFT;
            if(deg <= -45 && deg > -135) dir = RIGHT;
            if(deg > 135 || deg <= -135) dir = UP;
            if(deg > -45 && deg <= 45) dir = DOWN;

            mob.last_dir = dir;
        }

        //RUNNER
        if(mob.type == RUNNER)
        {
            string sprite = "mob_seeker_back";

            switch (mob.last_dir)
            {
                case UP:
                    sprite = "mob_seeker_back";
                    break;
                case DOWN:
                    sprite = "mob_seeker_front";
                    break;
                case LEFT:
                    sprite = "mob_seeker_left";
                    break;
                case RIGHT:
                    sprite = "mob_seeker_right";
                    break;
            }

            Animate_Params_Sprite anima = {
                key: mob.key,
                sprite: sprite,
                r_blend: r_blend,
                g_blend: g_blend,
                b_blend: b_blend,
                x: (float)mob.x,
                y: (float)mob.y,
                num_frames: 4,
                ms_per_step: 150,
                scale: ((float)game.params.MOB_SIZE / (float)game.params.TILE_SIZE),
                self_destruct_after_iterations: 1
            };
            updateSpriteAnimation(anima, game);
        }
        
        //SHOOTER
        else if(mob.type == SHOOTER)
        {
            string sprite = "mob_shooter_back";

            switch (mob.last_dir)
            {
                case UP:
                    sprite = "mob_shooter_back";
                    break;
                case DOWN:
                    sprite = "mob_shooter_front";
                    break;
                case LEFT:
                    sprite = "mob_shooter_left";
                    break;
                case RIGHT:
                    sprite = "mob_shooter_right";
                    break;
            }

            Animate_Params_Sprite anima = {
                key: mob.key,
                sprite: sprite,
                r_blend: r_blend,
                g_blend: g_blend,
                b_blend: b_blend,
                x: (float)mob.x,
                y: (float)mob.y,
                num_frames: 4,
                ms_per_step: 150,
                scale: ((float)game.params.MOB_SIZE / (float)game.params.TILE_SIZE),
                self_destruct_after_iterations: 1
            };
            updateSpriteAnimation(anima, game);

        }
        
        //FLYER
        else if(mob.type == FLYER)
        {
            string sprite = "mob_flyer_back";

            switch (mob.last_dir)
            {
                case UP:
                    sprite = "mob_flyer_back";
                    break;
                case DOWN:
                    sprite = "mob_flyer_front";
                    break;
                case LEFT:
                    sprite = "mob_flyer_left";
                    break;
                case RIGHT:
                    sprite = "mob_flyer_right";
                    break;
            }

            Animate_Params_Sprite anima = {
                key: mob.key,
                sprite: sprite,
                r_blend: r_blend,
                g_blend: g_blend,
                b_blend: b_blend,
                x: (float)mob.x,
                y: (float)mob.y,
                num_frames: 6,
                ms_per_step: 150,
                scale: ((float)game.params.MOB_SIZE / (float)game.params.TILE_SIZE),
                self_destruct_after_iterations: 1
            };
            updateSpriteAnimation(anima, game);

        }

    }
}

void drawProjectiles(sf::RenderWindow &window, Game_Instance &game)
{
    int projectiles_amount = game.projectiles.size();

    for(int i = 0; i < projectiles_amount; i++)
    {
        Projectile projectile;
        game.projectiles.retrieve(projectile, i);

        sf::RectangleShape rect(sf::Vector2f(game.params.PROJECTILE_SIZE, game.params.PROJECTILE_SIZE));
        rect.setTexture(&game.draw_resources.textures["projectile"]);
        rect.setScale(4, 4);

        float px = projectile.x * game.params.TILE_SIZE;
        float py = projectile.y * game.params.TILE_SIZE;

        rect.setPosition(px, py);

        if(sqrt(pow(projectile.x - game.player.x, 2) + pow(projectile.y - game.player.y, 2)) > game.necessary_distance) continue;
        if(sqrt(pow(projectile.x - game.player.x, 2) + pow(projectile.y - game.player.y, 2)) > game.visibility_distance + 0.5) continue;
        
        //Color blend
        Map_Tile tile = getMapTile(projectile.x, projectile.y, game);
        float r_blend = tile.blend_info.color[0];
        float g_blend = tile.blend_info.color[1];
        float b_blend = tile.blend_info.color[2];

        rect.setFillColor(sf::Color(r_blend, g_blend, b_blend, 255));
        
        window.draw(rect);
    }
}

void drawStats(sf::RenderWindow &window, Game_Instance &game)
{
    sf::Text text;

    //Draw CLOCK - 1
    text.setString(L"⌛");
    
    text.setFont(game.draw_resources.fonts["emoji"]);
    text.setCharacterSize(45);
    text.setFillColor(sf::Color::White);
    
    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(13, 27);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw CLOCK - 2
    string clock_str = tickToClockString(game.tick - game.run_start_tick, game);

    text.setString(clock_str);
    
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(80);
    text.setFillColor(sf::Color::White);
    
    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(80, 00);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw HEALTH 1
    text.setString(L"♥");
    
    text.setFont(game.draw_resources.fonts["emoji"]);
    text.setCharacterSize(50);
    text.setFillColor(sf::Color::White);
    
    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 100);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw HEALTH 2
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(80);
    text.setFillColor(sf::Color::White);
    text.setString(std::to_string(game.player.health));
    
    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(80, 75);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw STARS_COLLECTED 1
    text.setString(L"🌟");
    
    text.setFont(game.draw_resources.fonts["emoji"]);
    text.setCharacterSize(45);
    text.setFillColor(sf::Color::White);
    
    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(13, 173);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw STARS_COLLECTED 2
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(80);
    text.setString(std::to_string(game.player.stars_collected.size()) + "/" + std::to_string(BIOME_SIZE - 1));

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(80, 150);

    text.setOutlineThickness(2);

    window.draw(text);

}

void drawStartCount(sf::RenderWindow &window, Game_Instance &game)
{
    if(game.start_count == 0) return;

    sf::Text text;
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(200);
    text.setFillColor(sf::Color::White);

    string start_count_str = std::to_string(game.start_count);

    text.setString(start_count_str);

    text.setOutlineThickness(2);

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2 - 20, (window.getSize().y - text.getLocalBounds().getSize().y) / 2 - 50);

    window.draw(text);
}

void drawMinimap(sf::RenderWindow &window, Game_Instance &game)
{
    //Render distance
    const float player_to_tile_ratio = (float)game.params.PLAYER_SIZE / (float)game.params.TILE_SIZE;
    const int dist_to_check = std::max({ceil(player_to_tile_ratio) + 1, game.params.MINIMAP_RENDER_DISTANCE});

    //X, Y from the middle of the player
    float player_x = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    float player_y = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;

    //Tile under player
    int player_tile_x = floor(player_x / game.params.TILE_SIZE);
    int player_tile_y = floor(player_y / game.params.TILE_SIZE);

    //Center tile
    Map_Tile center = getMapTile(player_tile_x, player_tile_y, game);

    //Draw minimap
    for(int i = -dist_to_check; i < dist_to_check + 1; i++)
    {
        for(int j = -dist_to_check; j < dist_to_check + 1; j++)
        {
            //Circle render
            if(floor(sqrt(i * i + j * j)) > game.params.MINIMAP_RENDER_DISTANCE) continue;
            
            int tile_x = i + player_tile_x;
            int tile_y = j + player_tile_y;

            Map_Tile tile = getMapTile(tile_x, tile_y, game);
            drawMinimapTile(tile, center, window, game);
        }
    }

    //Draw player pointer - in the middle
    sf::ConvexShape convex;
    convex.setPointCount(3);
    convex.setPoint(0, sf::Vector2f(10, 0));
    convex.setPoint(1, sf::Vector2f(-10, 8));
    convex.setPoint(2, sf::Vector2f(-10, -8));

    convex.setFillColor(sf::Color::Red);
    convex.setOutlineThickness(1);
    convex.setOutlineColor(sf::Color::Black);

    float minimap_tile_size = (game.params.MINIMAP_SIZE / game.params.MINIMAP_RENDER_DISTANCE)/2;
    convex.setPosition(window.getSize().x - game.params.MINIMAP_SIZE/2 - minimap_tile_size - 10, game.params.MINIMAP_SIZE/2 + 10);

    //rotate
    //minimap direction approximate to player direction
    game.player.minimap_direction = (game.player.minimap_direction * 0.95) + (game.player.direction * 0.05);
    convex.rotate(game.player.minimap_direction);

    window.draw(convex);

    //Draw X and Y
    sf::Text text;
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(120);
    text.setFillColor(sf::Color::White);
    text.setString("x: " + std::to_string(player_tile_x) + "  y: " + std::to_string(player_tile_y));
    
    text.setScale(0.25, 0.25);
    text.setPosition(window.getSize().x - game.params.MINIMAP_SIZE/2 - text.getLocalBounds().getSize().x/8 - 10, game.params.MINIMAP_SIZE + 15);

    text.setOutlineThickness(4);

    window.draw(text);
}

void drawMinimapTile(Map_Tile tile, Map_Tile center, sf::RenderWindow &window, Game_Instance &game)
{
    //Size
    float minimap_tile_size = (game.params.MINIMAP_SIZE / game.params.MINIMAP_RENDER_DISTANCE)/2;

    //Position
    float px = (tile.x - center.x + game.params.MINIMAP_RENDER_DISTANCE) * minimap_tile_size;
    float py = (tile.y - center.y + game.params.MINIMAP_RENDER_DISTANCE) * minimap_tile_size;

    //Draw tile
    sf::RectangleShape rect(sf::Vector2f(minimap_tile_size, minimap_tile_size));
    rect.setPosition(px + window.getSize().x - game.params.MINIMAP_SIZE - minimap_tile_size - 10 , py + 10);

    //Color for each biome
    if(game.player.effects["blindness"].TTL > 0)
    {
        rect.setFillColor(sf::Color(0, 0, 0));
    }
    else
    {
        rect.setFillColor(sf::Color(tile.color[0], tile.color[1], tile.color[2]));
    }

    window.draw(rect);

    //Draw star
    if(tile.decoration == STAR)
    {
        sf::CircleShape circle(minimap_tile_size / 2);
        circle.setFillColor(sf::Color::White);
        circle.setPosition(px + window.getSize().x - game.params.MINIMAP_SIZE - minimap_tile_size - 10 , py + 10);
        window.draw(circle);
    }
}

void drawMenu(sf::RenderWindow &window, Game_Instance &game)
{
    switch(game.current_menu)
    {
        case MAIN:
            drawMenuMain(window, game);
            break;
        case PAUSE:
            drawMenuPause(window, game);
            break;
        case TUTORIAL:
            drawMenuTutorial(window, game);
            break;
        case GAME_OVER:
            drawMenuGameOver(window, game);
            break;
        case CONFIG:
            drawMenuConfig(window, game);
            break;
    }

    //Bug fix
    game.mouse_clicked[0] = false;
}

void drawMenuMain(sf::RenderWindow &window, Game_Instance &game)
{
    //Remake instance
    if(game.remake_instance)
    {
        game.remake_instance = false;
        remakeGameInstance(game, game.params);
        return;
    }

    //Game Title
    sf::Text text;
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(120);
    text.setFillColor(sf::Color::White);
    text.setString("STAR-T");

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 50);

    text.setOutlineThickness(4);

    window.draw(text);

    //Highscore
    sf::Text highscore_text;
    highscore_text.setFont(game.draw_resources.fonts["pixel"]);
    highscore_text.setCharacterSize(60);
    highscore_text.setFillColor(sf::Color::White);

    string clock_str = "-";
    if(game.current_highscore != 0) 
    {
        clock_str = tickToClockString(game.current_highscore, game);
    }
    highscore_text.setString("HIGHSCORE: " + clock_str);

    highscore_text.setPosition(0, 0);
    highscore_text.setScale(1, 1);
    highscore_text.move((window.getSize().x - highscore_text.getLocalBounds().getSize().x) / 2, 220);

    highscore_text.setOutlineThickness(1);

    window.draw(highscore_text);

    //Difficulty
    sf::Text diff_text;

    string diff_text_str;
    switch (game.params.DIFFICULTY)
    {
        case EASY:
            diff_text_str = "in EASY mode.";
            break;
        case NORMAL:
            diff_text_str = "in NORMAL mode.";
            break;
        case HARD:
            diff_text_str = "in HARD mode.";
            break;
        case OH_LORD:
            diff_text_str = "in Oh lord mode.";
            break;
    }

    diff_text.setFont(game.draw_resources.fonts["pixel"]);
    diff_text.setCharacterSize(30);
    diff_text.setFillColor(sf::Color::White);

    diff_text.setString(diff_text_str);

    diff_text.setPosition(0, 0);
    diff_text.setScale(1, 1);
    diff_text.move((window.getSize().x - diff_text.getLocalBounds().getSize().x) / 2, 290);

    diff_text.setOutlineThickness(1);

    window.draw(diff_text);

    //Start Button
    Menu_Button startBtn =
    {
        text: "PLAY",
        x: (int)(window.getSize().x / 2) - 350,
        y: (int)(window.getSize().y / 2) - 50,
        width: 700,
        height: 100,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };
    startBtn = createButton(startBtn, game);

    if(isMouseInsideButton(startBtn.rect, game))
    {
        startBtn.r = 0;
        startBtn.g = 200;
        startBtn.b = 0;
        startBtn = createButton(startBtn, game);

        //Button click
        if(game.mouse_clicked[0])
        {
            //Recreate game instance
            Game_Params params = game.params;
            remakeGameInstance(game, params);

            //Animation
            Animate_Params anima = {};
            animationHandler("start_count", anima, game);
            playSound("pop", game);

            game.mouse_clicked[0] = false;
            game.current_menu = GAME;
            return;
        }
    }

    drawButton(startBtn, window, game);

    //Config Button
    Menu_Button configBtn =
    {
        text: "CONFIG",
        x: (int)(window.getSize().x / 2) - 350,
        y: (int)(window.getSize().y / 2) + 100,
        width: 700,
        height: 100,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };
    configBtn = createButton(configBtn, game);

    if(isMouseInsideButton(configBtn.rect, game))
    {
        configBtn.r = 0;
        configBtn.g = 200;
        configBtn.b = 0;
        configBtn = createButton(configBtn, game);

        //Button click
        if(game.mouse_clicked[0])
        {
            //Change menu
            game.mouse_clicked[0] = false;

            Animate_Params anima = {
                TTL: 256
            };
            animationHandler("menu_fade", anima, game);
            menuPostRedirect(CONFIG, 128, game);
            playSound("pop", game);

            return;
        }
    }

    drawButton(configBtn, window, game);

    //Tutorial Button
    Menu_Button tutorialBtn =
    {
        text: "TUTORIAL",
        x: (int)(window.getSize().x / 2) - 350,
        y: (int)(window.getSize().y / 2) + 250,
        width: 700,
        height: 100,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };

    tutorialBtn = createButton(tutorialBtn, game);

    if(isMouseInsideButton(tutorialBtn.rect, game))
    {
        tutorialBtn.r = 0;
        tutorialBtn.g = 200;
        tutorialBtn.b = 0;
        tutorialBtn = createButton(tutorialBtn, game);

        //Button click
        if(game.mouse_clicked[0])
        {
            //Change menu
            game.mouse_clicked[0] = false;

            Animate_Params anima = {
                TTL: 256
            };
            animationHandler("menu_fade", anima, game);
            menuPostRedirect(TUTORIAL, 128, game);
            playSound("pop", game);

            return;
        }
    }

    drawButton(tutorialBtn, window, game);
}

void drawMenuConfig(sf::RenderWindow &window, Game_Instance &game)
{
    //Draw bg
    sf::RectangleShape rect(sf::Vector2f(window.getSize().x, window.getSize().y));
    rect.setPosition(0, 0);
    rect.setFillColor(sf::Color(255, 255, 255, 30));
    window.draw(rect);

    //Back Button
    Menu_Button backBtn =
    {
        text: "BACK",
        x: (int)(window.getSize().x / 2) - 100,
        y: (int)(window.getSize().y / 2) - 300,
        width: 200,
        height: 150,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };
    backBtn = createButton(backBtn, game);

    if(isMouseInsideButton(backBtn.rect, game))
    {
        backBtn.r = 0;
        backBtn.g = 200;
        backBtn.b = 0;
        
        //Button click
        if(game.mouse_clicked[0])
        {
            //Change menu
            game.mouse_clicked[0] = false;

            //SEED is empty
            if(game.params.MAP_SEED_STR == "")
            {
                game.params.MAP_SEED_STR = std::to_string(rand());
            }
        
            Animate_Params anima = {
                TTL: 256
            };
            animationHandler("menu_fade", anima, game);
            menuPostRedirect(MAIN, 128, game);
            playSound("pop", game);
        }   
    }

    backBtn = createButton(backBtn, game);
    drawButton(backBtn, window, game);

    //Difficulty Button
    string diff_text;
    switch (game.params.DIFFICULTY)
    {
        case EASY:
            diff_text = "DIFFICULTY: EASY";
            break;
        case NORMAL:
            diff_text = "DIFFICULTY: NORMAL";
            break;
        case HARD:
            diff_text = "DIFFICULTY: HARD";
            break;
        case OH_LORD:
            diff_text = "DIFFICULTY: Oh lord...";
            break;
    }

    Menu_Button diffBtn =
    {
        text: diff_text,
        x: (int)(window.getSize().x / 2) - 350,
        y: (int)(window.getSize().y / 2) - 50,
        width: 700,
        height: 100,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };
    diffBtn = createButton(diffBtn, game);
    
    if(isMouseInsideButton(diffBtn.rect, game))
    {
        diffBtn.r = 0;
        diffBtn.g = 200;
        diffBtn.b = 0;
        
        //Button click
        if(game.mouse_clicked[0])
        {
            game.mouse_clicked[0] = false;
            game.params.DIFFICULTY = (Difficulty)((game.params.DIFFICULTY + 1) % (int)Difficulty_COUNT);
            loadHighscore(game);
            playSound("pop", game);
        }
    }

    diffBtn = createButton(diffBtn, game);
    drawButton(diffBtn, window, game);

    //Seed input
    Menu_Button seedBtn =
    {
        text: "SEED: " + game.params.MAP_SEED_STR,
        x: (int)(window.getSize().x / 2) - 350,
        y: (int)(window.getSize().y / 2) + 100,
        width: 700,
        height: 100,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };
    seedBtn = createButton(seedBtn, game);

    if(isMouseInsideButton(seedBtn.rect, game))
    {
        //
        int MAX_CHARS = 14; //Bigger than 9 will cause overflow

        //Number input
        if(game.params.MAP_SEED_STR.length() < MAX_CHARS)
        {
            for(int i = 0; i < 10; i++)
            {
                if(game.key_clicked[i + 26])
                {
                    game.params.MAP_SEED_STR += std::to_string(i);
                    seedBtn.text = "SEED: " + game.params.MAP_SEED_STR;

                    game.key_clicked[i + 26] = false;

                    remakeGameInstance(game, game.params);
                }
            }
        }    

        //Backspace
        if(game.params.MAP_SEED_STR.length() > 0)
        {
            if(game.key_clicked[sf::Keyboard::Backspace])
            {
                if(game.params.MAP_SEED_STR.size() > 0)
                {
                    game.params.MAP_SEED_STR.pop_back();
                    seedBtn.text = "SEED: " + game.params.MAP_SEED_STR;

                    game.key_clicked[sf::Keyboard::Backspace] = false;

                    remakeGameInstance(game, game.params);
                }
            }
        }

        //Clear inputs
        for(int i = 0; i < 10; i++)
        {
            game.key_clicked[i + 26] = false;
        }
        game.key_clicked[sf::Keyboard::Backspace] = false;

        seedBtn.r = 0;
        seedBtn.g = 200;
        seedBtn.b = 0;
    }

    seedBtn = createButton(seedBtn, game);
    drawButton(seedBtn, window, game);
}

void drawMenuTutorial(sf::RenderWindow &window, Game_Instance &game)
{
    //Draw bg
    sf::RectangleShape rect(sf::Vector2f(window.getSize().x, window.getSize().y));
    rect.setPosition(0, 0);
    rect.setFillColor(sf::Color(255, 255, 255, 30));
    window.draw(rect);

    //Back Button
    Menu_Button backBtn =
    {
        text: "BACK",
        x: (int)(window.getSize().x / 2) - 100,
        y: (int)(window.getSize().y / 2) - 300,
        width: 200,
        height: 150,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };
    backBtn = createButton(backBtn, game);

    if(isMouseInsideButton(backBtn.rect, game))
    {
        backBtn.r = 0;
        backBtn.g = 200;
        backBtn.b = 0;
        
        //Button click
        if(game.mouse_clicked[0])
        {
            //Change menu
            game.mouse_clicked[0] = false;

            Animate_Params anima = {
                TTL: 256
            };
            animationHandler("menu_fade", anima, game);
            menuPostRedirect(MAIN, 128, game);
            playSound("pop", game);

            //SEED is empty
            if(game.params.MAP_SEED_STR == "")
            {
                game.params.MAP_SEED_STR = std::to_string(rand());
            }
        }
    }

    backBtn = createButton(backBtn, game);
    drawButton(backBtn, window, game);

    //Tutorial text
    sf::Text text;
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(50);

    string tutorial_text = 
    "W A S D > Move around.\n"
    "SCROLL > Camera zoom.\n"
    "ESC > Pause.\n\n"
    "Collect the 7 stars of the different biomes.\n" 
    "Try to beat your highscore.\n\n"
    "Avoid enemies.\n"
    "Avoid the bad bushes and quicksand.\n"
    "Use the minimap! Stars are small white circles.\n";

    text.setString(tutorial_text);

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(window.getSize().x / 2 - text.getLocalBounds().getSize().x / 2, window.getSize().y / 2 - text.getLocalBounds().getSize().y / 2 + 180);

    window.draw(text);

}

void drawMenuPause(sf::RenderWindow &window, Game_Instance &game)
{
    game.game_view_alpha = 0.3;
    game.game_paused = true;

    game.scroll_speed = -0.03;
    evaluateZoom(game);

    //Draw clock
    sf::Text text;
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(80);
    text.setFillColor(sf::Color::White);
    
    string clock_str = tickToClockString(game.tick - game.run_start_tick, game);
    text.setString(clock_str);

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 50);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw PAUSE text
    text.setCharacterSize(120);
    text.setString("PAUSED");

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 200);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw BACK TO GAME button
    Menu_Button backBtn =
    {
        text: "RESUME",
        x: (int)(window.getSize().x / 2) - 350,
        y: (int)(window.getSize().y / 2) - 50,
        width: 700,
        height: 100,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };

    backBtn = createButton(backBtn, game);
    
    if(isMouseInsideButton(backBtn.rect, game))
    {
        backBtn.r = 0;
        backBtn.g = 200;
        backBtn.b = 0;
        
        //Button click
        if(game.mouse_clicked[0])
        {
            game.mouse_clicked[0] = false;

            Animate_Params anima = {};
            animationHandler("start_count", anima, game);
            playSound("pop", game);

            game.current_menu = GAME;
        }
    }

    backBtn = createButton(backBtn, game);
    drawButton(backBtn, window, game);

    //Draw BACK TO MAIN MENU button
    Menu_Button mainBtn =
    {
        text: "BACK TO MAIN MENU",
        x: (int)(window.getSize().x / 2) - 350,
        y: (int)(window.getSize().y / 2) + 100,
        width: 700,
        height: 100,
        r:0, g:255, b:0,
        rect: sf::RectangleShape()
    };

    mainBtn = createButton(mainBtn, game);

    if(isMouseInsideButton(mainBtn.rect, game))
    {
        mainBtn.r = 0;
        mainBtn.g = 200;
        mainBtn.b = 0;
        
        //Button click
        if(game.mouse_clicked[0])
        {
            game.mouse_clicked[0] = false;
            
            Animate_Params anima = {
                TTL: 256
            };
            animationHandler("menu_fade", anima, game);
            menuPostRedirect(MAIN, 128, game);
            playSound("pop", game);
        }
    }

    mainBtn = createButton(mainBtn, game);
    drawButton(mainBtn, window, game);
}

void drawMenuGameOver(sf::RenderWindow &window, Game_Instance &game)
{
    game.game_view_alpha = 0.3;
    game.game_paused = true;

    //Player dead?
    bool player_dead = game.player.health <= 0;

    //GAME OVER
    if(player_dead)
    {
        //Draw clock
        sf::Text text;
        text.setFont(game.draw_resources.fonts["pixel"]);
        text.setCharacterSize(80);
        text.setFillColor(sf::Color::White);
        
        string clock_str = tickToClockString(game.tick - game.run_start_tick, game);
        text.setString(clock_str);

        text.setPosition(0, 0);
        text.setScale(1, 1);
        text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 50);

        text.setOutlineThickness(2);

        window.draw(text);

        //Draw GAMEOVER text
        text.setCharacterSize(150);
        text.setString("GAME OVER");

        text.setPosition(0, 0);
        text.setScale(1, 1);
        text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 200);

        text.setOutlineThickness(2);

        window.draw(text);
       
        //Draw BACK TO MAIN MENU button
        Menu_Button mainBtn =
        {
            text: "BACK TO MAIN MENU",
            x: (int)(window.getSize().x / 2) - 350,
            y: (int)(window.getSize().y / 2) + 100,
            width: 700,
            height: 100,
            r:0, g:255, b:0,
            rect: sf::RectangleShape()
        };

        mainBtn = createButton(mainBtn, game);
        
        if(isMouseInsideButton(mainBtn.rect, game))
        {
            mainBtn.r = 0;
            mainBtn.g = 200;
            mainBtn.b = 0;
            
            //Button click
            if(game.mouse_clicked[0])
            {
                game.mouse_clicked[0] = false;
                
                Animate_Params anima = {
                    TTL: 256
                };
                animationHandler("menu_fade", anima, game);
                menuPostRedirect(MAIN, 128, game);
                playSound("pop", game);
            }
        }

        mainBtn = createButton(mainBtn, game);
        drawButton(mainBtn, window, game);
    }

    //WIN
    else
    {
        //Draw clock
        sf::Text text;
        text.setFont(game.draw_resources.fonts["pixel"]);
        text.setCharacterSize(80);
        text.setFillColor(sf::Color::White);
        
        string clock_str = tickToClockString(game.tick - game.run_start_tick, game);
        text.setString("Your time:  " + clock_str);

        text.setPosition(0, 0);
        text.setScale(1, 1);
        text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 50);

        text.setOutlineThickness(2);

        window.draw(text);

        //Draw highscore
        string highscore_str = "-";
        if(game.tick - game.run_start_tick < game.current_highscore || game.current_highscore == 0)
        {
            highscore_str = tickToClockString(game.tick - game.run_start_tick, game);
        }
        else
        {
            highscore_str = tickToClockString(game.current_highscore, game);
        }
        text.setString("Highscore:  " + highscore_str);

        text.setPosition(0, 0);
        text.setScale(1, 1);
        text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 150);

        text.setOutlineThickness(2);

        window.draw(text);

        //Draw WIN text
        text.setCharacterSize(150);
        text.setString("WIN!");

        text.setPosition(0, 0);
        text.setScale(1, 1);
        text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 300);

        text.setOutlineThickness(2);

        window.draw(text);

        //Draw NEW HIGHSCORE text
        if((game.tick - game.run_start_tick < game.current_highscore || game.current_highscore == 0))
        {
            text.setCharacterSize(80);
            text.setString("NEW HIGHSCORE!");

            text.setPosition(0, 0);
            text.setScale(1, 1);
            text.move((window.getSize().x - text.getLocalBounds().getSize().x) / 2, 450);

            text.setOutlineThickness(2);

            window.draw(text);
        }

        //Draw BACK TO MAIN MENU button
        Menu_Button mainBtn =
        {
            text: "BACK TO MAIN MENU",
            x: (int)(window.getSize().x / 2) - 350,
            y: (int)(window.getSize().y / 2) + 100,
            width: 700,
            height: 100,
            r:0, g:255, b:0,
            rect: sf::RectangleShape()
        };

        mainBtn = createButton(mainBtn, game);
        
        if(isMouseInsideButton(mainBtn.rect, game))
        {
            mainBtn.r = 0;
            mainBtn.g = 200;
            mainBtn.b = 0;
            
            //Button click
            if(game.mouse_clicked[0])
            {
                game.mouse_clicked[0] = false;
                
                Animate_Params anima = {
                    TTL: 256
                };
                animationHandler("menu_fade", anima, game);
                menuPostRedirect(MAIN, 128, game);
                playSound("pop", game);
            }
        }

        mainBtn = createButton(mainBtn, game);
        drawButton(mainBtn, window, game);
    }
}

//Draw filters
Blend_Info getBiomeBlend(Map_Tile tile, Game_Instance &game)
{   
    float color[3] = {0, 0, 0};

    //Get neighbors
    int k = 0;
    for(int i = -game.params.BIOME_COLOR_BLEND; i < game.params.BIOME_COLOR_BLEND + 1; i++)
    {
        for(int j = -game.params.BIOME_COLOR_BLEND; j < game.params.BIOME_COLOR_BLEND + 1; j++)
        {
            Map_Tile neighbor = getMapTile(tile.x + i, tile.y + j, game);

            //Blend
            color[0] = (color[0] + neighbor.color[0]);
            color[1] = (color[1] + neighbor.color[1]);
            color[2] = (color[2] + neighbor.color[2]);
            k++;
        }
    }

    //Average
    color[0] = floor(color[0] / k);
    color[1] = floor(color[1] / k);
    color[2] = floor(color[2] / k);

    //Minimum of 1
    color[0] = std::max(1, (int)color[0]);
    color[1] = std::max(1, (int)color[1]);
    color[2] = std::max(1, (int)color[2]);

    Blend_Info blend_info = {
        color: {(int)round(color[0]), (int)round(color[1]), (int)round(color[2])}
    };

    return blend_info;
}

//Aux
string tickToClockString(long int tick, Game_Instance &game)
{
    long long int ms = floor((tick % game.params.TICKS_PER_SECOND) * 1000 / game.params.TICKS_PER_SECOND);
    long long int secs = floor(tick / game.params.TICKS_PER_SECOND);
    long long int mins = (secs / 60);
    long long int hours = (mins / 60);
    
    string hours_str = std::to_string((int)floor(hours));
    if(hours_str.length() == 1) hours_str = "0" + hours_str;

    string mins_str = std::to_string(mins % 60);
    if(mins_str.length() == 1) mins_str = "0" + mins_str;

    string secs_str = std::to_string(secs % 60);
    if(secs_str.length() == 1) secs_str = "0" + secs_str;

    string ms_str = std::to_string(ms % 1000);
    if(ms_str.length() == 1) ms_str = "00" + ms_str;
    else if(ms_str.length() == 2) ms_str = "0" + ms_str;
    //ms_str = ms_str.substr(0, ms_str.length() - 1);

    string clock_str = hours_str + ":" + mins_str + ":" + secs_str + ":" + ms_str;
    return clock_str;
}

