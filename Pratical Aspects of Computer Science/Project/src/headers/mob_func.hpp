Mob createMob(int x, int y);
void spawnMobs(Game_Instance &game, Player &player);
void despawnMobs(Game_Instance &game, Player &player);
void evaluateMobsMovement(Game_Instance &game);
Next_Tile Pathfind(Mob &mob, Player &player, Game_Instance &game);
Next_Tile StraightLine(float cx, float cy, float dx, float dy, int max_path_len, Game_Instance &game);
Next_Tile AStar(float cx, float cy, float dx, float dy, int max_path_len, Game_Instance &game);
List<Next_Tile> AStar_path(float cx, float cy, float dx, float dy, int max_path_len, Game_Instance &game);

void evaluateMobColision(Game_Instance &game, Mob &mob);