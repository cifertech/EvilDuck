#pragma once
#include <pgmspace.h>

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <meta name="color-scheme" content="dark light" />
  <title>EvilDuck</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <div class="logo" aria-hidden="true"></div>
      <div>
        <div class="title">EvilDuck</div>
        <div class="subtitle">AP control panel</div>
      </div>
    </div>
    <nav class="nav">
      <a class="navlink" href="/index.html">Home</a>
      <a class="navlink" href="/settings.html">Settings</a>
      <a class="navlink" href="/terminal.html">Terminal</a>
      <a class="navlink" href="/credits.html">Credits</a>
    </nav>
  </header>

  <main class="tabshell">
    <div class="tabbar" role="tablist" aria-label="Home sections">
      <button class="tabbtn active" type="button" role="tab" aria-selected="true" aria-controls="tab-dashboard" data-tab="dashboard">Dashboard</button>
      <button class="tabbtn" type="button" role="tab" aria-selected="false" aria-controls="tab-scripts" data-tab="scripts">Scripts</button>
      <button class="tabbtn" type="button" role="tab" aria-selected="false" aria-controls="tab-library" data-tab="library">Library</button>
      <button class="tabbtn" type="button" role="tab" aria-selected="false" aria-controls="tab-logs" data-tab="logs">Logs</button>
    </div>

    <section id="tab-dashboard" class="tabpanel active grid" role="tabpanel">
    <section class="card span2">
      <div class="cardhead">
        <h2>Status</h2>
        <div class="row gap">
          <span id="chipRunning" class="chip">Checking…</span>
          <span id="chipTemp" class="chip subtle">Temp: —</span>
          <span id="chipHID" class="chip subtle">HID</span>
          <span id="chipSD" class="chip subtle">SD</span>
          <span id="chipStorage" class="chip subtle">MSC</span>
          <span id="chipAP" class="chip subtle">AP</span>
        </div>
      </div>
      <div id="thermalBanner" style="display:none; margin:10px 0 14px 0; padding:10px 14px; border-radius:8px; background:rgba(255,107,107,0.18); border:1px solid #ff6b6b; color:#ff6b6b; font-weight:600;">⚠️ Thermal Throttle Active: Core temp is high (>70°C). Reduced LED & RF power to protect hardware.</div>
      <div class="kv">
        <div class="k">SSID</div><div class="v mono" id="ssidVal">—</div>
        <div class="k">Core Temp</div><div class="v mono" id="tempVal">—</div>
        <div class="k">Channel</div><div class="v mono" id="chVal">—</div>
        <div class="k">Autorun</div><div class="v mono" id="autorunVal">—</div>
        <div class="k">Running</div><div class="v mono" id="runLineVal">-</div>
        <div class="k">Web UI password</div><div class="v"><input class="input mono" id="duckPw" type="password" placeholder="(if set in Settings)" onchange="saveDuckPw()" /></div>
      </div>
      <div class="actions">
        <button class="btn" onclick="refreshStatus()">Refresh</button>
        <button class="btn ghost" onclick="loadList()">Refresh scripts</button>
      </div>
    </section>

    <section class="card">
      <div class="cardhead"><h2>Quick command</h2></div>
      <div class="stack">
        <label class="field">
          <span class="label">Send one line</span>
          <input class="input mono" id="line" placeholder='e.g. GUI r' />
        </label>
        <button class="btn" onclick="sendLine()">Send</button>
        <div class="hint">For interactive CLI, use the <a href="/terminal.html">Terminal</a>.</div>
      </div>
    </section>

    <section class="card">
      <div class="cardhead"><h2>Autorun</h2></div>
      <div class="stack">
        <label class="field">
          <span class="label">Script name</span>
          <input class="input mono" id="autorun" placeholder="payload.txt (or empty to disable)" />
        </label>
        <button class="btn" onclick="saveAutorun()">Save</button>
      </div>
    </section>
    </section>

    <section id="tab-scripts" class="tabpanel grid" role="tabpanel" hidden>
      <section class="card">
        <div class="cardhead"><h2>Upload payload</h2></div>
        <form id="uploadForm" class="stack">
          <label class="field">
            <span class="label">Script file</span>
            <input class="input" type="file" id="file" name="file" accept=".txt,.duck" required />
          </label>
          <label class="field">
            <span class="label">Destination</span>
            <select class="input" id="uploadStorage" name="storage">
              <option value="spiffs">SPIFFS</option>
              <option value="sd">SD card</option>
            </select>
          </label>
          <button class="btn primary" type="submit">Upload</button>
          <div class="hint">Use <code class="mono">.txt</code> or <code class="mono">.duck</code>.</div>
        </form>
      </section>

      <section class="card span2">
        <div class="cardhead">
          <h2>Scripts</h2>
          <div class="row gap">
            <input class="input" id="filter" placeholder="Filter scripts…" oninput="renderList()" />
            <button class="btn" onclick="loadList()">Reload</button>
          </div>
        </div>
        <ul id="scripts" class="list"></ul>
      </section>

      <section class="card span2">
        <div class="cardhead">
          <h2>Script editor</h2>
          <div class="row gap">
            <button class="btn ghost" onclick="newScript()">New</button>
            <button class="btn" onclick="loadScript()">Load</button>
            <button class="btn primary" onclick="saveScript()">Save</button>
            <button class="btn" onclick="runEditorScript()">Run</button>
          </div>
        </div>
        <div class="stack">
          <label class="field">
            <span class="label">Filename</span>
            <input class="input mono" id="editorName" placeholder="demos/01_hello_windows.txt" autocomplete="off" />
            <span class="hint">Click a script in the list below to load it into the editor.</span>
          </label>
          <label class="field">
            <span class="label">Save to</span>
            <select class="input" id="editorStorage">
              <option value="spiffs">SPIFFS (internal)</option>
              <option value="sd">SD card</option>
            </select>
          </label>
          <textarea class="editor mono" id="editorBody" rows="14" placeholder="Select a script from the list to edit it here."></textarea>
        </div>
      </section>
    </section>

    <section id="tab-library" class="tabpanel grid" role="tabpanel" hidden>
      <section class="card span2">
        <div class="cardhead"><h2>Script library</h2><button class="btn" onclick="loadLibraryList()">Refresh</button></div>
        <p class="hint muted">Built-in examples plus scripts on SPIFFS (flash data/ with Sketch Data Upload).</p>
        <ul id="library" class="list"></ul>
      </section>
    </section>

    <section id="tab-logs" class="tabpanel grid" role="tabpanel" hidden>
      <section class="card span2">
        <div class="cardhead"><h2>Run log</h2><button class="btn ghost" onclick="refreshExecLog()">Refresh</button></div>
        <pre id="execLog" class="log mono"></pre>
      </section>

      <section class="card span2">
        <div class="cardhead">
          <h2>Activity</h2>
          <button class="btn ghost" onclick="clearLog()">Clear</button>
        </div>
        <pre id="log" class="log" aria-live="polite"></pre>
      </section>
    </section>
  </main>

  <footer class="footer">
    <span class="muted">EvilDuck</span>
    <span class="muted">•</span>
    <a class="muted" href="/credits.html">Credits</a>
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
  <meta name="color-scheme" content="dark light" />
  <title>EvilDuck Settings</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <div class="logo" aria-hidden="true"></div>
      <div>
        <div class="title">Settings</div>
        <div class="subtitle">EvilDuck access point</div>
      </div>
    </div>
    <nav class="nav">
      <a class="navlink" href="/index.html">Home</a>
      <a class="navlink" href="/terminal.html">Terminal</a>
    </nav>
  </header>

  <main class="grid">
    <section class="card span2">
      <div class="cardhead"><h2>Wi‑Fi AP</h2></div>
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
          <span class="label">Wi‑Fi Power / Thermal</span>
          <select class="input" id="wifi_power">
            <option value="low">Low (5 dBm) — Coolest & Desk Range</option>
            <option value="med">Medium (11 dBm) — Balanced (Recommended)</option>
            <option value="high">High (19.5 dBm) — Maximum Range</option>
          </select>
          <span class="hint">Lower power significantly reduces chip heat.</span>
        </label>

        <label class="field">
          <span class="label"><input type="checkbox" id="wifi_enabled" checked /> Wi‑Fi AP Enabled</span>
          <span class="hint">Uncheck to run in silent zero-RF mode.</span>
        </label>
        <div class="actions">
          <button class="btn primary" onclick="saveSettings()">Save</button>
          <button class="btn ghost" onclick="loadSettings()">Reload</button>
        </div>
      </div>
    </section>

    <section class="card span2">
      <div class="cardhead"><h2>Stealth / OPSEC</h2></div>
      <div class="stack">
        <label class="field"><span class="label">Web UI password (empty = off)</span>
          <input class="input" id="web_password" type="password" autocomplete="off" />
        </label>
        <label class="field"><span class="label"><input type="checkbox" id="stealth" /> Stealth mode</span></label>
        <label class="field"><span class="label"><input type="checkbox" id="hidden_ap" /> Hidden AP (reboot)</span></label>
        <label class="field"><span class="label"><input type="checkbox" id="disable_led" /> Disable LED commands</span></label>
        <label class="field">
          <span class="label">RGB Brightness (0–255)</span>
          <input class="input" id="led_brightness" type="range" min="0" max="255" value="20" oninput="document.getElementById('brightVal').textContent=this.value; scheduleBrightnessApply();" />
          <span class="hint">Current: <span id="brightVal">20</span>. Lower brightness reduces chip heat.</span>
        </label>
        <label class="field"><span class="label">Static LED color</span>
          <input class="input" id="led_color" type="color" value="#000028" />
          <span class="hint">Used when idle/ready. Running scripts show purple and errors show red.</span>
        </label>
        <label class="field"><span class="label">Autorun delay min (ms)</span>
          <input class="input mono" id="autorun_delay_min" type="number" min="0" />
        </label>
        <label class="field"><span class="label">Autorun delay max (ms)</span>
          <input class="input mono" id="autorun_delay_max" type="number" min="0" />
        </label>
      </div>
    </section>

    <section class="card span2">
      <div class="cardhead"><h2>Activity</h2><button class="btn ghost" onclick="clearLog()">Clear</button></div>
      <pre id="log" class="log"></pre>
    </section>
  </main>

  <footer class="footer">
    <a class="muted" href="/index.html">Back to Home</a>
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
  <meta name="color-scheme" content="dark light" />
  <title>EvilDuck Terminal</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <div class="logo" aria-hidden="true"></div>
      <div>
        <div class="title">Terminal</div>
        <div class="subtitle mono">ws://<span id="host"></span>/ws</div>
      </div>
    </div>
    <nav class="nav">
      <a class="navlink" href="/index.html">Home</a>
      <a class="navlink" href="/settings.html">Settings</a>
    </nav>
  </header>

  <main class="grid">
    <section class="card span2">
      <div class="cardhead">
        <h2>Output</h2>
        <button class="btn ghost" onclick="clearOut()">Clear</button>
      </div>
      <div id="out" class="term"></div>
    </section>
    <section class="card span2">
      <div class="cardhead"><h2>Command</h2></div>
      <div class="row gap">
        <input class="input mono" id="cmd" placeholder="help" />
        <button class="btn primary" onclick="sendCmd()">Send</button>
      </div>
      <div class="hint">Press Enter to send.</div>
    </section>
  </main>

  <script src="/terminal.js"></script>
