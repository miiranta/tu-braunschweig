//Debug
void drawTestLines(sf::RenderWindow &window, Game_Instance &game)
{
    int lines_amount = game.test_lines.size();

    for(int i = 0; i < lines_amount; i++)
    {
        Test_Line line;
        game.test_lines.retrieve(line, i);

        sf::Vertex line_vertices[] =
        {
            sf::Vertex(sf::Vector2f(line.x1, line.y1)),
            sf::Vertex(sf::Vector2f(line.x2, line.y2))
        };

        window.draw(line_vertices, 2, sf::Lines);
    }

    //Clear
    game.test_lines.clear();

}

void drawDebugStats(sf::RenderWindow &window, Game_Instance &game)
{
    const int yoffset = -50;

    //Draw FPS
    sf::Text text;
    text.setFont(game.draw_resources.fonts["pixel"]);
    text.setCharacterSize(50);
    text.setFillColor(sf::Color::White);
    text.setString("FPS: " + std::to_string(game.slow_fps));
    
    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 300 + yoffset);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw Steping tile
    int player_tile_x = floor((game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE);
    int player_tile_y = floor((game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) / game.params.TILE_SIZE);

    Map_Tile player_tile = getMapTile(player_tile_x, player_tile_y, game);

    text.setString("Tile: " + std::to_string(player_tile.type) + " Dec: " + std::to_string(player_tile.decoration) + " Biome: " + std::to_string(player_tile.biome));

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 350 + yoffset);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw X
    text.setString("X: " + std::to_string(player_tile_x));

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 400 + yoffset);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw Y
    text.setString("Y: " + std::to_string(player_tile_y));

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 450 + yoffset);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw player effects
    string effects = "Effects: ";
    for(auto const& [key, val] : game.player.effects)
    {
        if(val.TTL > 0) effects += key + "(" + std::to_string(game.player.effects[key].TTL) +") ";
    }

    text.setString(effects);

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 500 + yoffset);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw animations running
    string animations = "Animations: ";
    for(auto const& [key, val] : game.animation_running)
    {
        if(val == true) animations += key + " ";
    }

    text.setString(animations);

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 550 + yoffset);

    text.setOutlineThickness(2);

    window.draw(text);

    //Draw assertion time
    string assert_time = "Assertion time > ";
    assert_time =  assert_time + "\nwinEvents: " + std::to_string(game.time_winEvents) + " ";
    assert_time =  assert_time + "\nplayer: " + std::to_string(game.time_player) + " ";
    assert_time =  assert_time + "\nmobs: " + std::to_string(game.time_mobs) + " ";
    assert_time =  assert_time + "\nprojectiles: " + std::to_string(game.time_projectiles) + " ";
    assert_time =  assert_time + "\ngameEvents: " + std::to_string(game.time_gameEvents) + " ";
    assert_time =  assert_time + "\nzoom: " + std::to_string(game.time_zoom) + " ";
    assert_time =  assert_time + "\nview: " + std::to_string(game.time_view) + " ";
    assert_time =  assert_time + "\ndraw: " + std::to_string(game.time_draw_last) + " ";

    text.setString(assert_time);
    text.setCharacterSize(40);

    text.setPosition(0, 0);
    text.setScale(1, 1);
    text.move(10, 610 + yoffset);

    text.setOutlineThickness(2);

    window.draw(text);
}