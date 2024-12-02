void playMusic(string name, Game_Instance &game)
{
    //Already playing
    if(game.music_playing[name]) return;
    game.music_playing[name] = true;

    //Stop other music
    for(auto &music : game.music_playing)
    {
        if(music.first != name)
        {
            music.second = false;
        }
    }

    //Menu theme
    if (name == "menu")
    {
        std::thread music_thread(musicThread, name, "../assets/music/Valse_Gymnopedie.ogg", std::ref(game));
        music_thread.detach();
    }
    
    //Game theme
    else if (name == "game")
    {
        std::thread music_thread(musicThread, name, "../assets/music/Kalimba.ogg", std::ref(game));
        music_thread.detach();
    }
}

void musicThread(string name, string file, Game_Instance &game)
{   
    sf::Music music;
    if (!music.openFromFile(file))
    {
        std::cout << "Error loading music" << std::endl;
    }

    music.setLoop(true);
    music.setVolume(100);
    music.play();

    while(game.music_playing[name])
    {
        music.setVolume(game.music_volume * game.music_volume_f);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return;
}


void playSound(string name, Game_Instance &game)
{
    if(name == "pop") 
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/pop.ogg", std::ref(game));
        sound_thread.detach();
    }

    else if(name == "lost")
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/lost.ogg", std::ref(game));
        sound_thread.detach();
    }

    else if(name == "win")
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/win.ogg", std::ref(game));
        sound_thread.detach();
    }

    else if(name == "grass")
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/grass.ogg", std::ref(game));
        sound_thread.detach();
    }

    else if(name == "star")
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/star.ogg", std::ref(game));
        sound_thread.detach();
    }

    else if(name == "snowball")
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/snowball.ogg", std::ref(game));
        sound_thread.detach();
    }

    else if(name == "dead")
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/dead.ogg", std::ref(game));
        sound_thread.detach();
    }

    else if(name == "damage")
    {
        std::thread sound_thread(soundThread, name, "../assets/sounds/damage.ogg", std::ref(game));
        sound_thread.detach();
    }

    else
    {
        std::cout << "Sound not found" << std::endl;
    }
   
}

void soundThread(string name, string file, Game_Instance &game)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(file))
    {
        std::cout << "Error loading sound" << std::endl;
    }

    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.setVolume(100);
    sound.play();

    while(sound.getStatus() == sf::Sound::Playing)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}