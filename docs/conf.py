import re
from pathlib import Path

# -- Project paths ------------------------------------------------------------

_repo_root = Path(__file__).parent.parent.resolve()
_cmake = (_repo_root / "CMakeLists.txt").read_text(encoding="utf-8")

# -- Extract project name and version from CMakeLists.txt ---------------------

project_regex_string = r"""project\s*\(\s*([a-z]+).*VERSION\s+([^\s]+)\s*\)\s*\n"""
project_match = re.search(project_regex_string, _cmake, re.IGNORECASE | re.VERBOSE | re.DOTALL)
if not project_match:
    raise RuntimeError("Could not extract project name from CMakeLists.txt")

# -- Project information ------------------------------------------------------

project = project_match.group(1)
author = "Khrysys"
release = project_match.group(2)
version = ".".join(release.split(".")[:2])  # major.minor only for |version|

# -- General configuration ----------------------------------------------------

extensions = [
    "myst_parser",
]

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

root_doc = "src/index"

exclude_patterns = [
    "_build",
    "Thumbs.db",
    ".DS_Store",
]

# -- MyST configuration -------------------------------------------------------

myst_enable_extensions = [
    "colon_fence",      # ::: fence syntax as alternative to ```
    "deflist",          # definition lists
    "fieldlist",        # field lists for structured metadata
]

# -- HTML output --------------------------------------------------------------

html_theme = "furo"

html_theme_options = {
    "source_repository": "https://github.com/Khrysys/aleph",
    "source_branch": "main",
    "source_directory": "docs/",
}