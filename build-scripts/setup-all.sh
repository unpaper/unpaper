#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2024 The unpaper authors
#
# SPDX-License-Identifier: MIT

if git status &>/dev/null; then
  ROOT=$(git rev-parse --show-toplevel)
elif sl root &>/dev/null; then
  ROOT=$(sl root)
else
  echo "Unable to detect SCM." >&2
  exit 1
fi

SUBENV="${1:-.}"

cd "$ROOT"
for file in meson-environments/${SUBENV}/*.ini; do
  environment=$(basename "${file}" .ini)

  meson setup --reconfigure --native-file "${file}" "builddir/${environment}" || exit $?
done
