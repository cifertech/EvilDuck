#pragma once
#include <pgmspace.h>

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>EvilDuck S3</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <span>🦆</span>
      <span>EvilDuck S3</span>
    </div>
    <nav class="nav">
      <a class="navlink active" href="/index.html">Dashboard</a>
      <a class="navlink" href="/settings.html">Settings</a>
      <a class="navlink" href="/terminal.html">Terminal</a>
    </nav>
  </header>

  <main class="grid">
    <!-- Thermal Warning Banner -->
    <div id="thermalBanner" style="display:none; grid-column:1/-1; padding:10px 14px; border-radius:4px; background:#330000; border:1px solid #cc2222; color:#ff6666; font-weight:600;">
      ⚠️ Thermal Throttle Active: Core temp is high (>70°C). Reduced LED & RF power to protect hardware.
    </div>

    <!-- Device Status -->
    <section class="card span2">
      <div class="cardhead">
        <h2>Device Status</h2>
        <div class="btn-group">
          <span id="chipRunning" class="chip">Checking…</span>
          <span id="chipTemp" class="chip">Temp: —</span>
          <span id="chipHID" class="chip">HID</span>
          <span id="chipSD" class="chip">SD</span>
        </div>
      </div>
      <div class="kv">
        <div class="k">SSID / Channel</div><div class="v mono"><span id="ssidVal">—</span> (Ch <span id="chVal">—</span>)</div>
        <div class="k">Core Temp</div><div class="v mono" id="tempVal">—</div>
        <div class="k">Autorun Payload</div><div class="v mono" id="autorunVal">—</div>
        <div class="k">Execution</div>
        <div class="v mono">
          <span id="runLineVal">—</span>
          <button id="btnStop" class="btn small danger" style="display:none; margin-left:8px;" onclick="stopScript()">Stop</button>
        </div>
        <div class="k">Web Password</div><div class="v"><input class="input mono" id="duckPw" type="password" placeholder="(if set in Settings)" onchange="saveDuckPw()" /></div>
      </div>
      <div style="margin-top:8px;">
        <button class="btn small" onclick="refreshStatus()">Refresh Status</button>
      </div>
    </section>

    <!-- Quick Keystroke Line -->
    <section class="card">
      <div class="cardhead"><h2>Quick Command</h2></div>
      <div class="stack">
        <input class="input mono" id="line" placeholder="e.g. GUI r or STRING Hello" onkeydown="if(event.key==='Enter')sendLine()" />
        <button class="btn primary" onclick="sendLine()">Send Keystroke</button>
      </div>
    </section>

    <!-- Autorun Payload -->
    <section class="card">
      <div class="cardhead"><h2>Autorun Payload</h2></div>
      <div class="stack">
        <input class="input mono" id="autorun" placeholder="payload.txt (or empty to disable)" />
        <button class="btn primary" onclick="saveAutorun()">Save Autorun</button>
      </div>
    </section>

    <!-- Payload Scripts -->
    <section class="card span2">
      <div class="cardhead">
        <h2>Payload Scripts</h2>
        <div class="btn-group">
          <input class="input" id="filter" placeholder="Filter..." oninput="renderList()" style="width:120px; padding:4px 8px; font-size:12px;" />
          <button class="btn small" onclick="loadList()">Reload</button>
        </div>
      </div>
      <form id="uploadForm" class="upload-row">
        <input class="input" type="file" id="file" name="file" accept=".txt,.duck" required style="flex:1; min-width:140px; padding:4px 6px;" />
        <select class="input" id="uploadStorage" name="storage" style="width:110px; padding:4px 6px;">
          <option value="spiffs">SPIFFS</option>
          <option value="sd">SD Card</option>
        </select>
        <button class="btn primary small" type="submit">Upload</button>
      </form>
      <ul id="scripts" class="list"></ul>
    </section>

    <!-- Script Editor -->
    <section class="card span2">
      <div class="cardhead">
        <h2>Script Editor</h2>
        <div class="btn-group">
          <button class="btn small ghost" onclick="newScript()">New</button>
          <button class="btn small" onclick="loadScript()">Reload</button>
          <button class="btn small primary" onclick="saveScript()">Save</button>
          <button class="btn small" onclick="runEditorScript()">Run</button>
        </div>
      </div>
      <div class="stack">
        <div style="display:flex; gap:8px; flex-wrap:wrap;">
          <input class="input mono" id="editorName" placeholder="payload.txt" autocomplete="off" style="flex:1; min-width:140px;" />
          <select class="input" id="editorStorage" style="width:140px;">
            <option value="spiffs">SPIFFS (Internal)</option>
            <option value="sd">SD Card</option>
          </select>
        </div>
        <textarea class="editor mono" id="editorBody" rows="12" placeholder="Click 'Edit' on any script above or type DuckScript code here..."></textarea>
      </div>
    </section>

    <!-- Activity Log -->
    <section class="card span2">
      <div class="cardhead">
        <h2>Activity Log</h2>
        <button class="btn small ghost" onclick="clearLog()">Clear</button>
      </div>
      <pre id="log" class="log" aria-live="polite"></pre>
    </section>
  </main>

  <footer class="footer">
    <span>EvilDuck S3</span>
  </footer>
  <script src="/index.js"></script>
