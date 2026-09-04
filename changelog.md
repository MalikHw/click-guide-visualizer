# changelog

### v0.10.0
- **Auto Updater**: at first time you open the menu, it will check if is there any update, if there is it will prompt you to install it from within the game

## v0.9.2

- **new Special Thanks button** in the pause menu popup, everyone who helped me build this and everyone who uses it
- the button row got a bit tighter to fit it, six buttons instead of five

## v0.9.1

- **fixed the wave path being upside down after a gravity portal**, holding flips you down when youre upside down, the line was still drawing it as up
- mirror mode, rotated gameplay and reverse already worked, the guide is attached to the level itself so it flips and turns with it

## v0.9.0

- **new setting, Show Wave Path**, on wave the mod now draws the actual line the macro takes, the whole zigzag, so you can see where the wave is meant to go before you get there instead of just seeing where the clicks are
- it reads the holds and releases straight out of the macro so the shape is exactly what the macro does, not a guess
- works in dual, each player gets its own line
- mini wave is handled, the slope doubles
- speed portals are handled, the line follows the real speed at that point in the level
- **fixed the guide using the wrong speed in normal levels**, it was reading the platformer speed field which is only set in platformer mode, so every normal level silently fell back to a fixed 1x guess, it now measures the real speed from the level itself

## v0.8.1

- **fixed the editor getting stuck on loading**, the hotkeys were listening everywhere, not just in levels, so Q and E in the editor were making the mod scan your macro folder and load a file while the editor was still opening, they only work during gameplay now and pass straight through everywhere else
- **fixed the guide being wrong in levels with reverse portals**, everything assumed you always move left to right, so in a reversed section the bands were drawn back to front and the mod hid the ones you were actually heading towards, thanks viperi for reporting it on limbo

## v0.8.0

- **removed the level scanner**, the mod no longer tries to work out clicks from the level itself, it only shows macros now, the scanner never handled pads, orbs or duals and the flight mode guesses were unreliable
- three settings are gone with it, Make Notes Without A Macro, Starting Speed, and their heading
- release posts can ping a role now

## v0.7.4

- **fixed high framerate macros putting the lines in completely the wrong places**, a macro recorded above 720tps that did not state its rate was being read as 240, which stretched the whole guide, a 1200tps macro was spread five times too wide so clicks landed nowhere near the real ones and extra marks appeared between them
- **fixed dots merging into a solid bar at high cps**, the dots were a fixed size no matter how close together the clicks were, past about 25 cps they overlapped, they shrink to fit now like the rhythm notes already did

## v0.7.3

- **changing a colour or opacity no longer rebuilds the whole guide**, before, every single setting rebuilt every band and marker in the macro, even ones that only change how things look, dragging an opacity slider did that once per step
- the marker smoothing no longer keeps growing a list of every marker you have scrolled past
- release announcements now post to discord automatically

## v0.7.2

- **fixed the early late number not always showing**, two separate reasons
- if you were using generated notes with no macro file loaded, it never showed at all, the number was tied to macro markers that generated notes do not create
- once the mod had lined itself up it stopped labelling any click more than half a second off, which is exactly when you most want to know, it now labels anything up to about a second and a quarter

## v0.7.1

- **everything moves the same on every monitor now**, the guide was smoothing per frame instead of per second, so markers snapped almost instantly on a 360hz screen and dragged for a third of a second on 30hz, same code, completely different feel
- markers now settle in the same time no matter your framerate
- a lag spike cannot make the guide or the hit animations jump anymore
- the macro name popup fades and drifts instead of blinking in
- softened the menu shading

## v0.7.0

- **hotkeys**, one key turns the guide on and off, and Q and E switch between the macros in your library without opening the menu
- the macro name fades in at the top right when you switch so you know what you swapped to
- all three keys can be changed in the settings, G, Q and E are just the defaults

## v0.6.0

- **the guide and rhythm mode work from a start pos now**, before, starting partway through a level shifted the whole guide because the mod thought you were at the beginning
- it works out how far in you spawned and shifts everything to match, normal play is unchanged
- assist mode was using a different clock to the guide from a start pos, both use the same one now
- **the mod id changed to geekedgdplayer.click-guide-visualizer**, you have to delete the old one before installing this, your settings and macros will not carry over

## v0.5.1

- fixed a crash where a badly nested json macro could take down the game
- rewrote the description and about page

## v0.5.0

- **online macro search**, theres an Online button in the macro menu now, press it in a level and it checks hyperbolus.net for a macro of that exact level and downloads it, no typing, it works out the level from the game
- its off by default, turn on **Find Macros Online** to use it
- formats the mod cant read get filtered out before you see them instead of showing up and failing

## v0.4.4

- notes are thin lines now, 5px instead of 22px, so you can see the exact frame instead of a rough area
- outlines shrink with the note, at small sizes the old fixed outline swallowed the whole thing

## v0.4.3

- bold font in the menu, the old one was too thin to read
- long macro names shrink to fit instead of running under the button
- the early late numbers ingame are bigger

## v0.4.2

- title was overlapping the buttons, gave it room

## v0.4.1

- rebuilt the macro menu, it was a mess, bigger popup, buttons spaced properly, list actually centred, rows alternate shading
- loaded macros show a greyed out Loaded button instead of one that looks clickable but does nothing

## v0.4.0

- **the early late number now shows straight away**, it used to wait for the mod to line itself up which took 6 clicks, so on start pos it often never showed at all
- renamed every setting to plain words, Opacity is Fade, Cull Padding is Draw Distance, Show Accuracy is Show How Early Or Late
- your settings are not reset, only the labels changed

## v0.3.2

- rhythm notes were overlapping into one blob when clicks were close together, they shrink now
- hold bars were drawing through their own note head

## v0.3.1

- **fixed the macro drifting further out of sync the longer you played**, the mod was guessing the framerate wrong when a file didnt say
- added a check that spots drift while you play and corrects it

## v0.3.0

- all the colours are changeable now, not just player 1
- theres an unload button for macros, before there was no way to get rid of one
- hold fade is its own setting, it used to wipe everything when you lowered it

## v0.2.0

- **auto align never actually worked**, rewrote it, it handles macros that are seconds out now
- the macro file setting in the options did nothing, wired it up
- the menu shows what the mod is doing while it lines up

## v0.1.0

- rhythm mode is a vertical column now instead of a sideways lane
- added a logo
