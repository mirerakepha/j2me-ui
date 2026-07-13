#### Use this program to run the old nokia or blackberry java games on your computer

- First install SDL via your package manager e.g **Fedora**
```bash
sudo dnf install SDL2-devel
```

- Make the build folder on the project root and run the Makefile
```bash
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make
```

- Download the .jar game of your choice and add it under the system/ folder

- Run the game **exp**
```bash
./build/j2me_ui ./cores/freej2me_libretro.so ./system ./system/assassins_creed_rev_253839.jar
```

**OR**
in the project root
```bash
cmake --build build-debug
```

then
```bash
./build-debug/j2me_ui ./cores/freej2me_libretro.so ./system ./system/GP_Bikes_3D_176x220_SE_K700-542097-mobiles24.jar
```

- Then enjoy that crap

--- 
<img width="1920" height="1080" alt="Screenshot From 2026-07-13 09-28-58" src="https://github.com/user-attachments/assets/41f4abf4-05e7-47e5-93e6-dd2778af1646" />

