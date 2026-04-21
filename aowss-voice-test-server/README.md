# Reference Voice Solution - AOWS Test Server

Simple AudioOnlyWebSocket endpoint reference server implementation for voice pipeline testing.
This implementation uses stunnel with a self-signed certificate to expose secure websocket support `wss`.

All setup steps below are tested on Ubuntu 22.04.

## 1. Install Host Dependencies

```bash
sudo apt update
sudo apt install -y nodejs npm stunnel4 openssl alsa-utils iproute2
```

Node.js `>=18` is recommended.

## 2. Clone and Run the AOWS Server (ws) on Host Machine

Use a release `tag` (for example, `v1.2.3`) for stable builds, or use `develop` for the latest development branch.

```bash
git clone --branch <tag-or-branch> --single-branch https://github.com/rdkcentral/feature-test-tools.git aows-cloned-repo
cd aows-cloned-repo/aowss-voice-test-server

# Install runtime dependencies
npm install
npm start
```

Keep this service running on `ws://127.0.0.1:9880/`.

Optional quick health check:

```bash
npm run health
```

#### AOWS Server - Automated Tests

See [test/README](./test/README.md) for test scope, layout, and commands.

Server defaults:

- Websocket endpoint: `ws://0.0.0.0:9880/`
- Health endpoint: `GET /health`
- Capture output folder: `./aows_captures`

### Server Configuration

Copy `.env.example` to `.env` and adjust as needed.

```bash
cp .env.example .env
```

| Variable | Default | Description |
| --- | --- | --- |
| `AOWS_HOST` | `0.0.0.0` | Bind host |
| `AOWS_PORT` | `9880` | Bind port |
| `AOWS_OUT_DIR` | `./aows_captures` | Output directory for raw captures |
| `AOWS_PATH` | `/` | Expected websocket path |
| `AOWS_IDLE_CLOSE_MS` | `1000` | Close websocket after last binary frame (0 disables auto-close) |
| `AOWS_DECODE_ADPCM` | `false` | Enable ADPCM decode output |
| `AOWS_ADPCM_NIBBLE_ORDER` | `lh` | ADPCM nibble order (`lh` or `hl`) |
| `AOWS_ADPCM_FRAME_BYTES` | `95` | ADPCM packet size in bytes |
| `AOWS_ADPCM_HEADER_BYTES` | `4` | ADPCM header size in bytes |
| `AOWS_ADPCM_OFFSET_STEP_INDEX` | `1` | Header offset of ADPCM step index |
| `AOWS_ADPCM_OFFSET_PRED_LSB` | `2` | Header offset of predictor LSB |
| `AOWS_ADPCM_OFFSET_PRED_MSB` | `3` | Header offset of predictor MSB |

Notes:

- These defaults match ControlMgr RF4CE ADPCM framing (`95` byte packet, `4` byte header).
- Decode runs only when websocket binary payload length is frame-aligned to `AOWS_ADPCM_FRAME_BYTES`.

<details>
<summary>Sample AOWS Server Output with a voice session</summary>

