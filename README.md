This project uses CMake and vcpkg for managing C++ dependencies. It serves as a simple example to test your build setup before we get into more complicted code.

## Building Using CMake Presets

We have several CMake Build Presets that are outlined in the CMakePresets.json. Some are for building for Release or Debug mode. Running the default setup is just fine too.

```
cd <path/to/this source>
cmake --preset=default
```

Each preset defines its own build directory and various build variables that are important on that system.

Then, to build this source, you would

```
cd buildVCPkg
cmake --build .
```

Your executables will then be in the build folder. They may be in sub-folders depending on the environment.



## Development Environment Setup

Before you get too deep into this, you will need some tools, depending on your operating system and hardware. To build this, you do need some development tools for C++. The following sections will help with each of your specific environments.  Minimally, you need a terminal, a good C++ editor, a git command line client, and of course a C++ compiler toolchain.

### Linux

The following command in Ubuntu (or related) Linux will get you most of what you might

```
sudo apt update
sudo apt install build-essential cmake git g++ pkg-config libgl-dev libglu-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev 
```

### macOS

On macOS, you'll will need to get Apple's Xcode development environment and IDE installed. It's on the AppStore.

After this, you will want to install Brew:

```
https://brew.sh/
```

Brew is a package installer for mac os that works well.  You can install pretty much any package with Brew that are available on Linux machines.

```
brew install cmake
```

will get you initially started.


### Windows

Building on Windows requires installing a few packages:

* Visual Studio Community Edition 2022 (or newer - such as 2026) - this includes Microsoft's C++ compiler
* CMake https://cmake.org/download/
* Git Bash https://git-scm.com/downloads - You can use the powershell, but this is a nice Unix terminal for Windows and it comes with git
* VS Code https://code.visualstudio.com - VSCode is a nice editor for C++

When you install Microsoft Visual Studio Community Edition, feel free
to add whatever development languages and frameworks you want, but
certainly add C++ and the graphics libraries (DirectX, etc...). Once
the development environments are installed, make sure to setup your
git SSH keys if you want.


# vcpkg - Generalized Build Instructions for Windows, macos and Linux

Once you have some of the items above, it's time to make sure you've got vcpkg ready on your development machine.

We support a more generalized build system using vcpkg [https://learn.microsoft.com/en-us/vcpkg/get_started/overview](https://learn.microsoft.com/en-us/vcpkg/get_started/overview) and CMake build presets. Vcpkg is a C++ package manager used to pull the dependencies needed to build this code. When used in this way, the cmake build system will pull the needed requirements and not rely on installed system dependencies (meaning you shouldn't have to install all sorts of things, ideally). This can result in the initial build being a little slower as the required dependencies are pulled and compiled, but it does mean that you do not have to manually install our dependencies.

## Setting up vcpkg

To setup vcpkg, you will need to clone the vcpkg repository and setup environment variables that CMake can use to locate your vcpkg install.  More information on vcpkg and specific details for setting it up on different systems (Windows vs. Linux-based systems) can be found here: [https://learn.microsoft.com/en-us/vcpkg/get_started/overview](https://learn.microsoft.com/en-us/vcpkg/get_started/overview). 

Determine a location where you want vcpkg installed. It can be in system location for all users or cloned into your own user account. After cloning, be sure to run the bootstrap batch file in the vcpkg folder.

### TLDR; On Windows

Using git-bash, change directories to where you store your development files. Then, clone vcpkg, as shown below:

```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

Next, you will need to create the VCPKG_ROOT environment variable to point to the location of the vcpkg local repository on your system. You should also add the vcpkg root to your PATH variable. The following focuses on Windows, but the same ideas are needed on Unix systems and your shell's environment variables. On Windows, the ideal way to do this so that it is permanent is to set the variables using the Windows System Environment Variables panel from Settings. You can get to this by searching for Environment in the Windows menu. You will need something like the following:

```
VCPKG_ROOT = "C:\path\to\vcpkg"
```
Then, make sure the VCPKG_ROOT is also in your user Path variable.  You should be able to just add another folder path to the VCPKG_ROOT location.

You will need to quit your Powershell (or git-bash) after this and bring up a new window so the PATH variable information gets propoated.


### TLDR; On Linux and macOS

Using a terminal, change directories to where you store your development files. Then, clone vcpkg, as shown below:

```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
```

Next, you will need to create the VCPKG_ROOT environment variable to point to the location of the vcpkg local repository on your system. You should also add the vcpkg root to your PATH variable. On Linux, you will need to determine which shell environment you use.  For bash, you would edit the ~/.bashrc file, and modify the PATH variable a bit, as shown below:

```
export VCPKG_ROOT=/home/willemsn/dev/vcpkg
export PATH="$PATH":"$VCPKG_ROOT"
```

The instructions are similar for macOS. Determine which shell you use (typically zsh), and then add similar lines to the ~/.zprofile file.

Then, when you restart your terminals, you should be able to run the vcpkg program:
```
vcpkg
```

### To run fbMain.cpp
build the project(should build automatically when opening) or cd into starterCode/buildVCPkg and run "cmake --build ."

cd into starterCode\buildVCPkg\src\Debug

run ./fbMain.exe with specified height, width, and output file name
ex: ./fbMain.exe -h 200 -w 200 -o test.png

this will output an image with a normal, Blinn Phong, Lambertian, and mirror sphere with shadows and a light source

other examples are commented out in fbMain.cpp but the pngs they generate are all in buildVCPkg/src/Debug

### To run glfwExample.cpp
build the project(should build automatically when opening) or cd into starterCode/buildVCPkg and run "cmake --build ."

cd into starterCode\buildVCPkg\OpenGL

run ./glfwExample.exe

to move the camera use:
W: move forward
A: move left
S: move backwards
D: move right

move mouse to rotate camera

B: switch to Blinn Phong shaders
L: switch to Lambertian shaders

### Visual Differences
There are various the ray tracer and OpenGL.
one of the main ones being that OpenGL currently does not create any shadows.Depending on the sphere you create in OpenGL you can see the triangle faces close up which is not present in the ray tracer. OpenGL is also real time meaning you can move around and see real time transformations where as the ray tracer is static. The colors are relatively the same but may be a little different depending on how the values are being computed.

GLFW Example:
<img width="1006" height="1002" alt="GLFWComparison" src="https://github.com/user-attachments/assets/d429e7e8-7852-4c22-89a4-2ba02df41bb1" />

Ray Tracer Example:
<img width="1000" height="1000" alt="fbMainComparison" src="https://github.com/user-attachments/assets/47ea05b3-2a36-4264-b348-2ac178757508" />
