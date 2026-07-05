# Cro-Mag Rally

## *The wildest racing game since man invented the wheel!*

This is a port of Pangea Software’s racing game **Cro-Mag Rally** to modern operating systems.

**Download the game for macOS, Windows and Linux here:** https://github.com/jorio/CroMagRally/releases

![Cro-Mag Rally Screenshot](docs/screenshot.webp)

## 🎮 Nintendo Switch homebrew port

This fork ports Cro-Mag Rally to the **Nintendo Switch** as unofficial homebrew. Like every Pangea port it is free of charge — and, like all homebrew, it is **not** an eShop title and cannot be sold (the game is licensed [CC BY-NC-SA 4.0](LICENSE.md); see [Legal info](#legal-info)).

### Requirements

- A Nintendo Switch running **custom firmware** (e.g. Atmosphère) with the **Homebrew Menu**.
- It will **not** run on a stock/unmodded console, and **not** through a game-card flashcart (MIG Switch and the like) — those only run signed retail games, whereas homebrew is unsigned code that requires CFW.

### Installing

1. Copy **`CroMagRally.nro`** to the `/switch/` folder of your SD card.
2. Launch it from the Homebrew Menu.
3. Preferences and records are saved to `sdmc:/switch/CroMagRally/`.

> **If the game fails to load or runs out of memory**, launch it in *application mode* for full RAM: hold **R** while opening any installed game from the Switch home screen (instead of opening the Homebrew Menu from the Album). Opening from the Album runs homebrew in *applet mode*, which has far less memory available.

### Playing on Switch

- Fully playable with **Joy-Con or a Pro Controller** — up to 4 players in local split-screen.
- When you pick a 2/3/4-player game, the standard **controller-connection screen** appears so everyone can pair a controller.
- Buttons use the physical Switch layout; **B** goes back in menus.
- See [SECRETS](SECRETS.md#on-nintendo-switch) for the gamepad cheat codes (the keyboard combos map to controller buttons).

### What's different from the desktop version

- Gamepad-first UI — keyboard-only prompts and the keyboard-setup screen are hidden.
- Windowed/fullscreen options are removed (the game always fills the screen).
- Saves go to the SD card at `sdmc:/switch/CroMagRally/`.

This port reuses the approach of [carstene1ns's Nanosaur Switch port](https://github.com/carstene1ns/Nanosaur/tree/ports) and is built with [devkitPro](https://devkitpro.org)/libnx. See [BUILD](BUILD.md#how-to-build-for-nintendo-switch-homebrew) to build the `.nro` yourself.

## About Cro-Mag Rally

> In Cro-Mag Rally you are a speed-hungry caveman named Brog who races through the Stone, Bronze, and Iron Ages in primitive vehicles such as the Geode Cruiser, Bone Buggy, Logmobile, Trojan Horse, and many others. Brog has at his disposal an arsenal of primitive weaponry ranging from Bone Bombs to Chinese Bottle Rockets and Heat Seeking Homing Pigeons.
> 
> In addition to single-player racing where one player races against the computer, there are also several different multi-player modes including Tag, Capture the Flag, and Survival. Up to four players can play on a single computer in split-screen mode.

CMR was released in 2000 by Pangea Software as a Mac exclusive, and it was a pack-in game on Macs that came out around that time.

## About this port

This is a port of the original OS 9 version of the game. It aims to provide the best way to experience CMR on today’s computers. It is an “enhanced” version insofar as it fixes bugs that may hinder the experience, and it brings in a few new features in keeping with the spirit of the original game.

Some of the new features include:
- Up to 4 players in split-screen multiplayer (up from 2 in the original).
- The UI is subtly animated and has been tweaked to be pleasant to look at on modern widescreens.
- Enable a timer in race modes to hone your racing skills, and keep track of your records in the all-new scoreboard!

I haven’t had time to restore NetSprockets multiplayer from the OS 9 version yet, but that may come in a later release.

### More documentation

- [BUILD](BUILD.md) – How to build the game from source
- [CHANGELOG](CHANGELOG.md) – Cro-Mag Rally version history
- [LICENSE](LICENSE.md) – Licensing info (see also below)
- [SECRETS](SECRETS.md) – Cheat codes!

### Legal info

Cro-Mag Rally © 2000 Pangea Software, Inc. Cro-Mag Rally is a trademark of Pangea Software, Inc. This version was made and re-released here (https://github.com/jorio/CroMagRally) under permission from Pangea Software, Inc.

This version is licensed under [CC-BY-NC SA 4.0](LICENSE.md).

## More Pangea stuff!

Check out my ports of [Bugdom](https://github.com/jorio/Bugdom), [Nanosaur](https://github.com/jorio/Nanosaur), [Mighty Mike (Power Pete)](https://github.com/jorio/MightyMike) and [Otto Matic](https://github.com/jorio/OttoMatic).

All ports are free of charge! If you’d like to support the development of Pangea game ports, feel free to visit https://jorio.itch.io and name your own price for any of the games there. Much appreciated! 😊