</body>
</html>
)rawliteral";

static const char CREDITS_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <meta name="color-scheme" content="dark light" />
  <title>Credits</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <div class="logo" aria-hidden="true"></div>
      <div>
        <div class="title">Credits</div>
        <div class="subtitle">Project info</div>
      </div>
    </div>
    <nav class="nav">
      <a class="navlink" href="/index.html">Home</a>
      <a class="navlink" href="/settings.html">Settings</a>
      <a class="navlink" href="/terminal.html">Terminal</a>
    </nav>
  </header>
  <main class="grid">
    <section class="card span2">
      <div class="cardhead"><h2>EvilDuck S3</h2></div>
      <p class="muted">
        EvilDuck S3 is a hardware and software project built around the ESP32-S3.
        It provides a Wi-Fi web UI for uploading, editing, and running Rubber Ducky-style scripts, plus terminal access and device settings.
      </p>
    </section>

    <section class="card">
      <div class="cardhead"><h2>Creator</h2></div>
      <div class="stack">
        <p class="muted">Created by <strong>CiferTech</strong>.</p>
        <a class="linkcard" href="https://cifertech.net/" target="_blank" rel="noopener">
          <span class="label">Website</span>
          <span class="mono">cifertech.net</span>
        </a>
        <a class="linkcard" href="https://github.com/cifertech/EvilDuck" target="_blank" rel="noopener">
          <span class="label">GitHub</span>
          <span class="mono">github.com/cifertech/EvilDuck</span>
        </a>
      </div>
    </section>
  </main>
