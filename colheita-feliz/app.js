(() => {
  'use strict';
  const $ = id => document.getElementById(id);
  const read = (key, fallback) => { try { return JSON.parse(localStorage.getItem(key)) || fallback; } catch { return fallback; } };
  const save = (key, value) => { try { localStorage.setItem(key, JSON.stringify(value)); } catch { /* Private/file browsing may disable persistence. */ } };
  let config = read('colheita.connection.v1', {url:'',id:''});
  let mode = 'offline', data = null, busy = false, events = [], lastRead = null;
  const notify = (text, error=false) => { $('notice').textContent=text; $('notice').classList.toggle('error',error); };
  function event(text) { events.unshift(new Date().toLocaleTimeString('pt-BR',{hour:'2-digit',minute:'2-digit'})+' · '+text); events=events.slice(0,8); $('history').replaceChildren(...events.map(text=>{const li=document.createElement('li');li.textContent=text;return li;})); }
  function paint() {
    $('status').textContent = mode==='demo'?'Demonstração · dados simulados':mode==='live'?'Conectado ao servidor':'Sem conexão';
    $('status').className='badge '+(mode==='demo'?'demo':mode==='live'?'live':'');
    $('mode-description').textContent=mode==='demo'?'Nenhum sensor ou equipamento real é acionado neste modo.':mode==='live'?'Dados recebidos da API configurada.':'Conecte seu servidor ou explore o modo de demonstração.';
    $('mode-label').textContent=mode==='demo'?'SIMULAÇÃO':mode==='live'?'DADOS DA API':'AGUARDANDO DADOS';
    $('plant-name').textContent=data?.name || 'Sua plantação';
    $('last-irrigation').textContent=data?.last || '—';
    $('humidity').textContent=data ? Math.round(data.humidity)+'%' : '—';
    $('gauge').style.setProperty('--value',(data?.humidity || 0)+'%');
    $('automation').setAttribute('aria-checked',String(data?.type==='IA'));
    $('irrigation-type').textContent=data ? (data.type==='IA'?'Automação ativada':'Controle manual') : 'Conecte para acompanhar';
    $('read-time').textContent=lastRead ? 'Atualizado às '+lastRead.toLocaleTimeString('pt-BR') : 'Nenhuma leitura recebida';
    for(const id of ['refresh','automation','irrigate']) $(id).disabled=busy || !data || mode==='offline';
    for(const id of ['demo','configure','connect','confirm-irrigate']) $(id).disabled=busy;
    document.querySelectorAll('[data-close]').forEach(b=>b.disabled=busy);
  }
  function normalize(json) {
    const p=json?.plantingSituation;
    if(!p || typeof p!=='object') throw new Error('O servidor respondeu sem os dados da plantação.');
    const raw=p.PlantingSituation_moisture;
    const humidity=typeof raw==='number'?raw:typeof raw==='string'&&raw.trim()!==''?Number(raw):NaN;
    if(!Number.isFinite(humidity)||humidity<0||humidity>100)throw new Error('O servidor enviou uma umidade inválida.');
    const type=p.typeOfIrrigation || p.PlantingSituation_typeOfIrrigation;
    if(!['IA','Manual'].includes(type))throw new Error('O servidor enviou um tipo de irrigação inválido.');
    return {name:String(p.namePlantation || 'Minha plantação'),humidity,type,last:String(p.PlantingSituation_IrrigationDate || '—')};
  }
  async function request(path, body={}) {
    const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),10000);
    try {
      const response=await fetch(config.url+'/'+path,{method:'POST',headers:{'Content-Type':'application/json','id':config.id},body:JSON.stringify(body),signal:controller.signal});
      if(!response.ok)throw new Error('O servidor respondeu com erro '+response.status+'.');
      const text=await response.text();
      if(!text.trim())return null;
      let json;try{json=JSON.parse(text);}catch{throw new Error('A API não retornou JSON válido.');}
      if(json?.success===false || json?.error)throw new Error('O servidor não confirmou a operação.');
      return json;
    }catch(error){
      if(error.name==='AbortError')throw new Error('O servidor demorou para responder. Verifique a conexão.');
      if(error instanceof TypeError)throw new Error('Não foi possível acessar a API. Verifique o endereço, o servidor e a permissão CORS.');
      throw error;
    }finally{clearTimeout(timer);}
  }
  async function action(fn,errorTarget) {
    if(busy)return;busy=true;paint();notify('');if(errorTarget)$(errorTarget).textContent='';
    try{await fn();}catch(error){notify(error.message,true);if(errorTarget)$(errorTarget).textContent=error.message;}
    finally{busy=false;paint();}
  }
  async function readLive(){data=normalize(await request('platation/get-plantation'));lastRead=new Date();}
  $('demo').onclick=()=>{
    if(busy)return;
    const previous=read('colheita.demo.v1',null);
    data=previous&&Number.isFinite(previous.humidity)&&['IA','Manual'].includes(previous.type)?previous:{name:'Horta de demonstração',humidity:64,type:'Manual',last:'Ainda não simulada'};
    mode='demo';lastRead=new Date();event('Demonstração iniciada.');notify('Você está usando dados simulados.');paint();
  };
  $('configure').onclick=()=>{$('api-url').value=config.url;$('plant-id').value=config.id;$('settings-error').textContent='';$('settings').showModal();};
  document.querySelectorAll('[data-close]').forEach(b=>b.onclick=()=>{if(!busy)$(b.dataset.close).close();});
  for(const id of ['settings','watering'])$(id).addEventListener('cancel',e=>{if(busy)e.preventDefault();});
  $('settings-form').onsubmit=e=>{e.preventDefault();action(async()=>{
    let url;try{url=new URL($('api-url').value.trim());}catch{throw new Error('Informe um endereço válido para a API.');}
    if(!['https:','http:'].includes(url.protocol)||url.username||url.password||url.search||url.hash)throw new Error('Use uma URL HTTP ou HTTPS sem senha, parâmetros ou fragmentos.');
    if(location.protocol==='https:'&&url.protocol!=='https:')throw new Error('Nesta página HTTPS, a API também precisa usar HTTPS.');
    const id=$('plant-id').value.trim();if(!id || /[\r\n]/.test(id))throw new Error('Informe o identificador da plantação.');
    config={url:url.href.replace(/\/$/,''),id};save('colheita.connection.v1',config);
    mode='offline';data=null;lastRead=null;
    await readLive();mode='live';$('settings').close();event('Conexão com o servidor estabelecida.');notify('Dados carregados com sucesso.');
  },'settings-error');};
  $('refresh').onclick=()=>action(async()=>{if(mode==='live')await readLive();else lastRead=new Date();notify(mode==='demo'?'Leitura simulada atualizada.':'Dados atualizados.');});
  $('automation').onclick=()=>action(async()=>{
    const type=data.type==='IA'?'Manual':'IA';
    if(mode==='demo'){data={...data,type};save('colheita.demo.v1',data);}
    else {
      const result=await request('platation/toggle-typeIrrigation',{typeOfIrrigation:type});
      if(result?.plantingSituation){data=normalize(result);lastRead=new Date();}
      else await readLive();
      if(data.type!==type)throw new Error('O servidor ainda não confirmou a mudança de automação. Atualize a leitura antes de tentar novamente.');
    }
    event((mode==='demo'?'Simulação: ':'')+(type==='IA'?'automação ativada.':'controle manual ativado.'));notify('Tipo de irrigação atualizado.');
  });
  $('irrigate').onclick=()=>{$('watering-description').textContent=mode==='demo'?'Esta ação simula a irrigação. Nenhum equipamento será acionado.':'Esta ação envia um comando de irrigação ao servidor configurado.';$('confirm-irrigate').textContent=mode==='demo'?'Simular irrigação':'Enviar comando de irrigação';$('watering-error').textContent='';$('watering').showModal();};
  $('watering-form').onsubmit=e=>{e.preventDefault();action(async()=>{
    const target=Number($('target').value);if(!Number.isInteger(target)||target<1||target>100)throw new Error('Escolha um valor inteiro entre 1 e 100.');
    if(mode==='demo'){
      data={...data,humidity:target,last:new Date().toLocaleString('pt-BR')};save('colheita.demo.v1',data);lastRead=new Date();event('Simulação: irrigação para '+target+'%.');notify('Irrigação simulada com sucesso.');
    }else {
      const result=await request('platation/activate-irrigation',{porcentagem:target/100});
      if(result?.plantingSituation){data=normalize(result);lastRead=new Date();}
      event('Comando de irrigação enviado: '+target+'%.');notify('Comando aceito pela API. Atualize a leitura para acompanhar o sensor.');
    }
    $('watering').close();
  },'watering-error');};
  paint();
})();
