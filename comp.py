from comp_backend import *
import pathlib

CLFLAGS="/favor:AMD64 /FS /Zi /DEBUG /arch:AVX2 /MD /EHsc"
LINKFLAGS="/DEBUG"
DLLFLAGS=""

BUILD_DIR = "build"
BIN_DIR = "bin"

CLFLAGS_DBG = "-g -Og -O0 -march=native"
LINKFLAGS_DBG = "-g -Og -O0 -march=native"

BUILD_DIR_DBG = "build-gcc"
BIN_DIR_DBG = "bin-gcc"

BUILD_DIR_ZIG = "build-zig"
BIN_DIR_ZIG = "bin-zig"

WEB_BUILD_DIR = "build-web"
WEB_BIN_DIR = "bin-web"


WORKDIR = pathlib.Path(__file__).parent.resolve()

add_backend("Msvc", "Msvc", BUILD_DIR, BIN_DIR, WORKDIR, CLFLAGS, LINKFLAGS)
add_backend("Mingw", "Mingw", BUILD_DIR_DBG, BIN_DIR_DBG, WORKDIR, CLFLAGS_DBG, LINKFLAGS_DBG)
add_backend("Zigcc", "Zigcc", BUILD_DIR_ZIG, BIN_DIR_ZIG, WORKDIR, CLFLAGS_DBG, LINKFLAGS_DBG)
#add_backend("Emcc", "Emcc", WEB_BUILD_DIR, WEB_BIN_DIR, WORKDIR, "", "")

set_backend("Msvc")

def main():
    sdl3 = find_package("SDL3")
    sdl3_image = find_package("SDL3_image")
    sdl3_ttf = find_package("SDL3_ttf")
    packages = [sdl3, sdl3_image, sdl3_ttf]

    engine_dir = WORKDIR / "src" / "engine"
    engine_src = [engine_dir / "engine.cpp",
                  engine_dir / "game.cpp",
                  engine_dir / "input.cpp",
                  engine_dir / "random.cpp",
                  engine_dir / "texture.cpp",
                  engine_dir / "ui.cpp",
                  engine_dir / "menu.cpp",
                  engine_dir / "events.cpp"]

    src = ["src/main.cpp", "src/game.cpp", "src/editbox.cpp",
           "src/editlines.cpp", "src/maze.cpp", "src/language.cpp",
           "src/parser.cpp", "src/slime.cpp", "src/equipment.cpp",
           "src/player.cpp", "src/utils.cpp"]

    with Context(namespace="engine"):
        engine = [Object(p.with_suffix(".obj").name, p,
                         packages=packages)
                  for p in engine_src]

    CopyToBin(*sdl3.dlls, *sdl3_image.dlls, *sdl3_ttf.dlls)
    exe = Executable("main.exe", *src, *engine,
                     packages=packages)
    
    build(__file__)


if __name__ == "__main__":
    main()
