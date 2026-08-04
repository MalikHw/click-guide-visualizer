# Click Guide Visualizer - Dev Log

## v0.4.3

**Made the text readable.**

Everything in the macro popup used chatFont, which is the thin font GD uses for chat messages and descriptions. At the small sizes it was drawn at, people said it was hard to see, and they were right.

The macro names, the status line and the empty message now use bigFont, the heavy font GD uses for anything that matters. The early and late numbers you see while playing are bigger too.

Long macro names shrink to fit instead of running underneath the Load button. That is handled by the same width limiting GD uses for its own labels rather than a guess at how wide a character is, so it holds up for names the mod has never seen.

## v0.4.2

**Gave the title room to breathe.**

The gap between the title and the button row was 4.7 pixels. Geode centres the title itself, so it was never off to one side, but a long title at that spacing would have touched the buttons. The row moved down 12 pixels and the title is slightly larger, giving 13 pixels of clearance. The list moved down to match so the spacing stays even top to bottom.

## v0.4.1

**Rebuilt the macro popup.**

The old one was a mess. The title and the button row were both pinned near the top of a 260 pixel popup and sat 8 pixels apart, so "Click Guide" was drawn straight through the Folder button. The list underneath was positioned with an offset meant for a bottom left anchor while the anchor had been changed to centre, so it sat off to one side and ran close to the frame edge.

The popup is now 420 by 290 instead of 380 by 260. The four buttons are spaced evenly across the width and sit 24 pixels clear of the title. The list is properly centred with a 26 pixel margin on both sides, rows alternate shading so they are easier to scan, and the already loaded macro shows a disabled Loaded button instead of one that looks clickable but does nothing.

The status line was one long run of text that overflowed the popup. It is shorter now and says "lining up 3/6" and "lined up (+137f)" rather than "aligning" and "aligned", since nobody outside the project knew what alignment meant.

Also split the row drawing out of the list rebuild into its own function. One function doing the scroll layer, the empty state and every row is what let the positioning bugs hide.

## v0.4.0

**Fixed the early/late number not showing up.**

A user reported it never appearing on StartPos runs, and only turning up partway through when playing from the start. Both were the same bug.

The mod spends the first several clicks working out how far off the macro is. During that time it was throwing those clicks away instead of also judging them, so no number appeared until it had locked on, which takes about six clicks. From the start of a level that meant the first six clicks were silent. On a StartPos it was worse, because the measurement restarts every attempt and short StartPos runs often die before six clicks, so it never locked and the number never appeared at all.

Clicks are now judged straight away. Numbers shown before the mod has finished lining up are drawn fainter so you can tell they are not confirmed yet.

There is a guard on this. If the macro is badly out of line, an early number would be meaningless, so it stays hidden until at least two clicks agree with each other to within 12 frames. Tested against a StartPos 30 seconds into a level, where the early readings would have been nonsense: correctly shows nothing rather than lying.

**Renamed every setting.**

People said the names were hard to read, and they were right. There were five different settings all called something Opacity, plus Cull Padding, Marker Row, Line Lead (units) and Learn Trajectory.

Everything now says what it does in plain words. Opacity is Fade, Cull Padding is Draw Distance, Show Accuracy is Show How Early Or Late, Learn Trajectory is Learn From Your Runs. Section headers too: Visuals is Look, Tuning is Sizes, Gamemodes is Turn On For.

Your saved settings are not affected. Only the labels changed.

## v0.3.2

**Fixed rhythm mode notes overlapping each other and the hold bodies.**

Note thickness was a fixed pixel height that had nothing to do with how fast notes were moving. At the default settings a note is 22 pixels tall and the lane scrolls at 80 pixels per second, so every note occupied 0.275 seconds of lane. Any two clicks closer together than that literally drew on top of each other. That is under 4 clicks per second, which normal gameplay passes constantly, so in a burst the notes fused into one solid block.

Notes now measure the gap to their nearest neighbour and shrink to fit, using at most 62 percent of the available spacing. Spread out notes look exactly as before; dense bursts stay readable as separate bars. The setting is renamed Max Note Thickness because that is what it now means.

Below about 38 milliseconds apart, roughly 27 clicks per second, notes hit a minimum size and will still touch. That is deliberate. At that density you cannot read individual notes anyway.

**The hold body was drawn straight through its own note head.**

The head is centred on the note time and the body started at the same point, so the bottom half of every head was covered twice. Both are semi transparent, so the overlap blended into a brighter band, and since the body is 28 percent narrower than the head you saw a wide block with a narrower bar growing out of its middle. The body now starts at the top edge of the head so the two sit flush.

Also split the note drawing into separate head and body functions while fixing this, since one function doing both is what allowed the seam to be wrong without it being obvious.

## v0.3.1

**Fixed the macro drifting further out of sync the longer you play.**

Several people reported the guide getting progressively more offset every second. That is a different bug from a constant offset. A constant offset means the macro started in the wrong place. Drift that grows means the mod had the wrong idea of how many frames per second the macro was recorded at, so every conversion from frame numbers to real time was stretched slightly, and the error piled up.

Two causes, both fixed.

The first was how the mod guessed a macro's framerate when the file did not state one. It divided the last input's frame number by the replay's total duration. Duration covers the whole recording, including any dead time after the final click. So a 60 second run with 20 seconds of silence on the end derived 180 fps instead of 240, and the guide slid by about 20 seconds' worth over a minute of play. It now only accepts a derived rate if it lands within 3 percent of a known rate, and falls back to 240 rather than trusting a bad guess.

