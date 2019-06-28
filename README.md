# Turn Based Demo

This is an experimental turn-based game simulator thingy. Basically trying to
write a turn based battle system in C++, eventually as part of a larger game.

It's very much a work in progress, so be gentle. Or not. Either way.

## Building

### Dependencies

This project requires a C++17 compatible compiler. Versions that should work:
- GCC 8.2
- Clang 7
- MSVC 2017

You also need SFML version 2.5; it may be available on your package manager,
otherwise you can download it from their website [here][SFML].

Finally, you need CMake to generate the project files. I've put it at a
minimum version of 3.12; to be honest, earlier versions could possibly work
fine, but I've never tried. *Shrugs*

[SFML]: https://www.sfml-dev.org

### Actually Building

It's CMake, business as usual. Ideally, if it finds your installation of SFML
correctly, it should just be

    $ mkdir build-dir
    $ cd build-dir
    $ cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release /path/to/project/
    $ cmake --build .

Replacing the generator as preferred. If it doesn't find the path to SFML, you
could use `ccmake` or `cmake-gui` instead and provide it there, or add
`-DSFML_DIR=/path/to/SFML` to the generator line.

### Renderers

At the moment we default to a console renderer, which doesn't actually use
SFML, so anything regarding SFML above can be ignored. If you _really_ want to
use the SFML renderer, add `-DRENDERER=sfml` to the cmake generator step, or
otherwise set the `RENDERER` value appropriately.

Currently the following renderers are available:

- `console`: a 1970's style text interface. _Very_ bare-bones. (Default)
    - the `colour_console` branch has a possibly-outdated version with (some)
      colours.
- `sfml`: a 2D graphical interface. (Really just a black screen at the moment)

## Documentation

The documentation can be found under the `doc/` directory, in the form of
LaTeX files. They require a relatively recent distribution of TeXLive to build
correctly. Notably, pandoc currently falls over trying to interpret it.

At some stage a pre-built PDF of the documentation will be available online.
Not yet, though — I don't have anywhere to put it ;)
