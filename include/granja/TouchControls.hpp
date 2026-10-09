#pragma once
#include "Types.hpp"
#include <map>
namespace granja {
// Pixel-space layout shared by SFML and the mobile WebAssembly frontend.
class TouchControls {
public:
    enum class Action { None, Joystick, Attack, Dodge, Eat, Interact };
    Vec stick{},attackButton{},dodgeButton{},eatButton{},interactButton{},knob{};
    float radius{72};
    bool visible{true};
    void resize(float width,float height) {
        reset(); radius=std::clamp(std::min(width,height)*.14f,40.f,76.f);
        float edge=radius+24;
        stick={edge,height-edge};knob=stick;attackButton={width-edge,height-edge};
        dodgeButton={width-edge,height-edge-2*radius};
        eatButton={width-edge-1.8f*radius,height-edge};
        interactButton={width-edge-1.8f*radius,height-edge-1.7f*radius};
    }
    Action down(int id,Vec p) {
        if(!visible || pointers.contains(id)) return Action::None;
        Action action=Action::None;
        if(distance(p,stick)<radius*1.3f && stickId==-999) {action=Action::Joystick;stickId=id;}
        else if(distance(p,attackButton)<radius*.9f) action=Action::Attack;
        else if(distance(p,dodgeButton)<radius*.6f) {action=Action::Dodge;pulses.dodge=true;}
        else if(distance(p,eatButton)<radius*.6f) {action=Action::Eat;pulses.eat=true;}
        else if(distance(p,interactButton)<radius*.6f) {action=Action::Interact;pulses.interact=true;}
        if(action!=Action::None) {pointers[id]=action;move(id,p);}
        return action;
    }
    void move(int id,Vec p) {
        if(id!=stickId) return;
        Vec offset=p-stick;float n=length(offset);
        if(n>radius) offset=offset*(radius/n);
        knob=stick+offset;
        float magnitude=length(offset)/radius;
        movement=magnitude<=.12f?Vec{}:normalized(offset)*((magnitude-.12f)/.88f);
    }
    void up(int id) {pointers.erase(id);if(id==stickId){stickId=-999;movement={};knob=stick;}}
    bool owns(int id) const {return pointers.contains(id);}
    bool attacking() const {for(auto [id,a]:pointers) if(a==Action::Attack)return true;return false;}
    Input sample() {Input result=pulses;pulses={};result.move=movement;result.attack=attacking();return result;}
    Vec moveVector() const {return movement;}
    void reset() {pointers.clear();stickId=-999;movement={};pulses={};knob=stick;}
private:
    std::map<int,Action> pointers;
    int stickId{-999};Vec movement{};Input pulses{};
};
}