```bash
d35@d35:/mnt/vmShared/aows$ npm start

> reference-voice-solution-aows-test-server@1.0.0 start
> node aows_test_server.js

[2026-03-12T15:52:03.501Z] AOWS test endpoint listening on ws://0.0.0.0:9880/
[2026-03-12T15:52:03.503Z] Saving audio to: /mnt/vmShared/aows/aows_captures
[2026-03-12T15:52:03.503Z] Reply messages to clients: disabled
[2026-03-12T15:52:03.503Z] Session idle auto-close: 1000 ms (set 0 to disable)
[2026-03-12T15:52:03.503Z] ADPCM frame/header config: frame=95 header=4 step_idx=1 pred_lsb=2 pred_msb=3
[2026-03-12T15:52:03.503Z] ADPCM decode to PCM16LE: disabled
[2026-03-12T15:52:03.503Z] Health check: GET /health
[2026-03-12T15:52:14.035Z] [connect] remote=127.0.0.1:55554 path=/ active=1
[2026-03-12T15:52:14.035Z] [open] remote=127.0.0.1 file=/mnt/vmShared/aows/aows_captures/audio-20260312-155214-033-127.0.0.1-b4b8.pcm
[2026-03-12T15:52:14.035Z] [session] reply_msgs=disabled idle_close_ms=1000
[2026-03-12T15:52:15.957Z] [disconnect] remote=127.0.0.1:55554 path=/ active=0
[2026-03-12T15:52:15.957Z] [close] remote=127.0.0.1 code=1006 reason= bytes=25728 frames=7 file=/mnt/vmShared/aows/aows_captures/audio-20260312-155214-033-127.0.0.1-b4b8.pcm
[2026-03-12T15:52:18.255Z] [connect] remote=127.0.0.1:55566 path=/ active=1
[2026-03-12T15:52:18.255Z] [open] remote=127.0.0.1 file=/mnt/vmShared/aows/aows_captures/audio-20260312-155218-255-127.0.0.1-67f1.pcm
[2026-03-12T15:52:18.255Z] [session] reply_msgs=disabled idle_close_ms=1000
[2026-03-12T15:52:20.313Z] [disconnect] remote=127.0.0.1:55566 path=/ active=0
[2026-03-12T15:52:20.313Z] [close] remote=127.0.0.1 code=1006 reason= bytes=29184 frames=8 file=/mnt/vmShared/aows/aows_captures/audio-20260312-155218-255-127.0.0.1-67f1.pcm
[2026-03-12T15:52:22.277Z] [connect] remote=127.0.0.1:55582 path=/ active=1
[2026-03-12T15:52:22.277Z] [open] remote=127.0.0.1 file=/mnt/vmShared/aows/aows_captures/audio-20260312-155222-276-127.0.0.1-96f1.pcm
[2026-03-12T15:52:22.277Z] [session] reply_msgs=disabled idle_close_ms=1000
[2026-03-12T15:52:25.009Z] [disconnect] remote=127.0.0.1:55582 path=/ active=0
[2026-03-12T15:52:25.009Z] [close] remote=127.0.0.1 code=1006 reason= bytes=28032 frames=8 file=/mnt/vmShared/aows/aows_captures/audio-20260312-155222-276-127.0.0.1-96f1.pcm
^C
d35@d35:/mnt/vmShared/aows$ ls aows_captures/
audio-20260312-155214-033-127.0.0.1-b4b8.pcm  audio-20260312-155218-255-127.0.0.1-67f1.pcm
audio-20260312-155222-276-127.0.0.1-96f1.pcm
d35@d35:/mnt/vmShared/aows$
```

</details>

## 3. Configure RDK DUT to connect to AOWS Server

