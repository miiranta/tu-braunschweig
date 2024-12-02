#include "game.cpp"

int main()
{
    Game_Params params = {};
    Game_Instance game = createGameInstance(params);
    newGame(game);

    return 0;
}
