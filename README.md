# C_sdl_soccer_game
DES
```txt
-------- DES --------
a simple soccer game forced to make by uni (damn uni)
2 mode:
pvp (top selection)
pve (bottom selection)

each team consist of 4 players: 3 attacker and 1 goalie
currently only have wind as external impact

-------- CONTROL --------
p1:
move: wasd
kick (hold): space
change teammate: tab

p2:
move: arrow key
kick (hold): n
change teammate: m



```


DEPENDENCY
```txt
gcc:        (find it yourself in window, mingw32 is normally used)
SDL2:        https://github.com/libsdl-org/SDL/releases/tag/release-2.32.10
SDL2_MIXER:  https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.1
SDL2_IMAGE:  https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.8
```

HOW TO COMPILE IN WINDOW
```txt
WINDOWS (MinGW/GCC)
-------------------
1. Download SDL2, SDL2_image, SDL2_mixer development libraries (MinGW version)
2. Extract to a folder (e.g., C:\SDL2)
3. Set paths and compile: (check the correct path of your own machine)

SET SDL2_PATH=C:\SDL2\SDL2-2.x.x\x86_64-w64-mingw32
SET SDL2_IMAGE_PATH=C:\SDL2\SDL2_image-2.x.x\x86_64-w64-mingw32
SET SDL2_MIXER_PATH=C:\SDL2\SDL2_mixer-2.x.x\x86_64-w64-mingw32

gcc main.c -o main.exe ^
  -I.\ ^
  -I%SDL2_PATH%\include\SDL2 ^
  -I%SDL2_IMAGE_PATH%\include\SDL2 ^
  -I%SDL2_MIXER_PATH%\include\SDL2 ^
  -L%SDL2_PATH%\lib ^
  -L%SDL2_IMAGE_PATH%\lib ^
  -L%SDL2_MIXER_PATH%\lib ^
  -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lm

4. Copy DLLs to executable folder:
   - SDL2.dll
   - SDL2_image.dll
   - SDL2_mixer.dll
   - libpng16-16.dll (from SDL2_image)
   - zlib1.dll (from SDL2_image)

```
