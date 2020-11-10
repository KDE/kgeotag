#! /usr/bin/env bash

# SPDX-FileCopyrightText: none
#
# SPDX-License-Identifier: CC0-1.0

$XGETTEXT `find . \( -name \*.cpp -o -name \*.h -o -name \*.qml \)` -o $podir/kgeotag.pot
