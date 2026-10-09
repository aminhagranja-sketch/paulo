#include "granja/Game.hpp"
#include <iostream>
namespace granja {
namespace { sf::ContextSettings settings() { sf::ContextSettings s;s.antiAliasingLevel=4;return s; } }
Game::Game(const std::filesystem::path& assets,const std::filesystem::path& save,bool smokeTest)
    :window(sf::VideoMode({1280,720}),"Meu Galinheiro | Aventura nos Campos",sf::Style::Default,sf::State::Windowed,settings()),renderer(assets),savePath(save),smoke(smokeTest) {
    window.setFramerateLimit(60); window.setKeyRepeatEnabled(false);
    hasSave=std::filesystem::exists(savePath);
    touch.resize(float(window.getSize().x),float(window.getSize().y));
    #ifdef _WIN32
    nativeTouch=std::make_unique<NativeTouch>(window.getNativeHandle(),[this](int phase,int id,Vec pixel){
        if(phase==0)pointerDown(id,pixel);else if(phase==1)touch.move(id,pixel);else pointerUp(id);
    });
#endif
    if(smoke) newGame();
}
void Game::newGame() { touch.reset();sim=Simulation{}; camera=sim.player.pos; screen=1; started=true; upgrades=false; }
void Game::resumeGame() {
    std::string error;
    if(Save::read(sim,savePath,error)) { touch.reset();screen=1; camera=sim.player.pos; started=true; }
    else { sim.notify("Save não carregado: "+error); hasSave=false; }
}
void Game::save() {
    std::string error;
    if(Save::write(sim,savePath,error)) { hasSave=true; sim.notify("Progresso salvo com segurança."); }
    else { sim.notify("Falha ao salvar: "+error); std::cerr<<"Save: "<<error<<'\n'; }
    sim.saveRequested=false;
}
void Game::pointerDown(int id,Vec pixel) {
    auto mapped=window.mapPixelToCoords({int(pixel.x),int(pixel.y)},Renderer::uiView(window.getSize()));
    Vec ui{mapped.x,mapped.y};
    if(screen==0) {if(hasSave)resumeGame();else newGame();touch.reset();return;}
    if(help) {help=false;touch.reset();return;}
    if(screen==2 || screen==3) {screen=1;touch.reset();return;}
    if(upgrades) {
        if(ui.x>550 && ui.x<1370 && ui.y>405 && ui.y<720) sim.upgrade(int((ui.y-405)/105));
        else upgrades=false;
        return;
    }
    if(pixel.x>window.getSize().x-65 && pixel.y<65) {screen=2;touch.reset();return;}
    touch.down(id,pixel);
}
void Game::pointerUp(int id) {touch.up(id);}
void Game::events() {
    while(auto event=window.pollEvent()) {
        if(event->is<sf::Event::Closed>()) { if(started) save(); window.close(); }
        if(event->is<sf::Event::FocusLost>() && !smoke) {if(screen==1)screen=2;touch.reset();}
        if(event->is<sf::Event::Resized>()) touch.resize(float(window.getSize().x),float(window.getSize().y));
        if(const auto* e=event->getIf<sf::Event::TouchBegan>()) pointerDown(int(e->finger),{float(e->position.x),float(e->position.y)});
        if(const auto* e=event->getIf<sf::Event::TouchMoved>()) touch.move(int(e->finger),{float(e->position.x),float(e->position.y)});
        if(const auto* e=event->getIf<sf::Event::TouchEnded>()) pointerUp(int(e->finger));
        if(const auto* e=event->getIf<sf::Event::MouseButtonPressed>();e && e->button==sf::Mouse::Button::Left) pointerDown(-1,{float(e->position.x),float(e->position.y)});
        if(const auto* e=event->getIf<sf::Event::MouseMoved>()) touch.move(-1,{float(e->position.x),float(e->position.y)});
        if(const auto* e=event->getIf<sf::Event::MouseButtonReleased>();e && e->button==sf::Mouse::Button::Left) pointerUp(-1);
        if(const auto* key=event->getIf<sf::Event::KeyPressed>()) {
            auto k=key->code;
            if(screen==0) {
                if(k==sf::Keyboard::Key::Enter) { if(hasSave) resumeGame(); else newGame(); }
                if(k==sf::Keyboard::Key::N) { newGame(); sim.notify("Nova aventura. F5 para salvar; o save anterior só será substituído ao salvar."); }
                if(k==sf::Keyboard::Key::Escape) window.close();
                continue;
            }
            if(k==sf::Keyboard::Key::Escape) { touch.reset();if(help || upgrades) {help=false; upgrades=false;} else screen=screen==1?2:1; }
            if(k==sf::Keyboard::Key::F2) {touch.visible=!touch.visible;touch.reset();}
            if(k==sf::Keyboard::Key::F1) help=!help;
            if(k==sf::Keyboard::Key::F5) save();
            if(k==sf::Keyboard::Key::F9) resumeGame();
            if(screen!=1 || help) continue;
            if(k==sf::Keyboard::Key::Q) pulses.eat=true;
            if(k==sf::Keyboard::Key::E) pulses.interact=true;
            if(k==sf::Keyboard::Key::LShift || k==sf::Keyboard::Key::RShift) pulses.dodge=true;
            if(k==sf::Keyboard::Key::U) { if(sim.atNest()) upgrades=!upgrades; else sim.notify("As melhorias estão disponíveis no ninho."); }
            if(k==sf::Keyboard::Key::Num1) sim.upgrade(0);
            if(k==sf::Keyboard::Key::Num2) sim.upgrade(1);
            if(k==sf::Keyboard::Key::Num3) sim.upgrade(2);
        }
    }
}
int Game::run() {
    sf::Clock clock; float accumulator=0,autosave=0; int frames=0; bool smokeSaved=false;
    while(window.isOpen()) {
        float dt=std::min(.1f,clock.restart().asSeconds()); events(); if(!window.isOpen()) break;
        if(smoke) dt=1.f/60;
        if(screen==1 && !help && !upgrades) {
            Input input=pulses; pulses={};
            if(smoke) { input.move=frames<75?Vec{1,0}:Vec{}; input.attack=frames>80 && frames<110; input.interact=frames==0; }
            else {
                auto held=[](sf::Keyboard::Key k){return sf::Keyboard::isKeyPressed(k);};
                input.move={float(held(sf::Keyboard::Key::D)||held(sf::Keyboard::Key::Right))-float(held(sf::Keyboard::Key::A)||held(sf::Keyboard::Key::Left)),
                            float(held(sf::Keyboard::Key::S)||held(sf::Keyboard::Key::Down))-float(held(sf::Keyboard::Key::W)||held(sf::Keyboard::Key::Up))};
                bool mouse=sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
                mouse=mouse && !touch.owns(-1);
                input.attack=held(sf::Keyboard::Key::Space)||mouse;
                Input finger=touch.sample();
                if(length(finger.move)>0) input.move=finger.move;
                input.attack=input.attack||finger.attack;
                input.dodge=input.dodge||finger.dodge;input.eat=input.eat||finger.eat;input.interact=input.interact||finger.interact;
                if(mouse) { auto p=window.mapPixelToCoords(sf::Mouse::getPosition(window),Renderer::worldView(window.getSize(),camera)); input.aim=Vec{p.x,p.y}-sim.player.pos; }
            }
            accumulator+=dt;
            bool first=true;
            while(accumulator>=1.f/60) {
                Input step=input; if(!first) step.dodge=step.eat=step.interact=false;
                sim.update(1.f/60,step);
                if(step.interact && (sim.atNest()||sim.atShop()) && !smoke) {upgrades=true;touch.reset();} accumulator-=1.f/60; first=false;
            }
            if(first) pulses=input; // preserve edge inputs if no fixed step elapsed
            camera+=(sim.player.pos-camera)*std::min(1.f,dt*7);
            autosave+=dt;
            if(sim.saveRequested || autosave>45) {save(); autosave=0;}
            if(sim.victoryEvent) {sim.victoryEvent=false; screen=3; save();}
        } else { accumulator=0; pulses={}; }
        renderer.draw(window,sim,camera,screen,help,upgrades,screen==1 && !help && !upgrades?&touch:nullptr); window.display(); ++frames;
        if(smoke && frames==130) {
            save(); Simulation loaded; std::string error;
            smokeSaved=Save::read(loaded,savePath,error) && loaded.player.food>=4 && loaded.player.pos.x>350 && loaded.world.active().size()==9;
            sf::Texture screenshot(window.getSize()); screenshot.update(window);
            if(!screenshot.copyToImage().saveToFile(savePath.parent_path()/"smoke.png")) { std::cerr<<"Screenshot failed\n"; return 1; }
            std::cout<<"SMOKE frames="<<frames<<" eggs="<<loaded.player.eggs<<" chunks="<<loaded.world.active().size()<<" save_reload="<<(smokeSaved?"PASS":"FAIL")<<'\n';
            if(!smokeSaved) std::cerr<<error<<'\n';
            window.close();
        }
    }
    return smoke && !smokeSaved?1:0;
}
}
