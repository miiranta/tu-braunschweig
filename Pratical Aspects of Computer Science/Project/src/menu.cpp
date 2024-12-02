//Menu
Menu_Button createButton(Menu_Button bnt, Game_Instance &game)
{
    //Rect
    sf::RectangleShape rect;

    rect.setSize(sf::Vector2f(bnt.width, bnt.height));
    rect.setPosition(bnt.x, bnt.y);
    
    rect.setFillColor(sf::Color(bnt.r, bnt.g, bnt.b, bnt.a));

    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2);

    bnt.rect = rect;

    //Text
    sf::Text txt;

    txt.setFont(game.draw_resources.fonts["pixel"]);
    txt.setCharacterSize(bnt.font_size);
    txt.setStyle(sf::Text::Bold);

    txt.setFillColor(sf::Color::White);

    txt.setString(bnt.text);

    txt.setPosition(bnt.x + bnt.width / 2 - txt.getLocalBounds().width / 2, bnt.y + bnt.height / 2 - txt.getLocalBounds().height + 5);

    txt.setOutlineColor(sf::Color::Black);
    txt.setOutlineThickness(1);

    bnt.txt = txt;

    return bnt;
}

void drawButton(Menu_Button bnt, sf::RenderWindow &window, Game_Instance &game)
{
    window.draw(bnt.rect);
    window.draw(bnt.txt);
}

//Etc
bool isMouseInsideButton(sf::RectangleShape rect, Game_Instance &game)
{
    float x = game.mouse_x;
    float y = game.mouse_y;

    float x1 = rect.getPosition().x;
    float y1 = rect.getPosition().y;
    float x2 = x1 + rect.getSize().x;
    float y2 = y1 + rect.getSize().y;

    if(x > x1 && x < x2 && y > y1 && y < y2) return true;
    return false;
}


void menuPostRedirect(Menu menu, int ms, Game_Instance &game)
{
    //Spawn thread
    std::thread t(menuPostRedirectThread, menu, ms, std::ref(game));
    t.detach();
}

void menuPostRedirectThread(Menu menu, int ms, Game_Instance &game)
{
    //Sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));

    if(menu == MAIN)
    {
        game.remake_instance = true;
    }

    //Redirect
    game.current_menu = (int)menu;
}

