# PokemonDemo <img src="https://emojis.slackmojis.com/emojis/images/1479080836/1363/eevee.gif?1479080836" />
Video game demo in the style of Pokemon. Made completely from scratch and in C, just like @ <a href="https://handmadehero.org/">Handmade Hero</a>. Disclaimer: I do not own any of the assets used in this project (images and sound used), these are the intellectual property of <a href="https://www.nintendo.com/">Nintendo</a>.

## Build Steps
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
Note that for reliable operation the data files for the game must be located at "../Data/Data". This location is relative to the game executable. 
