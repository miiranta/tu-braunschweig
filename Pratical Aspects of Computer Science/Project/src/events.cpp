//Event handling
void windowEventHandler(sf::Event event, sf::RenderWindow &window, Game_Instance &game)
{
    //On Window Close
    if (event.type == sf::Event::Closed)
    {
        window.close();
    }

    //On Window Resize
    if (event.type == sf::Event::Resized) 
    {
        game.window_resized = true;
    }

    //On wheel movement
    if (event.type == sf::Event::MouseWheelScrolled) game.scroll_delta = event.mouseWheelScroll.delta;

    //On key press/release
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) game.key_pressed[sf::Keyboard::W] = true; else game.key_pressed[sf::Keyboard::W] = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) game.key_pressed[sf::Keyboard::A] = true; else game.key_pressed[sf::Keyboard::A] = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) game.key_pressed[sf::Keyboard::S] = true; else game.key_pressed[sf::Keyboard::S] = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) game.key_pressed[sf::Keyboard::D] = true; else game.key_pressed[sf::Keyboard::D] = false;

    //Number keys clicked
    for(int i = 0; i < 10; i++)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num0 + i) game.key_clicked[sf::Keyboard::Num0 + i] = true;
    }

    //Backspace clicked
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace) game.key_clicked[sf::Keyboard::BackSpace] = true;

    //Mouse position, press
    game.mouse_x = sf::Mouse::getPosition(window).x;
    game.mouse_y = sf::Mouse::getPosition(window).y;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) game.mouse_pressed[0] = true; else game.mouse_pressed[0] = false;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) game.mouse_pressed[1] = true; else game.mouse_pressed[1] = false;
    
    //Mouse click is just registered once
    if (event.type == sf::Event::MouseButtonPressed)
    {
        if(event.mouseButton.button == sf::Mouse::Left) game.mouse_clicked[0] = true;
        if(event.mouseButton.button == sf::Mouse::Right) game.mouse_clicked[1] = true;
    }

    //ESC click
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        if(game.current_menu == GAME) game.current_menu = PAUSE;
        else if(game.current_menu == PAUSE) 
        {
            Animate_Params anima = {};
            animationHandler("start_count", anima, game);

            game.current_menu = GAME;
        }
    }

    //F3 click - enable debug
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F3)
    {
        game.debug = !game.debug;
    }
}

void gameEventHandler(sf::RenderWindow &window, Game_Instance &game)
{
    //Get current stepping tile - Player center
    float player_x = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    float player_y = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
    int player_tile_x = floor(player_x / game.params.TILE_SIZE);
    int player_tile_y = floor(player_y / game.params.TILE_SIZE);
    Map_Tile player_tile = getMapTile(player_tile_x, player_tile_y, game);
    
    //STAR COLLECTED
    if(player_tile.decoration == STAR)
    {

        //Is star type already collected?
        Star search = {key: player_tile.key};
        int star = game.player.stars_collected.search(search);
        if(star == -1) game.player.stars_collected.insert(search, 0);
        else return;
        
        //Animates star collection
        Animate_Params anima = {
            x: player_tile_x,
            y: player_tile_y,
            ms_per_step: 50
        };
        animationHandler("star_collection", anima, game);

        Animate_Params_Sprite anima2 = {
                key: player_tile.key + "_star_gone",
                sprite: "star_gone",
                x: (float)player_tile.x,
                y: (float)player_tile.y,
                num_frames: 10,
                ms_per_step: 100,
                scale: 1,
                self_destruct_after_iterations: 1,
                ms_start_delay: 200
        };
        createSpriteAnimation(anima2, game);
        
        playSound("star", game);

        //Add to Biomes cleared
        //Inside animateStarCollection
        //It would be nice to have a callback ;(
    }

    //EFFECTS
    for(auto const& [key, effect] : game.player.effects)
    {
        if(key == "blindness")
        {
            if(effect.TTL > 0)
            {

                //Animation running?
                if(game.animation_running["blindness"] == false)
                {
                    Animate_Params anima = {
                        ms_per_step: 10
                    };
                    animationHandler("blindness", anima, game);
                }

                game.player.effects[key].TTL--;
            }
        }

        else if(key == "invincibility")
        {
            if(effect.TTL > 0)
            {
                //Animation running?
                if(game.animation_running["invincibility"] == false)
                {

                    Animate_Params anima = {
                        ms_per_step: 150
                    };
                    animationHandler("invincibility", anima, game);
                }

                game.player.effects[key].TTL--;
            }
        }

    }

    //DEATH
    if(game.player.health <= 0)
    {
        game.player.health = 0;
        game.current_menu = GAME_OVER;
        playSound("lost", game);
    }

    //WIN
    if(game.player.stars_collected.size() == BIOME_SIZE-1)
    {
        game.current_menu = GAME_OVER;

        //Set highscore
        if((game.current_highscore == 0) || (game.current_highscore  > game.tick - game.run_start_tick)) 
        {
            setHighscore(game, game.tick - game.run_start_tick);
        }

        playSound("win", game);
    }
}

