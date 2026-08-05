# changelog

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
