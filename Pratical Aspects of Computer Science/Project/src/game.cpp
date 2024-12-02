#include <iostream>
#include <fstream>
#include <math.h>
#include <thread>
#include <memory>
#include <queue>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include "aux/AVL/avl.cpp"                      //This is an AVL tree implementation
#include "aux/LinkedList/list.cpp"              //This is a Linked List implementation
#include "aux/OpenSimplex/OpenSimplexNoise.cpp" //https://github.com/deerel/OpenSimplexNoise - Free software

// ---------------------------------------------

//Defs
#include "headers/resources.hpp"
#include "headers/menu_struc.hpp"
#include "headers/map_struc.hpp"
#include "headers/player_struc.hpp"
#include "headers/mob_struc.hpp"
#include "headers/projectile_struc.hpp"
#include "headers/animations_struc.hpp"
#include "headers/debug_struc.hpp"

#include "headers/game.hpp" //

#include "headers/menu_func.hpp"
#include "headers/events.hpp"
#include "headers/views.hpp"
#include "headers/map_func.hpp"
#include "headers/player_func.hpp"
#include "headers/mob_func.hpp"
#include "headers/projectile_func.hpp"
#include "headers/animations_func.hpp"
#include "headers/sounds.hpp"
#include "headers/debug_func.hpp"
#include "headers/draw.hpp"

//Code
#include "resources.cpp"
#include "sounds.cpp"
#include "menu.cpp"
#include "events.cpp"
#include "views.cpp"
#include "map.cpp"
#include "player.cpp"
#include "mob.cpp"
#include "projectile.cpp"
#include "animations.cpp"
#include "debug.cpp"
#include "draw.cpp"

// ---------------------------------------------

//Game instance handling
Game_Instance createGameInstance(Game_Params params)
{
    Game_Instance game = {
        params: params
    };

    game.params.MAP_SEED_STR = std::to_string(rand());

    loadDrawResources(game.draw_resources);
    game.map = createMap(game.params.MAP_SEED_STR);
    game.player = createPlayer();
    findPlayerSpawn(game.player, game);

    game.visibility_distance = game.params.RENDER_DISTANCE;

    game.params = setDifficultyConfig(game.params, game.params.DIFFICULTY);

    loadHighscore(game);

    game.run_start_tick = game.tick;

    //TESTING
    game.test_lines = List<Test_Line>();

    return game;
}

void remakeGameInstance(Game_Instance &game, Game_Params params)
{
    //New params
    game.params = params;

    //Game tick
    game.run_start_tick = game.tick;

    //New map
    destroyMap(game.map);
    game.map = createMap(game.params.MAP_SEED_STR);

    //New player
    game.player = createPlayer();

    //New mobs
    game.mobs_around = List<Mob>();

    //New projectiles
    game.projectiles = List<Projectile>();

    //Reload resources
    Draw_Resources draw_resources;
    loadDrawResources(draw_resources);

    //Clear animations
    game.animation_running = std::map<std::string, bool>();
    game.animation_running_sprite = List<Animate_Params_Sprite>();

    //Etc
    game.visibility_distance = game.params.RENDER_DISTANCE;
    findPlayerSpawn(game.player, game);

    //Difficulty
    game.params = setDifficultyConfig(game.params, game.params.DIFFICULTY);

    //Highscore
    loadHighscore(game);

    //TESTING
    game.test_lines = List<Test_Line>();

    return;
}

void newGame(Game_Instance &game)
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktop, game.params.TITLE); //sf::RenderWindow window(desktop, TITLE, sf::Style::Fullscreen);

    sf::Clock clock = sf::Clock();
    long long int clock_tick = 0;

    //Main loop
    while (window.isOpen())
    {
        //Events
        //Inside clockHandler

        //Clock & Ticks
        clock_tick = clockHandler(clock, clock_tick, window, game);
        
        //Draw
        if(game.debug) game.time_draw = clock.getElapsedTime().asMicroseconds();
        drawHandler(window, game);
        if(game.debug) 
        {
            game.time_draw = clock.getElapsedTime().asMicroseconds() - game.time_draw;
            game.time_draw_last = game.time_draw;
        }
        
    }

    //End / Destroy sfml objects
    destroyMap(game.map);
    return;
}

