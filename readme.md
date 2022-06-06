# Why the name ?

#### there's a saying that GL in OpenGL stands for **goodluck** (because you're gonna need it)

## Features

As of now this project loads a hard-coded fragment shader that draws the Mandelbrot set

It also has viewport controls with the following default keymap

-`w`, `a`, `s`, `d` **moves** the view
-`Space` to **zoom in**
-`Shift` to **zoom out**

***NEW !*** press `Tab` to switch to and from the corresponding Julia set

## Install instructions

If you're in Windows, just install CMake CLI and ninja manually and then run `./build.bat` to compile and `./build/main` to run

If you're on Linux run these commands 
```bash
git clone --recursive https://github.com/meutzitzu/goodluck.git
cd goodluck
sudo apt install ninja-build
sudo apt install xorg-dev
sudo apt install libglu1-dev
./build.sh
```

and run using `./build/main`

(it is assumed you are using a Debian-based distro in the prerequisite installation commands. If this is not the case then it is also assumed you can easily figure it out on your own 😉)
