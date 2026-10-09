#pragma once
#include "Simulation.hpp"
#include <filesystem>
namespace granja {
class Save {
public:
    static std::filesystem::path defaultPath();
    static bool write(const Simulation& sim,const std::filesystem::path& path,std::string& error);
    static bool read(Simulation& sim,const std::filesystem::path& path,std::string& error);
};
}
