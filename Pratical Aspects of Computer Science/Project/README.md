
**Title**

STAR(t)

---

**Compile and Run (Linux)**

`sudo apt-get install libsfml-dev`

`cd /..path to../src/`
`g++ -o 'app' 'app.cpp' -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system`
`./app`

---

**Description**

- 2D map made out of squares or 'blocks'.
- Procedurally generated biomes with different themes (~~4 or 5~~ **8** different ones that repeat 'infinitely').
- Each biome repetition has one or more stars somewhere. One type of star for each biome.
- Each biome type has challenges (mobs, ~~traps~~, etc). ~~If possible, unique challange for each biome.~~
- When a star of the biome is collected, all the start of same type disappear. The biome becomes a safe zone (challenges are disabled).

---

**Goal**

- Find all types of stars in the least amount of time.

---

**Mechanics**

*Player:*

- Life bar
- Time elapsed

*Biome Challenges:*

- ~~Holes in the ground~~
- Quicksand
- ~~"Slowsand"~~
- Ice
- Archer Mobs
- Melee Mobs
- **Ghost Mobs**
- ...

*Mobs:*

- ~~Mobs can only move in their own biome.~~

*Biomes:*

- Changes color when one of its stars is collected.
- **Each biome has a different color blend**

---

**~~Objects~~**
**Decorations**

- Tree
- Normal Bush
- **Bad bush**
- **Tall grass**
- ~~Logs~~

---

**Tiles**

- ~~Water~~
- Rock
- **Grass**
- **Sand**
- **Ice**

---

**Components**

- Single Player
- Menu (The main one, **pause, game over, settings and tutorial**)
- Sound **and music :)**
- Difficulty Levels
- Simple Animations
- Level Generation
- Physics Simulations
- Procedurally Generated World

---

**Other components (MIGHT BE IMPLEMENTED)**

- ~~Competitive multiplayer (who collects all the stars first)~~
- Infinite map
- ~~Shaders~~ **(or almost a shader)**

---

**CREDITS**

- Game made by Lucas Miranda

---

**LICENSE**

- Open Simplex Noise -
"deerel" - https://github.com/deerel/OpenSimplexNoise - Free software

- Pixellari Font -
Zacchary Dempsey-Plante - https://www.dafont.com/pixellari.font - Free for commercial use

- NotoEmoji-Regular -
Google - SIL Open Font License (OFL) - https://www.1001fonts.com/noto-emoji-font.html

- "Minstrel Guild", "Valse Gymnopedie" -
Kevin MacLeod (incompetech.com) -
Licensed under Creative Commons: By Attribution 3.0
http://creativecommons.org/licenses/by/3.0/

- "Kalimba Relaxation Music" -
Kevin MacLeod (incompetech.com) -
Licensed under Creative Commons: By Attribution 4.0 License
http://creativecommons.org/licenses/by/4.0/

- Pop 1 -
https://pixabay.com/sound-effects/pop-1-35897/ -
https://pixabay.com/service/license-summary/

- sfx_snowball_hit-01 -
https://pixabay.com/sound-effects/sfx-snowball-hit-01-102168/ -
https://pixabay.com/service/license-summary/

- Rustling Grass 1 -
https://pixabay.com/sound-effects/rustling-grass-1-101282/ -
https://pixabay.com/service/license-summary/

- Pipe -
https://pixabay.com/sound-effects/pipe-117724/ -
https://pixabay.com/service/license-summary/

- Badge Coin Win -
https://pixabay.com/sound-effects/badge-coin-win-14675/ -
https://pixabay.com/service/license-summary/

- Success Fanfare Trumpets -
https://pixabay.com/sound-effects/success-fanfare-trumpets-6185/ -
https://pixabay.com/service/license-summary/

- PuffOfSmoke -
https://pixabay.com/sound-effects/puffofsmoke-47176/ -
https://pixabay.com/service/license-summary/

- Metal Hit 93 -
https://pixabay.com/sound-effects/metal-hit-93-200423/ -
https://pixabay.com/service/license-summary/