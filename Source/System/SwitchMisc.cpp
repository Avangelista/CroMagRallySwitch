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

	SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "1");
}

extern "C" void userAppExit() {
	fsdevUnmountAll();		// flush & unmount sdmc: (and any other fsdev mounts)
	romfsExit();
	nxlinkExit();
}

#endif
