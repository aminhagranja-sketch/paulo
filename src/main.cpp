#include "granja/Game.hpp"
#include <iostream>
#include <chrono>
int main(int argc,char** argv) {
    try {
        std::filesystem::path executable=std::filesystem::absolute(argv[0]);
        std::filesystem::path assets=executable.parent_path()/"assets",save=granja::Save::defaultPath();
        bool smoke=false,explicitSave=false;
        for(int i=1;i<argc;++i) {
            std::string argument=argv[i];
            if(argument=="--smoke") smoke=true;
            else if(argument=="--assets" && i+1<argc) assets=argv[++i];
            else if(argument=="--save" && i+1<argc) { save=argv[++i];explicitSave=true; }
            else if(argument=="--help") { std::cout<<"Meu Galinheiro: --assets <directory> --save <file> --smoke\n"; return 0; }
            else { std::cerr<<"Argumento desconhecido: "<<argument<<'\n'; return 2; }
        }
        if(smoke && !explicitSave) {
            auto stamp=std::chrono::steady_clock::now().time_since_epoch().count();
            save=std::filesystem::temp_directory_path()/("meu-galinheiro-smoke-"+std::to_string(stamp))/"save.json";
        }
        granja::Game game(assets,save,smoke); return game.run();
    } catch(const std::exception& e) { std::cerr<<"Falha ao iniciar Meu Galinheiro: "<<e.what()<<'\n'; return 1; }
}
