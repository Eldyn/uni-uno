from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout


class UniUnoConan(ConanFile):
    """Conan recipe for the Uni-Uno C++23 backend.

    Equivalent to the former conanfile.txt, but expressed as a recipe so the
    CMakeToolchain generator can disable CMakeUserPresets.json generation. The
    project ships a committed CMakePresets.json that already defines the
    'conan-release' preset; letting Conan emit its own CMakeUserPresets.json
    would redefine the same preset name and trigger a duplicate-preset error.
    """

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    requires = (
        "openssl/[~3.2]",
        "libuv/1.46.0",
        "zlib/1.3.1",
        "uwebsockets/20.48.0",
        "sqlite3/3.45.1",
        "jwt-cpp/0.7.0",
        "nlohmann_json/3.11.3",
    )

    tool_requires = "ninja/1.13.2"

    default_options = {
        "sqlite3/*:omit_load_extension": True,
        "sqlite3/*:threadsafe": 1,
        "sqlite3/*:build_executable": False,
        "usockets/*:with_ssl": "openssl",
    }

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.generate()
