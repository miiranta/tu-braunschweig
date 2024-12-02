Map createMap(string seed);
void destroyMap(Map &map);
long int hashMapSeed(string seed);

Map_Tile getMapTile(int x, int y, Game_Instance &game);
void updateMapTile(int x, int y, Map_Tile tile, Game_Instance &game);

Map_Tile generateMapTile(int x, int y, Game_Instance &game);
Biome setBiome(int x, int y, Game_Instance &game);
Tile_Type setTileType(int x, int y, Game_Instance &game);
Tile_Decoration setTileDecoration(Map_Tile tile, Game_Instance &game);
Map_Tile setTileProperties(Map_Tile tile, Game_Instance &game);
Map_Tile setTileColor(Map_Tile tile, Game_Instance &game);
Map_Tile updateTileColor(Map_Tile tile, Game_Instance &game);

Map_Tile updateOutdatedTile(Map_Tile tile, Game_Instance &game);

Map_Tile AVL_search(int x, int y, Game_Instance &game);
void AVL_insert(Map_Tile tile, Game_Instance &game);
