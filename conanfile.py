#type: ignore
# Ignore typing for the whole file, since all it does is throw errors on my machine.
# Easy enough to validate by hand.
import re

from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeDeps, CMakeToolchain
from conan.tools.build import check_min_cppstd
from pathlib import Path

project_regex_string = r"""project\s*\(\s*([a-z]+).*VERSION\s+([^\s]+)\s*\)\s*\n"""

class AlephConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def build_requirements(self):
        self.build_requires('cmake/4.2.1')

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.generate()

    def layout(self):
        cmake_layout(self)

    def set_name(self):
        cmake = Path(self.recipe_folder) / "CMakeLists.txt"
        content = cmake.read_text(encoding="utf-8")

        m = re.search(project_regex_string, content, re.IGNORECASE | re.VERBOSE | re.DOTALL)

        if not m:
            raise RuntimeError("Could not extract project name from CMakeLists.txt")

        self.name = m.group(1).lower()

    def set_version(self):
        cmake = Path(self.recipe_folder) / "CMakeLists.txt"
        content = cmake.read_text(encoding="utf-8")

        m = re.search(project_regex_string, content, re.IGNORECASE | re.VERBOSE | re.DOTALL)

        if not m:
            raise RuntimeError("Could not extract version from project() in CMakeLists.txt")

        self.version = m.group(2)

    def requirements(self):
        self.requires('boost/1.90.0')
        self.requires('fmt/12.1.0')
        self.requires('libassert/2.2.1')
        self.requires('spdlog/1.17.0')
        
        self.test_requires('gtest/1.17.0')
            
    def validate(self):
        check_min_cppstd(self, 20)
