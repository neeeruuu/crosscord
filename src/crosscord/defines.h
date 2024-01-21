#pragma once

/*
	time to wait for a module's future to be finished
	(used @ modules.cpp)
*/
#define FUTURE_WAIT_INTERVAL 1000 / 4

/*
	time to wait for other futures to finish
	(used @ modules.cpp)
*/
#define FUTURE_TIMEOUT 2500

/*
	default window size
	(used @ modules/ui.cpp)
*/
#define WND_DEF_X 350.f
#define WND_DEF_Y 225.f

/*
	window limit multipliers
	(used @ modules/ui.cpp)
*/
#define MIN_MULT 0.95f
#define MAX_MULT 1.05f

/*
	id for umsg posted by tray context menu
	(used @ modules/tray.cpp)
*/
#define TRAY_MESSAGE_ID 0x1337

/*
	overlay intervals
	(used @ modules/overlay.cpp)
*/
#define FORCE_RENDER_INTERVAL 1000 / 2
#define RENDER_INTERVAL 1000 / 120
#define DETECTION_INTERVAL 1000 / 2