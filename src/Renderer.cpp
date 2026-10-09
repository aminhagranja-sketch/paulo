#include "granja/Renderer.hpp"
#include <numbers>
#include <stdexcept>
#include <functional>
namespace granja {
namespace {
const sf::Color Ink{23,39,38},Muted{173,195,175},Cream{250,243,215},Gold{255,207,89},Mint{136,222,157};
sf::Color alpha(sf::Color c,unsigned a) {c.a=static_cast<std::uint8_t>(a);return c;}
sf::String utf8(const std::string& s) {return sf::String::fromUtf8(s.begin(),s.end());}
}
Renderer::Renderer(const std::filesystem::path& assets) {
    if(!playerAtlas.loadFromFile(assets/"sprites"/"chicken-walk.png")) throw std::runtime_error("Atlas do personagem ausente.");
    playerAtlas.setSmooth(true);
    hasActions=actionAtlas.loadFromFile(assets/"sprites"/"chicken-actions.png");
    if(hasActions) actionAtlas.setSmooth(true);
    if(!font.openFromFile(assets/"fonts"/"DejaVuSans.ttf")) throw std::runtime_error("Fonte ausente. Execute junto da pasta assets ou use --assets.");
}
sf::View Renderer::uiView(sf::Vector2u size) {
    sf::View view(sf::FloatRect({0,0},{1920,1080}));
    float ratio=size.y?float(size.x)/size.y:16.f/9;
    if(ratio>16.f/9) {float w=(16.f/9)/ratio;view.setViewport({{(1-w)/2,0},{w,1}});}
    else {float h=ratio/(16.f/9);view.setViewport({{0,(1-h)/2},{1,h}});}
    return view;
}
sf::View Renderer::worldView(sf::Vector2u size,Vec camera) {
    auto view=uiView(size); view.setSize({1440,810});view.setCenter({camera.x,camera.y});return view;
}
void Renderer::oval(Vec p,Vec radius,sf::Color color,float outline,sf::Color border) {
    sf::CircleShape shape(1,24);shape.setScale({radius.x,radius.y});shape.setOrigin({1,1});shape.setPosition({p.x,p.y});shape.setFillColor(color);
    if(outline!=0) {shape.setOutlineThickness(outline/std::max(1.f,radius.x));shape.setOutlineColor(border);} out->draw(shape);
}
void Renderer::rect(Vec p,Vec size,sf::Color color,float) {
    sf::RectangleShape shape({size.x,size.y});shape.setPosition({p.x,p.y});shape.setFillColor(color);out->draw(shape);
}
void Renderer::line(Vec a,Vec b,float width,sf::Color color) {
    Vec d=b-a;sf::RectangleShape shape({length(d),width});shape.setOrigin({0,width*.5f});shape.setPosition({a.x,a.y});shape.setRotation(sf::radians(std::atan2(d.y,d.x)));shape.setFillColor(color);out->draw(shape);
}
void Renderer::polygon(std::initializer_list<Vec> points,sf::Color color) {
    sf::ConvexShape s(points.size());int i=0;for(auto p:points)s.setPoint(i++,{p.x,p.y});s.setFillColor(color);out->draw(s);
}
void Renderer::text(const std::string& value,Vec p,unsigned size,sf::Color color,bool center) {
    sf::Text t(font,utf8(value),size);t.setFillColor(color);
    if(center) {auto b=t.getLocalBounds();t.setOrigin({b.position.x+b.size.x*.5f,b.position.y});}
    t.setPosition({p.x,p.y});out->draw(t);
}
void Renderer::panel(Vec p,Vec size) {
    rect(p+Vec{0,7},size,{5,16,22,65});rect(p,size,{21,39,38,235});rect(p,{size.x,2},{91,124,100,220});
}
void Renderer::bar(Vec p,Vec size,float ratio,sf::Color color) {
    rect(p,size,{9,23,26,210});rect(p,{size.x*std::clamp(ratio,0.f,1.f),size.y},color);rect(p,{size.x,2},alpha(sf::Color::White,35));
}
void Renderer::chicken(Vec p,Vec facing,sf::Color body,float t,float scale,bool moving,bool boss) {
    auto q=[&](float x,float y){return p+Vec{x*scale,y*scale};};
    auto ellipse=[&](float x,float y,float rx,float ry,sf::Color color){oval(q(x,y),{rx*scale,ry*scale},color);};
    float bob=moving?std::sin(t*15)*2:std::sin(t*2)*1;
    ellipse(5,7,23,10,{20,46,34,80});
    float step=moving?std::sin(t*15)*4:0;
    line(q(-7,10),q(-8,20+step),3*scale,{234,153,45});line(q(7,10),q(9,20-step),3*scale,{234,153,45});
    line(q(-8,20+step),q(-14,22+step),2*scale,{255,185,69});line(q(9,20-step),q(15,22-step),2*scale,{255,185,69});
    // Tail feathers opposite the gaze, soft highlight and a dark rim.
    ellipse(-facing.x*18,1-facing.y*9+bob,9,13,body);
    ellipse(-facing.x*22,-3-facing.y*9+bob,5,11,Cream);
    ellipse(0,-3+bob,20,24,{70,69,51});ellipse(0,-6+bob,19,23,body);
    ellipse(-6,-12+bob,10,14,alpha(Cream,110));
    ellipse(facing.x>0?-10:10,0+bob,8,14,{207,188,139});
    ellipse(facing.x>0?-10:10,-3+bob,7,12,body);
    float hx=facing.x*8,hy=-19+facing.y*6+bob;
    ellipse(hx,hy,15,15,body);ellipse(hx-4,hy-4,9,8,alpha(Cream,135));
    ellipse(hx-6,hy-15,4,7,{209,58,54});ellipse(hx,hy-17,5,8,{232,68,57});ellipse(hx+6,hy-15,4,6,{214,48,48});
    ellipse(hx+facing.x*11,hy+9,4,6,{205,61,52});
    Vec beak=q(hx+facing.x*14,hy+facing.y*8);
    polygon({beak+Vec{-4*scale,-4*scale},beak+Vec{facing.x*12*scale,4*scale+facing.y*8*scale},beak+Vec{-4*scale,5*scale}},{255,179,48});
    float ex=hx+(facing.x>=0?5.f:-5.f);
    ellipse(ex,hy-2,4,5,{255,252,228});ellipse(ex+(facing.x>=0?1:-1),hy-2,2.4f,3,{32,37,37});ellipse(ex+1,hy-3,1,1,sf::Color::White);
    if(boss) {
        polygon({q(-14,-47),q(-16,-61),q(-7,-55),q(0,-68),q(8,-55),q(17,-61),q(14,-47)},Gold);
        line(q(-14,-47),q(14,-47),4*scale,{166,113,35});ellipse(0,-51,3,3,{201,51,73});
    }
}
void Renderer::playerSprite(const Player& p,float time,float size) {
    int direction=std::abs(p.facing.x)>std::abs(p.facing.y)?(p.facing.x<0?1:3):(p.facing.y<0?2:0);
    int frame=p.moving?int(p.walkTime*(p.dodgeTimer>0?20:9))%4:0;
    sf::Texture* texture=&playerAtlas;
    bool flip=false;int row=direction;
    if(hasActions && (p.attackVisual>0 || p.dodgeTimer>0 || p.invulnerable>.3f || (p.bossDefeated && !p.moving))) {
        texture=&actionAtlas;flip=p.facing.x<0;
        row=p.attackVisual>0?0:p.dodgeTimer>0?1:p.invulnerable>.3f?2:3;
        frame=p.attackVisual>0?std::min(3,int((.18f-p.attackVisual)/.18f*4)):int(time*14)%4;
    }
    int cw=int(texture->getSize().x/4),ch=int(texture->getSize().y/4);
    sf::Sprite sprite(*texture,sf::IntRect({frame*cw,row*ch},{cw,ch}));
    sprite.setOrigin({cw*.5f,ch*.94f});float scale=size/ch;
    sprite.setScale({flip?-scale:scale,scale});sprite.setPosition({p.pos.x,p.pos.y+12});
    oval(p.pos+Vec{3,8},{22,9},{24,45,29,85});
    if(p.invulnerable>0 && int(time*16)%2==0) sprite.setColor({255,215,175,180});
    out->draw(sprite);
}
void Renderer::touchHud(const TouchControls& touch) {
    float r=touch.radius;
    oval(touch.stick,{r,r},{17,34,40,130});oval(touch.stick,{r*.76f,r*.76f},{105,157,133,55});
    oval(touch.knob,{r*.44f,r*.44f},{214,234,208,190});
    auto button=[&](Vec pos,float size,const char* label,bool active){
        oval(pos,{size,size},active?sf::Color{226,161,60,230}:sf::Color{24,44,44,195});
        oval(pos,{size-3,size-3},active?sf::Color{238,179,77,230}:sf::Color{70,106,89,180});
        text(label,pos+Vec{0,-8},size>50?16:11,Cream,true);
    };
    button(touch.attackButton,r*.85f,"BICAR",touch.attacking());
    button(touch.dodgeButton,r*.56f,"ESQUIVA",false);button(touch.eatButton,r*.56f,"COMER",false);
    button(touch.interactButton,r*.56f,"USAR",false);
}
void Renderer::tree(Vec p,std::uint32_t h,float time) {
    float shift=std::sin(time*.7f+float(h%100))*.7f;
    oval(p+Vec{14,12},{42,20},{27,61,31,80});
    rect(p+Vec{-8,-35},{17,42},{104,68,40});rect(p+Vec{-5,-35},{5,41},{163,103,48});
    oval(p+Vec{shift,-46},{45,33},{28,81,51});oval(p+Vec{-15+shift,-55},{29,30},{42,115,58});
    oval(p+Vec{19+shift,-53},{27,29},{45,124,64});oval(p+Vec{shift,-73},{31,29},{64,147,66});
    oval(p+Vec{-9+shift,-82},{19,17},{88,170,76});oval(p+Vec{-14+shift,-87},{10,8},{111,190,86});
    if(h%3==0) {oval(p+Vec{-20,-63},{4,4},{236,94,67});oval(p+Vec{16,-74},{4,4},{242,108,72});}
    line(p+Vec{-6,3},p+Vec{-17,9},4,{114,82,47});line(p+Vec{6,3},p+Vec{17,7},4,{114,82,47});
}
void Renderer::icon(LootKind kind,Vec p,float time,float scale) {
    float bob=std::sin(time*3+p.x)*2;
    oval(p+Vec{1,8},{13*scale,5*scale},{19,54,33,65});p.y+=bob;
    if(kind==LootKind::Egg || kind==LootKind::GoldenEgg) {
        bool gold=kind==LootKind::GoldenEgg;
        oval(p,{9*scale,13*scale},gold?sf::Color{171,111,31}:sf::Color{182,162,122});
        oval(p+Vec{0,-2*scale},{8*scale,12*scale},gold?Gold:Cream);
        oval(p+Vec{-3*scale,-5*scale},{2*scale,4*scale},sf::Color{255,254,231});
        if(gold) {line(p+Vec{15*scale,-15*scale},p+Vec{15*scale,-7*scale},1.8f,Gold);line(p+Vec{11*scale,-11*scale},p+Vec{19*scale,-11*scale},1.8f,Gold);}
    } else if(kind==LootKind::Food) {
        polygon({p+Vec{-8*scale,-8*scale},p+Vec{9*scale,-5*scale},p+Vec{1*scale,14*scale}},{238,126,44});
        line(p+Vec{0,-6*scale},p+Vec{-7*scale,-17*scale},4*scale,{54,131,66});
        line(p+Vec{1*scale,-6*scale},p+Vec{7*scale,-18*scale},4*scale,{89,171,73});
        line(p+Vec{-4*scale,0},p+Vec{5*scale,2*scale},2,{195,80,31});
    } else {
        rect(p+Vec{-16*scale,-11*scale},{32*scale,23*scale},{94,54,32});
        rect(p+Vec{-16*scale,-15*scale},{32*scale,14*scale},{156,89,38});
        rect(p+Vec{-13*scale,-14*scale},{3*scale,25*scale},Gold);rect(p+Vec{10*scale,-14*scale},{3*scale,25*scale},Gold);
        rect(p+Vec{-4*scale,-5*scale},{8*scale,9*scale},Gold);
    }
}
void Renderer::terrain(const Simulation& s,Vec camera) {
    int x0=int(std::floor((camera.x-760)/64)),y0=int(std::floor((camera.y-450)/64));
    for(int y=y0;y<y0+16;++y) for(int x=x0;x<x0+25;++x) {
        Vec p{x*64.f,y*64.f};auto t=s.world.terrain(x,y);auto h=s.world.hash(x,y);int shade=int(h%7);
        sf::Color ground{std::uint8_t(101+shade),std::uint8_t(159+shade),std::uint8_t(71+shade)};
        if(t==Terrain::Meadow) ground={std::uint8_t(137+shade),std::uint8_t(177+shade),std::uint8_t(78+shade)};
        if(t==Terrain::Dirt) ground={std::uint8_t(203+shade),std::uint8_t(174+shade),std::uint8_t(100+shade)};
        if(t==Terrain::Water) ground={49,137,std::uint8_t(160+shade)};
        rect(p,{64,64},ground);
        if(t==Terrain::Water) {
            auto shore=[&](int dx,int dy,Vec position,Vec size){if(s.world.terrain(x+dx,y+dy)!=Terrain::Water){rect(position,size,{39,99,117});}};
            shore(0,-1,p,{64,6});shore(-1,0,p,{6,64});shore(0,1,p+Vec{0,58},{64,6});shore(1,0,p+Vec{58,0},{6,64});
            float phase=std::sin(s.elapsed*1.5f+x+y)*4;
            line(p+Vec{9+phase,20},p+Vec{27+phase,20},2,{108,186,182,145});
            line(p+Vec{34-phase,45},p+Vec{49-phase,45},2,{99,184,188,155});
        } else if(t==Terrain::Dirt) {
            for(int i=0;i<3;++i) oval(p+Vec{float((h>>(i*4))%52+6),float((h>>(i*6+2))%52+6)},{2,1},{178,150,83,125});
        } else {
            for(int i=0;i<4;++i) {
                float gx=float((h>>(i*4))%52+6),gy=float((h>>(i*5+2))%52+6);
                Vec root=p+Vec{gx,gy};float sway=std::sin(s.elapsed*1.4f+x+i)*1.2f;
                line(root,root+Vec{-2+sway,-6},1.5f,{72,133,60,145});line(root,root+Vec{3+sway,-4},1.5f,{175,198,100,150});
            }
            if(h%9==0) {Vec f=p+Vec{22,35};oval(f,{3,3},{246,220,164});oval(f+Vec{5,3},{3,3},{241,196,152});oval(f+Vec{2,1},{2,2},Gold);}
            if(h%13==0) oval(p+Vec{48,22},{5,3},{104,144,66});
        }
    }
    // Tall tufts: dense patches around the forest floor, hand-drawn blades.
    for(int y=y0;y<y0+16;++y) for(int x=x0;x<x0+25;++x) {
        auto h=s.world.hash(x,y,31);auto t=s.world.terrain(x,y);
        if((t==Terrain::Grass || t==Terrain::Meadow) && h%7==0) {
            Vec p{(x+.5f)*64,(y+.5f)*64};
            oval(p+Vec{0,8},{22,8},{43,102,47,80});
            for(int i=0;i<7;++i) {
                float gx=-19+i*6.f, sway=std::sin(s.elapsed*1.7f+x+i*.3f)*2;
                float height=16+float((h>>(i*3))%13);
                polygon({p+Vec{gx-3,10},p+Vec{gx-4+sway,-height},p+Vec{gx+5,10}},{39,std::uint8_t(122+i*5),66});
                line(p+Vec{gx,8},p+Vec{gx-3+sway,-height+3},1.2f,{126,186,82});
            }
        }
    }
    // Circular paths and the safe homestead clearing.
    oval(World::nest+Vec{0,4},{110,64},{203,173,98,110});
    for(int i=0;i<10;++i) {float a=i*6.28318f/10;oval(World::bossHome+Vec{std::cos(a)*167,std::sin(a)*150},{20,13},{125,140,115});oval(World::bossHome+Vec{std::cos(a)*167,std::sin(a)*150-3},{19,12},{185,186,146});}
}
void Renderer::scenery(const Simulation& s,Vec camera) {
    struct Draw {float y;std::function<void()> call;};std::vector<Draw> objects;
    int x0=int(std::floor((camera.x-790)/64)),y0=int(std::floor((camera.y-470)/64));
    for(int y=y0;y<y0+17;++y) for(int x=x0;x<x0+26;++x) {
        Vec p{(x+.5f)*64,(y+.5f)*64};auto t=s.world.terrain(x,y);auto h=s.world.hash(x,y);
        if(t==Terrain::Tree) objects.push_back({p.y,[&,p,h]{tree(p,h,s.elapsed);}});
        if(t==Terrain::Rock) objects.push_back({p.y,[&,p]{oval(p+Vec{5,8},{26,13},{33,58,32,80});oval(p,{25,20},{101,115,107});oval(p+Vec{-3,-6},{23,15},{160,167,138});oval(p+Vec{-7,-10},{13,8},{186,187,156});line(p+Vec{3,-13},p+Vec{0,4},2,{111,126,111});}});
    }
    for(auto k:s.world.active()) {
        const auto& chunk=s.world.chunks().at(k);
        for(const auto& l:chunk.loot) if(!l.collected && distance(l.pos,camera)<950) {
            const Loot* item=&l;objects.push_back({l.pos.y,[&,item]{icon(item->kind,item->pos,s.elapsed);}});
        }
        for(const auto& e:chunk.enemies) if(e.hp>0 && distance(e.pos,camera)<950) {
            const Enemy* enemy=&e;objects.push_back({e.pos.y,[&,enemy]{
                const auto& e=*enemy;
                if(e.brain==Brain::Windup) {
                    oval(e.pos,{e.boss?104.f:74.f,e.boss?74.f:48.f},{224,65,45,65});
                    line(e.pos,e.pos+e.aim*(e.boss?105.f:76.f),8,{255,124,75,170});
                }
                chicken(e.pos,e.aim,e.boss?sf::Color{115,79,137}:sf::Color{171,104,67},s.elapsed+e.id,e.boss?1.6f:1,e.brain==Brain::Chase,e.boss);
                if(e.hp<e.maxHp || e.brain==Brain::Chase || e.boss) {
                    bar(e.pos+Vec{-29,e.boss?-90.f:-58.f},{58,5},e.hp/e.maxHp,{231,103,75});
                    if(e.boss) text(s.player.readyForBoss()?"GUARDIÃO":"GUARDIÃO • SELADO",e.pos+Vec{0,-112},12,Gold,true);
                }
            }});
        }
    }
    objects.push_back({World::secretHome.y-55,[&]{
        Vec p=World::secretHome;
        for(float side:{-85.f,85.f}) {
            Vec q=p+Vec{side,-55};oval(q+Vec{8,7},{28,14},{35,61,34,90});
            rect(q+Vec{-19,-70},{38,73},{120,133,115});rect(q+Vec{-16,-68},{13,70},{185,188,157});
            rect(q+Vec{-25,-76},{50,12},{161,171,139});rect(q+Vec{-25,-8},{50,12},{148,155,125});
            line(q+Vec{-12,-51},q+Vec{14,-45},3,{93,115,101});oval(q+Vec{13,-16},{12,6},{67,124,68});
        }
        text("POMAR ESQUECIDO",p+Vec{0,-117},14,Gold,true);
        for(int i=0;i<5;++i) oval(p+Vec{-30+i*15.f,-67},{7,4},{164,170,139});
    }});
    objects.push_back({65,[&]{
        Vec p{160,30};oval(p+Vec{12,20},{77,32},{20,51,31,80});
        rect(p+Vec{-61,-64},{122,84},{142,90,48});rect(p+Vec{-56,-61},{111,73},{210,163,85});
        for(int i=0;i<5;++i) line(p+Vec{-53,-48+i*14.f},p+Vec{53,-48+i*14.f},2,{174,124,63});
        rect(p+Vec{-16,-27},{33,48},{69,49,37});rect(p+Vec{-19,17},{39,8},{171,145,98});
        rect(p+Vec{29,-42},{20,23},{71,113,123});line(p+Vec{39,-42},p+Vec{39,-19},2,Cream);
        polygon({p+Vec{-76,-54},p+Vec{0,-113},p+Vec{76,-54}},{111,56,43});
        polygon({p+Vec{-76,-60},p+Vec{0,-119},p+Vec{76,-60}},{211,109,65});
        for(int i=0;i<4;++i) line(p+Vec{-58+i*11.f,-73-i*9.f},p+Vec{58-i*11.f,-73-i*9.f},3,{232,139,79});
        text("LAR, DOCE NINHO",p+Vec{0,-7},8,Cream,true);
        oval(p+Vec{64,19},{15,12},{120,92,54});oval(p+Vec{64,15},{13,10},{184,159,69});
        icon(LootKind::Egg,p+Vec{63,7},s.elapsed,.65f);
    }});
    objects.push_back({215,[&]{chicken({86,215},{.7f,-.2f},{227,205,143},s.elapsed,1.05f);text("Dona Cocó",{86,161},12,Cream,true);}});
    objects.push_back({s.player.pos.y,[&]{
        if(s.player.dodgeTimer>0) {oval(s.player.pos-s.player.dodgeDirection*20,{24,15},{252,240,197,70});}
        playerSprite(s.player,s.elapsed);
        if(s.player.attackVisual>0) {
            float base=std::atan2(s.player.facing.y,s.player.facing.x);
            for(int i=0;i<10;++i) {
                float a=base-1.05f+i*.21f,b=a+.2f;float r=56+(1-s.player.attackVisual/.18f)*26;
                line(s.player.pos+Vec{std::cos(a)*r,std::sin(a)*r},s.player.pos+Vec{std::cos(b)*r,std::sin(b)*r},6,{255,248,210,std::uint8_t(160+i*8)});
            }
        }
    }});
    std::stable_sort(objects.begin(),objects.end(),[](const Draw& a,const Draw& b){return a.y<b.y;});for(auto& obj:objects)obj.call();
    for(const auto& e:s.effects) {
        sf::Color color=e.kind==1?sf::Color{255,128,101}:e.kind==0?Cream:e.kind==3?Mint:Gold;
        color.a=std::uint8_t(255*std::clamp(e.life/e.total,0.f,1.f));
        if(e.text.empty()) oval(e.pos,{18*(1-e.life/e.total)+5,9},color);
        else text(e.text,e.pos+Vec{0,-48},16,color,true);
    }
    if(s.atNest()) text("E  descansar   •   U  melhorias",World::nest+Vec{0,99},13,Cream,true);
}
void Renderer::hud(const Simulation& s,bool upgradeOpen,bool touchMode) {
    const auto& p=s.player;
    panel({32,30},{440,178});
    Player portrait=p;portrait.pos={86,105};portrait.facing={1,0};portrait.moving=false;portrait.attackVisual=portrait.dodgeTimer=portrait.invulnerable=0;playerSprite(portrait,s.elapsed,66);
    text("Pipoca",{136,48},27,Cream);text("GALINHA AVENTUREIRA",{136,86},12,Muted);
    text("NV. "+std::to_string(p.level),{364,54},21,Gold);
    bar({136,115},{294,17},p.hp/p.maxHp(),{232,106,93});
    text(std::to_string(int(p.hp))+" / "+std::to_string(int(p.maxHp())),{282,113},12,Cream,true);
    bar({136,140},{294,8},p.stamina/100,{119,205,167});
    bar({56,176},{374,5},float(p.xp)/p.nextLevelXp(),Gold);
    text("XP "+std::to_string(p.xp)+" / "+std::to_string(p.nextLevelXp()),{56,154},12,Muted);
    panel({493,30},{462,66});
    icon(LootKind::GoldenEgg,{524,64},s.elapsed,.75f);text(std::to_string(p.coins)+" moedas",{550,48},20,Gold);
    icon(LootKind::Food,{758,63},s.elapsed,.7f);text(std::to_string(p.food)+"  [Q]",{786,48},20,Cream);
    // A sampled terrain minimap, with markers clamped to the border.
    panel({1642,30},{246,279});text("CAMPOS DO ALVORECER",{1660,45},13,Cream);
    float unit=7.3f;Vec mapPos{1660,76};
    int tx=int(std::floor(p.pos.x/64)),ty=int(std::floor(p.pos.y/64));
    for(int y=-14;y<=14;++y) for(int x=-14;x<=14;++x) {
        auto t=s.world.terrain(tx+x,ty+y);
        sf::Color c=t==Terrain::Water?sf::Color{70,140,158}:t==Terrain::Tree?sf::Color{45,97,60}:t==Terrain::Dirt?sf::Color{194,166,103}:sf::Color{125,155,82};
        rect(mapPos+Vec{(x+14)*unit,(y+14)*unit},{unit,unit},c);
    }
    Vec middle=mapPos+Vec{14.5f*unit,14.5f*unit};
    auto marker=[&](Vec point,sf::Color c){Vec offset=(point-p.pos)*(unit/64);offset.x=std::clamp(offset.x,-98.f,98.f);offset.y=std::clamp(offset.y,-98.f,98.f);oval(middle+offset,{5,5},Ink);oval(middle+offset,{3.5,3.5},c);};
    marker(World::nest,{123,208,255});marker(World::bossHome,Gold);marker(p.pos,Cream);
    text("● Ninho   ● Guardião",{1660,291},11,Muted);
    float questY=touchMode?232.f:800.f;
    panel({32,questY},{365,166});
    text(p.bossDefeated?"AVENTURA CONCLUÍDA":"O DESAFIO DO GUARDIÃO",{52,questY+17},15,Gold);
    auto objective=[&](const std::string& name,int count,int needed,float y){bool done=count>=needed;text(done?"✓":"○",{52,y},20,done?Mint:Muted);text(name,{83,y+2},18,Cream);text(std::to_string(std::min(count,needed))+" / "+std::to_string(needed),{312,y+2},17,done?Mint:Muted);};
    objective("Ovos",p.eggs,6,questY+52);objective("Ovos de ouro",p.goldenEggs,2,questY+87);objective("Rivais vencidos",p.kills,5,questY+122);
    if(!touchMode) {
    panel({32,994},{1856,55});
    text("WASD  mover     ESPAÇO / CLIQUE  bicar     SHIFT  esquivar     Q  comer     E  interagir     U  melhorias",{56,1011},17,Cream);
    text("F1 ajuda   F5 salvar   F2 toque   ESC pausa",{1450,1012},14,Muted);
    }
    if(s.messageTimer>0) {
        panel({465,904},{1133,60});text(s.message,{490,924},15,Cream);
    } else if(p.readyForBoss() && !p.bossDefeated) {
        panel({465,904},{1040,60});text("O selo foi quebrado. Encontre o Guardião na marca dourada do mapa!",{490,925},19,Gold);
    }
    if(upgradeOpen) {
        rect({0,0},{1920,1080},{10,22,27,150});panel({510,245},{900,566});
        text("OFICINA DO NINHO",{960,285},36,Cream,true);text("Dona Cocó cuida de quem se aventura.",{960,342},18,Muted,true);
        const char* names[]={"VIDA  +25","ATAQUE  +8","VELOCIDADE  +15"};int levels[]={p.healthUp,p.attackUp,p.speedUp};
        for(int i=0;i<3;++i) {
            float y=405+i*105.f;rect({550,y},{820,86},{37,58,49});text("["+std::to_string(i+1)+"]  "+names[i],{575,y+17},23,Cream);
            text("Nível "+std::to_string(levels[i])+" / 5",{575,y+52},14,Muted);
            text(levels[i]==5?"MÁXIMO":std::to_string(30+levels[i]*25)+" moedas",{1160,y+25},19,Gold);
        }
        text("Moedas: "+std::to_string(p.coins)+"     •     U / ESC para voltar",{960,755},18,Gold,true);
    }
}
void Renderer::draw(sf::RenderWindow& window,const Simulation& s,Vec camera,int screen,bool help,bool upgradeOpen,const TouchControls* touch) {
    out=&window;window.clear({13,27,33});window.setView(worldView(window.getSize(),camera));
    terrain(s,camera);scenery(s,camera);
    // Six-minute day cycle; dusk tint is subtle enough to keep the map readable.
    float dusk=std::max(0.f,std::sin(s.elapsed/360.f*6.28318f));
    rect(camera-Vec{760,450},{1520,900},{37,43,89,std::uint8_t(dusk*35)});
    window.setView(uiView(window.getSize()));hud(s,upgradeOpen,touch && touch->visible);
    if(touch && touch->visible) {
        window.setView(window.getDefaultView());touchHud(*touch);
        rect({float(window.getSize().x)-58,12},{46,40},{21,39,38,215});
        text("II",{float(window.getSize().x)-44,20},20,Cream);
        window.setView(uiView(window.getSize()));
    }
    if(screen==0) {
        rect({0,0},{1920,1080},{9,28,31,170});panel({390,150},{1140,780});
        text("UMA AVENTURA PELOS CAMPOS",{960,204},17,Gold,true);
        text("MEU GALINHEIRO",{960,262},67,Cream,true);
        text("O MUNDO É GRANDE. SUA CORAGEM TAMBÉM.",{960,355},18,Muted,true);
        Player hero=s.player;hero.pos={960,630};hero.facing={1,0};hero.moving=false;hero.attackVisual=hero.dodgeTimer=hero.invulnerable=0;playerSprite(hero,s.elapsed,235);
        text("Explore. Colete. Evolua. Desafie o Guardião.",{960,654},24,Cream,true);
        text("ENTER   iniciar / continuar",{960,730},24,Gold,true);
        text("N   nova aventura     •     ESC   sair",{960,782},18,Muted,true);
        text("F1: controles durante a aventura   |   Salvamento automático a cada 45 segundos",{960,862},14,Muted,true);
        if(s.messageTimer>0 && s.message.find("Save não")!=std::string::npos) text(s.message,{960,830},13,{255,155,137},true);
    }
    if(screen==2 || screen==3) {
        rect({0,0},{1920,1080},{9,22,28,165});panel({570,280},{780,490});
        text(screen==3?"UM GALINHEIRO LENDÁRIO":"UMA PAUSA NO CAMINHO",{960,328},31,Cream,true);
        if(screen==3) {text("Você venceu o Galo Guardião!",{960,404},24,Gold,true);text("Continue explorando e fortaleça seu galinheiro.",{960,452},18,Muted,true);}
        else {text("Seu mundo está esperando por você.",{960,410},23,Muted,true);text("F5 salvar    •    F9 carregar",{960,466},20,Gold,true);}
        text("ESC   voltar à aventura",{960,564},24,Gold,true);
        text("F1   guia de campo",{960,619},18,Cream,true);
        text("Fechar a janela salva o progresso.",{960,704},15,Muted,true);
    }
    if(help) {
        rect({0,0},{1920,1080},{8,20,27,190});panel({400,140},{1120,810});
        text("GUIA DE CAMPO",{960,182},40,Cream,true);
        std::vector<std::pair<std::string,std::string>> rows={{"WASD / SETAS","Mover; movimento diagonal normalizado"},{"ESPAÇO / CLIQUE","Bicar na direção do movimento / do cursor"},{"SHIFT","Esquiva: 30 de energia e breve invulnerabilidade"},{"Q","Comer alimento: recupera até 45 de vida"},{"E","Conversar e descansar no ninho; cura e salva"},{"U / 1 / 2 / 3","Comprar melhorias no ninho"},{"F5 / F9","Salvar / carregar progresso"},{"ESC / F1","Pausar / fechar este guia"}};
        for(std::size_t i=0;i<rows.size();++i){float y=268+i*56.f;text(rows[i].first,{450,y},18,Gold);text(rows[i].second,{740,y},18,Cream);}
        text("Pegue itens aproximando-se. Árvores, pedras e água bloqueiam a passagem.",{450,748},18,Muted);
        text("Os círculos vermelhos antecipam golpes: saia da área ou esquive.",{450,786},18,Muted);
        text("Complete a missão; encontre o Guardião na marca dourada. O azul aponta seu ninho.",{450,824},17,Muted);
        text("F1 / ESC   fechar",{960,891},20,Gold,true);
    }
}
}