long long int clockHandler(sf::Clock clock, long long int current_tick, sf::RenderWindow &window, Game_Instance &game)
{
    //Difference in ticks
    long long int new_tick = floor(clock.getElapsedTime().asSeconds() * game.params.TICKS_PER_SECOND);
    long long int tick_diff = new_tick - current_tick;

    //Update run_start_tick
    if(game.game_paused == true) game.run_start_tick += game.past_tick_diff;


    //Update game global tick
    game.tick = current_tick;
    game.past_tick_diff = tick_diff;

    //Last draw time and FPS
    float time_diff = clock.getElapsedTime().asMicroseconds() - game.last_draw_time;
    game.fps = 1000000 / time_diff;
    game.last_draw_time = clock.getElapsedTime().asMicroseconds();
    if(game.tick % 100) game.slow_fps = game.fps;

    //Do stuff
    for(int i = 0; i < tick_diff; i++)
    {

        //ALL

        //Evaluate window events
        if(game.debug) game.time_winEvents = clock.getElapsedTime().asMicroseconds();
        sf::Event event;
        while (window.pollEvent(event))
        {
            windowEventHandler(event, window, game);
        }
        if(game.debug) game.time_winEvents = clock.getElapsedTime().asMicroseconds() - game.time_winEvents;

        //GAME
        if((game.current_menu == GAME) && (game.game_paused == false))
        {
            //Player
            if(game.debug) game.time_player = clock.getElapsedTime().asMicroseconds();
            evaluatePlayerDamage(game, game.player); //First cuz of knockback
            evaluatePlayerMovement(game);
            if(game.debug) game.time_player = clock.getElapsedTime().asMicroseconds() - game.time_player;
            
            //Mobs
            if(game.debug) game.time_mobs = clock.getElapsedTime().asMicroseconds();
            evaluateMobsMovement(game);
            spawnMobs(game, game.player);
            if(game.debug) game.time_mobs = clock.getElapsedTime().asMicroseconds() - game.time_mobs;

            //Projectiles
            if(game.debug) game.time_projectiles = clock.getElapsedTime().asMicroseconds();
            despawnProjectiles(game);
            evaluateProjectilesMovement(game);
            if(game.debug) game.time_projectiles = clock.getElapsedTime().asMicroseconds() - game.time_projectiles;

            //Evaluate game events
            if(game.debug) game.time_gameEvents = clock.getElapsedTime().asMicroseconds();
            gameEventHandler(window, game);
            if(game.debug) game.time_gameEvents = clock.getElapsedTime().asMicroseconds() - game.time_gameEvents;

            //Evaluate zoom
            if(game.debug) game.time_zoom = clock.getElapsedTime().asMicroseconds();
            evaluateZoom(game);
            if(game.debug) game.time_zoom = clock.getElapsedTime().asMicroseconds() - game.time_zoom;
        }

        //ALL x2

        //Evaluate view
        if(game.debug) game.time_view = clock.getElapsedTime().asMicroseconds();
        viewHandler(window, game);
        if(game.debug) game.time_view = clock.getElapsedTime().asMicroseconds() - game.time_view;

    }

    

    return new_tick;
}
