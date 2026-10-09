#pragma once
#include "Simulation.hpp"
#include "TouchControls.hpp"
#include "SpriteAtlas.hpp"
#include <SFML/Graphics.hpp>
#include <filesystem>
namespace granja {
class Renderer {
public:
    explicit Renderer(const std::filesystem::path& assets);
    void draw(sf::RenderWindow& window,const Simulation& sim,Vec camera,int screen,bool help,bool upgradeOpen,const TouchControls* touch=nullptr);
    static sf::View uiView(sf::Vector2u size);
    static sf::View worldView(sf::Vector2u size,Vec camera);
private:
    sf::Font font;
    SpriteAtlas sprites;
    void playerSprite(const Player& p,float time,float size=80);
    void touchHud(const TouchControls& touch);
    sf::RenderTarget* out{};
    void oval(Vec p,Vec radius,sf::Color color,float outline=0,sf::Color border=sf::Color::Transparent);
    void rect(Vec p,Vec size,sf::Color color,float radius=0);
    void line(Vec a,Vec b,float width,sf::Color color);
    void text(const std::string& value,Vec p,unsigned size,sf::Color color=sf::Color::White,bool center=false);
    void polygon(std::initializer_list<Vec> points,sf::Color color);
    void chicken(Vec p,Vec facing,sf::Color body,float t,float scale=1,bool moving=false,bool boss=false);
    void tree(Vec p,std::uint32_t hash,float time);
    void terrain(const Simulation& s,Vec camera);
    void scenery(const Simulation& s,Vec camera);
    void hud(const Simulation& s,bool upgradeOpen,bool touchMode=false);
    void panel(Vec p,Vec size);
    void bar(Vec p,Vec size,float ratio,sf::Color color);
    void icon(LootKind kind,Vec p,float time,float scale=1);
};
}
