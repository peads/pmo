#!/bin/bash

# This file is part of the pmo (peads Memory Operations) distribution
# (https://github.com/peads/pmo).
# Copyright (c) 2026 Patrick Eads.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

list="$(tail -n +5 "${1}" | sed -E 's/\[([^]]+)\]/\1/g' | sed 's/ \+[A-Za-z0-9 ;,\.+_,]\+//g' | sed 's/^\s\+[a-z ]\+$//g' | head -n -2 | sed -z 's/\s\+/,/g')" 
sed -i "1ibits 64\n" "${1}"
sed -i "s/^EXTERN\s\+\([A-Za-z0-9_]\+\):\s\+PROC/extern \1\nglobal ${list}\n/g" "${1}"
sed -i 's/^\.code$/section .text/g' "${1}"
sed -i "s/^\([A-Za-z0-9]\+\)\s\+PROC$/\1:/g" "${1}"
sed -i 's/^[A-Z0-9a-z]\+\s\+ENDP$//g' "${1}"
sed -i 's/^end$//g' "${1}"
printf "\n%s\n%s\n" "section .data data align=8" "section .bss bss align=8" >> "${1}"