</body>
</html>
)rawliteral";

static const char ERROR404_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <meta name="color-scheme" content="dark light" />
  <title>404</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <header class="topbar">
    <div class="brand">
      <div class="logo" aria-hidden="true"></div>
      <div>
        <div class="title">404</div>
        <div class="subtitle">Page not found</div>
      </div>
    </div>
    <nav class="nav">
      <a class="navlink" href="/index.html">Home</a>
    </nav>
  </header>
  <main class="grid">
    <section class="card span2">
      <div class="cardhead"><h2>Oops</h2></div>
      <p class="muted">This page doesn’t exist.</p>
      <div class="actions"><a class="btn primary" href="/index.html">Go home</a></div>
    </section>
  </main>
</body>
</html>
)rawliteral";

static const char STYLE_CSS[] PROGMEM = R"rawliteral(
:root{
  --bg:#171717;
  --panel:#242424;
  --panel2:#171717;
  --text:#f2f4f8;
  --muted:#b7bcc8;
  --border:rgba(255,255,255,.10);
  --grid:rgba(255,255,255,.035);
  --accent:#FF5F15;
  --accent2:#FF5F15;
  --good:#37d39b;
  --warn:#FF5F15;
  --bad:#ff4d4d;
  --shadow:0 12px 34px rgba(0,0,0,.55);
  --radius:14px;
}
*{box-sizing:border-box}
html{
  min-height:100%;
  background:#171717;
}
body{min-height:100vh}
body{
  margin:0;
  font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, Arial, "Noto Sans", "Liberation Sans", sans-serif;
  position:relative;
  isolation:isolate;
  background:
    radial-gradient(900px 620px at 15% -10%, rgba(255,95,21,.23), transparent 62%),
    radial-gradient(820px 560px at 88% 8%, rgba(255,95,21,.14), transparent 60%),
    linear-gradient(135deg, #171717 0%, #242424 44%, #171717 100%);
  background-attachment:fixed;
  background-color:#171717;
  color:var(--text);
}
body::before{
  content:"";
  position:fixed;
  inset:0;
  z-index:-2;
  pointer-events:none;
  background:
    linear-gradient(var(--grid) 1px, transparent 1px),
    linear-gradient(90deg, var(--grid) 1px, transparent 1px);
  background-size:44px 44px;
}
body::after{
  content:"";
  position:fixed;
  inset:0;
  z-index:-1;
  pointer-events:none;
  background:
    radial-gradient(circle at 50% 0%, rgba(255,95,21,.10), transparent 36%),
    radial-gradient(circle at 20% 80%, rgba(255,95,21,.08), transparent 32%),
    linear-gradient(180deg, transparent 0%, rgba(0,0,0,.45) 100%);
}
a{color:var(--accent);text-decoration:none}
a:hover{text-decoration:underline}
.mono{font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace}
.muted{color:var(--muted)}

.topbar{
  position:sticky;top:0;z-index:2;
  padding:14px 16px;
  backdrop-filter: blur(14px);
  background: linear-gradient(180deg, rgba(36,36,36,.90), rgba(23,23,23,.72));
  border-bottom:1px solid var(--border);
  box-shadow:0 10px 30px rgba(0,0,0,.28);
  display:flex;align-items:center;justify-content:space-between;gap:14px;flex-wrap:wrap;
}
.brand{display:flex;align-items:center;gap:12px}
.logo{
  width:38px;height:38px;border-radius:12px;
  background:#FF5F15 url(data:image/png;base64,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) center/cover no-repeat;
  box-shadow:
    0 0 0 1px rgba(255,95,21,.35),
    0 10px 30px rgba(0,0,0,.6),
    0 0 26px rgba(255,95,21,.18);
  overflow:hidden;
}
.title{font-weight:750;letter-spacing:.2px}
.subtitle{color:var(--muted);font-size:12px;margin-top:2px}
.nav{display:flex;gap:10px;flex-wrap:wrap}
.navlink{padding:8px 10px;border-radius:10px;border:1px solid transparent;color:var(--text)}
.navlink:hover{border-color:var(--border);background:rgba(255,255,255,.04);text-decoration:none}

.tabshell{
  max-width:1080px;
  margin:0 auto;
}
.tabbar{
  padding:16px 16px 0;
  display:flex;
  gap:10px;
  flex-wrap:wrap;
}
.tabbtn{
  padding:10px 12px;
  border-radius:999px;
  border:1px solid var(--border);
  background:rgba(255,255,255,.04);
  color:var(--muted);
  cursor:pointer;
}
.tabbtn:hover{background:rgba(255,255,255,.08);color:var(--text)}
.tabbtn.active{
  border-color:rgba(255,95,21,.55);
  color:#171717;
  background:linear-gradient(135deg, rgba(255,95,21,.95), rgba(255,95,21,.62));
  font-weight:700;
}
.tabpanel[hidden]{display:none}

.grid{
  max-width:1080px;
  margin:0 auto;
  padding:16px;
  display:grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap:14px;
}
@media (max-width:860px){
  .grid{grid-template-columns:1fr}
}
.span2{grid-column: span 2}
@media (max-width:860px){
  .span2{grid-column:auto}
}
.card{
  background:
    linear-gradient(180deg, rgba(255,255,255,.035), transparent 42%),
    linear-gradient(180deg, rgba(36,36,36,.94), rgba(23,23,23,.92));
  border:1px solid var(--border);
  border-radius:var(--radius);
  box-shadow: var(--shadow);
  padding:14px;
  min-width:0;
  overflow:hidden;
}
.cardhead{display:flex;align-items:center;justify-content:space-between;gap:10px;flex-wrap:wrap;margin-bottom:10px}
h1,h2{margin:0}
h2{font-size:16px;letter-spacing:.2px}

.row{display:flex;align-items:center}
.gap{gap:10px}
.stack{display:flex;flex-direction:column;gap:10px}
.field{display:flex;flex-direction:column;gap:6px}
.label{font-size:12px;color:var(--muted)}
.hint{font-size:12px;color:var(--muted)}
.linkcard{
  display:flex;
  flex-direction:column;
  gap:4px;
  padding:12px;
  border:1px solid var(--border);
  border-radius:12px;
  background:rgba(255,255,255,.04);
  color:var(--text);
}
.linkcard:hover{
  border-color:rgba(255,95,21,.45);
  background:rgba(255,95,21,.08);
  text-decoration:none;
}

.input{
  width:100%;
  padding:10px 12px;
  background: rgba(0,0,0,.18);
  color: var(--text);
  border:1px solid var(--border);
  border-radius:12px;
  outline:none;
}
.input:focus{border-color: rgba(255,95,21,.65); box-shadow: 0 0 0 3px rgba(255,95,21,.18)}

.btn{
  display:inline-flex;align-items:center;justify-content:center;
  padding:10px 12px;
  background: rgba(255,255,255,.06);
  color: var(--text);
  border:1px solid var(--border);
  border-radius:12px;
  cursor:pointer;
  user-select:none;
}
.btn:hover{background: rgba(255,255,255,.10)}
.btn:active{transform: translateY(1px)}
.btn.primary{
  border-color: rgba(255,95,21,.55);
  background: linear-gradient(135deg, rgba(255,95,21,.95), rgba(255,95,21,.62));
  color:#171717;
  font-weight:700;
  box-shadow: 0 0 0 1px rgba(255,95,21,.16), 0 10px 24px rgba(0,0,0,.55), 0 0 24px rgba(255,95,21,.10);
}
.btn.ghost{background: transparent}

.actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:8px}
.chip{
  display:inline-flex;align-items:center;gap:8px;
  padding:6px 10px;
  border-radius:999px;
  border:1px solid var(--border);
  background: rgba(255,255,255,.06);
  color: var(--text);
  font-size:12px;
}
.chip.subtle{color:var(--muted)}
.chip.good{border-color: rgba(67,209,158,.35); background: rgba(67,209,158,.12)}
.chip.warn{border-color: rgba(255,95,21,.35); background: rgba(255,95,21,.12)}
.chip.bad{border-color: rgba(255,107,107,.35); background: rgba(255,107,107,.12)}

