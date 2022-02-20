# Chess3D - A 3d chess game built with OpenGL and C++

## Requirments
- OpenGL 3.3 or above
- cmake
- gcc or clang
		
## Compiling

Use cmake to make the build files
```sh
$ git clone --recursive https://github.com/mdrosiadis/Chess3D.git
$ cd Chess3D
$ mkdir build && cd build
$ cmake ..
```
Using clang on windows is recomended. 
Building with cmake on Windows requires elevation to symlink the assets folder.

## Running 

```sh
$ ./Chess3D
```

To load a custom position, run with the game FEN as an argument.

```sh
$ ./Chess3D "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 b - - 1 23"
```

## Screenshots

Starting position
![Starting Position](/screenshots/chess3d1.png?raw=true)

"The Imortal Game" checkmate (FEN provided above)
![The Imortal Game](/screenshots/chess3d2.png?raw=true)
