//Player handling
Player createPlayer()
{
    Player player;

    player.x = 0;
    player.y = 0;

    return player;
}

void evaluatePlayerMovement(Game_Instance &game)
{
    Player &player = game.player;
    float SPEED_MAX = game.params.PLAYER_SPEED_MAX / 100;
    float ACCELERATION = game.params.PLAYER_ACCELERATION / 1000 + game.params.PLAYER_DEACCERATION / 1000;

    //Add speed based on press
    if(game.key_pressed[sf::Keyboard::W])
    {
        player.y_speed = player.y_speed - ACCELERATION;
    }
    if(game.key_pressed[sf::Keyboard::A])
    {
        player.x_speed = player.x_speed - ACCELERATION;
    }
    if(game.key_pressed[sf::Keyboard::S])
    {
        player.y_speed = player.y_speed + ACCELERATION;
    }
    if(game.key_pressed[sf::Keyboard::D])
    {
        player.x_speed = player.x_speed + ACCELERATION;
    }

    //Knockback correction
    if(player.knockback_correction > 1) player.knockback_correction = player.knockback_correction * 0.99;
    else player.knockback_correction = 1;

    //Slow down
    player.x_speed = player.x_speed * (1 - (game.params.PLAYER_DEACCERATION / player.knockback_correction) / 100);
    player.y_speed = player.y_speed * (1 - (game.params.PLAYER_DEACCERATION / player.knockback_correction) / 100);

    //Limit speed
    if(player.x_speed > SPEED_MAX) player.x_speed = SPEED_MAX;
    if(player.x_speed < -SPEED_MAX) player.x_speed = -SPEED_MAX;
    if(player.y_speed > SPEED_MAX) player.y_speed = SPEED_MAX;
    if(player.y_speed < -SPEED_MAX) player.y_speed = -SPEED_MAX;

    //Speed too small
    if(abs(player.x_speed) < 0.0000001) player.x_speed = 0;
    if(abs(player.y_speed) < 0.0000001) player.y_speed = 0;

    //Test for Colision
    evaluatePlayerColision(game, player);

    //Evaluate speed modifier
    evaluateSpeedModifier(game);

    //Move player
    player.x = player.x + player.x_speed;
    player.y = player.y + player.y_speed;

    //Set direction, in degrees
    if(player.x_speed != 0 || player.y_speed != 0) player.direction = atan2(player.y_speed, player.x_speed) * 180 / M_PI;

    //Play walking sound
    if(player.x_speed != 0 || player.y_speed != 0) game.walk_sound_counter = game.walk_sound_counter + 1;
    game.walk_sound_counter = game.walk_sound_counter % 64;
    if(game.walk_sound_counter == 0) 
    {
        playSound("grass", game);
        game.walk_sound_counter = 1;
    }


}

void evaluateSpeedModifier(Game_Instance &game)
{
    Player &player = game.player;

    //X, Y from the middle of the player
    float player_x = player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    float player_y = player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;

    //Tile under player
    int player_tile_x = floor(player_x / game.params.TILE_SIZE);
    int player_tile_y = floor(player_y / game.params.TILE_SIZE);

    player.x_speed = player.x_speed * getMapTile(player_tile_x, player_tile_y, game).speed_modifier;
    player.y_speed = player.y_speed * getMapTile(player_tile_x, player_tile_y, game).speed_modifier;

}

void evaluateZoom(Game_Instance &game)
{

    //Add speed based on delta
    if(game.scroll_delta > 0)
    {
        game.scroll_speed = game.scroll_speed + 0.001 * game.params.ZOOM_SENS;
    }
    if(game.scroll_delta < 0)
    {
        game.scroll_speed = game.scroll_speed - 0.001 * game.params.ZOOM_SENS;
    }

    //Slow down
    game.scroll_speed = game.scroll_speed * 0.99;

    //Speed too small
    if(abs(game.scroll_speed) < 0.0000001) {game.scroll_speed = 0;}

    //Change scale
    game.draw_scale += game.scroll_speed;

    //Limit scale
    if(game.draw_scale > game.params.DRAW_SCALE_MAX) game.draw_scale = game.params.DRAW_SCALE_MAX;
    if(game.draw_scale < game.params.DRAW_SCALE_MIN) game.draw_scale = game.params.DRAW_SCALE_MIN;

    game.scroll_delta = 0;

}

