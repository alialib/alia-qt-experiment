Experimental alia/Qt Integration
================================

This is an experimental integration between alia and Qt. It's intended as a
proof of concept and is outdated. **This is not intended for real use.** It's
not even currently up-to-date with the core library. (It might build, but it
doesn't utilize some newer alia features that it really should, like object
trees.)

I might pursue this further at some point, but it's not an immediate priority
for me. If you're interested in it and are willing to put in some work, feel
free to get in touch.

Building
--------

1. Clone this repository and change to this directory:
   ```shell
   git clone https://github.com/alialib/alia-qt-experiment
   cd alia-qt-experiment
   ```

2. Get a copy of `alia.hpp`:
   ```shell
   wget https://alia.dev/alia.hpp
   ```

3. Create a build directory:

   ```shell
   mkdir build
   cd build
   ```

4. Either install Qt directly or use Conan to install it:

   ```shell
   conan install ..
   ```

5. Build the project:
   ```shell
   cmake ..
   cmake --build .
   ```

   (Or on Windows, you might need to explicitly specify a generator...)

   ```shell
   cmake -G"NMake Makefiles" ..
   cmake --build .
   ```

The Sandbox
-----------

This produces a ``sandbox`` executable. You can edit `sandbox.cpp` to play
around with the contents of the sandbox.
