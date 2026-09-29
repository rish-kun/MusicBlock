# MusicBlock

MusicBlock is a background-only macOS app built in C. It
checks in with the Process Manager under the bundle ID `com.apple.Music`, then
sleeps on `pause()`. On the tested Mac, pressing Play while it runs does not
leave the system Music app open. The Music icon still bounces briefly in the
Dock. It does not redirect media controls. On the tested Mac, the installed
copy is registered to open at login.

The check-in uses `GetCurrentProcess()`. Apple's SDK describes this as forcing
Process Manager check-in, but marks the API deprecated since macOS 10.9. This
is an observed workaround on macOS 27, not a documented `rcd` contract.

## Build

On an Apple Silicon Mac with Xcode or Command Line Tools:

```sh
./build.sh
```

The deprecation warning during build is expected. Launch by **bundle path**;
`open -b com.apple.Music` is ambiguous because the system Music app has the
same bundle ID. To find and stop MusicBlock:

```sh
pgrep -fl '/MusicBlock.app/Contents/MacOS/MusicBlock'
musicblock_pid=$(pgrep -x MusicBlock | head -n 1)
kill "$musicblock_pid"
```

Quit the running app before replacing its bundle. While MusicBlock runs, sharing
Music's bundle ID may interfere with intentionally opening or scripting Music.
Quit MusicBlock when you want to use Music normally.

## Installed copy on this Mac

The signed bundle is installed at `~/Applications/MusicBlock.app`. System
Settings → General → Login Items & Extensions lists **MusicBlock.app** under
**Open at Login**. `sfltool dumpbtm` reports this entry enabled and points to
the installed path. The source build alone does not register a login item.

To update the installed copy after changing the source, quit MusicBlock, then:

```sh
./build.sh
ditto build/MusicBlock.app "$HOME/Applications/MusicBlock.app"
codesign --verify --verbose=2 "$HOME/Applications/MusicBlock.app"
open "$HOME/Applications/MusicBlock.app"
```

The login item keeps referring to the same installed path.

## Verify on another Mac

1. With MusicBlock stopped, close Music and other players. Press Play once to
   confirm that this control opens Music on that Mac. Close Music again.
2. Build and open MusicBlock by path. Confirm its process stays alive. Press
   the same Play control three times and check whether Music stays open,
   briefly bounces in the Dock, or remains absent.
3. After at least 30 seconds idle, run:

```sh
plutil -lint build/MusicBlock.app/Contents/Info.plist
codesign --verify --verbose=2 build/MusicBlock.app
file build/MusicBlock.app/Contents/MacOS/MusicBlock
otool -L build/MusicBlock.app/Contents/MacOS/MusicBlock
musicblock_pid=$(pgrep -x MusicBlock | head -n 1)
ps -o pid,rss,%cpu,comm -p "$musicblock_pid"
footprint "$musicblock_pid"
vmmap -summary "$musicblock_pid"
top -l 1 -pid "$musicblock_pid" -stats pid,threads,cpu,mem
pgrep -x Music || true
```

`footprint` physical footprint is the memory metric used here; RSS includes
shared resident pages and is reported separately. Measure again after Play,
because the process may grow when macOS routes the event.

## Results on this Mac

Tested 2026-09-29 on Apple Silicon, macOS 27.0 (26A428). The baseline Play
press opened Music with MusicBlock stopped. Each running variant was opened
through Launch Services by path; the user pressed the same Play control three
times. Footprints were recorded after at least 30 seconds idle unless marked
"after Play."

| Variant | Commit | Play result | Physical footprint |
| --- | --- | --- | ---: |
| C, `pause()` only, `LSBackgroundOnly` | `8a9b8dd` | Music stayed open | 1,152 KB |
| AppKit `NSApplicationMain` | `2c9791d` | Music stayed closed; Dock bounce | 7,969 KB idle |
| C, `pause()` only, `LSUIElement` | `1e2c2e8` | Music stayed open | Not measured |
| C, `GetCurrentProcess()`, ApplicationServices | `396bb3e` | Music stayed closed; Dock bounce | 4,433 KB idle |
| Same call, direct HIServices link | `aee2429` | Music stayed closed; Dock bounce | 4,385 KB idle |
| C, CoreFoundation run loop | `87a7e89` | Music stayed open | 1,680 KB |
| C, dynamic HIServices load then unload | `c390503` | Music stayed closed; Dock bounce | 2,321 KB before Play; 4,417 KB after Play |
| C, `TransformProcessType()` only | `5ee0725` | Exited with status 1 | Not measured |

The current build uses the ApplicationServices `GetCurrentProcess()` variant.
Its plist and ad-hoc signature validate, `file` reports a thin `arm64` Mach-O,
and `otool -L` lists ApplicationServices and libSystem. The measured idle CPU
was 0.0%. The strict original target of **Music never attempting to open and
physical footprint ≤2 MB** was not met. The practical result is that Music
does not remain open, at about **4.4 MB** physical footprint with a brief Dock
bounce. The dynamic-load variant's lower initial footprint did not persist
after Play.

Music Decoy's author describes the `rcd` bundle-ID behavior in its
[README](https://github.com/FuzzyIdeas/MusicDecoy). The
[AntiMusic comparison](https://nift4.org/2023/12/09/antimusic/) also reports a
Dock bounce with Music Decoy. Neither source is an Apple guarantee for future
macOS releases.
