//View handling
void viewHandler(sf::RenderWindow &window, Game_Instance &game)
{
    //Clear views
    game.views.clear();
    game.view_names.clear();

    switch (game.current_menu)
    {
        case MAIN:
            setGameView(window, game);
            setMenuView(window, game);

            game.music_volume = 70;
            playMusic("menu", game);

            window.setMouseCursorGrabbed(false);
            window.setMouseCursorVisible(true);
            break;

        case CONFIG:
            setGameView(window, game);
            setMenuView(window, game);

            game.music_volume = 50;
            playMusic("menu", game);

            window.setMouseCursorGrabbed(false);
            window.setMouseCursorVisible(true);
            break;

        case TUTORIAL:
            setGameView(window, game);
            setMenuView(window, game);

            game.music_volume = 50;
            playMusic("menu", game);

            window.setMouseCursorGrabbed(false);
            window.setMouseCursorVisible(true);
            break;

        case PAUSE:
            setGameView(window, game);
            setGameOverlayView(window, game);
            setMenuView(window, game);

            game.music_volume = 50;
            playMusic("game", game);

            window.setMouseCursorGrabbed(false);
            window.setMouseCursorVisible(true);
            break;

        case GAME_OVER:
            setGameView(window, game);
            setMenuView(window, game);

            game.music_volume = 50;
            playMusic("menu", game);

            window.setMouseCursorGrabbed(false);
            window.setMouseCursorVisible(true);
            break;

        case GAME:
            setGameView(window, game);
            setGameOverlayView(window, game);

            game.music_volume = 70;
            playMusic("game", game);

            window.setMouseCursorGrabbed(true);
            window.setMouseCursorVisible(false);
            break;
    }

}

void setGameView(sf::RenderWindow &window, Game_Instance &game)
{
    int MAX_EDGE_DIST;
    MAX_EDGE_DIST = std::min({window.getSize().x / 2 * game.draw_scale, window.getSize().y / 2 * game.draw_scale});
    MAX_EDGE_DIST = std::min({(int)((game.params.MINIMAP_SIZE + game.params.PLAYER_SIZE) * game.draw_scale), MAX_EDGE_DIST});
    game.edge_dist = MAX_EDGE_DIST;

    float &vox = game.view_offset_x;
    float &voy = game.view_offset_y;

    if(game.window_center_player) 
    {
        vox = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
        voy = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2;
        game.window_center_player = false;
    }

    float visible_x = window.getSize().x * game.draw_scale;
    float visible_y = window.getSize().y * game.draw_scale;

    //Is player near the edge?
    float dist_bot = visible_y / 2 - (game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) + voy;
    float dist_top = visible_y - dist_bot;
    float dist_right = visible_x / 2 - (game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2) + vox;
    float dist_left = visible_x - dist_right;

    //Movement case
    if(dist_bot < MAX_EDGE_DIST && game.player.y_speed > 0){
        voy = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 - visible_y / 2 + MAX_EDGE_DIST;
    }

    if(dist_top < MAX_EDGE_DIST && game.player.y_speed < 0){
        voy = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 + visible_y / 2 - MAX_EDGE_DIST;
    }

    if(dist_right < MAX_EDGE_DIST && game.player.x_speed > 0){
        vox = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 - visible_x / 2 + MAX_EDGE_DIST;
    }

    if(dist_left < MAX_EDGE_DIST && game.player.x_speed < 0){
        vox = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 + visible_x / 2 - MAX_EDGE_DIST;
    }

    //Zoom and Resize case
    if((abs(game.scroll_speed) > 0.0001) || (game.window_resized == true))
    {
        game.window_resized = false;

        if(dist_bot < MAX_EDGE_DIST){
            voy = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 - visible_y / 2 + MAX_EDGE_DIST;
        }

        if(dist_top < MAX_EDGE_DIST){
            voy = game.player.y * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 + visible_y / 2 - MAX_EDGE_DIST;
        }

        if(dist_right < MAX_EDGE_DIST){
            vox = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 - visible_x / 2 + MAX_EDGE_DIST;
        }

        if(dist_left < MAX_EDGE_DIST){
            vox = game.player.x * game.params.TILE_SIZE + game.params.PLAYER_SIZE / 2 + visible_x / 2 - MAX_EDGE_DIST;
        }
    }

    //Game visible area
    game.vis_a_x1 = vox - visible_x / 2;
    game.vis_a_y1 = voy - visible_y / 2;
    game.vis_a_x2 = vox + visible_x / 2;
    game.vis_a_y2 = voy + visible_y / 2;

    sf::FloatRect visibleArea(game.vis_a_x1, game.vis_a_y1, visible_x, visible_y);
    
    //Add to views
    game.views["game"] = sf::View(visibleArea);
    game.view_names.insert("game", game.view_names.size());

}

void setGameOverlayView(sf::RenderWindow &window, Game_Instance &game)
{
    //Overlay view
    sf::FloatRect overlayArea(0, 0, window.getSize().x, window.getSize().y);
    game.views["game_overlay"] = sf::View(overlayArea);
    game.view_names.insert("game_overlay", game.view_names.size());
}

void setMenuView(sf::RenderWindow &window, Game_Instance &game)
{
    //Overlay view
    sf::FloatRect overlayArea(0, 0, window.getSize().x, window.getSize().y);
    game.views["menu"] = sf::View(overlayArea);
    game.view_names.insert("menu", game.view_names.size());
}
