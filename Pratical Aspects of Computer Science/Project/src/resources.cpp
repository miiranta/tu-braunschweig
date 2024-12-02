//Resources Handling
void loadDrawResources(Draw_Resources &res)
{
    //Fonts
    res.fonts = std::map<std::string, sf::Font>();
    res.fonts["pixel"] = sf::Font();
    res.fonts["pixel"].loadFromFile("../assets/fonts/Pixellari.ttf");

    res.fonts["emoji"] = sf::Font();
    res.fonts["emoji"].loadFromFile("../assets/fonts/Emoji.ttf");

    //Textures
    res.textures = std::map<std::string, sf::Texture>();
    res.textures["grass_tile"] = sf::Texture();
    res.textures["rock_tile"] = sf::Texture();
    res.textures["sand_tile"] = sf::Texture();
    res.textures["ice_tile"] = sf::Texture();
    res.textures["grass_tile"].loadFromFile("../assets/textures/biomes/tiles/GRASS.png");
    res.textures["rock_tile"].loadFromFile("../assets/textures/biomes/tiles/ROCK.png");
    res.textures["sand_tile"].loadFromFile("../assets/textures/biomes/tiles/SAND.png");
    res.textures["ice_tile"].loadFromFile("../assets/textures/biomes/tiles/ICE.png");

    res.textures["tall_grass_dec"] = sf::Texture();
    res.textures["bush_dec"] = sf::Texture();
    res.textures["bad_bush_dec"] = sf::Texture();
    res.textures["tree_dec"] = sf::Texture();
    res.textures["tall_grass_dec"].loadFromFile("../assets/textures/biomes/decorations/TALL_GRASS.png");
    res.textures["bush_dec"].loadFromFile("../assets/textures/biomes/decorations/BUSH.png");
    res.textures["bad_bush_dec"].loadFromFile("../assets/textures/biomes/decorations/BAD_BUSH.png");
    res.textures["tree_dec"].loadFromFile("../assets/textures/biomes/decorations/TREE.png");

    res.textures["projectile"] = sf::Texture();
    res.textures["projectile"].loadFromFile("../assets/textures/projectiles/PROJECTILE.png");

    //Sprites
    res.sprites = std::map<std::string, sf::Sprite>();

    res.textures["star_gone"] = sf::Texture();
    res.textures["star_gone"].loadFromFile("../assets/sprites/stars/STAR_GONE.png");
    res.sprites["star_gone"] = sf::Sprite(res.textures["star_gone"]);

    res.textures["star"] = sf::Texture();
    res.textures["star"].loadFromFile("../assets/sprites/stars/STAR.png");
    res.sprites["star"] = sf::Sprite(res.textures["star"]);

    res.textures["death"] = sf::Texture();
    res.textures["death"].loadFromFile("../assets/sprites/etc/DEATH.png");
    res.sprites["death"] = sf::Sprite(res.textures["death"]);

    res.textures["player_front"] = sf::Texture();
    res.textures["player_front"].loadFromFile("../assets/sprites/player/PLAYER_FRONT.png");
    res.sprites["player_front"] = sf::Sprite(res.textures["player_front"]);

    res.textures["player_back"] = sf::Texture();
    res.textures["player_back"].loadFromFile("../assets/sprites/player/PLAYER_BACK.png");
    res.sprites["player_back"] = sf::Sprite(res.textures["player_back"]);

    res.textures["player_left"] = sf::Texture();
    res.textures["player_left"].loadFromFile("../assets/sprites/player/PLAYER_LEFT.png");
    res.sprites["player_left"] = sf::Sprite(res.textures["player_left"]);

    res.textures["player_right"] = sf::Texture();
    res.textures["player_right"].loadFromFile("../assets/sprites/player/PLAYER_RIGHT.png");
    res.sprites["player_right"] = sf::Sprite(res.textures["player_right"]);

    res.textures["mob_flyer_front"] = sf::Texture();
    res.textures["mob_flyer_front"].loadFromFile("../assets/sprites/mobs/FLYER_FRONT.png");
    res.sprites["mob_flyer_front"] = sf::Sprite(res.textures["mob_flyer_front"]);

    res.textures["mob_flyer_back"] = sf::Texture();
    res.textures["mob_flyer_back"].loadFromFile("../assets/sprites/mobs/FLYER_BACK.png");
    res.sprites["mob_flyer_back"] = sf::Sprite(res.textures["mob_flyer_back"]);

    res.textures["mob_flyer_left"] = sf::Texture();
    res.textures["mob_flyer_left"].loadFromFile("../assets/sprites/mobs/FLYER_LEFT.png");
    res.sprites["mob_flyer_left"] = sf::Sprite(res.textures["mob_flyer_left"]);

    res.textures["mob_flyer_right"] = sf::Texture();
    res.textures["mob_flyer_right"].loadFromFile("../assets/sprites/mobs/FLYER_RIGHT.png");
    res.sprites["mob_flyer_right"] = sf::Sprite(res.textures["mob_flyer_right"]);

    res.textures["mob_shooter_front"] = sf::Texture();
    res.textures["mob_shooter_front"].loadFromFile("../assets/sprites/mobs/SHOOTER_FRONT.png");
    res.sprites["mob_shooter_front"] = sf::Sprite(res.textures["mob_shooter_front"]);

    res.textures["mob_shooter_back"] = sf::Texture();
    res.textures["mob_shooter_back"].loadFromFile("../assets/sprites/mobs/SHOOTER_BACK.png");
    res.sprites["mob_shooter_back"] = sf::Sprite(res.textures["mob_shooter_back"]);

    res.textures["mob_shooter_left"] = sf::Texture();
    res.textures["mob_shooter_left"].loadFromFile("../assets/sprites/mobs/SHOOTER_LEFT.png");
    res.sprites["mob_shooter_left"] = sf::Sprite(res.textures["mob_shooter_left"]);

    res.textures["mob_shooter_right"] = sf::Texture();
    res.textures["mob_shooter_right"].loadFromFile("../assets/sprites/mobs/SHOOTER_RIGHT.png");
    res.sprites["mob_shooter_right"] = sf::Sprite(res.textures["mob_shooter_right"]);

    res.textures["mob_seeker_front"] = sf::Texture();
    res.textures["mob_seeker_front"].loadFromFile("../assets/sprites/mobs/SEEKER_FRONT.png");
    res.sprites["mob_seeker_front"] = sf::Sprite(res.textures["mob_seeker_front"]);

    res.textures["mob_seeker_back"] = sf::Texture();
    res.textures["mob_seeker_back"].loadFromFile("../assets/sprites/mobs/SEEKER_BACK.png");
    res.sprites["mob_seeker_back"] = sf::Sprite(res.textures["mob_seeker_back"]);

    res.textures["mob_seeker_left"] = sf::Texture();
    res.textures["mob_seeker_left"].loadFromFile("../assets/sprites/mobs/SEEKER_LEFT.png");
    res.sprites["mob_seeker_left"] = sf::Sprite(res.textures["mob_seeker_left"]);

    res.textures["mob_seeker_right"] = sf::Texture();
    res.textures["mob_seeker_right"].loadFromFile("../assets/sprites/mobs/SEEKER_RIGHT.png");
    res.sprites["mob_seeker_right"] = sf::Sprite(res.textures["mob_seeker_right"]);

    //Sounds and Music
    //In sounds.cpp

}
