import re
from pathlib import Path

# -- Project paths ------------------------------------------------------------

_repo_root = Path(__file__).parent.parent.resolve()
_cmake = (_repo_root / "CMakeLists.txt").read_text(encoding="utf-8")

# -- Extract project name and version from CMakeLists.txt ---------------------

_name_match = re.search(
    r"""
    project
    \s*\(
    \s*([A-Za-z_][A-Za-z0-9_+-]*)
    """,
    _cmake,
    re.IGNORECASE | re.VERBOSE | re.DOTALL,
)
if not _name_match:
    raise RuntimeError("Could not extract project name from CMakeLists.txt")

_version_match = re.search(
    r"""
    project
    \s*\(
    \s*[A-Za-z_][A-Za-z0-9_+-]*
    [^)]*?
    VERSION\s+
    ([^\s\)]+)
    """,
    _cmake,
    re.IGNORECASE | re.VERBOSE | re.DOTALL,
)
if not _version_match:
    raise RuntimeError("Could not extract version from project() in CMakeLists.txt")

# -- Project information ------------------------------------------------------

project = _name_match.group(1)
author = "Khrysys"
release = _version_match.group(1)
version = ".".join(release.split(".")[:2])  # major.minor only for |version|

# -- General configuration ----------------------------------------------------

extensions = [
    "myst_parser",
]

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

root_doc = "index"

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