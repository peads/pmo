//
// Created by peads on 09/15/2026.
//

#ifndef EXPORTINFO_HPP
#define EXPORTINFO_HPP
#include <string>
#include <windows.h>
namespace PMO
{
    struct ExportInfo
    {
        uintptr_t thunkAddr;
        uintptr_t funcAddr;
        WORD ordinal;
        std::string name;

        bool operator<(const ExportInfo &a) const noexcept
        {
            return ordinal < a.ordinal;
        }

        bool operator==(const ExportInfo &a) const noexcept
        {
            return ordinal == a.ordinal;
        }
    };
}

template <>
struct std::hash<PMO::ExportInfo>
{
    size_t operator()(const PMO::ExportInfo &a) const noexcept
    {
        return std::hash<std::string>{}(a.name);
    }
};
#endif //EXPORTINFO_HPP
