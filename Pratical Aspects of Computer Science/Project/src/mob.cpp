//Mob handling
Mob createMob(int x, int y)
{
    Mob mob;

    //key
    mob.key = std::to_string(x) + "." + std::to_string(y) + "." + std::to_string(rand());

    //Equal chances of each type
    Mob_Type type = (Mob_Type) (rand() % MOB_TYPE_SIZE);

    //Mob properties
    if(type == RUNNER) 
    {
        mob.colision = true;
    }
    else if(type == SHOOTER) 
    {
        mob.colision = true;
    }
    else if(type == FLYER) 
    {
        mob.colision = false;
    }

    mob.type = type;

    mob.x = x;
    mob.y = y;

    mob.is_alert = false;

    return mob;
}

void spawnMobs(Game_Instance &game, Player &player)
{
    //Check for despawn
    despawnMobs(game, player);

    //Check for MAX
    int mobs_amount = game.mobs_around.size();
    if(mobs_amount >= game.params.MOB_COUNT_MAX) return;

    //Get a random position in the render distance
    float smax_x = player.x + game.params.RENDER_DISTANCE;
    float smin_x = player.x - game.params.RENDER_DISTANCE;
    float smax_y = player.y + game.params.RENDER_DISTANCE;
    float smin_y = player.y - game.params.RENDER_DISTANCE;

    float x = smin_x + rand() / (RAND_MAX / (smax_x - smin_x + 1) + 1);
    float y = smin_y + rand() / (RAND_MAX / (smax_y - smin_y + 1) + 1);

    //Invalid position?
    float distance_to_player = sqrt((x - player.x) * (x - player.x) + (y - player.y) * (y - player.y));
    
    Map_Tile tile = getMapTile(x, y, game);

    if(distance_to_player > game.params.RENDER_DISTANCE) return; //Too far away
    if(distance_to_player < game.params.MOB_SPAWN_PROTECTION_RANGE) return; //Too close
    if(tile.colision == true) return; //Colision
    if(tile.biome == HOPE) return; //Biome

    //Spawn
    Mob mob = createMob((int)x, (int)y);

    //Add
    game.mobs_around.insert(mob, mobs_amount);

}

void despawnMobs(Game_Instance &game, Player &player) //Might have problems with multiple players
{
    int mobs_amount = game.mobs_around.size();

    for(int i = 0; i < mobs_amount; i++)
    {
        Mob mob;
        game.mobs_around.retrieve(mob, i);

        Map_Tile tile = getMapTile(mob.x, mob.y, game);

        float distance_to_player = sqrt((mob.x - player.x) * (mob.x - player.x) + (mob.y - player.y) * (mob.y - player.y));
        
        //Rules
        if(distance_to_player > game.params.RENDER_DISTANCE)
        {
            game.mobs_around.remove(mob, i);
        }
         
        if(tile.biome == HOPE) 
        {
            //Neighbouring tiles are all HOPE
            bool HOPE_neighbour = true;
            for(int j = -1; j < 2; j++)
            {
                for(int k = -1; k < 2; k++)
                {
                    Map_Tile tile = getMapTile(mob.x + j, mob.y + k, game);
                    if(tile.biome != HOPE) HOPE_neighbour = false;
                }
            }
            
            if(HOPE_neighbour == true)
            {
                //animate death
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


                //Is mob close to player?
                if(distance_to_player < game.params.RENDER_DISTANCE / 2)
                {
                    //play sound
                    playSound("dead", game);
                }

                game.mobs_around.remove(mob, i);
            }
             
        }

    }

    return;
}

