

# ntru.js

NTRU.js aims to provide post-quantum security to NodeJS applications written in JavaScript.

## Installation
npm install ntrujs

## Building on Linux
Download the code, run the command: git submodule init
and then, run: git submodule update
These commands will download the NTRU sources from Security Innovation's GIT repository.
To compile these sources, simply run the ./build_ntru script from your terminal. This will compile NTRU, placing it in the correct folders automatically.
After NTRU is compiled, run: npm install
After running npm install, run: node main.js
If there were no errors, you should be good to go!

## Building on Windows

This is a bit more complicated. Follow the steps to download the NTRU submodule as shown in Linux, then try to compile for your platform in Visual Studio.
NTRU doesn't seem to compile easily in the latest version of Visual Studio, so this may require some setup-specific tweaking.
Once you have gotten NTRU to compile, place the NTRU DLL and LIB files in the lib/lib folder of this project.
Run npm install. If everything goes well, this should create a build folder containing the node addon linked with the binary LIB file.
Copy the NTRU DLL file to the build/Release folder, and then try running main.js.

If main.js runs, you're good to go! You've successfully compiled this module!

If; after much fiddling around with Visual Studio; you still can't get it to compile, please file a bug report on GitHub at https://github.com/IDWMaster/ntrujs/issues.

## Building on MAC

MACs..... Because SHINY! Sorry. There's currently no build available for MAC. We are looking for someone with MAC experience to contribute.


## Running

TODO: Add example code


## Developing
See https://github.com/IDWMaster/ntrujs



