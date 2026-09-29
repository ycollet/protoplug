protoplug
=========
Create audio plugins on-the-fly with LuaJIT.

- **Official website**: http://www.osar.fr/protoplug
- **Downloads**: https://github.com/pac-dev/protoplug/releases
- **Forums**: http://forums.osar.fr

Protoplug is a VST/AU plugin that lets you load and edit Lua scripts as audio effects and instruments. The scripts can process audio and MIDI, display their own interface, and use external libraries. Transform any music software into a live coding environment! 

**Cross-platform :** builds for Windows, Linux, and macOS. This means that all protoplug scripts are compatible with these platforms and can be loaded into a huge amount of audio software (glory to [JUCE](http://juce.com/)) 

**Fast :** Use the speed of [LuaJIT](http://luajit.org/) to perform complex DSP tasks in realtime.

**Free and open source :** The source is MIT-licensed. Hack away.


Compiling from Source
---------------------
There are [prebuilt binaries](https://github.com/pac-dev/protoplug/releases), but building it from source is also simple. protoplug is built with **CMake** (3.22+); JUCE is fetched automatically at configure time via `FetchContent`, so you don't need to download it yourself.

**Prerequisites (all platforms) :**

- CMake 3.22 or newer
- A C++17 compiler (Visual Studio 2019+ on Windows, a recent Xcode on macOS, GCC/Clang on Linux)
- System **Lua 5.4** development files (not LuaJIT):
  - Fedora: `sudo dnf install lua5.4-devel`
  - Debian/Ubuntu: `sudo apt install liblua5.4-dev`
  - macOS (Homebrew): `brew install lua@5.4` (keg-only — see note below)

**Linux :** also install the JUCE GUI/audio dependencies, for example on Ubuntu:

	sudo apt-get install build-essential pkg-config libfftw3-dev libgtk-3-dev libfreetype6-dev \
		libx11-dev libasound2-dev libxinerama-dev libxcursor-dev libxi-dev libcurl4-openssl-dev

**Building (all platforms) :**

	cmake -B build
	cmake --build build --config Release

On macOS, since Homebrew's `lua@5.4` is keg-only, pass explicit paths at configure time:

	cmake -B build \
		-DLUA_INCLUDE_DIR=$(brew --prefix lua@5.4)/include/lua5.4 \
		-DLUA_LIBRARY=$(brew --prefix lua@5.4)/lib/liblua.a \
		-DLUA_LIBRARIES=$(brew --prefix lua@5.4)/lib/liblua.a

By default, protoplug builds VST3 and AU (macOS only). Additional formats can be enabled at configure time:

	cmake -B build -DPLUGIN_USE_LV2=ON    # build the LV2 format
	cmake -B build -DPLUGIN_USE_CLAP=ON   # build the CLAP format

Built plugins are placed under `build/protoplug_fx_artefacts` and `build/protoplug_gen_artefacts` (one subfolder per format). Copy the binaries to your system's plugin folder, or run the platform's standard CMake install step if you prefer.