void evaluateMobsMovement(Game_Instance &game)
{
    //For each mob
    int mobs_amount = game.mobs_around.size();
    for(int i = 0; i < mobs_amount; i++)
    {
        Mob mob;
        game.mobs_around.retrieve(mob, i);

        float MOB_SPEED_MAX = game.params.MOB_SPEED_MAX * 0.005;

        bool reachable = true; //Assume reachable

        //Player distance
        float player_x = game.player.x;
        float player_y = game.player.y;
        float mob_x = mob.x;
        float mob_y = mob.y;
        float distance_to_player = sqrt((mob_x - player_x) * (mob_x - player_x) + (mob_y - player_y) * (mob_y - player_y));

        //Alert or not
        int action_range = game.params.MOB_ACTION_RANGE;
        if(mob.is_alert == true) action_range = game.params.MOB_ACTION_RANGE_IF_ALERT;

        //NEAR
        if(distance_to_player < action_range)
        {
            //Every 2 ticks
            if((game.tick + i) % 2 == 0)
            {
                //Try to pathfind
                Next_Tile ntile = Pathfind(mob, game.player, game);
               
                //Go for it
                if(ntile.reachable == true) 
                {
                    if(ntile.keep_position == false)
                    {
                        //Is tile in HOPE biome?
                        Map_Tile tile = getMapTile(ntile.x, ntile.y, game);
                        if(tile.biome == HOPE)
                        {
                            ntile.keep_position = true;
                        }
                        else
                        {
                            float angle = atan2(ntile.y - mob_y, ntile.x - mob_x);
                            mob.x_speed = cos(angle) * MOB_SPEED_MAX;
                            mob.y_speed = sin(angle) * MOB_SPEED_MAX;

                            if(game.debug == true)
                            {
                                float mob_x_center = mob_x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;
                                float mob_y_center = mob_y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;
                                float ntile_x_center = ntile.x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;
                                float ntile_y_center = ntile.y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2;
                                Test_Line line = {mob_x_center, mob_y_center, ntile_x_center, ntile_y_center};
                                game.test_lines.insert(line, game.test_lines.size());
                            }

                        }

                    }
                    
                    //Keep position ON
                    else
                    {
                        mob.x_speed = 0;
                        mob.y_speed = 0;
                    }
                }
                else reachable = false;

                //SHOOTER - Shoot
                //Reachable true, Keep position true, Mob type is SHOOTER
                if((ntile.reachable == true) && (ntile.keep_position == true) && (mob.type == SHOOTER) && ((game.tick + i) % 2 == 0))
                {
                    if(mob.cooldown == 0)
                    {
                        spawnProjectile(player_x, player_y, mob_x, mob_y, game);
                        mob.cooldown = game.params.SHOOTER_SHOT_COOLDOWN;  
                    }
                    else
                    {
                        mob.cooldown = mob.cooldown - 1;
                    }
                }
                    
            }
            
        }

        //FAR
        if((reachable == false) || (distance_to_player >= game.params.MOB_ACTION_RANGE))
        {
      
            //Every 100 ticks
            if((game.tick + i) % 100 == 0)
            {
                //Random movement
                float smin = -MOB_SPEED_MAX * 0.5;
                float smax = MOB_SPEED_MAX * 0.5;

                mob.x_speed = ((float)rand() / RAND_MAX) * (smax - smin) + smin;
                mob.y_speed = ((float)rand() / RAND_MAX) * (smax - smin) + smin;
            }

        }

        //Colision
        evaluateMobColision(game, mob);

        //Max speed
        if(mob.x_speed > MOB_SPEED_MAX) mob.x_speed = MOB_SPEED_MAX;
        if(mob.x_speed < -MOB_SPEED_MAX) mob.x_speed = -MOB_SPEED_MAX;

        //Move
        mob.x = mob.x + mob.x_speed;
        mob.y = mob.y + mob.y_speed;

        //Update
        game.mobs_around.replace(mob, i);

    }

}

