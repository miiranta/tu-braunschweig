Projectile createProjectile(float x, float y, float x_speed, float y_speed, int TTL);
void spawnProjectile(float x_target, float y_target, float x_origin, float y_origin, Game_Instance &game);
void despawnProjectiles(Game_Instance &game);
void evaluateProjectilesMovement(Game_Instance &game);