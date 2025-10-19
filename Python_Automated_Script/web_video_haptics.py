# web_video_haptics.py
# Run:  pip install flask
# Then: python web_video_haptics.py
# Open: http://127.0.0.1:5000/  (works offline while connected to GloveAP)

import socket
from pathlib import Path
from flask import Flask, send_from_directory, request, jsonify, Response

# ===== ESP32 UDP target (your AP) =====
ESP_IP = "192.168.4.1"
ESP_PORT = 5005
UDP_TIMEOUT = 2.0  # seconds

# Map buttons to commands (matches your ESP32 code)
VIDEO_MAP = {
    "defense": {"file": "defense.mp4", "cmd": "1"},
    "crash":   {"file": "crash.mp4",   "cmd": "2"},
    "pitstop": {"file": "pitstop.mp4", "cmd": "3"},
}
BOOST_CMD = "4"  # nitro (no video required)

app = Flask(__name__, static_folder=None)
BASE = Path(__file__).resolve().parent

def send_udp(cmd: str):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(UDP_TIMEOUT)
    try:
        s.sendto(cmd.encode(), (ESP_IP, ESP_PORT))
        try:
            data, addr = s.recvfrom(1024)
            return True, data.decode(errors="ignore")
        except socket.timeout:
            return False, "No ACK (timeout)"
    finally:
        s.close()

# ---- Routes ----

@app.get("/")
def index():
    # simple inline HTML so you only need one file
    html = f"""
<!doctype html>
<meta charset="utf-8">
<title>Video + Haptics (ESP32)</title>
<style>
  body {{ font-family: system-ui, Arial; margin: 20px; }}
  .row {{ display: grid; grid-template-columns: 320px 1fr; gap: 16px; margin-bottom: 28px; align-items: start; }}
  video {{ width: 320px; height: 180px; background:#000; }}
  button {{ padding: 8px 14px; margin-right: 8px; }}
  .log {{ white-space: pre-wrap; font-family: ui-monospace, Consolas, monospace; background:#f6f6f8; padding:10px; border-radius:8px; }}
  .title {{ font-weight:600; margin-bottom: 8px; }}
</style>

<h2>Video + Haptics (Press Play → Send Pattern)</h2>
<p>Make sure your laptop is connected to <b>GloveAP</b> before playing. This page works offline.</p>

<div class="row">
  <video id="v-defense" src="/media/defense" controls preload="metadata"></video>
  <div>
    <div class="title">Defense (Pattern 1)</div>
    <button onclick="playWithCmd('defense')">▶ Play (send P1)</button>
    <button onclick="pauseVid('defense')">⏸ Pause</button>
    <button onclick="stopVid('defense')">⏹ Stop</button>
  </div>
</div>

<div class="row">
  <video id="v-crash" src="/media/crash" controls preload="metadata"></video>
  <div>
    <div class="title">Crash (Pattern 2)</div>
    <button onclick="playWithCmd('crash')">▶ Play (send P2)</button>
    <button onclick="pauseVid('crash')">⏸ Pause</button>
    <button onclick="stopVid('crash')">⏹ Stop</button>
  </div>
</div>

<div class="row">
  <video id="v-pitstop" src="/media/pitstop" controls preload="metadata"></video>
  <div>
    <div class="title">Pit Stop (Pattern 3)</div>
    <button onclick="playWithCmd('pitstop')">▶ Play (send P3)</button>
    <button onclick="pauseVid('pitstop')">⏸ Pause</button>
    <button onclick="stopVid('pitstop')">⏹ Stop</button>
  </div>
</div>

<div class="row">
  <div>
    <button onclick="boost()">🔥 Send BOOST (P4)</button>
  </div>
</div>

<h3>Log</h3>
<div id="log" class="log"></div>

<script>
function log(msg) {{
  const el = document.getElementById('log');
  el.textContent += msg + "\\n";
  el.scrollTop = el.scrollHeight;
}}

async function playWithCmd(key) {{
  // Send command first, then play immediately
  log("Sending command for " + key + " …");
  try {{
    const r = await fetch('/send/' + key);
    const js = await r.json();
    log("ESP32 reply: " + js.status + (js.detail ? (" — " + js.detail) : ""));
  }} catch(e) {{
    log("Failed to send: " + e);
  }}
  const v = document.getElementById('v-' + key);
  v.play().catch(err => log("Play blocked: " + err));
}}

function pauseVid(key) {{
  const v = document.getElementById('v-' + key);
  v.pause();
}}

function stopVid(key) {{
  const v = document.getElementById('v-' + key);
  v.pause();
  v.currentTime = 0;
}}

async function boost() {{
  log("Sending BOOST (P4) …");
  try {{
    const r = await fetch('/send/boost');
    const js = await r.json();
    log("ESP32 reply: " + js.status + (js.detail ? (" — " + js.detail) : ""));
  }} catch(e) {{
    log("Failed to send: " + e);
  }}
}}
</script>
"""
    return Response(html, mimetype="text/html")

@app.get("/media/<key>")
def media(key: str):
    if key not in VIDEO_MAP:
        return "Not found", 404
    fn = VIDEO_MAP[key]["file"]
    path = (BASE / fn)
    if not path.exists():
        return f"Missing file: {fn}", 404
    # Let browser stream it; works offline
    return send_from_directory(BASE, fn, as_attachment=False)

@app.get("/send/<key>")
def send_key(key: str):
    if key == "boost":
        ok, detail = send_udp(BOOST_CMD)
        return jsonify({"status": "OK" if ok else "WARN", "detail": detail})

    item = VIDEO_MAP.get(key)
    if not item:
        return jsonify({"status": "ERR", "detail": "Unknown key"}), 400
    cmd = item["cmd"]
    ok, detail = send_udp(cmd)
    return jsonify({"status": "OK" if ok else "WARN", "detail": detail})

if __name__ == "__main__":
    # Bind to localhost only; keep it simple
    print("Serving on http://127.0.0.1:5000")
    print("Files expected in this folder:", [VIDEO_MAP[k]["file"] for k in VIDEO_MAP], "+ (optional) your BOOST has no video")
    app.run(host="127.0.0.1", port=5000, debug=False)
