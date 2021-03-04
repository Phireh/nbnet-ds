## Cloning
git clone https://github.com/Phireh/nbnet-ds.git --recurse-submodules

## Compiling
First, compile the nbnet server.

In `include/nbnet/examples/echo` write:
    * `gcc server.c shared.c -lm -o server`

Secondly, compile the 3ds executable. If you have `devkitpro` installed and the `$DEVKITPRO` env variable defined, it should be done by simply writing `make` in the root dir.

## Usage
Execute `include/nbnet/examples/echo/server`
Open `nbnet-ds.3dsx` with Citra. Remember that you need a Citra version that has been compiled with networking capabilities.

## About
Just some light experimentation with nathhB's excellent library.

## TODO
    * Fix my bad style
    * Figure out how to make this work on Citra public rooms
