# IPALauncher

C++17 launcher application that plays a manifest URL through `PlayerInstanceAAMP`.

## Structure

- `source/` - application source files

## Build

`IPALauncher` expects the sibling `lib32-aamp` project to be present and buildable.

```bash
cmake -S . -B build
cmake --build build
```
## Testing the player
Step 1 : In the box create a window client
```
root@ipstb-brcm974116sff:~# curl -s -X POST 'http://127.0.0.1:9998/jsonrpc' -d ' {"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKWindowManager.createDisplay", "params": {"clientId":"mywindow","displayName":"wst-mywindow","focus":true}}';echo
{"jsonrpc":"2.0","id":3,"result":null}
root@ipstb-brcm974116sff:~#
```
step 2: Launch the app
```
root@ipstb-brcm974116sff:~# export GST_DEBUG=3
root@ipstb-brcm974116sff:~# export XDG_RUNTIME_DIR=/tmp
root@ipstb-brcm974116sff:~# IPALauncher
```
Connect it using websocat from external device (or from box)
```
websocat ws://10.0.0.11:10101
//Open session
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.player.openSession", "params":{"instanceId":"123-1232-321312-3123","displayId":"wst-mywindow"}}
{"id":"123454","jsonrpc":"2.0","result":{"sessionId":"session_1783267426","status":true}}

//Play a manifest
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.player.play", "params":{"url":"http://dash.akamaized.net/dash264/TestCases/1a/qualcomm/1/MultiRate.mpd","sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Content playback started.","status":true}}

//Stop playback
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.player.stop", "params":{"sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Playback stopped successfully.","status":true}}

//Close the session
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.player.closeSession", "params":{"sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Session closed successfully.","status":true}}

//Invalid playback
{"jsonrpc":"2.0", "id":"123454","method":"org.rdk.player.play", "params":{"url":"http://dash.akamaized.net/dash264/TestCases/1a/qualcomm/1/MultiRate.mpd","sessionId":"session_1783267426"}}
{"id":"123454","jsonrpc":"2.0","result":{"message":"Session is not initialized.","status":false}}
```
You can exit the player using a kill signal.
```
root@ipstb-brcm974116sff:~# ps -eaf|grep IPA
root     23377  4751  0 16:03 pts/0    00:00:05 IPALauncher
root     31710  7129  0 16:23 pts/1    00:00:00 grep IPA
root@ipstb-brcm974116sff:~# kill -15 23377
root@ipstb-brcm974116sff:~#
```