void findPlayerSpawn(Player &player, Game_Instance &game)
{
    long int x = 0;
    long int y = 0;

    int search_bubble = 10;
    int i = 0;

    Map_Tile tile = getMapTile(x, y, game);
    while((tile.biome != HOPE) || (tile.colision == true))
    {
        x = rand() % search_bubble;
        y = rand() % search_bubble;
        i++;
        search_bubble = search_bubble + i;
        tile = getMapTile(x, y, game);
    }

    player.x = x + 0.5;
    player.y = y + 0.5;
    player.spawn_x = x;
    player.spawn_y = y;

    //Centrilize Camera
    game.draw_scale = game.params.DRAW_SCALE_MIN;
    game.window_center_player = true;
}

//Colision handling
void evaluatePlayerColision(Game_Instance &game, Player &player)
{
    float player_left = player.x * game.params.TILE_SIZE;
    float player_right = player_left + game.params.PLAYER_SIZE;
    float player_top = player.y * game.params.TILE_SIZE;
    float player_bot = player_top + game.params.PLAYER_SIZE;

    bool stop_left = false;
    bool stop_right = false;
    bool stop_top = false;
    bool stop_bot = false;

    //Map tile colision
    const float player_to_tile_ratio = (float)game.params.PLAYER_SIZE / (float)game.params.TILE_SIZE;
    const int dist_to_check = ceil(player_to_tile_ratio) + 1;

    int player_tile_x = floor(player.x);
    int player_tile_y = floor(player.y);

    for(int i = -dist_to_check; i < dist_to_check; i++)
    {
        for(int j = -dist_to_check; j < dist_to_check; j++)
        {
            int tile_x = i + player_tile_x;
            int tile_y = j + player_tile_y;

            //Invalid tile - no colision
            if(getMapTile(tile_x, tile_y, game).colision == false) continue;

            //Tile colision
            float tile_left = tile_x * game.params.TILE_SIZE;
            float tile_right = tile_left + game.params.TILE_SIZE;
            float tile_top = tile_y * game.params.TILE_SIZE;
            float tile_bot = tile_top + game.params.TILE_SIZE;

            float player_left_new = player_left + player.x_speed;
            float player_right_new = player_right + player.x_speed;
            float player_top_new = player_top + player.y_speed;
            float player_bot_new = player_bot + player.y_speed;

            //Is player in tile?
            if(player_right_new > tile_left && player_left_new < tile_right && player_bot_new > tile_top && player_top_new < tile_bot)
            {
                //Which side is the nearest?
                float dist_left = abs(player_right_new - tile_left);
                float dist_right = abs(player_left_new - tile_right);
                float dist_top = abs(player_bot_new - tile_top);
                float dist_bot = abs(player_top_new - tile_bot);

                float min_dist = std::min({dist_left, dist_right, dist_top, dist_bot});

                //Which side to stop
                if(min_dist == dist_left){

                    if(player.x_speed < 0) player.x_speed = -player.x_speed;

                    stop_left = true;
                }
                if(min_dist == dist_right){

                    if(player.x_speed > 0) player.x_speed = -player.x_speed;

                    stop_right = true;
                }
                if(min_dist == dist_top){

                    if(player.y_speed < 0) player.y_speed = -player.y_speed;

                    stop_top = true;
                }
                if(min_dist == dist_bot){

                    if(player.y_speed > 0) player.y_speed = -player.y_speed;

                    stop_bot = true;
                }

            }

        }

    }

    //Stopping
    if(stop_left) player.x_speed = -player.x_speed * game.params.PLAYER_BONCE;
    if(stop_right) player.x_speed = -player.x_speed * game.params.PLAYER_BONCE;
    if(stop_top) player.y_speed = -player.y_speed * game.params.PLAYER_BONCE;
    if(stop_bot) player.y_speed = -player.y_speed * game.params.PLAYER_BONCE;

}

