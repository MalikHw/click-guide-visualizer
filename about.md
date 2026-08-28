# Click Guide Visualizer

shows you where the clicks are before you get to them

load a macro and the mod draws its inputs into the level ahead of you, so you can see whats coming and how many frames off you were after each click

it does not play the level for you, it just shows you the timing

## getting a macro in

pause in a level and hit the green button, then either

- **Import** to pick a file off your device
- **Online** to check hyperbolus.net for a macro of the level youre in

online is off by default, turn on **Find Macros Online** in the settings first

reads gdr, gdr2, mhr and json, it works out the format from the file itself so the extension doesnt matter

## two ways to see it

**in the level**, bars and dots drawn onto the level itself that scroll with it, this is the default

**rhythm mode**, a column down the middle of the screen with notes falling onto a line, like a one key mania chart, turn it on with **Use Rhythm Mode**

## reading it

after each click a number pops up telling you how many frames early or late you were, green is basically perfect, orange is close, red is off

it lines itself up on its own, macros store frame numbers not positions so a macro recorded from a different startpos would land in the wrong place, play normally for about 6 clicks and it works out the offset and fixes itself

## settings worth touching

- **Note Thickness**, thin lines show the exact frame, thicker is easier to see
- **How Far Ahead**, how much of the level you can see coming
- **Main Colour**, and separate colours for player 2, holds, dots and the hit line
- **Turn On For**, pick which gamemodes it shows up in
- **Show How Early Or Late**, the frame counter

## assist mode

theres an optional cheat that holds an early click and fires it on the right frame, its off behind **Turn On Cheats** and it stays off unless you go looking for it

it cannot save a late click, the physics already happened by then

## known issues

- ios sometimes wont load it, thats a geode launcher signing thing not the mod, force close and reopen

## credits

gdr parsing uses [GDReplayFormat](https://github.com/maxnut/GDReplayFormat) by maxnut

online macro search uses [hyperbolus.net](https://hyperbolus.net)
