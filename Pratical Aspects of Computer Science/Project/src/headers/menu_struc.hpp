enum Menu
{
    MAIN,
    CONFIG,
    TUTORIAL,
    PAUSE,
    GAME_OVER,
    GAME,

    MENU_SIZE
};

struct Menu_Button
{
    string text;
    
    int x=0;
    int y=0;

    int width=500;
    int height=50;

    int font_size = 60;

    int r=0;
    int g=0;
    int b=0;
    int a=255;

    bool focused = false;

    sf::RectangleShape rect;
    sf::Text txt;
};