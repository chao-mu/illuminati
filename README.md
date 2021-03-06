# illuminati

## Install Dependencies

On ArchLinux

```
pacman -S hdf5 opencv vtk tclap
```

### MacOS

At the time of this writing you will need to build with something other than AppleClang due to a lack of support of std::filesystem. In these instructions we have you install llvm/clang.

(Assuming Homebrew is installed)

```
brew install llvm glew glfw tclap opencv
```

## Building

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

On MacOS, (assuming you installed llvm via brew) you will need to run the cmake command above like so:

```
$ CC=/usr/local/opt/llvm/bin/clang CXX=/usr/local/opt/llvm/bin/clang++ cmake ..
```
