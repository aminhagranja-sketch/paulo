#include "granja/Simulation.hpp"
namespace granja {
void Simulation::notify(std::string text) { message=std::move(text); messageTimer=5; }
void Simulation::effect(Vec p,int kind,std::string text) {
    effects.push_back({p,{0,-32},.8f,.8f,kind,std::move(text)});
}
void Simulation::addXp(int amount) {
    player.xp+=amount;
    while(player.xp>=player.nextLevelXp()) {
        player.xp-=player.nextLevelXp(); ++player.level; player.hp=player.maxHp();
        effect(player.pos,3,"NÍVEL "+std::to_string(player.level));
        notify("Nova evolução! Vida e ataque aumentaram.");
    }
}
void Simulation::attack(Vec aim) {
    if(player.attackCooldown>0 || player.stamina<12 || player.dodgeTimer>0) return;
    player.facing=normalized(aim); player.attackCooldown=.34f; player.attackVisual=.18f; player.stamina-=12;
    for(auto k:world.active()) for(auto& e:world.ensure(k).enemies) {
        if(e.hp<=0 || (e.boss && !player.readyForBoss())) continue;
        Vec delta=e.pos-player.pos;
        if(length(delta)<(e.boss?112.f:95.f) && dot(normalized(delta),player.facing)>.25f) {
            e.hp=std::max(0.f,e.hp-player.damage()); effect(e.pos,0,std::to_string(int(player.damage())));
            if(e.hp>0) {
                e.pos=world.move(e.pos,normalized(delta)*22.f,e.boss?27.f:18.f);
                e.brain=Brain::Recover; e.timer=.24f;
            } else if(!e.rewarded) {
                e.rewarded=true; e.brain=Brain::Dead; ++player.kills;
                player.coins+=e.boss?150:12*e.tier; addXp(e.boss?250:25*e.tier);
                if(e.boss) { player.bossDefeated=true; victoryEvent=true; notify("O Guardião caiu! Seu galinheiro agora é lendário."); }
                else effect(e.pos,2,"+"+std::to_string(12*e.tier)+" moedas");
            }
        }
    }
}
void Simulation::collect() {
    for(auto k:world.active()) for(auto& item:world.ensure(k).loot) {
        if(item.collected || distance(item.pos,player.pos)>38) continue;
        item.collected=true;
        switch(item.kind) {
        case LootKind::Egg: ++player.eggs; player.coins+=3; addXp(4); effect(item.pos,2,"+1 ovo"); break;
        case LootKind::GoldenEgg: ++player.goldenEggs; player.coins+=25; addXp(15); effect(item.pos,2,"Ovo de ouro!"); break;
        case LootKind::Food: ++player.food; effect(item.pos,2,"+1 alimento"); break;
        case LootKind::Chest: if(item.id==3000) notify("Você encontrou o pomar secreto! Seus tesouros são seus."); player.coins+=40; player.food+=2; addXp(15); effect(item.pos,2,"Baú: +40 moedas"); break;
        }
    }
}
bool Simulation::upgrade(int attribute) {
    if(!atNest()) { notify("Volte ao ninho para comprar melhorias."); return false; }
    if(attribute<0 || attribute>2) return false;
    int* target=attribute==0?&player.healthUp:attribute==1?&player.attackUp:&player.speedUp;
    int cost=30+*target*25;
    if(*target>=5) { notify("Essa melhoria já está no máximo."); return false; }
    if(player.coins<cost) { notify("Moedas insuficientes. Explore e vença rivais."); return false; }
    player.coins-=cost; ++*target; player.hp=player.maxHp(); notify("Melhoria comprada!"); saveRequested=true; return true;
}
void Simulation::hurt(float damage,Vec source) {
    if(player.invulnerable>0) return;
    player.hp=std::max(0.f,player.hp-damage); player.invulnerable=.7f; effect(player.pos,1,"-"+std::to_string(int(damage)));
    player.pos=world.move(player.pos,normalized(player.pos-source)*25,PlayerRadius);
    if(player.hp<=0) {
        ++player.deaths; player.coins=player.coins*9/10; player.pos=World::nest;
        player.hp=player.maxHp(); player.stamina=100; player.invulnerable=3; player.dodgeTimer=0;
        notify("Você voltou ao ninho e perdeu 10% das moedas. Tente novamente!"); saveRequested=true;
    }
}
void Simulation::updateEnemies(float dt) {
    for(auto k:world.active()) for(auto& e:world.ensure(k).enemies) {
        if(e.hp<=0 || distance(e.pos,player.pos)>900) continue;
        if(e.boss && !player.readyForBoss()) { e.brain=Brain::Wander; continue; }
        if(atNest() && !e.boss) { e.brain=Brain::Wander; }
        e.timer-=dt; e.cooldown=std::max(0.f,e.cooldown-dt);
        float d=distance(e.pos,player.pos),reach=e.boss?90.f:62.f;
        if(e.brain==Brain::Windup) {
            if(e.timer<=0) {
                if(d<reach+28 && dot(normalized(player.pos-e.pos),e.aim)>-.15f) hurt(e.boss?32.f:10.f+e.tier*4,e.pos);
                effect(e.pos+e.aim*reach,4); e.brain=Brain::Recover; e.timer=e.boss?.75f:.65f; e.cooldown=1;
            }
            continue;
        }
        if(e.brain==Brain::Recover) { if(e.timer<=0) e.brain=Brain::Chase; else continue; }
        float aggro=e.boss?480.f:320.f;
        if(d<aggro && !atNest() && distance(e.pos,e.home)<650) {
            e.brain=Brain::Chase; e.aim=normalized(player.pos-e.pos);
            if(d<reach && e.cooldown<=0) { e.brain=Brain::Windup; e.timer=e.boss?.55f:.4f; }
            else e.pos=world.move(e.pos,e.aim*(dt*(e.boss?155.f:95.f+e.tier*12)),e.boss?27.f:18.f);
        } else {
            e.brain=Brain::Wander;
            Vec target=e.home+Vec{std::cos(elapsed*.45f+e.id)*55,std::sin(elapsed*.31f+e.id)*55};
            e.aim=normalized(target-e.pos);
            if(distance(e.pos,target)>5) e.pos=world.move(e.pos,e.aim*(dt*40),18);
        }
    }
}
void Simulation::update(float dt,const Input& input) {
    dt=std::clamp(dt,0.f,.05f); elapsed+=dt; messageTimer=std::max(0.f,messageTimer-dt);
    player.invulnerable=std::max(0.f,player.invulnerable-dt);
    player.attackCooldown=std::max(0.f,player.attackCooldown-dt);
    player.attackVisual=std::max(0.f,player.attackVisual-dt);
    player.stamina=std::min(100.f,player.stamina+dt*25);
    Vec direction=length(input.move)>.01f?input.move*(1.f/std::max(1.f,length(input.move))):Vec{};
    if(length(input.aim)>.01f) player.facing=normalized(input.aim);
    else if(length(direction)>.01f) player.facing=normalized(direction);
    if(input.dodge && player.stamina>=30 && player.dodgeTimer<=0) {
        player.stamina-=30; player.dodgeTimer=.2f; player.invulnerable=.25f;
        player.dodgeDirection=length(direction)>.01f?normalized(direction):player.facing;
    }
    if(player.dodgeTimer>0) {
        player.pos=world.move(player.pos,player.dodgeDirection*(dt*620),PlayerRadius); player.dodgeTimer-=dt;
    } else player.pos=world.move(player.pos,direction*(player.speed()*dt),PlayerRadius);
    player.moving=length(direction)>.01f || player.dodgeTimer>0;
    if(player.moving) player.walkTime+=dt;
    else player.walkTime=0;
    world.stream(player.pos);
    if(input.attack) attack(player.facing);
    if(input.eat && player.food>0 && player.hp<player.maxHp()) { --player.food; player.hp=std::min(player.maxHp(),player.hp+45); effect(player.pos,3,"+45 vida"); }
    collect(); updateEnemies(dt);
    if(input.interact) {
        if(atNest()) { player.hp=player.maxHp(); player.stamina=100; saveRequested=true;
            notify("Dona Cocó: descanse! 1: vida  2: ataque  3: velocidade. Explore, depois vença o Guardião.");
        } else if(distance(player.pos,World::bossHome)<250 && !player.readyForBoss()) notify("O Guardião exige 6 ovos, 2 ovos de ouro e 5 rivais vencidos.");
        else notify("Colete aproximando-se dos itens. O ninho fica na marca azul do mapa.");
    }
    for(auto& e:effects) { e.life-=dt; e.pos+=e.velocity*dt; }
    std::erase_if(effects,[](const Effect& e){return e.life<=0;});
}
}