Next_Tile Pathfind(Mob &mob, Player &player, Game_Instance &game)
{
    Next_Tile ntile;
    ntile.reachable = false;

    //Alert or not
    int action_range = game.params.MOB_ACTION_RANGE;
    if(mob.is_alert == true) action_range = game.params.MOB_ACTION_RANGE_IF_ALERT;

    //RUNNER
    //Path find to player
    if(mob.type == RUNNER)
    {

        //JUST GO FOR IT
        //If distance is small than 1 tile, just go for it
        float distance_to_player = sqrt((mob.x - player.x) * (mob.x - player.x) + (mob.y - player.y) * (mob.y - player.y));
        if(distance_to_player < 1)
        {
            ntile.x = player.x;
            ntile.y = player.y;
            ntile.reachable = true;
        }

        //STRAGHT LINE
        if(ntile.reachable == false)
        {
            ntile = StraightLine(mob.x, mob.y, player.x, player.y, action_range, game);
        }
        
        //if reachable by straight line, then mob is alert
        if(ntile.reachable == true) mob.is_alert = true;

        //ASTAR
        if((mob.is_alert == true) && (ntile.reachable == false))
        {
            int mob_x_center_tile = floor((mob.x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE);
            int mob_y_center_tile = floor((mob.y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE);
            int player_x_center_tile = floor((player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE);
            int player_y_center_tile = floor((player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE);
            ntile = AStar(mob_x_center_tile, mob_y_center_tile, player_x_center_tile, player_y_center_tile, action_range, game);
            ntile.x = (ntile.x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE;
            ntile.y = (ntile.y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE;
        }

    }

    //SHOOTER
    //Path find to player soroundings
    else if(mob.type == SHOOTER)
    {

        //STRAGHT LINE
        ntile = StraightLine(mob.x, mob.y, player.x, player.y, action_range, game);

        //if reachable by straight line, then mob is alert
        if(ntile.reachable == true) mob.is_alert = true;

        //If this mobs sees you, it will try to get closer until it can shoot
        float distance_to_player = sqrt((mob.x - player.x) * (mob.x - player.x) + (mob.y - player.y) * (mob.y - player.y));
        if((distance_to_player < game.params.SHOOTER_RANGE) && (ntile.reachable == true))
        {
            //Keep distance
            ntile.keep_position = true;
            ntile.reachable = true;
            return ntile;
        }
        if((distance_to_player < 1.5))
        {
            //Keep distance - non conditional
            ntile.keep_position = true;
            ntile.reachable = true;
            return ntile;
        }

        //ASTAR
        if((mob.is_alert == true) && (ntile.reachable == false))
        {
            int mob_x_center_tile = floor((mob.x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE);
            int mob_y_center_tile = floor((mob.y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE);
            int player_x_center_tile = floor((player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE);
            int player_y_center_tile = floor((player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE);
            ntile = AStar(mob_x_center_tile, mob_y_center_tile, player_x_center_tile, player_y_center_tile, action_range, game);
            ntile.x = (ntile.x * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE;
            ntile.y = (ntile.y * game.params.TILE_SIZE + game.params.MOB_SIZE / 2) / game.params.TILE_SIZE;
        }

    }

    //FLYER
    //Follow player
    else if(mob.type == FLYER)
    {
        //Player distance
        float distance_to_player = sqrt((mob.x - player.x) * (mob.x - player.x) + (mob.y - player.y) * (mob.y - player.y));
        
        //In range
        if(distance_to_player < action_range)
        {
            ntile.x = player.x;
            ntile.y = player.y;
            ntile.reachable = true;

            //Mob gets alert
            mob.is_alert = true;
            
        }

    }

    return ntile;

}

Next_Tile StraightLine(float cx, float cy, float dx, float dy, int max_path_len, Game_Instance &game)
{
    Next_Tile ntile = Next_Tile();
    bool straight_line = true;
    float step = 0.01;
    int x_last_tested = 0, y_last_tested = 0;

    //Borders - mob & player
    float cx1 = cx, cy1 = cy, dx1 = dx, dy1 = dy;
    float cx2 = (cx * game.params.TILE_SIZE + game.params.MOB_SIZE) / game.params.TILE_SIZE, cy2 = cy, dx2 = (dx * game.params.TILE_SIZE + game.params.PLAYER_SIZE) / game.params.TILE_SIZE, dy2 = dy;
    float cx3 = cx, cy3 = (cy * game.params.TILE_SIZE + game.params.MOB_SIZE) / game.params.TILE_SIZE, dx3 = dx, dy3 = (dy * game.params.TILE_SIZE + game.params.PLAYER_SIZE) / game.params.TILE_SIZE;
    float cx4 = (cx * game.params.TILE_SIZE + game.params.MOB_SIZE) / game.params.TILE_SIZE, cy4 = (cy * game.params.TILE_SIZE + game.params.MOB_SIZE) / game.params.TILE_SIZE, dx4 = (dx * game.params.TILE_SIZE + game.params.PLAYER_SIZE) / game.params.TILE_SIZE, dy4 = (dy * game.params.TILE_SIZE + game.params.PLAYER_SIZE) / game.params.TILE_SIZE;

    //TESTING Draw testlines
    //Test_Line line1 = {cx1 * game.TILE_SIZE, cy1 * game.TILE_SIZE, dx1 * game.TILE_SIZE, dy1 * game.TILE_SIZE};
    //Test_Line line2 = {cx2 * game.TILE_SIZE, cy2 * game.TILE_SIZE, dx2 * game.TILE_SIZE, dy2 * game.TILE_SIZE};
    //Test_Line line3 = {cx3 * game.TILE_SIZE, cy3 * game.TILE_SIZE, dx3 * game.TILE_SIZE, dy3 * game.TILE_SIZE};
    //Test_Line line4 = {cx4 * game.TILE_SIZE, cy4 * game.TILE_SIZE, dx4 * game.TILE_SIZE, dy4 * game.TILE_SIZE};
    //game.test_lines.insert(line1, 0);
    //.test_lines.insert(line2, 0);
    //game.test_lines.insert(line3, 0);
    //game.test_lines.insert(line4, 0);

    //B1
    for(float t = 0; t < 1; t = t + step)
    {
        float x = cx1 + (dx1 - cx1) * t;
        float y = cy1 + (dy1 - cy1) * t;

        //See if it's a new tile
        int rx = round(x);
        int ry = round(y);
        if(rx == x_last_tested && ry == y_last_tested) continue;
        x_last_tested = rx;
        y_last_tested = ry;

        Map_Tile tile = getMapTile(rx, ry, game);
        if(tile.colision == true) 
        {
            straight_line = false;
            break;
        }
    }

    //B2
    if(straight_line == true)
    {
        for(float t = 0; t < 1; t = t + step)
        {
            float x = cx2 + (dx2 - cx2) * t;
            float y = cy2 + (dy2 - cy2) * t;

            //See if it's a new tile
            int rx = round(x);
            int ry = round(y);
            if(rx == x_last_tested && ry == y_last_tested) continue;
            x_last_tested = rx;
            y_last_tested = ry;

            Map_Tile tile = getMapTile(rx, ry, game);
            if(tile.colision == true) 
            {
                straight_line = false;
                break;
            }
        }
    }

    //B3
    if(straight_line == true)
    {
        for(float t = 0; t < 1; t = t + step)
        {
            float x = cx3 + (dx3 - cx3) * t;
            float y = cy3 + (dy3 - cy3) * t;

            //See if it's a new tile
            int rx = round(x);
            int ry = round(y);
            if(rx == x_last_tested && ry == y_last_tested) continue;
            x_last_tested = rx;
            y_last_tested = ry;

            Map_Tile tile = getMapTile(rx, ry, game);
            if(tile.colision == true) 
            {
                straight_line = false;
                break;
            }
        }
    }

    //B4
    if(straight_line == true)
    {
        for(float t = 0; t < 1; t = t + step)
        {
            float x = cx4 + (dx4 - cx4) * t;
            float y = cy4 + (dy4 - cy4) * t;

            //See if it's a new tile
            int rx = round(x);
            int ry = round(y);
            if(rx == x_last_tested && ry == y_last_tested) continue;
            x_last_tested = rx;
            y_last_tested = ry;

            Map_Tile tile = getMapTile(rx, ry, game);
            if(tile.colision == true) 
            {
                straight_line = false;
                break;
            }
        }
    }
    
    //Straight line valid
    if(straight_line == true)
    {
        ntile.x = dx;
        ntile.y = dy;
        ntile.reachable = true;
    }

    //Straight line invalid
    else
    {
        ntile.reachable = false;
    }

    return ntile;

}

Next_Tile AStar(float cx, float cy, float dx, float dy, int max_path_len, Game_Instance &game)
{
    Next_Tile ntile = Next_Tile();
    ntile.reachable = false; //Assume not reachable

    //ASTAR
    int search_factor = game.params.ASTAR_SEARCH_FACTOR;
    List<Next_Tile> path = AStar_path(cx, cy, dx, dy, max_path_len * search_factor, game);
    
    //No path found
    if(path.size() == 0) 
    {
        ntile.reachable = false;
        return ntile;
    }

    //Path found
    else if(path.size() == 1)
    {
        path.retrieve(ntile, 0);
        ntile.reachable = true;
    }

    //Path found
    else
    {
        path.retrieve(ntile, 1);
        ntile.reachable = true;
        return ntile;
    }

    return ntile;

}

List<Next_Tile> AStar_path(float cx, float cy, float dx, float dy, int max_it, Game_Instance &game)
{
    //cx, cy - current position
    //dx, dy - destination position
    //HEURISTIC - Euclidean distance
    //COSTS - 1 per tile
    //map is game.map, tiles are get by using getMapTile(x, y, game)

    //Floor coords
    cx = floor(cx);
    cy = floor(cy);
    dx = floor(dx);
    dy = floor(dy);

    //Linked list of next tiles
    List<Next_Tile> current_nodes = List<Next_Tile>();
    List<Next_Tile> analyzed_nodes = List<Next_Tile>();
    List<Next_Tile> fpath = List<Next_Tile>();

    //Trivial case - destination is current
    if(cx == dx && cy == dy)
    {
        Next_Tile node = {
            x: dx,
            y: dy,
            cost: 0,
            heuristic: 0,
            path: 0
        };
        fpath.insert(node, 0);
        return fpath;
    }

    //Add initial nodes
    Next_Tile start = {
        x: cx,
        y: cy,
        cost: 0,
        heuristic: sqrt((dx - cx) * (dx - cx) + (dy - cy) * (dy - cy)),
        path: 0
    };
    current_nodes.insert(start, 0);

    //Iterate 
    bool found = false;
    for(int i = 0; i < max_it; i++)
    {
        //Find best node
        Next_Tile best_node;
        current_nodes.retrieve(best_node, 0);

        int j_best = 0;
        for(int j = 0; j < current_nodes.size(); j++)
        {
            Next_Tile node;
            current_nodes.retrieve(node, j);

            //Copy reference
            if(node.cost + node.heuristic < best_node.cost + best_node.heuristic) 
            {
                best_node = node;
                j_best = j;
            }

        }

        //Is it the destination?
        if(best_node.x == dx && best_node.y == dy)
        {

            //Add to analyzed
            analyzed_nodes.insert(best_node, analyzed_nodes.size());

            //Remove best node
            current_nodes.remove(best_node, j_best);

            found = true;
            break;
        }

        //Add neighbours
        for(int j = -1; j < 2; j++)
        {
            for(int k = -1; k < 2; k++)
            {
                //Not the same tile
                if(j == 0 && k == 0) continue;

                //Diagonals not allowed
                if(abs(j) + abs(k) == 2) continue;
                
                //New neighbour
                Next_Tile neighbour = {
                    x: best_node.x + j,
                    y: best_node.y + k,
                    cost: best_node.cost + 1,
                    heuristic: sqrt((dx - best_node.x - j) * (dx - best_node.x - j) + (dy - best_node.y - k) * (dy - best_node.y - k)),
                    path: analyzed_nodes.size()
                };

                //Ignore neighbour with colision
                Map_Tile tile = getMapTile(neighbour.x, neighbour.y, game);
                if(tile.colision == true) continue;

                //Add to list
                current_nodes.insert(neighbour, current_nodes.size());

            }
        }

        //Add to analyzed - before neighbours
        analyzed_nodes.insert(best_node, analyzed_nodes.size());

        //Remove best node
        current_nodes.remove(best_node, j_best);

    }

    //Build path
    if(found == true) //Found
    {
        Next_Tile node;
        analyzed_nodes.retrieve(node, analyzed_nodes.size() - 1);
        
        while(node.x != cx || node.y != cy)
        {
            fpath.insert(node, 0);
            analyzed_nodes.retrieve(node, node.path);
        }

        //Add last node
        fpath.insert(node, 0);

    }

    else //No path found
    {
        //Do nothing - path is empty
    }

    return fpath;
}

//Colision handling
void evaluateMobColision(Game_Instance &game, Mob &mob)
{
    float mob_left = mob.x * game.params.TILE_SIZE;
    float mob_right = mob_left + game.params.MOB_SIZE;
    float mob_top = mob.y * game.params.TILE_SIZE;
    float mob_bot = mob_top + game.params.MOB_SIZE;

    bool stop_left = false;
    bool stop_right = false;
    bool stop_top = false;
    bool stop_bot = false;

    //Map tile colision
    const float mob_to_tile_ratio = (float)game.params.MOB_SIZE / (float)game.params.TILE_SIZE;
    const int dist_to_check = ceil(mob_to_tile_ratio) + 1;

    int mob_tile_x = floor(mob.x);
    int mob_tile_y = floor(mob.y);

    for(int i = -dist_to_check; i < dist_to_check; i++)
    {
        for(int j = -dist_to_check; j < dist_to_check; j++)
        {
            int tile_x = i + mob_tile_x;
            int tile_y = j + mob_tile_y;

            //Invalid tile - no colision
            Map_Tile mtile = getMapTile(tile_x, tile_y, game);
            if(mtile.biome != HOPE)
            {
                if(mob.colision == false) continue;
                if(mtile.colision == false) continue;
            }

            //Tile colision
            float tile_left = tile_x * game.params.TILE_SIZE;
            float tile_right = tile_left + game.params.TILE_SIZE;
            float tile_top = tile_y * game.params.TILE_SIZE;
            float tile_bot = tile_top + game.params.TILE_SIZE;

            float mob_left_new = mob_left + mob.x_speed;
            float mob_right_new = mob_right + mob.x_speed;
            float mob_top_new = mob_top + mob.y_speed;
            float mob_bot_new = mob_bot + mob.y_speed;

            //Is mob in tile?
            if(mob_right_new > tile_left && mob_left_new < tile_right && mob_bot_new > tile_top && mob_top_new < tile_bot)
            {
                //Which side is the nearest?
                float dist_left = abs(mob_right_new - tile_left);
                float dist_right = abs(mob_left_new - tile_right);
                float dist_top = abs(mob_bot_new - tile_top);
                float dist_bot = abs(mob_top_new - tile_bot);

                float min_dist = std::min({dist_left, dist_right, dist_top, dist_bot});

                //Which side to stop
                if(min_dist == dist_left){

                    if(mob.x_speed < 0) mob.x_speed = -mob.x_speed;

                    stop_left = true;
                }
                if(min_dist == dist_right){

                    if(mob.x_speed > 0) mob.x_speed = -mob.x_speed;

                    stop_right = true;
                }
                if(min_dist == dist_top){

                    if(mob.y_speed < 0) mob.y_speed = -mob.y_speed;

                    stop_top = true;
                }
                if(min_dist == dist_bot){

                    if(mob.y_speed > 0) mob.y_speed = -mob.y_speed;

                    stop_bot = true;
                }

            }

        }

    }

    //Stopping
    if(stop_left) mob.x_speed = -mob.x_speed * game.params.MOB_BOUNCE;
    if(stop_right) mob.x_speed = -mob.x_speed * game.params.MOB_BOUNCE;
    if(stop_top) mob.y_speed = -mob.y_speed * game.params.MOB_BOUNCE;
    if(stop_bot) mob.y_speed = -mob.y_speed * game.params.MOB_BOUNCE;

}
