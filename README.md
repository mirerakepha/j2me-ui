#### Use this program to run the old nokia or blackberry java games on your computer

- First install SDL via your package manager e.g **Fedora**
```sudo dnf install SDL2-devel```

- Make the build folder on the project root and run the Makefile
```mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make```

- Download the .jar game of your choice and add it under the system/ folder

- Run the game **exp**
```./build/j2me_ui ./cores/freej2me_libretro.so ./system ./system/assassins_creed_rev_253839.jar```

