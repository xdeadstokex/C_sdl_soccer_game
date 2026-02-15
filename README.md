# C_sdl_soccer_game

HOW TO COMPILE IN WINDOW
```txt
WINDOWS (MinGW/GCC)
-------------------
1. Download SDL2, SDL2_image, SDL2_mixer development libraries (MinGW version)
2. Extract to a folder (e.g., C:\SDL2)
3. Set paths and compile:

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