//Game config
Game_Params setDifficultyConfig(Game_Params params, Difficulty diff)
{
    switch (diff)
    {
        case EASY:
            params.MOB_COUNT_MAX = 30;

            params.RUNNER_DAMAGE = 2;
            params.FLYER_DAMAGE = 5;
            params.PROJECTILE_DAMAGE = 5;

            params.STAR_RARITY = 0.002;
            break;
        case NORMAL:
            params.MOB_COUNT_MAX = 50;

            params.RUNNER_DAMAGE = 4;
            params.FLYER_DAMAGE = 10;
            params.PROJECTILE_DAMAGE = 10;

            params.STAR_RARITY = 0.001;
            break;
        case HARD:
            params.MOB_COUNT_MAX = 70;

            params.RUNNER_DAMAGE = 6;
            params.FLYER_DAMAGE = 15;
            params.PROJECTILE_DAMAGE = 15;

            params.STAR_RARITY = 0.0005;
            break;
        case OH_LORD:
            params.MOB_COUNT_MAX = 90;

            params.RUNNER_DAMAGE = 8;
            params.FLYER_DAMAGE = 20;
            params.PROJECTILE_DAMAGE = 20;

            params.STAR_RARITY = 0.00025;
            break;
        default:
            break;
    }

    return params;
}

//Game data
void loadHighscore(Game_Instance &game)
{
    //Data folder exists?
    ifstream data_folder("../data");
    if(!data_folder)
    {
        system("mkdir ../data");
    }

    //Files exist?
    ifstream file1("../data/highscore_easy.txt");
    ifstream file2("../data/highscore_normal.txt");
    ifstream file3("../data/highscore_hard.txt");
    ifstream file4("../data/highscore_oh_lord.txt");

    if(!file1 || !file2 || !file3 || !file4)
    {
        ofstream file1("../data/highscore_easy.txt");
        ofstream file2("../data/highscore_normal.txt");
        ofstream file3("../data/highscore_hard.txt");
        ofstream file4("../data/highscore_oh_lord.txt");

        file1 << 0;
        file2 << 0;
        file3 << 0;
        file4 << 0;

        file1.close();
        file2.close();
        file3.close();
        file4.close();
    }

    switch (game.params.DIFFICULTY)
    {
        case EASY:
        {
            ifstream file("../data/highscore_easy.txt");
            string line = "";
            getline(file, line);
            game.current_highscore = stol(line);
            file.close();
            break;
        }
            
        case NORMAL:
        {
            ifstream file("../data/highscore_normal.txt");
            string line = "";
            getline(file, line);
            game.current_highscore = stol(line);
            file.close();
            break;
        }    
        case HARD:
        {
            ifstream file("../data/highscore_hard.txt");
            string line = "";
            getline(file, line);
            game.current_highscore = stol(line);
            file.close();
            break;
        }    
        case OH_LORD:
        {
            ifstream file("../data/highscore_oh_lord.txt");
            string line = "";
            getline(file, line);
            game.current_highscore = stol(line);
            file.close();
            break;
        }    
        default:
            break;
    }

    return;
}

void setHighscore(Game_Instance &game, long int highscore)
{
    switch (game.params.DIFFICULTY)
    {
        case EASY:
        {
            ofstream file("../data/highscore_easy.txt");
            file << highscore;
            file.close();
            break;
        }
        case NORMAL:
        {
            ofstream file("../data/highscore_normal.txt");
            file << highscore;
            file.close();
            break;
        }
        case HARD:
        {
            ofstream file("../data/highscore_hard.txt");
            file << highscore;
            file.close();
            break;
        }    
        case OH_LORD:
        {
            ofstream file("../data/highscore_oh_lord.txt");
            file << highscore;
            file.close();
            break;
        }    
        default:
            return;
    }

    return;
}