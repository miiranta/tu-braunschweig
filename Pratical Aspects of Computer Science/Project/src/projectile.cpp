//Projectile handling
Projectile createProjectile(float x, float y, float x_speed, float y_speed, int TTL)
{
    Projectile projectile;

    projectile.x = x;
    projectile.y = y;
    projectile.x_speed = x_speed;
    projectile.y_speed = y_speed;
    projectile.TTL = TTL;

    projectile.key = "projectile_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(rand() % 1000);

    return projectile;
}

void spawnProjectile(float x_target, float y_target, float x_origin, float y_origin, Game_Instance &game)
{
    //Center
    x_target = (x_target * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE;
    y_target = (y_target * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE;
    x_origin = (x_origin * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE;
    y_origin = (y_origin * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE;

    //Create
    float distance_to_player = sqrt((x_origin - x_target) * (x_origin - x_target) + (y_origin - y_target) * (y_origin - y_target));

    float proj_speed_x = (x_target - x_origin) / distance_to_player * game.params.PROJECTILE_SPEED;
    float proj_speed_y = (y_target - y_origin) / distance_to_player * game.params.PROJECTILE_SPEED;

    Projectile projectile = createProjectile(x_origin, y_origin, proj_speed_x, proj_speed_y, game.params.PROJECTILE_TTL);
    game.projectiles.insert(projectile, game.projectiles.size());

    playSound("snowball", game);

    return;
}

void despawnProjectiles(Game_Instance &game)
{
    int amount = game.projectiles.size();

    //Decrease TTL
    for(int i = 0; i < amount; i++)
    {
        Projectile projectile;
        game.projectiles.retrieve(projectile, i);

        projectile.TTL = projectile.TTL - 1;

        //Destroy if TTL is 0
        if(projectile.TTL <= 0)
        {
            //Animate death
            Animate_Params_Sprite death = {
                key: "death_" + projectile.key,
                sprite: "death",
                x: projectile.x,
                y: projectile.y,
                num_frames: 8,
                ms_per_step: 80,
                scale: ((float)game.params.PROJECTILE_SIZE / (float)game.params.TILE_SIZE) * 5,
                self_destruct_after_iterations: 1
            };
            createSpriteAnimation(death, game);

            game.projectiles.remove(projectile, i);
            i--;
            amount--;

            continue;
        }

        game.projectiles.replace(projectile, i);

    }

    return;
}

void evaluateProjectilesMovement(Game_Instance &game)
{
    int amount = game.projectiles.size();

    for(int i = 0; i < amount; i++)
    {
        Projectile projectile;
        game.projectiles.retrieve(projectile, i);

        //Move
        projectile.x = projectile.x + projectile.x_speed * 0.005;
        projectile.y = projectile.y + projectile.y_speed * 0.005;

        //Update
        game.projectiles.replace(projectile, i);

    }

}
