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

// ---------------------------------------------------------------------------------------
// Single (sideways) Joy-Con input.
//
// The devkitPro Switch SDL2 backend under-reports a single Joy-Con: it exposes the stick
// only as a DIGITAL D-pad (never as analog axes), and maps +/- to BACK (there's no START).
// The real values are still available straight from libnx, so we read them here and the
// input layer (SDLInput.c) treats the stick as the pad's LEFTX/LEFTY and +/- as pause. The
// devkitPro backend maps SDL device index i to libnx npad (HidNpadIdType_No1 + i) -- that's
// our correlation key.

// One PadState per npad slot so we can sample a controller's raw HID independently of SDL
// (libnx allows multiple PadStates on one npad). Returns the padUpdate'd state IF this pad
// is a single Joy-Con, else nullptr (caller falls back to SDL's normal analog path).
static PadState* Switch_SingleJoyconPad(int deviceIndex) {
	if (deviceIndex < 0 || deviceIndex >= 8)
		return nullptr;

	static PadState s_pads[8];
	static bool s_padInit[8] = { false };
	if (!s_padInit[deviceIndex]) {
		padInitialize(&s_pads[deviceIndex], (HidNpadIdType)(HidNpadIdType_No1 + deviceIndex));
		s_padInit[deviceIndex] = true;
	}

	padUpdate(&s_pads[deviceIndex]);

	u32 style = padGetStyleSet(&s_pads[deviceIndex]);
	if (!(style & (HidNpadStyleTag_NpadJoyLeft | HidNpadStyleTag_NpadJoyRight)))
		return nullptr;					// not a single Joy-Con
	return &s_pads[deviceIndex];
}

// The left-stick position of a single Joy-Con, in SDL axis convention/range (X = right+,
// Y = down+, ~[-32767,32767]), or false if `deviceIndex` isn't a single Joy-Con. The input
// layer feeds this in as the pad's LEFTX/LEFTY so the stick behaves like a normal left stick.
//
// A single Joy-Con's stick sits in its NATIVE slot: a left Joy-Con -> the left stick (index
// 0), a right Joy-Con -> the right stick (index 1). It's reported in the UPRIGHT frame, so we
// rotate 90 degrees into the player's sideways frame -- MIRRORED between a left and a right
// Joy-Con (they're held rotated opposite ways). The horizontal (X) axis is confirmed on
// hardware; the vertical (Y) is its orthogonal component.
extern "C" bool Switch_GetSingleJoyconStick(int deviceIndex, int* outX, int* outY) {
	PadState* pad = Switch_SingleJoyconPad(deviceIndex);
	if (!pad)
		return false;

	bool isRight = (padGetStyleSet(pad) & HidNpadStyleTag_NpadJoyRight) != 0;
	HidAnalogStickState s = padGetStickPos(pad, isRight ? 1 : 0);
	if (isRight) { *outX =  s.y; *outY =  s.x; }
	else         { *outX = -s.y; *outY = -s.x; }
	return true;
}

// True while +/- is held on a single Joy-Con -- the input layer routes this to the pause
// need. SDL maps a single Joy-Con's +/- to BACK (it has no START), and changing the pause
// binding's default wouldn't help existing users anyway (saved prefs override it).
extern "C" bool Switch_SingleJoyconPauseHeld(int deviceIndex) {
	PadState* pad = Switch_SingleJoyconPad(deviceIndex);
	if (!pad)
		return false;
	return (padGetButtons(pad) & (HidNpadButton_Plus | HidNpadButton_Minus)) != 0;
}

// True if the controller at this device index is a single (sideways) Joy-Con.
extern "C" bool Switch_IsSingleJoycon(int deviceIndex) {
	return Switch_SingleJoyconPad(deviceIndex) != nullptr;
}

#endif
