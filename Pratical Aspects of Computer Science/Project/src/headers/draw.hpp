void drawHandler(sf::RenderWindow &window, Game_Instance &game);
void drawSprites(sf::RenderWindow &window, Game_Instance &game);
void drawMap(sf::RenderWindow &window, Game_Instance &game);
void drawMapDecoration(sf::RenderWindow &window, Game_Instance &game);
void drawTile(Map_Tile tile, sf::RenderWindow &window, Game_Instance &game);
void drawTileDecoration(Map_Tile tile, sf::RenderWindow &window, Game_Instance &game);
void drawPlayer(sf::RenderWindow &window, Game_Instance &game);
void drawMobs(sf::RenderWindow &window, Game_Instance &game);
void drawProjectiles(sf::RenderWindow &window, Game_Instance &game);
void drawStats(sf::RenderWindow &window, Game_Instance &game);
void drawStartCount(sf::RenderWindow &window, Game_Instance &game);
void drawMinimap(sf::RenderWindow &window, Game_Instance &game);
void drawMinimapTile(Map_Tile tile, Map_Tile center, sf::RenderWindow &window, Game_Instance &game);
void drawMenu(sf::RenderWindow &window, Game_Instance &game);
void drawMenuMain(sf::RenderWindow &window, Game_Instance &game);
void drawMenuConfig(sf::RenderWindow &window, Game_Instance &game);
void drawMenuTutorial(sf::RenderWindow &window, Game_Instance &game);
void drawMenuPause(sf::RenderWindow &window, Game_Instance &game);
void drawMenuGameOver(sf::RenderWindow &window, Game_Instance &game);

Blend_Info getBiomeBlend(Map_Tile tile, Game_Instance &game);

string tickToClockString(long int tick, Game_Instance &game);