.kv{
  display:grid;
  grid-template-columns: 120px 1fr;
  gap:8px 10px;
  padding:10px 0;
  border-top:1px solid var(--border);
  border-bottom:1px solid var(--border);
  margin:8px 0;
}
@media (max-width:860px){
  .kv{grid-template-columns: 100px 1fr}
}
.k{color:var(--muted);font-size:12px}
.v{overflow:hidden;text-overflow:ellipsis;white-space:nowrap}

.list{list-style:none;margin:0;padding:0;display:flex;flex-direction:column;gap:8px;min-width:0}
.li,.list .row{
  display:flex;align-items:center;justify-content:space-between;gap:10px;
  padding:10px 10px;
  border:1px solid var(--border);
  background: rgba(0,0,0,.10);
  border-radius:12px;
  min-width:0;
}
.li-name,.li code,.list .row code{
  flex:1 1 auto;
  min-width:0;
  overflow:hidden;
  text-overflow:ellipsis;
  white-space:nowrap;
  display:block;
  color:var(--text);
  cursor:pointer;
}
.li .right,.list .row .actions{
  flex:0 0 auto;
  display:flex;
  gap:6px;
  flex-wrap:nowrap;
  align-items:center;
}
.btn.small{padding:6px 10px;font-size:12px;white-space:nowrap}

