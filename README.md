![image](https://user-images.githubusercontent.com/38915815/134362903-acc868d5-98be-452b-a483-94594024f606.png)

# Pokemon-Demo <img src="https://emojis.slackmojis.com/emojis/images/1479080836/1363/eevee.gif?1479080836" />

Small pokemon game made completely from scratch and in C, just like @ <a href="https://handmadehero.org/">Handmade Hero</a>. Disclaimer: I do not own any of the assets used in this project (images and sound used), these are the intellectual property of <a href="https://www.nintendo.com/">Nintendo</a>.

To play the game, you can head over to https://pokemondemo.surge.sh/


# Steps for Building

## Build to Web

The platform layer of the project has been rewritten via the popular https://www.raylib.com/ library to support building for the web. Simply run 

```bash
build.sh
```

This will generate all needed files in build/raylib/surge. To run the web app, use

```bash
run.sh
```

This will launch a server on your local machine via emrun to serve the static site. emrun should launch your browser as well.

## Native Windows Build
This project uses the visual studio compiler, so you will need to install <a href="https://visualstudio.microsoft.com/vs/">Visual Studio</a> if you haven't already. At the time of writing, the latest version is 2019, so I cannot guarentee proper building for any subsequent versions. Also note that if your visual studio version is different than 2019, you will need to change *shell.bat* accordingly.   

After installing visual studio, clone this project and run the following commands. This will set up the visual studio compiler for use on the command line via the "cl" command. The *build.bat* script takes care of compiling the platform layer and the game.dll
```
$ shell.bat
$ build.bat
```
To run the project
```
$ cd build
$ win32_PokemonDemo.exe
```

## Features Specific to Native Windows Build 
**Hot-reloading:** The game compiles as a dll, allowing the platform layer to dynamically load the game.

**Live Loop Recording:** The player takes a snaphot of the game and records subsequent inputs. Then, without them ever pressing another button, the game continues to play back the same sequence. 

# Controls
- W, A, S, D to control the player. 
- Z to interact with things (this button can be thought of as equivalent to the A button in most video games). 
- Press F to toggle fullscreen mode.
- **Only on Windows:** Use L to start a "recording". L again to stop and enter playback of the recording. L again to exit playback and return normal control to the player.