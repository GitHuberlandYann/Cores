# Cores
Playing around with particules in openGL.

## About
I needed a project to get my hands on transform feedbacks in openGL, and here it is. I am hoping ot create a nice background screen shared by all computers on the same local networks, each station being able to generate particule-emitting cores, resulting in cooperative creations.

## Building
```
$ git clone --recurse-submodules git@github.com:GitHuberlandYann/Cores.git
$ cd Cores
$ make setup
$ make
$ ./cores
```
make setup will install the needed [static libraries.](#libraries)

make will create the needed executables - server and client.

## Libraries
* [GLFW](https://github.com/glfw/glfw.git) is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development. It provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.
* [GLEW](https://github.com/nigels-com/glew.git) provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target platform. I am using the [latest stable version.](https://github.com/nigels-com/glew/releases/tag/glew-2.2.0)
* [SOIL](https://github.com/littlstar/soil.git) is a tiny C library used primarily for uploading textures into OpenGL.
