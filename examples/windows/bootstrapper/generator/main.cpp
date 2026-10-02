/*
 * This file is part of the pmo (peads Memory Operations) distribution
 * (https://github.com/peads/pmo).
 * Copyright (c) 2026 Patrick Eads.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "bootstrapper.hpp"
#include <fstream>

int main(const int argc, char **argv)
{
    if (argc < 3)
        return -1;
    std::filesystem::path fpath(argv[1]);
    generateDllPath(fpath);
    if (!populateDllInfo(fpath))
        return -2;

    const auto fstem = fpath.stem();
    const auto &[module, ordinalBase, exports] = dllInfo;

    std::filesystem::path path(argv[2]);
    path.append(fstem.string() + ".asm");
    std::ofstream asmOut(path);
    if (!asmOut.is_open())
        return -3;
    std::stringstream asmHeader{};
    std::stringstream asmBody{};

    path = path.parent_path().append(fstem.string() + ".def");
    std::ofstream defOut(path);
    if (!defOut.is_open())
        return -4;

    const char *asm_header = nullptr;
    const char *asm_line[2];
    const char *asm_rdata[2];
    if (argc > 3)
    {
        asm_header = ARM64_ASM_HEADER;
        asm_line[0] = ARM64_ASM_LINE_1;
        asm_line[1] = ARM64_ASM_LINE_2;
        asm_rdata[0] = ARM64_ASM_RDATA_1;
        asm_rdata[1] = ARM64_ASM_RDATA_2;
    }
    else
    {
        asm_header = X64_ASM_HEADER;
        asm_line[0] = X64_ASM_LINE_1;
        asm_line[1] = X64_ASM_LINE_2;
        asm_rdata[0] = X64_ASM_RDATA_1;
        asm_rdata[1] = X64_ASM_RDATA_2;
    }

    asmHeader << asm_header;
    asmBody << "\n";
    defOut << std::format("LIBRARY {}\nEXPORTS\n", fstem.string());

    for (const auto &[ord, tup] : exports)
    {
        const auto &[addr, name, isNamed] = tup;
        const auto oord = ord + ordinalBase;
        const auto foord = std::format("f{}", ord);

        asmBody << std::format("{}", foord) << asm_line[0]
            << std::format("{}", ord << 3) << asm_line[1];
        asmHeader << std::format("{},", foord);

        if (isNamed)
            defOut << std::format("{}=", name);
        else
            defOut << std::format("ordinal{}=", oord);
        defOut << std::format("{} @{}\n", foord, oord);
    }

    asmOut << asmHeader.str() << "dllName\n" << asmBody.str()
        << asm_rdata[0] << std::format("{}", fpath.filename().string()) << asm_rdata[1];
    defOut << "\n";

    path = path.parent_path().append("bootstrap.txt");
    std::ofstream bootOut(path);
    if (!bootOut.is_open())
        return -5;
    bootOut << fstem.string() << std::endl;

    bootOut.close();
    asmOut.close();
    defOut.close();
    FreeLibrary(module);
    return 0;
}
