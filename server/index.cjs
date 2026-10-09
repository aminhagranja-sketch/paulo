'use strict';
const http=require('node:http'),fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const {WebSocketServer,WebSocket}=require('ws');
const createGame=require('../web/granja.js');
const root=path.resolve(__dirname,'../web');
const dataDir=path.resolve(process.env.GRANJA_DATA_DIR||path.join(__dirname,'../server-data'));
const port=Number(process.env.PORT||9090),capacity=Number(process.env.MAX_PLAYERS||20);
if(!Number.isInteger(capacity)||capacity<1||capacity>100)throw new Error('MAX_PLAYERS deve estar entre 1 e 100');
fs.mkdirSync(dataDir,{recursive:true,mode:0o700});
const config=JSON.parse(fs.readFileSync(path.resolve(process.env.GRANJA_COMBAT_CONFIG||path.join(__dirname,'../config/combat.json')),'utf8'));
const sessions=new Map(),pending=new Set();
const allowedOrigins=new Set((process.env.ALLOWED_ORIGINS||`http://127.0.0.1:${port},http://localhost:${port},https://aminhagranja-sketch.github.io`).split(','));
const mime={'.html':'text/html; charset=utf-8','.js':'text/javascript; charset=utf-8','.json':'application/json','.wasm':'application/wasm','.png':'image/png','.css':'text/css'};
const server=http.createServer((req,res)=>{
 if(req.url==='/health'){res.writeHead(200,{'Content-Type':'application/json'});res.end(JSON.stringify({ok:true,players:sessions.size,capacity}));return;}
 let decoded;try{decoded=decodeURIComponent(new URL(req.url,'http://server').pathname);}catch{res.writeHead(400);res.end();return;}
 const file=path.resolve(root,'.'+(decoded==='/'?'/index.html':decoded));
 if(!file.startsWith(root+path.sep)||!['GET','HEAD'].includes(req.method)){res.writeHead(404);res.end();return;}
 fs.stat(file,(error,stat)=>{if(error||!stat.isFile()){res.writeHead(404);res.end();return;}res.writeHead(200,{'Content-Type':mime[path.extname(file)]||'application/octet-stream','Cache-Control':'no-cache','X-Content-Type-Options':'nosniff'});if(req.method==='HEAD')res.end();else if(path.basename(file)==='index.html')res.end(fs.readFileSync(file,'utf8').replace('<head>','<head><meta name="granja-server" content="/ws">'));else fs.createReadStream(file).on('error',()=>res.destroy()).pipe(res);});
});
const wss=new WebSocketServer({noServer:true,maxPayload:2048});
server.on('upgrade',(req,socket,head)=>{
 if(req.url!=='/ws'||req.headers.origin&&!allowedOrigins.has(req.headers.origin)){socket.end('HTTP/1.1 403 Forbidden\r\n\r\n');return;}
 wss.handleUpgrade(req,socket,head,ws=>wss.emit('connection',ws));
});
function send(ws,data){if(ws.readyState!==WebSocket.OPEN)return;if(ws.bufferedAmount>1024*1024){ws.close(4008,'Cliente muito lento');return;}ws.send(JSON.stringify(data));}
function snapshot(entry){return JSON.parse(entry.module.UTF8ToString(entry.module._game_snapshot(entry.width,entry.height)));}
function persist(entry){
 const save=JSON.parse(entry.module.UTF8ToString(entry.module._game_save()));
 const file=path.join(dataDir,entry.key+'.json'),temporary=file+'.tmp';
 const fd=fs.openSync(temporary,'w',0o600);try{fs.writeFileSync(fd,JSON.stringify({version:1,id:entry.id,name:entry.name,save}));fs.fsyncSync(fd);}finally{fs.closeSync(fd);}
 fs.renameSync(temporary,file);entry.savedAt=Date.now();
}
function finite(value,min,max){return typeof value==='number'&&Number.isFinite(value)&&value>=min&&value<=max;}
function neutral(){return{mx:0,my:0,ax:0,ay:0,flags:0};}
wss.on('connection',ws=>{
 let entry=null,joining=false,count=0,windowAt=Date.now();
 const helloTimeout=setTimeout(()=>{if(!entry)ws.close(4000,'Identificação necessária');},10000);
 ws.on('error',()=>{});
 ws.on('message',async raw=>{
  if(Date.now()-windowAt>1000){windowAt=Date.now();count=0;}if(++count>60){ws.close(4008,'Muitas mensagens');return;}
  let packet;try{packet=JSON.parse(raw);}catch{send(ws,{type:'error',message:'Mensagem inválida'});return;}
  try{
   if(!entry){
    if(packet.type!=='hello'||joining)return;
    joining=true;
    const token=packet.token||crypto.randomBytes(32).toString('hex');
    if(typeof token!=='string'||!/^[a-f0-9]{64}$/.test(token)){ws.close(4000,'Sessão inválida');return;}
    const key=crypto.createHash('sha256').update(token).digest('hex');
    if(pending.has(key)){ws.close(4009,'Reconexão em andamento');return;}
    const existing=sessions.get(key);
    if(existing){existing.ws.close(4001,'Sessão retomada em outra janela');entry=existing;}
    else {
     if(sessions.size+pending.size>=capacity){ws.close(4003,'Mundo cheio');return;}
     pending.add(key);
     try{
      const module=await createGame({locateFile:filename=>path.join(root,filename)});
      if(ws.readyState!==WebSocket.OPEN)return;
      const file=path.join(dataDir,key+'.json');let profile;
      if(fs.existsSync(file)){profile=JSON.parse(fs.readFileSync(file,'utf8'));if(!module.ccall('game_load','number',['string'],[JSON.stringify(profile.save)]))throw new Error('Progresso inválido');}
      else if(packet.token)throw new Error('Sessão não encontrada');
      else module._game_seed_rng(crypto.randomBytes(4).readUInt32LE());
      if(!module.ccall('game_configure','number',['string'],[JSON.stringify(config)]))throw new Error('Configuração de drops inválida');
      entry={key,ws,id:profile?.id||crypto.randomBytes(8).toString('hex'),name:profile?.name||String(packet.name||'Galinha').slice(0,24),module,width:1000,height:700,savedAt:0};
      sessions.set(key,entry);persist(entry);
     }finally{pending.delete(key);}
    }
    clearTimeout(helloTimeout);entry.ws=ws;entry.input=neutral();entry.paused=true;entry.lastInput=Date.now();entry.tilesKey='';
    send(ws,{type:'welcome',id:entry.id,token,capacity});send(ws,{type:'state',state:{...snapshot(entry),players:[]}});return;
   }
   if(entry.ws!==ws)return;
   switch(packet.type){
    case 'input': {
     const i=packet.input;if(!i||!finite(i.mx,-1,1)||!finite(i.my,-1,1)||!finite(i.ax,-10000,10000)||!finite(i.ay,-10000,10000)||!Number.isInteger(i.flags)||i.flags<0||i.flags>15){send(ws,{type:'error',message:'Entrada rejeitada'});break;}
     entry.input={...i,flags:(entry.input.flags&14)|i.flags};entry.lastInput=Date.now();break;
    }
    case 'pause':entry.paused=packet.paused!==false;entry.input=neutral();persist(entry);break;
    case 'viewport':if(finite(packet.width,300,2200)&&finite(packet.height,300,1500)){entry.width=packet.width;entry.height=packet.height;entry.tilesKey='';}break;
    case 'upgrade':if(Number.isInteger(packet.attribute)&&packet.attribute>=0&&packet.attribute<=2){entry.module._game_upgrade(packet.attribute);persist(entry);}break;
    case 'interact':entry.module._game_tick(0,0,0,8,0,0);persist(entry);break;
    case 'eat':entry.module._game_tick(0,0,0,4,0,0);persist(entry);break;
    case 'save':persist(entry);break;
    default:send(ws,{type:'error',message:'Comando não permitido'}); // No position, level, damage, drop or inventory writes.
   }
  }catch(error){console.error('Sessão:',error.message);ws.close(1011,'Não foi possível salvar ou carregar o progresso');}
 });
 ws.on('close',()=>{clearTimeout(helloTimeout);if(entry&&entry.ws===ws){try{persist(entry);}catch(error){console.error('Save:',error.message);}sessions.delete(entry.key);}});
});
let ticks=0;
const timer=setInterval(()=>{
 for(const entry of sessions.values())try{
  if(!entry.paused){if(Date.now()-entry.lastInput>500)entry.input=neutral();const i=entry.input;entry.module._game_tick(.05,i.mx,i.my,i.flags,i.ax,i.ay);entry.input.flags&=1;}
  if(entry.module._game_needs_save()||Date.now()-entry.savedAt>1000)persist(entry);
 }catch(error){console.error('Tick:',error.message);entry.ws.close(1011,'Falha de persistência');}
 if(++ticks%2)return;
 const states=new Map();for(const entry of sessions.values())states.set(entry.key,snapshot(entry));
 for(const entry of sessions.values()){
  const state=states.get(entry.key);state.players=[];
  for(const other of sessions.values())if(other!==entry){const p=states.get(other.key).player;if(Math.hypot(p.x-state.player.x,p.y-state.player.y)<1500)state.players.push({id:other.id,name:other.name,x:p.x,y:p.y,fx:p.fx,fy:p.fy,moving:p.moving,walkTime:p.walkTime,level:p.level,attack:p.attack,dodge:p.dodge});}
  const tilesKey=JSON.stringify([state.tiles[0]?.slice(0,2),state.tiles.length,entry.width,entry.height]);if(tilesKey===entry.tilesKey)delete state.tiles;else entry.tilesKey=tilesKey;
  send(entry.ws,{type:'state',state});
 }
},50);
server.listen(port,process.env.HOST||'0.0.0.0',()=>console.log(`Meu Galinheiro: porta ${port}, capacidade ${capacity}, persistência habilitada`));
function shutdown(){clearInterval(timer);for(const entry of sessions.values()){try{persist(entry);}catch(error){console.error(error.message);}entry.ws.close(1001,'Servidor reiniciando');}wss.close();server.close(()=>process.exit(0));setTimeout(()=>process.exit(0),3000).unref();}
process.on('SIGTERM',shutdown);process.on('SIGINT',shutdown);
