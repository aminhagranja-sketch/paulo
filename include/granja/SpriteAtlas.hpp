#pragma once
#include "Types.hpp"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
namespace granja {
class SpriteAtlas {
    struct Sheet {nlohmann::json data;std::vector<std::unique_ptr<sf::Texture>> pages;};
    std::map<std::string,Sheet> sheets;
public:
    explicit SpriteAtlas(const std::filesystem::path& assets) {
        for(const std::string category:{"chests","buildings","trees","environment","enemies/snake","enemies/fox","characters/chicks","characters/chicken"}) {
            Sheet sheet;std::ifstream input(assets/"animations"/(category+".json"));input>>sheet.data;
            for(const auto& file:sheet.data.at("pages")) {auto texture=std::make_unique<sf::Texture>();if(!texture->loadFromFile(assets/"atlases"/file.get<std::string>()))throw std::runtime_error("Atlas ausente: "+category);texture->setSmooth(false);sheet.pages.push_back(std::move(texture));}
            sheets.emplace(category,std::move(sheet));
        }
    }
    void draw(sf::RenderTarget& target,const std::string& category,const std::string& animation,Vec position,float time,float scale=1,bool flip=false,float progress=-1,sf::Color color=sf::Color::White) {
        auto& sheet=sheets.at(category);std::string id=animation;
        if(sheet.data.at("animations").contains(animation)) {
            const auto& a=sheet.data["animations"][animation];const auto& frames=a["frames"];int index=progress>=0?int(progress*frames.size()):int(time*a["fps"].get<float>());
            index=a.value("loop",false)&&progress<0?index%int(frames.size()):std::clamp(index,0,int(frames.size())-1);id=frames.at(index).get<std::string>();
        }
        const auto& frame=sheet.data["frames"].at(id);const auto& r=frame["rect"];const auto& o=frame["origin"];
        sf::Sprite sprite(*sheet.pages.at(frame["page"].get<int>()),sf::IntRect({r[0].get<int>(),r[1].get<int>()},{r[2].get<int>(),r[3].get<int>()}));
        scale*=frame.value("scaleAdjustment",1.f);sprite.setOrigin({o[0].get<float>(),o[1].get<float>()});sprite.setScale({flip?-scale:scale,scale});sprite.setPosition({std::round(position.x),std::round(position.y)});sprite.setColor(color);target.draw(sprite);
    }
};
}
