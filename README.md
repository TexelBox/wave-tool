# wave-tool
C++/OpenGL tool for realistic ocean wave rendering. Originally developed as a winter 2020 term project for CPSC591 @ University of Calgary.

---

Aaron Hornby
10176084

---

based on my other repo...
- <a href="https://github.com/TexelBox/cpp-xplatform" target="_blank">cpp-xplatform - (C++17 cross-platform template repository using CMake + Git Submodules.) - CC0-1.0 licensed.</a>
and building off some parts of the code from my CPSC589 project...
- <a href="https://github.com/TexelBox/cage-tool" target="_blank">cage-tool</a>

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

### Building (currently for developers only)
- note: the enforced generators by the scripts are "Visual Studio 2017"/"Visual Studio 2017 Win64" and "Unix Makefiles" for their respective platforms. I won't be testing this repository with any other generator (use `cmake --help` for a list), but anybody can try using cmake manually to see if another generator works.
- note: `cd scripts` before running any scripts (i.e. must run scripts from scripts/ directory).
#### Windows:
- double-click or use your favourite terminal to run
```
dev-build-all.bat
```
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

### Dependencies
Many thanks to the contributors of the following projects! Please check them out!
TODO - add links here!!!

---

enjoy!<br />
:wink:

---
