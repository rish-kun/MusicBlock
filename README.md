# MusicBlock experiment

MusicBlock is a faceless macOS app that stays asleep under the bundle ID
`com.apple.Music`. It tests whether a C process with `LSBackgroundOnly` prevents
the system Music app from opening when Play is pressed. It does not redirect
media controls or start at login.

## Build and run

On an Apple Silicon Mac with Command Line Tools installed:

```sh
sh build.sh
open "$(pwd)/build/MusicBlock.app"
```

Launch the bundle by **path**, not with `open -b com.apple.Music`, because the
system Music app has the same bundle ID. To find and quit this build:

```sh
pgrep -fl '/MusicBlock.app/Contents/MacOS/MusicBlock'
musicblock_pid=$(pgrep -f '/MusicBlock.app/Contents/MacOS/MusicBlock' | head -n 1)
kill "$musicblock_pid"
```

After an edit, quit the running build, run `sh build.sh`, then open the bundle
again. There is no login item, installer, or fallback implementation.

Sharing Music's bundle ID may interfere with intentionally opening or scripting
Music while MusicBlock runs. Quit MusicBlock to restore the normal behavior.

## Verification procedure

1. With MusicBlock stopped, quit Music and stop other players. Press the Play
   control that normally triggers Music. Confirm Music opens; otherwise the
   behavior experiment is inconclusive.
2. Quit Music, launch MusicBlock through `open` by path, and find its PID.
   Confirm the process remains alive. Press the same Play control three times.
   The behavior check passes only if the system Music process stays closed.
3. After MusicBlock has been idle for at least 30 seconds, run the following
   commands. The memory check passes only if the **physical
   footprint** reported by `footprint` is at most 2 MB. RSS is recorded
   separately and is not the pass criterion.

```sh
plutil -lint build/MusicBlock.app/Contents/Info.plist
codesign --verify --verbose=2 build/MusicBlock.app
file build/MusicBlock.app/Contents/MacOS/MusicBlock
otool -L build/MusicBlock.app/Contents/MacOS/MusicBlock
musicblock_pid=$(pgrep -f '/MusicBlock.app/Contents/MacOS/MusicBlock' | head -n 1)
ps -o pid,rss,%cpu,comm -p "$musicblock_pid"
footprint "$musicblock_pid"
vmmap -summary "$musicblock_pid"
top -l 1 -pid "$musicblock_pid" -stats pid,threads,cpu,mem
pgrep -fl '/System/Applications/Music.app/Contents/MacOS/Music'
```

## Result on this Mac

Tested on 2026-09-29, macOS 27.0 (26A428), Apple Silicon (`arm64`). The build,
Launch Services launch, memory check, and Play-control baseline passed. The
three-press blocker check **failed**: Music opened while MusicBlock was
running. Overall result: **failed**.

Commands run:

```sh
./build.sh
plutil -lint build/MusicBlock.app/Contents/Info.plist
codesign --verify --verbose=2 build/MusicBlock.app
file build/MusicBlock.app/Contents/MacOS/MusicBlock
otool -L build/MusicBlock.app/Contents/MacOS/MusicBlock
open "$PWD/build/MusicBlock.app"
pgrep -fl '/MusicBlock.app/Contents/MacOS/MusicBlock'
# After more than 30 seconds idle, using the observed PID:
ps -o pid,rss,%cpu,comm -p 46446
footprint 46446
vmmap -summary 46446
top -l 1 -pid 46446 -stats pid,threads,cpu,mem
kill 46446
# With MusicBlock stopped, press the physical Play control once.
open "$PWD/build/MusicBlock.app"
pgrep -fl '/MusicBlock.app/Contents/MacOS/MusicBlock'
lsappinfo list | rg -i 'musicblock|MusicBlock.app'
# With Music closed, press the same physical Play control three times.
kill 53105
```

Observed: the bundle launched as PID 46446 and remained alive; the plist and
signature validated, `file` reported a thin `arm64` Mach-O, and `otool -L`
listed only `/usr/lib/libSystem.B.dylib`. After idle, `footprint` and `vmmap`
reported **1,152 KB physical footprint** (peak 1,184 KB), below the 2 MB cap.
`ps` reported **1,600 KB RSS** and **0.0% CPU**; `top` reported **one thread**,
0.0% CPU, and 1,152 KB memory. MusicBlock was then stopped for the baseline
behavior test. With MusicBlock stopped and Music initially closed, pressing the
physical Play control opened Music. After Music was closed, MusicBlock was
launched again as PID 53105; `lsappinfo list` showed the MusicBlock bundle
registered with Launch Services. The user pressed the same Play control three
times and observed the system Music app open. MusicBlock was still running
when checked afterward; Music had already been closed before the process
check. MusicBlock was then stopped. This macOS version did not treat the bare
C background bundle as a sufficient blocker in this test.