//Damage handling
void evaluatePlayerDamage(Game_Instance &game, Player &player)
{
    //Is player invincible?
    if(player.effects["invincibility"].TTL > 0) return;

    //Get current stepping tile - Player center
    float player_x = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    float player_y = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    int player_tile_x = floor(player_x / game.params.TILE_SIZE);
    int player_tile_y = floor(player_y / game.params.TILE_SIZE);
    Map_Tile player_tile = getMapTile(player_tile_x, player_tile_y, game);

    //TILE DAMAGE

    if(player_tile.damage_modifier > 0)
    {
        player.health = player.health - player_tile.damage_modifier;
        player.effects["invincibility"].TTL = 256;

        //Random vector from -1 to 1
        float x_knockback = ((float)rand() / RAND_MAX) * 2 - 1;
        float y_knockback = ((float)rand() / RAND_MAX) * 2 - 1;
        
        evaluateKnockback(game.params.TILE_KNOCKBACK, x_knockback, y_knockback, player, game);

        //Sound
        playSound("damage", game);
    }
    
    //MOB DAMAGE

    for(int i = 0; i < game.mobs_around.size(); i++)
    {
        Mob mob;
        game.mobs_around.retrieve(mob, i);

        //Runner
        if(mob.type == RUNNER)
        {
            float mob_x = mob.x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;
            float mob_y = mob.y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;

            float distance_to_mob = sqrt((player_x - mob_x) * (player_x - mob_x) + (player_y - mob_y) * (player_y - mob_y));

            if(distance_to_mob < game.params.MOB_SIZE / 1.5)
            {
                player.health = player.health - game.params.RUNNER_DAMAGE;
                player.effects["invincibility"].TTL = 256;

                //Evaluate knockback
                float x_knockback = player_x - mob_x;
                float y_knockback = player_y - mob_y;
                evaluateKnockback(game.params.RUNNER_KNOCKBACK, x_knockback, y_knockback, player, game);

                //Sound
                playSound("damage", game);
            }
        }

        //Shooter
        else if(mob.type == SHOOTER)
        {
            //Do nothing here - projectile damage
        }

        //Flyer
        else if(mob.type == FLYER)
        {
            float mob_x = mob.x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;
            float mob_y = mob.y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;

            float distance_to_mob = sqrt((player_x - mob_x) * (player_x - mob_x) + (player_y - mob_y) * (player_y - mob_y));

            if(distance_to_mob < game.params.MOB_SIZE / 1.5)
            {
                player.health = player.health - game.params.FLYER_DAMAGE;
                player.effects["invincibility"].TTL = 512;

                //Evaluate knockback
                float x_knockback = player_x - mob_x;
                float y_knockback = player_y - mob_y;
                evaluateKnockback(game.params.FLYER_KNOCKBACK, x_knockback, y_knockback, player, game);

                //Flyer Attack - Add effect
                player.effects["blindness"].TTL += 4096;

                //Destroy sprite animation
                Animate_Params_Sprite anima = {key: mob.key};
                destroySpriteAnimation(anima, game);

                //Run death animation
                Animate_Params_Sprite death = {
                    key: "death_" + mob.key,
                    sprite: "death",
                    x: mob.x,
                    y: mob.y,
                    num_frames: 8,
                    ms_per_step: 100,
                    scale: ((float)game.params.MOB_SIZE / (float)game.params.TILE_SIZE),
                    self_destruct_after_iterations: 1
                };
                createSpriteAnimation(death, game);

                //Sound
                playSound("damage", game);

                //Despawn mob
                game.mobs_around.remove(mob, i);
            }

        }

    }

    //PROJECTILE DAMAGE

    for(int i = 0; i < game.projectiles.size(); i++)
    {
        Projectile projectile;
        game.projectiles.retrieve(projectile, i);

        float projectile_x = projectile.x * game.params.TILE_SIZE;
        float projectile_y = projectile.y * game.params.TILE_SIZE;

        float distance_to_projectile = sqrt((player_x - projectile_x) * (player_x - projectile_x) + (player_y - projectile_y) * (player_y - projectile_y));

        if(distance_to_projectile < game.params.PROJECTILE_SIZE / 0.25)
        {
            player.health = player.health - game.params.PROJECTILE_DAMAGE;
            player.effects["invincibility"].TTL = 2;

            //Evaluate knockback
            float x_knockback = player_x - projectile_x;
            float y_knockback = player_y - projectile_y;
            evaluateKnockback(game.params.PROJECTILE_KNOCKBACK, x_knockback, y_knockback, player, game);

            //Despawn projectile
            game.projectiles.remove(projectile, i);

            //Sound
            playSound("damage", game);
        }

    }

}

//Knockback handling
void evaluateKnockback(float knockback, float x, float y, Player &player, Game_Instance &game)
{
    //Normalize x, y
    float distance = sqrt(x * x + y * y);
    x = x / distance;
    y = y / distance;

    player.knockback_correction = knockback;

    player.x_speed = player.x_speed + x * pow(knockback, 2);
    player.y_speed = player.y_speed + y * pow(knockback, 2);

    return;
}