</body>
</html>
)rawliteral";

static const char SETTINGS_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>EvilDuck Settings</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <span>🦆</span>
      <span>Settings</span>
    </div>
    <nav class="nav">
      <a class="navlink" href="/index.html">Dashboard</a>
      <a class="navlink" href="/terminal.html">Terminal</a>
    </nav>
  </header>

  <main class="grid">
    <section class="card span2">
      <div class="cardhead"><h2>Wi-Fi AP</h2></div>
      <div class="stack">
        <label class="field">
          <span class="label">SSID</span>
          <input class="input" id="ssid" maxlength="32" autocomplete="off" />
        </label>
        <label class="field">
          <span class="label">Password</span>
          <input class="input" id="password" minlength="8" maxlength="63" type="password" autocomplete="off" />
          <span class="hint">8–63 characters.</span>
        </label>
        <label class="field">
          <span class="label">Channel</span>
          <input class="input mono" id="channel" type="number" min="1" max="13" />
        </label>
        <label class="field">
          <span class="label">Wi-Fi Power / Thermal</span>
          <select class="input" id="wifi_power">
            <option value="low">Low (15 dBm) - Coolest</option>
            <option value="med">Medium (18.5 dBm) - Balanced</option>
            <option value="high">High (19.5 dBm) - Maximum Range</option>
          </select>
        </label>
        <label class="field">
          <span class="label"><input type="checkbox" id="wifi_enabled" checked /> Wi-Fi AP Enabled</span>
          <span class="hint">Uncheck to disable Wi-Fi radio on boot.</span>
        </label>
        <div style="margin-top:6px; display:flex; gap:8px;">
          <button class="btn primary small" onclick="saveSettings()">Save Settings</button>
          <button class="btn ghost small" onclick="loadSettings()">Reload</button>
        </div>
      </div>
    </section>

    <section class="card span2">
      <div class="cardhead"><h2>Stealth & Hardware</h2></div>
      <div class="stack">
        <label class="field"><span class="label">Web UI Password (empty = off)</span>
          <input class="input" id="web_password" type="password" autocomplete="off" />
        </label>
        <label class="field"><span class="label"><input type="checkbox" id="stealth" /> Stealth Mode</span></label>
        <label class="field"><span class="label"><input type="checkbox" id="hidden_ap" /> Hidden AP (reboot required)</span></label>
        <label class="field"><span class="label"><input type="checkbox" id="disable_led" /> Disable Status LED</span></label>
        <label class="field">
          <span class="label">RGB Brightness (0–255)</span>
          <input class="input" id="led_brightness" type="range" min="0" max="255" value="20" oninput="document.getElementById('brightVal').textContent=this.value; scheduleBrightnessApply();" />
          <span class="hint">Current: <span id="brightVal">20</span>. Lower brightness reduces chip heat.</span>
        </label>
        <label class="field"><span class="label">Static LED Color</span>
          <input class="input" id="led_color" type="color" value="#000028" style="height:36px;" />
        </label>
        <label class="field"><span class="label">Autorun Delay Min (ms)</span>
          <input class="input mono" id="autorun_delay_min" type="number" min="0" />
        </label>
        <label class="field"><span class="label">Autorun Delay Max (ms)</span>
          <input class="input mono" id="autorun_delay_max" type="number" min="0" />
        </label>
      </div>
    </section>

    <section class="card span2">
      <div class="cardhead"><h2>Activity Log</h2><button class="btn ghost small" onclick="clearLog()">Clear</button></div>
      <pre id="log" class="log"></pre>
    </section>
  </main>

  <footer class="footer">
    <a href="/index.html">Back to Dashboard</a>
  </footer>
  <script src="/settings.js"></script>