The second was the snapping itself. It accepted anything within 6 percent of a common rate and took the first match rather than the closest, so 980 snapped to 960 instead of 1000. Tightened and corrected.

**Added drift detection as a backstop.**

Fixing the guesses is not enough, because a macro can simply declare a rate that does not match how it was actually recorded. The mod now watches your timing error after it locks on. If the error trends steadily in one direction over at least 4 seconds and 8 clicks, it concludes the rate is wrong, corrects it, and remeasures. Random noise around zero does not trigger it, and corrections beyond 25 percent are refused rather than letting a bad reading make things worse.

Tested with 20 framerate cases and 18 drift cases, all passing.

**Also removed a dead frame counter.** The input hook was computing a frame number for every click that nothing ever read. Harmless, but it used the uncorrected framerate, so it would have become quietly wrong once drift correction kicked in.

## v0.3.0

**Three bug fixes.**

You could not change the colour of the vertical bars. There was only one colour setting in the whole mod, and it only covered player 1. Player 2 was hardcoded orange, the markers and the judgement line were hardcoded white. There are now five colour settings: Guide Colour, Player 2 Colour, Hold Colour, Marker Colour and Judgement Line Colour. No colour is hardcoded anywhere anymore.

You could not unload a macro once it was loaded. There was genuinely no way to do it, no button and no code path, so the only way out was deleting the file. Added an Unload button next to Import and Folder. It greys out when nothing is loaded, and the unload sticks across restarts instead of the macro coming straight back.

Lowering the hold opacity erased everything. The setting you were reaching for was Fill Opacity, which multiplied every filled shape in the mod: hold bodies, note heads and the in-level bands all at once. At zero they all vanished together and left just outlines. Hold Opacity is now its own setting that only touches the hold body.

## v0.2.0

**Auto-alignment rewritten. It never actually worked before.**

Macros store frame numbers, not positions, so if a macro was recorded from a different StartPos the whole guide lands in the wrong place. The mod is supposed to measure that offset for you while you play.

The old version matched each of your clicks to the nearest macro click and averaged the differences. Problem: "nearest" is measured from the offset it is trying to find, which starts at zero. So it only worked when the macro was already aligned. Any real offset and each click snapped to a different wrong macro click, the numbers scattered, and it never locked. Tested it before changing anything: offsets of 30, 120, 480 and -200 frames all failed. One accidental success out of five cases.

New approach is voting. Every click you make is compared against every macro click in range, each pairing counts as one candidate offset, and the candidates go into a histogram. Heaviest bin wins. Now recovers all 12 test offsets, worst error 4.5 frames, including a sloppy player with 9 frames of jitter either way.

Added a guard so it cannot align to nonsense: at least 6 different clicks must agree. Tested against random clicking, 0 false locks out of 5 runs. First version of the guard counted raw votes instead of distinct clicks, so one click in a busy part of the macro could outvote everyone. The test caught it, fixed. 24 of 24 assertions passing.

---

**Macro file setting was dead.**

There was a "Macro File" setting in the options, but no code anywhere read it. You could pick a file and nothing would happen. It was also using a setting type Geode has deprecated. Rewired properly, so picking a file now loads it straight away and remembers it for next launch. Tells you if the file has been moved instead of failing silently.

**You can now see what alignment is doing.**

Part of why the above was hard to notice is that nothing showed you the state. Status line now reads:

```
mymacro.gdr2 - 412 inputs @ 240 FPS - aligning (3/6)
mymacro.gdr2 - 412 inputs @ 240 FPS - aligned +137f
mymacro.gdr2 - 412 inputs @ 240 FPS - not aligned
```

---

## v0.1.0

**Rhythm mode is now a vertical osu!mania style column.**

Was a horizontal lane with notes scrolling sideways into a vertical hit line. Now it is a single wide column down the middle of the screen, notes falling from the top onto a horizontal judgement line.

- Notes span the full width of the column
- Judgement line spans the full width too, so a note sits exactly on it at the moment you should press
- Holds extend upward from the note head instead of trailing sideways
- Hit animation recentred, ring size now keyed off the lane width, bursts at the note's actual position so a late click visibly bursts below the line
- Player 2 notes are inset narrower instead of vertically offset, since there is no vertical room in a falling lane

New settings: Lane Position, Lane Width (default 220px), Judgement Line Height (default centre screen), Note Thickness. The four old lane settings are gone.

Worth knowing: at the default centre position notes only have the top half of the screen to fall, so you get about half the reading time of real mania where the bar sits near the bottom. Raise Judgement Line Height or Lead Time if it feels rushed.

**Version numbering reset.**

Was tagged 1.0.0 before it had ever been run in the game, which was overconfident. Restarted at 0.1.0. 1.0.0 gets tagged when it has actually been played and the settings are settled.

**Added a logo and fixed a packaging bug.**

Turns out about.md was never committed to the repo, it only existed inside an old local build. Every CI build was shipping with no description page and nobody noticed. Recovered it and added a CI check so it cannot silently vanish again.

---

## Current state

Working and confirmed in game: hit animation, which also proves the render path, the hooks and the pause menu UI are all sound.

Built and tested outside the game but never run in it: world guide, macro parsing, rhythm mode, level based note generation, assist buffering, line lead.

Known limitations: the note generator does not handle pads, orbs or dual sections. Flight mode notes for ship, wave, UFO and swing are corridor following guesses and get unreliable in tight sections, wave especially.

Contains an optional cheat (Assist Mode input buffering), off by default behind a master switch.

Credit to maxnut/GDReplayFormat for the GDR2 parser.
