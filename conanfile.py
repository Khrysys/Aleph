from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeDeps, CMakeToolchain
from conan.tools.build import check_min_cppstd

import re
from pathlib import Path

class AlephConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def build_requirements(self):
        self.requires('cmake/4.2.1', force=True)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def layout(self):
        cmake_layout(self)
        
    def package(self):
        cmake = CMake(self)
        cmake.install()

    def set_name(self):
        cmake = Path(self.recipe_folder) / "CMakeLists.txt"
        content = cmake.read_text(encoding="utf-8")

        m = re.search(
            r"""
            project
            \s*\(
            \s*([A-Za-z_][A-Za-z0-9_+-]*)   # project name
            """,
            content,
            re.IGNORECASE | re.VERBOSE | re.DOTALL,
        )

        if not m:
            raise RuntimeError("Could not extract project name from CMakeLists.txt")

        self.name = m.group(1).lower()

    def set_version(self):
        cmake = Path(self.recipe_folder) / "CMakeLists.txt"
        content = cmake.read_text(encoding="utf-8")

        m = re.search(
            r"""
            project
            \s*\(
            \s*[A-Za-z_][A-Za-z0-9_+-]*
            [^)]*?
            VERSION\s+
            ([^\s\)]+)
            """,
            content,
            re.IGNORECASE | re.VERBOSE | re.DOTALL,
        )

        if not m:
            raise RuntimeError("Could not extract version from project() in CMakeLists.txt")

        self.version = m.group(1)

    def requirements(self):
        self.requires('abseil/20250814.0')
        self.requires('argparse/3.2')
        self.requires('boost/1.90.0')
        self.requires('half/2.2.0')
        self.requires('spdlog/1.17.0')
        self.requires('tomlplusplus/3.4.0')
        if self.options.get_safe('with_libtorch', False):
            self.requires('libtorch/2.9.1')
        if self.options.get_safe('with_onnxruntime', False):
            self.requires('onnxruntime/1.23.2')
            
        self.test_requires('gtest/1.17.0')
            
    def validate(self):
        check_min_cppstd(self, 20)
