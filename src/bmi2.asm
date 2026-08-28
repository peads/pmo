; This file is part of the pmo (peads Memory Operations) distribution
; (https://github.com/peads/pmo).
; Copyright (c) 2026 Patrick Eads.
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, version 3.
;
; This program is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
; General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <http://www.gnu.org/licenses/>.

DEFAULT REL
global pDep, pExt

%ifidni __OUTPUT_FORMAT__, win64
; Windows x64 ABI args calling convention
    %define arg0    rcx
    %define arg1    rdx
    %define arg2    r8
    %define arg3    r9
%elifidni __OUTPUT_FORMAT__, elf64
; Linux x64 ABI args calling convention
    %define arg0    rdi
    %define arg1    rsi
    %define arg2    rdx
    %define arg3    rcx
    %define arg4    r8
    %define arg5    r9
    %define farg4   xmm4
    %define farg5   xmm5
    %define farg6   xmm6
    %define farg7   xmm7
%endif

%ifidni __OUTPUT_FORMAT__, elf64 || __OUTPUT_FORMAT__, win64
    %define farg0   xmm0
    %define farg1   xmm1
    %define farg2   xmm2
    %define farg3   xmm3
%endif

section .text
pDep:
    xor     eax, eax
    pdep    rax, arg0, arg1
;    pdep    rax, rcx, rdx
    ret
pExt:
    xor     eax, eax
    pext    rax, arg0, arg1
;    pext    rax, rcx, rdx
    ret
