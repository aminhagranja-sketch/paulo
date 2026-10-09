#pragma once
#include "Renderer.hpp"
#include "Save.hpp"
#include "TouchControls.hpp"
#include "NativeTouch.hpp"
namespace granja {
class Game {
public:
    Game(const std::filesystem::path& assets,const std::filesystem::path& save,bool smoke=false);
    int run();
private:
    sf::RenderWindow window;
    Renderer renderer;
    Simulation sim;
    std::filesystem::path savePath;
    Vec camera{160,160};
    int screen{0}; // 0 title, 1 play, 2 pause, 3 victory
    bool help{},upgrades{},smoke{},hasSave{},started{};
    Input pulses;
    TouchControls touch;
    std::unique_ptr<NativeTouch> nativeTouch;
    void pointerDown(int id,Vec pixel);
    void pointerUp(int id);
    void events();
    void save();
    void newGame();
    void resumeGame();
};
}
