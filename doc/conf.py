# SPDX-FileCopyrightText: 2020 The unpaper Authors
#
# SPDX-License-Identifier: MIT

project = "unpaper"
copyright = "2020, The unpaper Authors"
author = "The unpaper Authors"

# Avoid generate sub-directory for the man pages during build:
# https://github.com/sphinx-doc/sphinx/issues/9217
# This appears incompatible with the Meson expectation.
man_make_section_directory = False

man_pages = [
    ("unpaper.1", "unpaper", "unpaper", "The unpaper authors", 1),
]