</body>
</html>
)rawliteral";

static const char TERMINAL_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>EvilDuck Terminal</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <span>🦆</span>
      <span>Terminal</span>
    </div>
    <nav class="nav">
      <a class="navlink" href="/index.html">Dashboard</a>
      <a class="navlink" href="/settings.html">Settings</a>
    </nav>
  </header>

  <main class="grid">
    <section class="card span2">
      <div class="cardhead">
        <h2>Output</h2>
        <button class="btn ghost small" onclick="clearOut()">Clear</button>
      </div>
      <div id="out" class="term"></div>
    </section>
    <section class="card span2">
      <div class="cardhead"><h2>Command</h2></div>
      <div style="display:flex; gap:8px;">
        <input class="input mono" id="cmd" placeholder="help" onkeydown="if(event.key==='Enter')sendCmd()" style="flex:1;" />
        <button class="btn primary small" onclick="sendCmd()">Send</button>
      </div>
    </section>
  </main>

  <script src="/terminal.js"></script>
</body>
</html>
)rawliteral";

static const char ERROR404_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>404 Not Found</title>
  <link rel="stylesheet" href="/style.css"/>
</head>
<body>
  <main class="grid" style="text-align:center; padding-top:40px;">
    <section class="card span2">
      <h2>404 Not Found</h2>
      <p style="margin:16px 0;"><a class="btn primary" href="/index.html">Back to Dashboard</a></p>
    </section>
  </main>
</body>
</html>
)rawliteral";