.log{
  margin:0;
  padding:10px;
  border-radius:12px;
  border:1px solid var(--border);
  background: rgba(0,0,0,.18);
  min-height:110px;
  white-space:pre-wrap;
  overflow:auto;
}
.term{
  height:65vh;
  overflow:auto;
  white-space:pre-wrap;
  padding:10px;
  border-radius:12px;
  border:1px solid var(--border);
  background: rgba(0,0,0,.18);
}
.footer{
  max-width:1080px;
  margin:0 auto;
  padding:0 16px 22px;
  display:flex;gap:10px;flex-wrap:wrap;
}
.editor{
  width:100%;
  min-height:220px;
  padding:12px;
  border-radius:12px;
  border:1px solid var(--border);
  background:rgba(0,0,0,.18);
  color:var(--text);
  font-size:13px;
  line-height:1.45;
  resize:vertical;
  box-sizing:border-box;
}
)rawliteral";

static const char INDEX_JS[] PROGMEM = R"rawliteral(
const log=(m)=>{const e=document.getElementById('log');e.textContent+=m+"\n";e.scrollTop=e.scrollHeight;};
const clearLog=()=>{const e=document.getElementById('log');e.textContent='';};
function esc(s){return s.replace(/[&<>"]/g, c=>({ '&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;' }[c]));}
let scriptsCache=[];

function activateTab(name){
  document.querySelectorAll('.tabbtn').forEach(btn=>{
    const active=btn.dataset.tab===name;
    btn.classList.toggle('active',active);
    btn.setAttribute('aria-selected',active?'true':'false');
  });
  document.querySelectorAll('.tabpanel').forEach(panel=>{
    const active=panel.id==='tab-'+name;
    panel.classList.toggle('active',active);
    panel.hidden=!active;
  });
  if(location.hash!=='#'+name) history.replaceState(null,'','#'+name);
}

function appendScriptRow(ul,name,{showRun=true,showDelete=true}={}){
  const li=document.createElement('li');
  li.className='li';
  const code=document.createElement('code');
  code.className='mono li-name';
  code.title=name;
  code.textContent=name;
  code.onclick=()=>openEditor(name);
  const right=document.createElement('div');
  right.className='right';
  const bEdit=document.createElement('button');
  bEdit.className='btn small'; bEdit.textContent='Edit';
  bEdit.onclick=()=>openEditor(name);
  right.appendChild(bEdit);
  if(showRun){
    const bRun=document.createElement('button');
    bRun.className='btn small primary'; bRun.textContent='Run';
    bRun.onclick=()=>runScript(name);
    right.appendChild(bRun);
  }
  if(showDelete){
    const bDel=document.createElement('button');
    bDel.className='btn small'; bDel.textContent='Del';
    bDel.onclick=()=>delScript(name);
    right.appendChild(bDel);
  }
  li.append(code,right);
  ul.appendChild(li);
}

function renderList(){
  const q=(document.getElementById('filter')?.value||'').trim().toLowerCase();
  const ul=document.getElementById('scripts');
  ul.innerHTML='';
  const list=scriptsCache.filter(s=>!q||s.toLowerCase().includes(q));
  if(!list.length){
    const empty=document.createElement('div');
    empty.className='muted';
    empty.textContent=q?'No matches.':'No scripts on device. Flash data/ (Sketch Data Upload), upload a file, or enter Web UI password if set.';
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
  r=await fetch('/library?s='+encodeURIComponent(name),{headers:duckHeaders()});
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
    const preview=text.split('\n')[0].slice(0,60);
    log('Loaded: '+name+' | '+preview+(text.length>60?'...':''));
  }catch(e){
    body.value='REM Load failed: '+e.message+'\nREM File: '+name;
    log('Load failed: '+e.message);
  }
}

async function openEditor(name){
  const sel=document.getElementById('editorStorage');
  if(sel) sel.value=(name.startsWith('sd:')||name.startsWith('SD:'))?'sd':'spiffs';
  activateTab('scripts');
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

function setChip(el, text, cls){
  if(!el) return;
  el.className='chip '+(cls||'');
  el.textContent=text;
}

function saveDuckPw(){
  const v=document.getElementById('duckPw')?.value||'';
  sessionStorage.setItem('duckPw', v);
}
function duckHeaders(){
  const pw=sessionStorage.getItem('duckPw')||'';
  const h={};
  if(pw) h['X-Duck-Password']=pw;
  return h;
}

async function refreshStatus(){
  try{
    const r=await fetch('/status',{headers:duckHeaders()});
    const s=await r.json();
    document.getElementById('ssidVal').textContent=s.ssid||'—';
    document.getElementById('chVal').textContent=(s.channel??'—');
    document.getElementById('autorunVal').textContent=s.autorunEnabled ? (s.autorunScript||'') : 'disabled';
    document.getElementById('autorun').value=s.autorunScript||'';
    setChip(document.getElementById('chipRunning'), s.running?'Running':'Idle', s.running?'good':'subtle');
    const chipSd=document.getElementById('chipSD');
    if(chipSd) setChip(chipSd, s.sdReady?'SD OK':'No SD', s.sdReady?'good':'subtle');
    const chipHid=document.getElementById('chipHID');
    if(chipHid) setChip(chipHid, s.hid?'HID on':'HID off', s.hid?'good':'subtle');
    const runEl=document.getElementById('runLineVal');
    if(runEl) runEl.textContent=s.running ? ((s.script||'?')+' @ line '+s.line) : '—';
    const chipSt=document.getElementById('chipStorage');
    if(chipSt) setChip(chipSt, s.storage?'MSC on':'MSC off', s.storage?'good':'subtle');

    if(s.temp !== undefined) {
      const tEl=document.getElementById('tempVal');
      if(tEl) tEl.textContent=s.temp + ' °C' + (s.throttled ? ' (THROTTLED)' : '');
      const chipT=document.getElementById('chipTemp');
      if(chipT) {
        if(s.throttled) {
          setChip(chipT, s.temp + ' °C (HOT)', 'bad');
        } else if(s.temp > 60) {
          setChip(chipT, s.temp + ' °C', 'warn');
        } else {
          setChip(chipT, s.temp + ' °C', 'good');
        }
      }
      const bEl=document.getElementById('thermalBanner');
      if(bEl) {
        bEl.style.display=s.throttled ? 'block' : 'none';
        if(s.throttled) {
          bEl.textContent='⚠️ Thermal Throttle Active: Core temperature reached ' + s.temp + ' °C (>70 °C)! Dimmed LED & reduced Wi-Fi RF power to cool down.';
        }
      }
    }
  }catch(e){
    setChip(document.getElementById('chipRunning'), 'Offline', 'bad');
  }
}

async function loadLibraryList(){
  const ul=document.getElementById('library');
  if(!ul) return;
  ul.innerHTML='';
  try{
    const r=await fetch('/library?all=1',{headers:duckHeaders()});
    if(!r.ok){log('Library: '+await r.text());return;}
    const j=await r.json();
    const items=[...new Set(Array.isArray(j)?j:(j.items||[]))];
    if(!items.length){
      const empty=document.createElement('li');
      empty.className='muted';
      empty.textContent='No scripts. Flash SPIFFS data or use built-in examples after reboot.';
      ul.appendChild(empty);
      return;
    }
    items.forEach(p=>appendScriptRow(ul,p,{showRun:true,showDelete:false}));
  }catch(e){log('Library: '+e);}
}

async function refreshExecLog(){
  const el=document.getElementById('execLog');
  if(!el) return;
  try{
    const r=await fetch('/exec/log?format=text',{headers:duckHeaders()});
    el.textContent=await r.text();
    el.scrollTop=el.scrollHeight;
  }catch(e){el.textContent=String(e);}
}

async function runScript(s){
  try{
    const r=await fetch('/run?s='+encodeURIComponent(s),{headers:duckHeaders()});
    const t=await r.text();
    if(!r.ok){
      log(`Run failed (${r.status}): ${t}`);
      return;
    }
    log(t);
    refreshStatus();
  }catch(e){
    log('Run error: '+(e?.message||String(e)));
  }
}

async function delScript(s){
  if(!confirm(`Delete ${s}?`)) return;
  const r=await fetch('/delete?s='+encodeURIComponent(s),{headers:duckHeaders()});
  log(await r.text());
  await loadList();
  refreshStatus();
}

document.getElementById('uploadForm').addEventListener('submit',async(e)=>{
  e.preventDefault();
  const f=document.getElementById('file').files[0];
  if(!f){log('No file selected');return;}
  const fd=new FormData();
  fd.append('file', f);
  const dest=document.getElementById('uploadStorage')?.value||'spiffs';
  const r=await fetch('/upload?storage='+encodeURIComponent(dest),{method:'POST',body:fd,headers:duckHeaders()});
  log(await r.text());
  await loadList();
});

async function saveAutorun(){
  const fd=new FormData();
  fd.append('autorun_script',document.getElementById('autorun').value.trim());
  const r=await fetch('/settings',{method:'POST',body:fd,headers:duckHeaders()});
  log(await r.text());
  refreshStatus();
}

async function sendLine(){
  const el=document.getElementById('line');
  const line=el.value.trim();
  if(!line) return;
  const fd=new FormData();
  fd.append('line',line);
  const r=await fetch('/send',{method:'POST',body:fd,headers:duckHeaders()});
  log(await r.text());
  el.value='';
}

(function(){
  document.querySelectorAll('.tabbtn').forEach(btn=>{
    btn.addEventListener('click',()=>activateTab(btn.dataset.tab));
  });
  const initial=(location.hash||'').replace('#','');
  if(['dashboard','scripts','library','logs'].includes(initial)) activateTab(initial);
  const p=document.getElementById('duckPw');
  if(p) p.value=sessionStorage.getItem('duckPw')||'';
  const en=document.getElementById('editorName');
  if(en){
    en.addEventListener('keydown',e=>{
      if(e.key==='Enter'){e.preventDefault();loadScript();}
    });
  }
})();
loadList();
loadLibraryList();
refreshStatus();
refreshExecLog();
setInterval(refreshStatus, 1500);
setInterval(refreshExecLog, 2000);
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
