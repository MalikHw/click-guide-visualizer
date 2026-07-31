# Click Guide Visualizer

A Geometry Dash 2.2 mod that loads a macro and draws its upcoming inputs onto
the level ahead of you, so you can see where the next click lands and how many
frames off your own timing was.

It does not play the level for you. It shows you a macro's clicks as part of the
level, the way the level's own geometry is part of the level.

## Installing

Install through the Geode mod index, or drop
`you.click-guide-visualizer.geode` into your `geode/mods` folder.

Requires Geode 5.8.2 and GD 2.2081. Windows, macOS, Android and iOS are
supported.

## Loading a macro

1. Start a level and pause.
2. Press the round hammer button in the pause menu.
3. Press **Import Macro** and pick your file.

The file is copied into the mod's own folder, so it appears in the list from
then on and you never need the system file dialog for it again.

The library lives at:

```
geode/config/you.click-guide-visualizer/macros/
```

**Folder** opens that directory on desktop, or shows you the path on Android.
Anything you drop in there by hand shows up in the list too.

### Supported formats

| Format | Extension |
| --- | --- |
| GDR2 | `.gdr2` |
| GDR (legacy) | `.gdr` |
| MegaHack replay | `.mhr` |
| Echo, zBot, xdBot | `.json` |

The format is detected from the file's contents rather than its extension, so a
macro that was saved with the wrong extension still loads. A corrupt or
truncated file gives you an error message, not a crash.

## Rhythm mode

By default the guide is drawn *into the level*, so bands sit on the geometry and
scroll past with it. Rhythm mode replaces that with a single wide column pinned
to the middle of your screen, like a one-key osu!mania chart: notes fall from the
top toward a stationary judgement line, and you press when a note lands on it.
The level keeps playing behind it.

Turn it on with **Rhythm Mode** in the settings.

```
        |          |
        |  ######  |   <- notes fall down
        |          |
        |  ######  |
        |          |
      ==|##########|==  <- judgement line (fixed, screen middle)
        |          |
        |          |
```

Notes span the full width of the column, and so does the judgement line, so a
note sits exactly on the line at the moment you should press. The line flashes
on every press. Timing labels appear beside the column.

Settings worth tuning:

| Setting | Does what |
| --- | --- |
| **Lead Time** | How many seconds ahead notes appear. Lower = faster fall, less warning. Default 2s. |
| **Judgement Line Height** | Where the line sits vertically, 0 = bottom, 1 = top. Default 0.5, the middle of the screen. |
| **Lane Position** | Where the column sits horizontally. Default 0.5, centred. |
| **Lane Width** | How wide the column is, in pixels. Default 220. |
| **Note Thickness** | How tall each note bar is. Width always matches the column. |
| **Lane Opacity** | How visible the column background is. |
| **Hide In-Level Guide** | On by default. Turn off to keep the in-level bands *as well as* the column. |

A note on the default line height: at 0.5 the notes only have the top half of
the screen to fall, so you get about half the reading time you would in real
mania, where the bar sits near the bottom. If notes feel like they arrive too
fast, either raise **Judgement Line Height** or raise **Lead Time**.

Holds appear as a bar extending upward from the note head; taps are a single
bar. Player 2 notes in dual sections are drawn in orange, inset slightly
narrower than player 1 so overlapping notes stay distinguishable.

## Reading the overlay

| What you see | What it means |
| --- | --- |
| Coloured vertical band | A hold — starts at the press, ends at the release |
| Thin sliver | A tap |
| White line inside a band | The exact press instant |
| White line through your player | Where you are, to line up against the bands |
| Small white box | A press point, drawn level with the player |
| Orange band or box | The same, but for player 2 in a dual section |
| Floating `-2f` / `+3f` | How early (`-`) or late (`+`) your click was |

Accuracy labels are colour-coded: green within 1 frame, yellow within 4, red
beyond that.

Bands are clipped as you pass through them, so the part behind you disappears
progressively rather than the whole band vanishing at once.

## Auto-alignment

Macros store frame numbers, not positions. If a macro was recorded from a
different StartPos, everything would land in the wrong place.

Instead of asking you to dial in an offset, the mod measures it. Play normally;
after about six clicks it locks on and rebuilds the guide. You will see this in
the log:

```
Alignment locked: macro offset 137 frames (0.57s) from 8 votes
```

The popup's status line also shows it live, so you can tell the difference
between "still working it out" and "gave up":

```
mymacro.gdr2 - 412 inputs @ 240 FPS - aligning (3/6)
mymacro.gdr2 - 412 inputs @ 240 FPS - aligned +137f
```

### How it decides

Every click you make is compared against *every* macro click within the search
window, and each pairing is one candidate offset. Those candidates are binned
into a histogram, and the heaviest bin wins — but only if at least six
*different* clicks of yours support it. One frantic click cannot outvote the
rest.

This matters because the obvious approach does not work. Matching each click to
its nearest macro click only ever finds offsets near zero, since "nearest"
is measured from the offset you are trying to discover. Voting has no such blind
spot: it recovers offsets of several seconds in either direction just as
reliably as small ones.

If your clicks genuinely have nothing to do with the macro, no bin gets enough
support and it stays unlocked rather than inventing an answer.

It re-measures on every attempt, since your StartPos can change. If you would
rather it never did this, turn off **Auto Align**.

## Settings

Open them from the popup's **Settings** button, or from Geode's mod list.

**Visuals** — the master toggle, guide colour, and separate opacity controls for
bands, fills, borders and lines. Fill and border opacity are independent, so you
can have a barely-there fill with a crisp outline. Also here: click markers,
accuracy labels, the player line, and marker row height.

**Gamemodes** — one toggle per gamemode. Turn the guide off in the modes where
you do not want it, for example wave.

**Tuning** — player line thickness, marker size, and whether to also mark
releases.

**Macro** — the macro file, auto-align, auto-loading the last macro on startup,
and debug logging.

### Heights

Most macros carry no Y data. The mod resolves marker height in this order: a Y
stored in the macro, then a forward physics prediction, then a height it saw on
an earlier run at that spot, and finally your current height.

**Learn Trajectory** (on by default) is the cheap one — it remembers where you
actually were and reuses that on later runs.

**Predict Heights** (off by default) simulates physics forward about two
seconds. It is noticeably more expensive and does not know about pads, portals
or orbs, so treat it as a hint. **Show Predicted Path** draws that projection as
a line.

## Performance

The guide is designed to be close to free at runtime. Frame-to-position mapping
is computed once when the macro loads, settings are cached so the drawing code
never looks one up, and only the handful of bands near the camera are drawn.

If you are on a low-end device and want to shave off more, leave **Predict
Heights** off and reduce **Cull Padding** to 0.

## Building from source

Requires the Geode SDK 5.8.2 and CLI, plus CMake and Ninja.

```bash
export GEODE_SDK=/path/to/geode
cmake -B build -G Ninja
cmake --build build
```

To build all three targets and package them into a single `.geode`:

```bash
export GEODE_SDK=/path/to/geode
export ANDROID_NDK=/path/to/android-ndk
export XWIN_DIR=/path/to/xwin-splat
./build-all.sh
```

`BUILD_WINDOWS=0` and `BUILD_ANDROID=0` skip either side.

Cross-compiling Windows from Linux needs `clang-cl`, `lld-link` and an xwin
splat; no Visual Studio required. See `ARCHITECTURE.md` for the details and the
traps involved.

## Credits

GDR parsing uses [GDReplayFormat](https://github.com/maxnut/GDReplayFormat) by
maxnut.