static const char STYLE_CSS[] PROGMEM = R"rawliteral(
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
  background: #141414;
  color: #e0e0e0;
  font-size: 14px;
  line-height: 1.4;
}
a { color: #4da3ff; text-decoration: none; }
a:hover { text-decoration: underline; }
.mono { font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace; }

/* Topbar */
.topbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px 16px;
  background: #1c1c1c;
  border-bottom: 1px solid #333;
  flex-wrap: wrap;
  gap: 8px;
}
.brand { display: flex; align-items: center; gap: 8px; font-weight: 700; font-size: 15px; color: #fff; }
.nav { display: flex; gap: 6px; }
.navlink { color: #888; padding: 4px 10px; border-radius: 3px; font-size: 13px; border: 1px solid transparent; }
.navlink:hover, .navlink.active { color: #fff; background: #282828; border-color: #444; }

/* Layout Grid */
.grid {
  max-width: 960px;
  margin: 0 auto;
  padding: 12px;
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: 12px;
}
.span2 { grid-column: span 2; }
@media (max-width: 650px) {
  .grid { grid-template-columns: 1fr; }
  .span2 { grid-column: span 1; }
}

/* Cards */
.card {
  background: #1c1c1c;
  border: 1px solid #333;
  border-radius: 4px;
  padding: 12px;
  min-width: 0;
}
.cardhead {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
  flex-wrap: wrap;
  gap: 6px;
}
.cardhead h2 { font-size: 14px; font-weight: 600; color: #fff; }

/* Buttons & Inputs */
.btn {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 6px 12px;
  background: #2a2a2a;
  color: #fff;
  border: 1px solid #444;
  border-radius: 3px;
  cursor: pointer;
  font-size: 13px;
  user-select: none;
}
.btn:hover { background: #383838; }
.btn.primary { background: #0066cc; border-color: #0066cc; font-weight: 600; }
.btn.primary:hover { background: #0052a3; }
.btn.danger { background: #cc2222; border-color: #cc2222; color: #fff; }
.btn.danger:hover { background: #aa1818; }
.btn.small { padding: 3px 8px; font-size: 12px; }
.btn.ghost { background: transparent; border-color: #444; }

.input {
  width: 100%;
  padding: 6px 10px;
  background: #111;
  color: #fff;
  border: 1px solid #444;
  border-radius: 3px;
  font-size: 13px;
}
.input:focus { border-color: #0066cc; outline: none; }

.btn-group {
  display: flex;
  gap: 5px;
  align-items: center;
  flex-shrink: 0;
}
.stack { display: flex; flex-direction: column; gap: 8px; }
.field { display: flex; flex-direction: column; gap: 4px; }
.label { font-size: 12px; color: #888; }
.hint { font-size: 11px; color: #777; }

/* Status Chips */
.chip {
  display: inline-block;
  padding: 2px 6px;
  border-radius: 3px;
  font-size: 11px;
  border: 1px solid #444;
  color: #888;
  background: #222;
}
.chip.good { border-color: #28a745; color: #28a745; }
.chip.warn { border-color: #ff9900; color: #ff9900; }
.chip.bad { border-color: #dc3545; color: #dc3545; }

/* KV Table */
.kv {
  display: grid;
  grid-template-columns: 130px 1fr;
  gap: 6px 10px;
  padding: 8px 0;
  border-top: 1px solid #2a2a2a;
  border-bottom: 1px solid #2a2a2a;
  margin: 6px 0;
  align-items: center;
}
@media (max-width: 500px) {
  .kv { grid-template-columns: 100px 1fr; }
}
.k { color: #888; font-size: 12px; }
.v { color: #eee; min-width: 0; overflow: hidden; text-overflow: ellipsis; }

/* Script List Item - Guaranteed No Button Overflow */
.upload-row {
  display: flex;
  gap: 6px;
  align-items: center;
  flex-wrap: wrap;
  margin-bottom: 10px;
  padding-bottom: 10px;
  border-bottom: 1px solid #2a2a2a;
}
.list { list-style: none; display: flex; flex-direction: column; gap: 6px; }
.li {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 6px 10px;
  border: 1px solid #2e2e2e;
  background: #181818;
  border-radius: 3px;
  gap: 8px;
  min-width: 0;
}
.li-name {
  flex: 1 1 auto;
  min-width: 0;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  cursor: pointer;
  color: #ddd;
  font-size: 13px;
}
.li-name:hover { color: #4da3ff; }

@media (max-width: 520px) {
  .li { flex-wrap: wrap; }
  .li-name { width: 100%; margin-bottom: 2px; }
  .li .btn-group { width: 100%; justify-content: flex-end; }
}

/* Editor & Logs */
.editor {
  width: 100%;
  min-height: 200px;
  padding: 8px 10px;
  background: #111;
  color: #fff;
  border: 1px solid #444;
  border-radius: 3px;
  font-size: 12px;
  line-height: 1.4;
  resize: vertical;
}
.log, .term {
  width: 100%;
  background: #111;
  color: #ccc;
  border: 1px solid #444;
  border-radius: 3px;
  padding: 8px 10px;
  font-size: 12px;
  overflow: auto;
  white-space: pre-wrap;
}
.log { min-height: 80px; max-height: 180px; }
.term { height: 60vh; }

.footer {
  max-width: 960px;
  margin: 0 auto;
  padding: 10px 12px 20px;
  color: #666;
  font-size: 12px;
}
)rawliteral";

static const char INDEX_JS[] PROGMEM = R"rawliteral(
const log=(m)=>{const e=document.getElementById('log');if(e){e.textContent+=m+"\n";e.scrollTop=e.scrollHeight;}};
const clearLog=()=>{const e=document.getElementById('log');if(e)e.textContent='';};
let scriptsCache=[];
let autorunUserEditing=false;

function setChip(el, text, cls){
  if(!el) return;
  el.className='chip '+(cls||'');
  el.textContent=text;
}

function saveDuckPw(){
  sessionStorage.setItem('duckPw', document.getElementById('duckPw')?.value||'');
}
function duckHeaders(){
  const pw=sessionStorage.getItem('duckPw')||'';
  return pw ? {'X-Duck-Password':pw} : {};
}

async function refreshStatus(){
  try{
    const r=await fetch('/status',{headers:duckHeaders()});
    if(!r.ok) throw new Error('HTTP '+r.status);
    const s=await r.json();
    const ssidEl=document.getElementById('ssidVal'); if(ssidEl) ssidEl.textContent=s.ssid||'—';
    const chEl=document.getElementById('chVal'); if(chEl) chEl.textContent=(s.channel??'—');
    const arVal=document.getElementById('autorunVal'); if(arVal) arVal.textContent=s.autorunEnabled ? (s.autorunScript||'') : 'disabled';

    // FIX: Only update autorun input if user is NOT currently typing or focused in it
    const arInput=document.getElementById('autorun');
    if(arInput && !autorunUserEditing && document.activeElement !== arInput){
      arInput.value = s.autorunScript || '';
    }

    setChip(document.getElementById('chipRunning'), s.running?'Running':'Idle', s.running?'good':'');
    const chipSd=document.getElementById('chipSD');
    if(chipSd) setChip(chipSd, s.sdReady?'SD OK':'No SD', s.sdReady?'good':'');
    const chipHid=document.getElementById('chipHID');
    if(chipHid) setChip(chipHid, s.hid?'HID on':'HID off', s.hid?'good':'');

    const runEl=document.getElementById('runLineVal');
    if(runEl) runEl.textContent=s.running ? ((s.script||'?')+' @ line '+s.line) : 'Idle';
    const btnStop=document.getElementById('btnStop');
    if(btnStop) btnStop.style.display=s.running ? 'inline-flex' : 'none';

    // Thermal monitoring
    if(s.temp !== undefined) {
      const tEl=document.getElementById('tempVal');
      if(tEl) tEl.textContent=s.temp + ' °C' + (s.throttled ? ' (THROTTLED)' : '');
      const chipT=document.getElementById('chipTemp');
      if(chipT) {
        if(s.throttled) setChip(chipT, s.temp + ' °C (HOT)', 'bad');
        else if(s.temp > 60) setChip(chipT, s.temp + ' °C', 'warn');
        else setChip(chipT, s.temp + ' °C', 'good');
      }
      const bEl=document.getElementById('thermalBanner');
      if(bEl) bEl.style.display=s.throttled ? 'block' : 'none';
    }
  }catch(e){
    setChip(document.getElementById('chipRunning'), 'Offline', 'bad');
  }
}

function appendScriptRow(ul,name){
  const li=document.createElement('li');
  li.className='li';
  const code=document.createElement('span');
  code.className='mono li-name';
  code.title='Click to open in editor';
  code.textContent=name;
  code.onclick=()=>openEditor(name);

  const right=document.createElement('div');
  right.className='btn-group';

  const bEdit=document.createElement('button');
  bEdit.className='btn small ghost'; bEdit.textContent='Edit';
  bEdit.onclick=()=>openEditor(name);
  right.appendChild(bEdit);

  const bRun=document.createElement('button');
  bRun.className='btn small primary'; bRun.textContent='Run';
  bRun.onclick=()=>runScript(name);
  right.appendChild(bRun);

  const bDel=document.createElement('button');
  bDel.className='btn small danger'; bDel.textContent='Del';
  bDel.onclick=()=>delScript(name);
  right.appendChild(bDel);

  li.append(code,right);
  ul.appendChild(li);
}

function renderList(){
  const q=(document.getElementById('filter')?.value||'').trim().toLowerCase();
  const ul=document.getElementById('scripts');
  if(!ul) return;
  ul.innerHTML='';
  const list=scriptsCache.filter(s=>!q||s.toLowerCase().includes(q));
  if(!list.length){
    const empty=document.createElement('div');
    empty.style.color='#777';
    empty.style.padding='6px 0';
    empty.textContent=q?'No matches.':'No scripts found on device.';
    ul.appendChild(empty);
    return;
  }
  list.forEach(s=>appendScriptRow(ul,s));
}

async function loadList(){
  try{
    const r=await fetch('/list',{headers:duckHeaders()});
    if(!r.ok){
      log('Script list: '+await r.text());
      scriptsCache=[];
    }else{
      const data=await r.json();
      scriptsCache=Array.isArray(data)?[...new Set(data)]:[];
    }
  }catch(e){
    log('Script list error: '+e);
    scriptsCache=[];
  }
  renderList();
}

function newScript(){
  document.getElementById('editorName').value='payload.txt';
  document.getElementById('editorBody').value='REM New script\nDEFAULTDELAY 50\n';
  document.getElementById('editorBody').focus();
}

async function fetchScriptContent(name){
  const fd=new FormData();
  fd.append('name',name);
  let r=await fetch('/script/load',{method:'POST',body:fd,headers:duckHeaders()});
  let text=await r.text();
  if(r.ok) return text;
  r=await fetch('/script?s='+encodeURIComponent(name),{headers:duckHeaders()});
  text=await r.text();
  if(r.ok) return text;
  throw new Error(text||('HTTP '+r.status));
}

async function loadScript(forcedName){
  const name=(forcedName||document.getElementById('editorName').value).trim();
  const body=document.getElementById('editorBody');
  if(!name){log('Enter a filename');return;}
  document.getElementById('editorName').value=name;
  if(name.startsWith('sd:')||name.startsWith('SD:')){
    const sel=document.getElementById('editorStorage');
    if(sel) sel.value='sd';
  }
  body.value='Loading...';
  try{
    const text=await fetchScriptContent(name);
    body.value=text;
    log('Loaded: '+name);
  }catch(e){
    body.value='REM Load failed: '+e.message+'\nREM File: '+name;
    log('Load failed: '+e.message);
  }
}

async function openEditor(name){
  const sel=document.getElementById('editorStorage');
  if(sel) sel.value=(name.startsWith('sd:')||name.startsWith('SD:'))?'sd':'spiffs';
  await loadScript(name);
  document.getElementById('editorBody').scrollIntoView({behavior:'smooth',block:'center'});
}

function scriptRefName(raw, storage){
  let n=raw.trim();
  if(!n) return '';
  if(n.startsWith('sd:')) return n;
  if(storage==='sd') return 'sd:'+n;
  return n;
}

async function saveScript(){
  const raw=document.getElementById('editorName').value.trim();
  const storage=document.getElementById('editorStorage').value;
  const name=scriptRefName(raw, storage);
  const content=document.getElementById('editorBody').value;
  if(!name){log('Enter a filename');return;}
  const fd=new FormData();
  fd.append('name',name);
  fd.append('content',content);
  fd.append('storage', storage);
  const r=await fetch('/script',{method:'POST',body:fd,headers:duckHeaders()});
  log(await r.text());
  await loadList();
}

async function runEditorScript(){
  const raw=document.getElementById('editorName').value.trim();
  const storage=document.getElementById('editorStorage').value;
  const name=scriptRefName(raw, storage);
  if(!name){log('Enter a filename');return;}
  await saveScript();
  await runScript(name);
}

async function runScript(s){
  try{
    const r=await fetch('/run?s='+encodeURIComponent(s),{headers:duckHeaders()});
    const t=await r.text();
    log((r.ok?'Run: ':'Run failed: ')+t);
    refreshStatus();
  }catch(e){
    log('Run error: '+(e?.message||String(e)));
  }
}

async function stopScript(){
  try{
    const r=await fetch('/run?stop=1',{headers:duckHeaders()});
    const t=await r.text();
    log(t);
    refreshStatus();
  }catch(e){
    log('Stop error: '+e);
  }
}

async function delScript(s){
  if(!confirm(`Delete ${s}?`)) return;
  const r=await fetch('/delete?s='+encodeURIComponent(s),{headers:duckHeaders()});
  log(await r.text());
  await loadList();
  refreshStatus();
}

async function saveAutorun(){
  const ar=document.getElementById('autorun');
  const val=ar ? ar.value.trim() : '';
  const fd=new FormData();
  fd.append('autorun_script', val);
  try{
    const r=await fetch('/settings',{method:'POST',body:fd,headers:duckHeaders()});
    log(await r.text());
    autorunUserEditing=false;
    refreshStatus();
  }catch(e){
    log('Save error: '+e);
  }
}

async function sendLine(){
  const el=document.getElementById('line');
  const line=el.value.trim();
  if(!line) return;
  const fd=new FormData();
  fd.append('line',line);
  try{
    const r=await fetch('/send',{method:'POST',body:fd,headers:duckHeaders()});
    log(await r.text());
    el.value='';
  }catch(e){
    log('Send error: '+e);
  }
}

document.getElementById('uploadForm')?.addEventListener('submit',async(e)=>{
  e.preventDefault();
  const f=document.getElementById('file').files[0];
  if(!f){log('No file selected');return;}
  const fd=new FormData();
  fd.append('file', f);
  const dest=document.getElementById('uploadStorage')?.value||'spiffs';
  try{
    const r=await fetch('/upload?storage='+encodeURIComponent(dest),{method:'POST',body:fd,headers:duckHeaders()});
    log(await r.text());
    document.getElementById('file').value='';
    await loadList();
  }catch(err){
    log('Upload error: '+err);
  }
});

const arInput=document.getElementById('autorun');
if(arInput){
  arInput.addEventListener('focus', () => { autorunUserEditing=true; });
  arInput.addEventListener('input', () => { autorunUserEditing=true; });
  arInput.addEventListener('blur', () => { setTimeout(() => { autorunUserEditing=false; }, 300); });
}

refreshStatus();
loadList();
setInterval(refreshStatus, 2000);
)rawliteral";

static const char SETTINGS_JS[] PROGMEM = R"rawliteral(
const log=(m)=>{const e=document.getElementById('log');e.textContent+=m+"\n";e.scrollTop=e.scrollHeight;};
const clearLog=()=>{document.getElementById('log').textContent='';};
function duckHeaders(){
  const pw=sessionStorage.getItem('duckPw')||'';
  const h={};
  if(pw) h['X-Duck-Password']=pw;
  return h;
}
async function loadSettings(){
  const r=await fetch('/settings',{headers:duckHeaders()});
  const s=await r.json();
  document.getElementById('ssid').value=s.ssid||'';
  document.getElementById('password').value=s.password||'';
  document.getElementById('channel').value=s.channel||1;
  if(document.getElementById('stealth')) document.getElementById('stealth').checked=!!s.stealth;
  if(document.getElementById('hidden_ap')) document.getElementById('hidden_ap').checked=!!s.hidden_ap;
  if(document.getElementById('disable_led')) document.getElementById('disable_led').checked=!!s.disable_led;
  if(document.getElementById('led_brightness')) {
    document.getElementById('led_brightness').value = s.led_brightness || 20;
    if(document.getElementById('brightVal')) document.getElementById('brightVal').textContent = s.led_brightness || 20;
  }
  if(document.getElementById('wifi_power')) document.getElementById('wifi_power').value = s.wifi_power || 'med';
  if(document.getElementById('wifi_enabled')) document.getElementById('wifi_enabled').checked = s.wifi_enabled !== false;
  if(document.getElementById('led_color')) document.getElementById('led_color').value=s.led_color||'#00FF88';
  if(document.getElementById('web_password')) document.getElementById('web_password').value=s.web_password||'';
  if(document.getElementById('autorun_delay_min')) document.getElementById('autorun_delay_min').value=s.autorun_delay_min||0;
  if(document.getElementById('autorun_delay_max')) document.getElementById('autorun_delay_max').value=s.autorun_delay_max||0;
}
async function saveSettings(){
  const fd=new FormData();
  fd.append('ssid',document.getElementById('ssid').value.trim());
  fd.append('password',document.getElementById('password').value);
  fd.append('channel',document.getElementById('channel').value);
  if(document.getElementById('stealth')) fd.append('stealth',document.getElementById('stealth').checked?'1':'0');
  if(document.getElementById('hidden_ap')) fd.append('hidden_ap',document.getElementById('hidden_ap').checked?'1':'0');
  if(document.getElementById('disable_led')) fd.append('disable_led',document.getElementById('disable_led').checked?'1':'0');
  if(document.getElementById('led_brightness')) fd.append('led_brightness',document.getElementById('led_brightness').value);
  if(document.getElementById('wifi_power')) fd.append('wifi_power',document.getElementById('wifi_power').value);
  if(document.getElementById('wifi_enabled')) fd.append('wifi_enabled',document.getElementById('wifi_enabled').checked?'1':'0');
  if(document.getElementById('led_color')) fd.append('led_color',document.getElementById('led_color').value);
  if(document.getElementById('web_password')) fd.append('web_password',document.getElementById('web_password').value);
  if(document.getElementById('autorun_delay_min')) fd.append('autorun_delay_min',document.getElementById('autorun_delay_min').value);
  if(document.getElementById('autorun_delay_max')) fd.append('autorun_delay_max',document.getElementById('autorun_delay_max').value);
  const r=await fetch('/settings',{method:'POST',body:fd,headers:duckHeaders()});
  log(await r.text());
}
let ledColorTimer=null;
function scheduleLedColorApply(){
  const el=document.getElementById('led_color');
  if(!el) return;
  clearTimeout(ledColorTimer);
  ledColorTimer=setTimeout(applyLedColor,150);
}
let brightTimer = null;
function scheduleBrightnessApply(){
  clearTimeout(brightTimer);
  brightTimer = setTimeout(applyBrightness, 120);
}
async function applyBrightness(){
  const el = document.getElementById('led_brightness');
  if(!el) return;
  const fd = new FormData();
  fd.append('brightness', el.value);
  try {
    await fetch('/led', {method:'POST', body:fd, headers:duckHeaders()});
  } catch(e){}
}
async function applyLedColor(){
  const el=document.getElementById('led_color');
  if(!el) return;
  const fd=new FormData();
  fd.append('color',el.value);
  try{
    const r=await fetch('/led',{method:'POST',body:fd,headers:duckHeaders()});
    log(await r.text());
  }catch(e){
    log('LED color error: '+(e?.message||String(e)));
  }
}
loadSettings();
const ledColorEl=document.getElementById('led_color');
if(ledColorEl){
  ledColorEl.addEventListener('input',scheduleLedColorApply);
  ledColorEl.addEventListener('change',applyLedColor);
}
)rawliteral";

static const char TERMINAL_JS[] PROGMEM = R"rawliteral(
const out=document.getElementById('out');const cmd=document.getElementById('cmd');
const ws=new WebSocket(`ws://${location.host}/ws`);
ws.onmessage=e=>{out.textContent+=e.data+"\n";out.scrollTop=out.scrollHeight;};
document.getElementById('host').textContent=location.host;
function clearOut(){out.textContent='';}
function sendCmd(){if(!cmd.value.trim())return;ws.send(cmd.value);cmd.value='';}
cmd.addEventListener('keydown',e=>{if(e.key==='Enter')sendCmd();});
)rawliteral";
