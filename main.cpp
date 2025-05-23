#include <prism/framerateselectscreen.h>
#include <prism/physics.h>
#include <prism/file.h>
#include <prism/drawing.h>
#include <prism/log.h>
#include <prism/wrapper.h>
#include <prism/system.h>
#include <prism/stagehandler.h>
#include <prism/mugentexthandler.h>
#include <prism/debug.h>

#include "gamescreen.h"
#include "storyscreen.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

#endif

// #define DEVELOP

void exitGame() {
	shutdownPrismWrapper();

#ifdef DEVELOP
	if (isOnDreamcast()) {
		abortSystem();
	}
	else {
		returnToMenu();
	}
#else
	returnToMenu();
#endif
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	#ifdef DEVELOP
	setDevelopMode();
	#endif

	setGameName("Hell R Us");
	setScreenSize(320, 240);
	
	initPrismWrapperWithConfigFile("data/config.cfg");
	setFont("$/rd/fonts/segoe.hdr", "$/rd/fonts/segoe.pkg");

	addMugenFont(-1, "font/f4x6.fnt");
	addMugenFont(1, "font/f6x9.fnt");
	addMugenFont(2, "font/jg.fnt");
	setMugenAnimationHandlerPixelCenter(Vector2D(0.0, 0.0));
	logg("Check framerate");
	FramerateSelectReturnType framerateReturnType = selectFramerate();
	if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
		exitGame();
	}

	if(isInDevelopMode()) {
		disableWrapperErrorRecovery();	
		setMinimumLogType(LOG_TYPE_NORMAL);
		//setVolume(0.0);
		//setSoundEffectVolume(0.0);
	}
	else {
		setMinimumLogType(LOG_TYPE_NONE);
	}

	setCurrentStoryDefinitionFile("game/OUTRO.def", 0);
	resetGame();
	startScreenHandling(getGameScreen());

	exitGame();
	
	return 0;
}


