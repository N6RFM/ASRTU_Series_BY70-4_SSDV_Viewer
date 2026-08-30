# ASRTU SSDV Viewer

A small standalone tool that connects as a TCP client to an
externally-decoded telemetry frame source (for example a GNU Radio
flowgraph feeding frames out through a `network_socket_pdu` TCP
server) and reassembles any SSDV image data it finds into JPEGs,
without needing to run a local audio/IQ receiver at all.

This project is built directly on top of
[ASRTU_Series_Receiver](https://github.com/BG7ZDQ/ASRTU_Series_Receiver)
by BG7ZDQ, reusing its `SsdvReceiver` and `SsdvImageWindow` components
so it automatically inherits whatever satellites those already
recognise — currently ASRTU-1, BY70-4, and JAMX01, auto-detected per
packet with no manual mode switch required. See *Credits* below for
the full lineage.

## Features

- Connects as a TCP client to any source of raw, fixed-length (223
  byte) telemetry frames on virtual channel 1
- Live SSDV image reconstruction with a gallery of every image
  received in the session, progress/packet-loss indicators, and a
  "clear" action
- Auto-detects which satellite a given image belongs to per packet,
  including correctly separating ASRTU-1 from JAMX01 even though they
  share the same spacecraft ID (see *Credits* below for why this
  works)
- Configurable output directory (persisted across runs), with the
  detected satellite name included directly in each saved image's
  filename

## Building

Requires CMake 3.20+, a C and C++17 compiler (e.g. GCC or Clang), and
Qt5's Core, Gui, Widgets, and Network modules with development
headers.

On Debian/Ubuntu (and derivatives like Linux Mint):

```
sudo apt install cmake ninja-build build-essential qtbase5-dev qtbase5-dev-tools
```

`ninja-build` is only needed if you use the `-G Ninja` generator shown
below; CMake's default "Unix Makefiles" generator works too if you
drop that flag (and use `make` instead of `ninja` if you invoke the
build system directly, though `cmake --build` below hides that
difference either way).

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --parallel
```

The resulting binary is `build/ASRTU_SSDV_Viewer`. Before running it,
the English/Japanese translation files (`assets/translations/*.qm`)
need to sit in a `translations/` folder next to it:

```
mkdir -p build/translations
cp assets/translations/*.qm build/translations/
./build/ASRTU_SSDV_Viewer --language=en
```

Alternatively, `cmake --install` places the binary and its
translations in the correct relative layout for you automatically:

```
cmake --install build --prefix /wherever/you/want
/wherever/you/want/bin/ASRTU_SSDV_Viewer --language=en
```

## Installing system-wide

To make `ASRTU_SSDV_Viewer` runnable by name from any directory,
install it to CMake's default prefix (`/usr/local` on Linux, already
on the standard `PATH` for normal user accounts) instead of a custom
one:

```
sudo cmake --install build
```

This places `/usr/local/bin/ASRTU_SSDV_Viewer` and
`/usr/local/bin/translations/*.qm`. From then on, from anywhere:

```
ASRTU_SSDV_Viewer --language=en
```

Note that the app's saved settings and default image save directory
live under `~/.local/share/ASRTU/ASRTU_SSDV_Viewer` regardless of
where the binary itself is installed, since that's based on the Qt
application name rather than the install location.

To uninstall, since CMake doesn't track installed files by default,
remove them manually:

```
sudo rm /usr/local/bin/ASRTU_SSDV_Viewer
sudo rm -rf /usr/local/bin/translations
```

(worth a quick look inside that `translations/` folder first in case
anything else you have installed also happens to use that same,
fairly generic directory name next to `/usr/local/bin`)

## Usage

```
./build/ASRTU_SSDV_Viewer --language=en
```

The UI's original source strings are Chinese, with English and
Japanese available as translations. **English is the default**
regardless of system locale; `--language=ja` selects Japanese, and
`--language=zh` shows the UI's original, untranslated Chinese source
strings.

Point **TCP Host** / **TCP Port** at wherever your frame source is
listening (e.g. a GRC flowgraph's `network_socket_pdu` block in
`TCP_SERVER` mode), click **Connect**, and any SSDV data on that
stream will start appearing automatically.

The running version is shown both in the window title and as a small
label in the bottom-right corner of the window, taken directly from
this repository's `project(... VERSION ...)` in `CMakeLists.txt` --
useful for confirming which build you're actually running when you
have more than one checkout around.

## License

This project's own code (`apps/ssdv_viewer/`) is released under the
MIT License, matching the terms of the [`LICENSE`](LICENSE) file in
this repository.

Several files here (`libs/demod/ssdv_receiver.*`,
`apps/dsp/ssdv_image_window.*`, `libs/proxy/pmt_frame_decoder.h`) are
reused from BG7ZDQ's MIT-licensed ASRTU_Series_Receiver project
essentially unmodified, and `libs/common/translation.*` with one
small change (English as the default language regardless of system
locale, rather than following it). All of these remain under that
same MIT License and copyright per its terms — see
[`LICENSE`](LICENSE).

The vendored SSDV codec in `third_party/ssdv_dslwp/` is
**GPL-3.0-or-later** (see `third_party/ssdv_dslwp/COPYING`), originally
by Philip Heron and adapted by Daniel Estévez. Linking it into
`ASRTU_SSDV_Viewer` means the **compiled binary as a whole** is subject
to GPL-3.0-or-later terms, regardless of the MIT license covering the
rest of this repository's own source. If you redistribute built
copies of `ASRTU_SSDV_Viewer`, you must also satisfy the GPL's source
availability and notice requirements for the `ssdv_dslwp` component.

## Credits

This tool exists on top of a considerable amount of prior work by
other people. In particular:

- **BG7ZDQ** — author and maintainer of
  [ASRTU_Series_Receiver](https://github.com/BG7ZDQ/ASRTU_Series_Receiver),
  whose `SsdvReceiver`/`SsdvImageWindow` code this tool reuses
  directly. BG7ZDQ is also the author of `ASRTU-1-EN.exe`, the
  original Windows SSDV decoder for ASRTU-1 — comparing its output
  against this project's own decoder was what made it possible to
  confirm the JAMX01 CRC handling below was correct.
- **IzumiChino** — the original Linux port of ASRTU_Series_Receiver
  ([#13](https://github.com/BG7ZDQ/ASRTU_Series_Receiver/pull/13)),
  without which none of this would run outside Windows at all, and
  later hardening of the SSDV decoder against repeated/duplicate
  downlinks.
- **山雨欲来风满楼 / X-MQSI** (`maqingshui@outlook.com`) — author, in
  ASRTU_Series_Receiver's own git history, of JAMX01 SSDV support and
  the SSDV packet-counter-regression fix that this tool builds
  directly on top of. This is the same `maqingshui@outlook.com`
  address used elsewhere in that project's history under the "BG7ZDQ"
  name directly, so this is BG7ZDQ's own alternate git identity rather
  than a separate contributor.
- **Han Zhang** (`doublehan07`) — fixed the PMT/PDU metadata-stripping
  in `libs/demod/frame_monitor.cpp` (upstream, not vendored here) that
  makes the decoder republish frames as clean, fixed-length 223-byte
  payloads rather than metadata-wrapped ones -- the exact assumption
  this tool's own TCP frame parsing relies on -- along with a GCC
  build fix.
- **Xue Zihao / 薛子豪** (`odorajbotoj`) — fixed a raw-data-content bug
  in `apps/proxy/upload_proxy.cpp` (upstream, not vendored here), the
  same file ASRTU_Series_Receiver's TCP frame source support (used by
  JAMX01/ASRTU-1 external-decode workflows) was later added to.
- **Philip Heron** ([fsphil](https://github.com/fsphil/ssdv)) — author
  of the original SSDV protocol and reference codec that all of the
  above (and this tool) ultimately decodes with.
- **Daniel Estévez, EA4GPZ** ([daniestevez](https://github.com/daniestevez/ssdv)) —
  adapted Philip Heron's decoder for DSLWP-B's compact frame format,
  including working out the `0x4EE4FDE1` CRC initial value used for
  that whole family of missions (ASRTU-1 and BY70-4 included); see his
  own writeup for exactly how and why:
  <https://destevez.net/2018/08/first-ssdv-transmission-from-dslwp-b/>.
  JAMX01's `0x6AAAC1C5` CRC initial value in this codebase follows the
  same technique, applied to JAMX01's own onboard callsign/header
  fields instead of DSLWP-B's.
- **Wei Mingchuan, BG2BHC** — author of
  [gr-lilacsat](https://github.com/bg2bhc/gr-lilacsat), and the person
  who originally supplied DSLWP-B's undocumented callsign/header
  details that made EA4GPZ's CRC derivation above possible in the
  first place.

## Authors

This project was written by Bob, N6RFM, working with Claude
(Anthropic) as an AI coding assistant, including the
reverse-engineering and real-frame verification work that confirmed
the already-upstream JAMX01 CRC handling was working correctly end to
end.
