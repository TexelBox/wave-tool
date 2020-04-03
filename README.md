# wave-tool
C++/OpenGL tool for realistic ocean wave rendering. Originally developed as a winter 2020 term project for CPSC591 @ University of Calgary.

## [WIP]

---

Aaron Hornby<br />
10176084

---

based on my other repo...
- <a href="https://github.com/TexelBox/cpp-xplatform" target="_blank">cpp-xplatform - (C++17 cross-platform template repository using CMake + Git Submodules.) - CC0-1.0 licensed.</a>
- and building off some parts of the code from my CPSC589 project... <a href="https://github.com/TexelBox/cage-tool" target="_blank">cage-tool.</a>

---

### Supported compilers
#### Minimum versions:
- GCC (libstdc++) 7
- Clang (libc++) 6
- MSVC (MSVC Standard Library) 19.14 (VS 2017 15.7)
- Apple Clang 10.0.0
###### [Refer here for a comprehensive overview of which C++ standard features are supported by each compiler.](https://en.cppreference.com/w/cpp/compiler_support)
- note: all C++17 core language features are supported by these compilers, but some library features are unsupported.

---

### Cloning
easiest (preferred) method:
```
git clone --recursive https://github.com/TexelBox/wave-tool.git
```
or (if you did a normal clone already (first two lines below)...)
```
git clone https://github.com/TexelBox/wave-tool.git
cd wave-tool/
git submodule update --init --recursive
```

---

### Building (currently for developers only)
- note: the enforced generators by the scripts are "Visual Studio 2017"/"Visual Studio 2017 Win64" and "Unix Makefiles" for their respective platforms. I won't be testing this repository with any other generator (use `cmake --help` for a list), but anybody can try using cmake manually to see if another generator works.
- note: `cd scripts` before running any scripts (i.e. must run scripts from scripts/ directory).
#### Windows:
- double-click or use your favourite terminal to run
```
dev-build-all.bat
```
- note: you may have to run `./dev-build-all.bat` or `.\dev-build-all.bat` instead (depending on your shell).
#### Linux/Mac:
- use your favourite terminal to run
```
./dev-build-all.sh
```
- note: if for some reason the script is non-executable, then run the script through an interpreter (only `bash` is tested, but you can try another like `sh`).
```
bash dev-build-all.sh
```
or add executable permissions and then run
```
chmod +x dev-build-all.sh
./dev-build-all.sh
```

---

### Running
#### For users...
##### Windows:
- for best performance, double-click/run `wave-tool.exe` in either x86/Release (32-bit) or x64/Release (64-bit).
##### Linux/Mac:
- for best performance, double-click/run `wave-tool` in build/Release.

#### For developers...
##### Windows:
- if using `VS2017`, open either x86/wave-tool.sln or x64/wave-tool.sln and click the green arrow (you can also change the build config with the drop-down - use Debug when you need the debugger and Release when testing for performance).
##### Linux:
```
make
./wave-tool
```

---

### Dependencies
Many thanks to the contributors of the following projects! Please check them out!
- <a href="https://www.boost.org/" target="_blank">Boost</a>
- <a href="https://github.com/ocornut/imgui" target="_blank">Dear ImGui</a>
- <a href="https://github.com/Dav1dde/glad" target="_blank">glad</a>
- <a href="https://github.com/glfw/glfw" target="_blank">GLFW</a>
- <a href="https://github.com/g-truc/glm" target="_blank">GLM</a>
- <a href="https://www.opengl.org/" target="_blank">OpenGL</a>
- <a href="https://github.com/nothings/stb" target="_blank">stb</a>

---

### References
- mainly implementing features outlined by <a href="https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/projgrid-hq.pdf" target="_blank">Real-time water rendering / Introducing the projected grid concept - Claes Johanson [2004]</a>.
- with clarification using the demo code downloaded from here <a href="https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/" target="_blank">Tech Demo</a>.
- uses some skybox textures from <a href="https://github.com/BabylonJS/Babylon.js/tree/master/Playground/textures" target="_blank">Babylon.js</a>.
- wave noise heightmap textures sourced from <a href="https://opengameart.org/content/seamless-looping-waves-heightmaps" target="_blank">opengameart.org - Seamless Looping Waves Heightmaps</a>.
- Mount Everest model sourced from <a href="https://free3d.com/3d-model/everest-mountain-930871.html" target="_blank">free3d.com - Everest Mountain</a>.

---

enjoy!<br />
:wink:

---
