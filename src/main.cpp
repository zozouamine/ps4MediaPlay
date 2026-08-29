#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
// Fix for OpenOrbis SDK: u_short/u_int not defined in some toolchain versions
#ifndef u_short
typedef unsigned short u_short;
#endif
#ifndef u_int
typedef unsigned int u_int;
#endif
#include <stdio.h>
#include <string>

#ifndef PC_SIMULATOR
// PS4 SDK Headers (OpenOrbis) - must be before C++ headers that need nanosleep
#include <orbis/libkernel.h>
#include <orbis/VideoOut.h>
#include <orbis/AudioOut.h>
#include <orbis/Pad.h>
#include <orbis/UserService.h>
#include <orbis/SystemService.h>
#endif

#include "ui/ui_manager.h"
#include "player/media_player.h"
#include "utils/logger.h"

#ifdef PC_SIMULATOR
int main(int argc, char* argv[]) {
    Logger::init();
    Logger::info("PS4 Media Play - PC Simulator Mode");
    UIManager ui;
    MediaPlayer player;
    ui.init(1920, 1080);
    ui.run(&player);
    return 0;
}
#else
// PS4 Entry Point
extern "C" int _main(struct SceKernelArg *args) {
    Logger::init();
    Logger::info("PS4 Media Play v1.00 Started");
    Logger::info("Firmware: Jailbroken - GoldHEN");

    // Init PS4 Services
    sceKernelLoadStartModule("libSceVideoOut.sprx", 0, 0, 0, 0, 0);
    sceKernelLoadStartModule("libSceAudioOut.sprx", 0, 0, 0, 0, 0);
    sceKernelLoadStartModule("libScePad.sprx", 0, 0, 0, 0, 0);
    sceKernelLoadStartModule("libSceUserService.sprx", 0, 0, 0, 0, 0);

    // Init Pad for DualShock 4 (using Orbis API as per SDK sample)
    scePadInit();
    OrbisUserServiceInitializeParams param;
    param.priority = ORBIS_KERNEL_PRIO_FIFO_LOWEST;
    sceUserServiceInitialize(&param);
    OrbisUserServiceUserId userId;
    sceUserServiceGetInitialUser(&userId);
    int padHandle = scePadOpen(userId, 0, 0, nullptr);
    if (padHandle < 0) {
        Logger::error("Failed to open Pad");
        return -1;
    }
    Logger::info("Pad initialized: handle=%d", padHandle);

    // Init UI & Player
    UIManager ui;
    MediaPlayer player;

    // PS4 resolution is 1920x1080
    if (!ui.init(1920, 1080)) {
        Logger::error("UI init failed");
        return -1;
    }

    // Main Loop
    OrbisPadData padData;
    bool running = true;
    while (running) {
        // Poll Pad Input
        if (scePadReadState(padHandle, &padData) == 0) {
            // Map buttons to UI actions (ORBIS_ prefix as per SDK)
            if (padData.buttons & ORBIS_PAD_BUTTON_CROSS) {
                ui.onButtonCross();
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_CIRCLE) {
                ui.onButtonCircle();
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_OPTIONS) {
                ui.onButtonOptions();
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_TRIANGLE) {
                ui.onButtonTriangle();
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_SQUARE) {
                ui.onButtonSquare();
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_L1) {
                player.previous();
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_R1) {
                player.next();
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_L2) {
                player.seekRelative(-10); // -10 sec
            }
            if (padData.buttons & ORBIS_PAD_BUTTON_R2) {
                player.seekRelative(10); // +10 sec
            }
            // Stick for navigation
            if (padData.leftStick.x < 30) ui.onNavigateLeft();
            if (padData.leftStick.x > 220) ui.onNavigateRight();
            if (padData.leftStick.y < 30) ui.onNavigateUp();
            if (padData.leftStick.y > 220) ui.onNavigateDown();
        }

        // Handle PS button exit - use OPTIONS+CIRCLE as exit (PS button is system reserved)
        // if (padData.buttons & 0x1000) running = false; // PS button if available

        ui.update();
        ui.render();
        player.update();

        sceKernelUsleep(16666); // ~60 FPS
    }

    scePadClose(padHandle);
    ui.shutdown();
    Logger::info("PS4 Media Play Exited Cleanly");
    return 0;
}
#endif
