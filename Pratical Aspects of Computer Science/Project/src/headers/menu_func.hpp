Menu_Button createButton(Menu_Button bnt, Game_Instance &game);
void drawButton(Menu_Button bnt, sf::RenderWindow &window, Game_Instance &game);

bool isMouseInsideButton(sf::RectangleShape rect, Game_Instance &game);

void menuPostRedirect(Menu menu, int ms, Game_Instance &game);
void menuPostRedirectThread(Menu menu, int ms, Game_Instance &game);