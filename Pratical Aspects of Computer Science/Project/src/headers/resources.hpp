struct Draw_Resources
{
    //Fonts
    std::map<std::string, sf::Font> fonts;

    //Textures
    std::map<std::string, sf::Texture> textures;

    //Sprites
    std::map<std::string, sf::Sprite> sprites;

};

void loadDrawResources(Draw_Resources &res);