// Nintendo Switch homebrew glue (nxlink stdio + romfs mount + SDL hints).
// Copied verbatim from carstene1ns's Nanosaur Switch port (game-agnostic).
// Guarded by __SWITCH__ so it is inert on every other platform.

#ifdef __SWITCH__

#include <switch.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <SDL.h>

static int s_nxlinkSock = -1;

static void nxlinkInit() {
	if (R_FAILED(socketInitializeDefault()))
		return;

	s_nxlinkSock = nxlinkStdio();
	if (s_nxlinkSock >= 0)
		printf("nxlink activated...\n");
	else
		socketExit();
}

static void nxlinkExit() {
	if (s_nxlinkSock >= 0) {
		close(s_nxlinkSock);
		socketExit();
		s_nxlinkSock = -1;
	}
}

extern "C" void userAppInit() {
	// Route stderr (SDL_Log + our error prints) and stdout to svcOutputDebugString
	// so they appear in emulator "Guest" logs / the system debug log. On real
	// hardware, nxlinkInit() additionally streams stdout to a connected nxlink host.
	consoleDebugInit(debugDevice_SVC);
	dup2(STDERR_FILENO, STDOUT_FILENO);

	nxlinkInit();
	romfsInit();
	fsdevMountSdmc();		// mount the SD card as "sdmc:/" so prefs/saves can be written

	// Route the game's prefs/saves to the standard homebrew location on the SD card.
	// Pomme's FindFolder(kPreferencesFolderType) reads XDG_CONFIG_HOME on non-Win/Mac
	// platforms, so pointing it at sdmc:/switch lands prefs in sdmc:/switch/CroMagRally/
	// WITHOUT having to patch (and fork) the Pomme submodule.
	mkdir("sdmc:/switch", 0777);							// ensure the base dir exists (usually already does)
	setenv("XDG_CONFIG_HOME", "sdmc:/switch", 1);

	// This SDL build for Switch maps face buttons positionally (SDL_CONTROLLER_BUTTON_A is
	// the bottom button, printed "B" on a Nintendo pad) and doesn't compile in the label-swap
	// this hint would trigger, so on its own the hint does nothing here. Force the positional
	// baseline ("0") -- we remap to the printed A/B/X/Y labels ourselves in SDLInput.c
	// (SwitchRemapFaceButton). Explicit "0" also stops a future SDL that honors this hint from
	// double-swapping our remap.
	SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
}

extern "C" void userAppExit() {
	fsdevUnmountAll();		// flush & unmount sdmc: (and any other fsdev mounts)
	romfsExit();
	nxlinkExit();
}

// Show the Switch "connect controllers" system applet, requesting between
// minPlayers and maxPlayers controllers. Called from the game (MainMenu) when the
// player picks a 2/3/4-player game. After this returns, SDL's HID poll surfaces the
// newly-connected pads as SDL_CONTROLLERDEVICEADDED events, which the game opens as
// extra local players (see SDLInput.c).
//
// Returns true if the required controllers were confirmed; false if the user backed
// out of the applet (so the caller can abort starting the multiplayer game). Note the
// applet skips its UI entirely when enough controllers are already connected.
extern "C" bool Switch_ConnectControllers(int minPlayers, int maxPlayers) {
	HidLaControllerSupportArg arg;
	hidLaCreateControllerSupportArg(&arg);				// sane defaults (take-over on, joy-dual permitted, ...)
	arg.hdr.player_count_min = minPlayers;
	arg.hdr.player_count_max = maxPlayers;
	arg.hdr.enable_single_mode = false;					// each player uses a full controller (no single-Joy-Con split)

	HidLaControllerSupportResultInfo result = {};
	Result rc = hidLaShowControllerSupport(&result, &arg);	// blocks while the system UI is up

	return R_SUCCEEDED(rc) && result.player_count >= minPlayers;
}

#endif