> Note: Before proceeding with this section, complete [section 4](#4-enable-aowss-via-stunnel) to generate the `aowss.crt` certificate file.

1. Add the self-signed TLS certificate into the DUT trust store.
Copy the `aowss.crt` file generated in [section 4](#4-enable-aowss-via-stunnel) to the DUT, place it at `/usr/share/ca-certificates/aows.crt`, then update certificates using `update-ca-certificates`.
```bash
# Sample details
root@raspberrypi4-64-rdke:~# cp /opt/aows.crt /usr/share/ca-certificates/aows.crt
root@raspberrypi4-64-rdke:~# update-ca-certificates
Updating certificates in /etc/ssl/certs...
0 added, 0 removed; done.
Running hooks in /etc/ca-certificates/update.d...
done.
root@raspberrypi4-64-rdke:~#
```

2. Configure the RDK Voice stack.
Using the VoiceControl plugin, configure control-manager to connect to the secure endpoint `aowss://${SERVERADDRESS}:9443/`, where `SERVERADDRESS` can be obtained from `hostname -I | awk '{print $1}'` on the host. Ensure DUT can access the server with a `ping ${SERVERADDRESS}` test.

```bash
curl -X POST http://127.0.0.1:9998/jsonrpc -d '{"jsonrpc":"2.0","id":301,"method":"org.rdk.VoiceControl.configureVoice","params":{"urlAll":"aowss://10.0.0.35:9443/","mic":{"enable":true}}}'
```

3. Verify the configuration status.
```bash
# Sample response
root@raspberrypi4-64-rdke:~# curl -X POST http://127.0.0.1:9998/jsonrpc -d '{"jsonrpc":"2.0","id":42,"method":"org.rdk.VoiceControl.voiceStatus"}'
{"jsonrpc":"2.0","id":42,"result":{"success":true,"urlMicTap":"aowss:\/\/10.0.0.35:9443\/","capabilities":["PRV"],"urlHf":"aowss:\/\/10.0.0.35:9443\/","maskPii":false,"wwFeedback":false,"mic_tap":{"status":"ready"},"mic":{"status":"ready"},"prv":false,"ff":{"status":"ready"},"urlPtt":"aowss:\/\/10.0.0.35:9443\/","ptt":{"status":"ready"}}}
```

4. Test voice capture.
Pair an RDK voice-supported remote control with the DUT and initiate a voice session by pressing the `PTT` button. Each session should produce an audio capture on the AOWS server, which you can verify using [section 6](#6-playback-captured-audio).

## 4. Enable AOWSS via stunnel

The Node.js server listens on plain websocket (`ws`).
Use stunnel as a TLS terminator to expose secure websocket (`wss`).

1. Generate a certificate and key, then combine into one PEM file:

```bash
sudo mkdir -p /etc/stunnel/certs
sudo openssl req -x509 -newkey rsa:2048 -sha256 -days 3650 -nodes \
   -keyout /etc/stunnel/certs/aowss.key \
   -out /etc/stunnel/certs/aowss.crt \
   -subj "/CN=$(hostname -f)"
sudo sh -c 'cat /etc/stunnel/certs/aowss.key /etc/stunnel/certs/aowss.crt > /etc/stunnel/certs/aowss.pem'
sudo chmod 600 /etc/stunnel/certs/aowss.pem
```

2. Install the provided stunnel config.
Ubuntu 22.04 often does not ship `/etc/stunnel/conf.d` by default, so copy to `/etc/stunnel/aows.conf`:

```bash
sudo cp stunnel/aowss-stunnel.conf /etc/stunnel/aows.conf
```

3. Ensure stunnel4 is enabled in `/etc/default/stunnel4`:

```bash
sudo sed -i 's/^ENABLED=.*/ENABLED=1/' /etc/default/stunnel4
grep -E '^ENABLED=' /etc/default/stunnel4
```

4. Enable and restart stunnel:

Command:

```bash
sudo systemctl enable stunnel4
sudo systemctl restart stunnel4
sudo systemctl status stunnel4 --no-pager
```

<details>
<summary>Sample output</summary>

```text
stunnel4.service is not a native service, redirecting to systemd-sysv-install.
Executing: /lib/systemd/systemd-sysv-install enable stunnel4

● stunnel4.service - LSB: Start or stop stunnel 4.x (TLS tunnel for network daemons)
   Loaded: loaded (/etc/init.d/stunnel4; generated)
   Active: active (running)
```

</details>

5. Use these endpoints:

- Internal server: `ws://127.0.0.1:9880/`
- External secure endpoint: `wss://$HOST:$STUNNELPORT/`

## 5. Test WSS with OpenSSL

1. Export test variables once for your session:

```bash
export HOST="$(hostname -I | awk '{print $1}')"
export STUNNELPORT=9443
```

2. Verify stunnel is listening on the selected port:

Command:

```bash
ss -ltnp | grep "$STUNNELPORT"
```

<details>
<summary>Sample output</summary>

```text
LISTEN 0      4096         0.0.0.0:9443       0.0.0.0:*
```

</details>

3. Verify TLS listener is up:

Command:

```bash
openssl s_client -connect "$HOST:$STUNNELPORT" -servername "$HOST" -brief
```

<details>
<summary>Sample output</summary>

```text
depth=0 CN = d35
verify error:num=18:self-signed certificate
CONNECTION ESTABLISHED
Protocol version: TLSv1.3
```

</details>

4. Verify websocket upgrade over TLS.
Expected response includes `HTTP/1.1 101 Switching Protocols`:

Command:

```bash
printf 'GET / HTTP/1.1\r\nHost: %s:%s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n' "$HOST" "$STUNNELPORT" | \
openssl s_client -connect "$HOST:$STUNNELPORT" -servername "$HOST" -quiet
```

<details>
<summary>Sample output</summary>

```text
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
```

</details>

If you use a self-signed certificate and want validation:

```bash
openssl s_client -connect "$HOST:$STUNNELPORT" -servername "$HOST" -CAfile /etc/stunnel/certs/aowss.crt
```

Example using port `9881`:

```bash
export HOST="$(hostname -I | awk '{print $1}')"
export STUNNELPORT=9881
ss -ltnp | grep "$STUNNELPORT"
openssl s_client -connect "$HOST:$STUNNELPORT" -servername "$HOST" </dev/null
```

## 6. Playback Captured Audio

Raw capture files are written for each session. A quick listen attempt for 16-bit mono 16 kHz data:

```bash
aplay -f S16_LE -r 16000 -c 1 audio-YYYYMMDD-HHMMSS-mmm-raw-s16le.pcm
```

Decoded output (only when `AOWS_DECODE_ADPCM=true`) is written as:

```text
audio-YYYYMMDD-HHMMSS-mmm-adpcm-decoded-s16le.pcm
```
