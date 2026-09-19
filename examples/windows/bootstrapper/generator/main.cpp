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

int main(const int argc, char **argv)
{
    if (argc < 2)
        return -1;

    const auto fpath = generateDllPath(argv[1]);
    if (!populateDllInfo(fpath))
        return -2;

    const auto fstem = fpath.stem();
    const auto &[module, ordinalBase, exports] = dllInfo;

    std::filesystem::path path(OUT_PATH);
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

    asmHeader << "bits 64\nsection .text\nextern mapping\nglobal ";
    asmBody << "\n";
    defOut << std::format("LIBRARY {}\nEXPORTS\n", fstem.string());

    for (const auto &[ord, tup] : exports)
    {
        const auto &[addr, name, isNamed] = tup;
        const auto oord = ord + ordinalBase;
        const auto foord = std::format("f{}", ord);

        asmBody << std::format("{}:\n\tmov rax, [rel mapping]\n\tjmp [rax + {}]\n",
                               foord,
                               ord << 3);
        asmHeader << std::format("{},", foord);

        if (isNamed)
        {
            defOut << std::format("{}=", name);
        }
        else
        {
            defOut << std::format("ordinal{}=", oord);
        }
        defOut << std::format("{} @{}\n", foord, oord);
    }

    asmOut << asmHeader.str() << "dllName\n" << asmBody.str() <<
        std::format("section .rdata\n\tdllName db \"{}\", 0\n", fpath.filename().string());
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
