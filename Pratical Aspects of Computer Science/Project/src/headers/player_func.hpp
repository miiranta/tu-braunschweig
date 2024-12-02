Player createPlayer();
void evaluatePlayerMovement(Game_Instance &game);
void evaluateSpeedModifier(Game_Instance &game);
void evaluateZoom(Game_Instance &game);
void findPlayerSpawn(Player &player, Game_Instance &game);

void evaluatePlayerColision(Game_Instance &game, Player &player);

void evaluatePlayerDamage(Game_Instance &game, Player &player);

void evaluateKnockback(float knockback, float x, float y, Player &player, Game_Instance &game);