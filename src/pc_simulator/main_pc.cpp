// محاكي PC لتجربة التطبيق قبل نقله لـ PS4
// يسمح لك بتجربة الواجهة على الكمبيوتر باستخدام الكيبورد (يحاكي يد PS4)
#define PC_SIMULATOR 1
#include "ui/ui_manager.h"
#include "player/media_player.h"
#include "utils/logger.h"
#include <iostream>

void printBanner() {
    std::cout << R"(
╔══════════════════════════════════════════╗
║   PS4 Media Play - PC Simulator         ║
║   محاكي لتجربة التطبيق على الكمبيوتر   ║
╠══════════════════════════════════════════╣
║  X (Enter) = تشغيل / اختيار             ║
║  O (ESC/Backspace) = رجوع               ║
║  Arrow Up/Down = تنقل                   ║
║  Arrow Left/Right = تقديم/ترجيع        ║
║  A = Play/Pause (R3)                    ║
║  Q = خروج                               ║
╚══════════════════════════════════════════╝
)" << std::endl;
}

int main(int argc, char* argv[]) {
    printBanner();
    Logger::init();
    Logger::info("PC Simulator Started");

    UIManager ui;
    MediaPlayer player;
    ui.setPlayer(&player);
    ui.init(1920, 1080);

    // Demo: add sample files
    std::string demoPath = (argc > 1) ? argv[1] : ".";
    ui.navigateTo(demoPath);

    std::cout << "\n[Simulator] اكتب مسار مجلد فيه فيديوهات للتجربة، أو اضغط Enter للمتابعة\n";
    std::cout << "> Current: " << demoPath << "\n";
    std::cout << "[Simulator] للخروج اكتب q\n";

    std::string cmd;
    while (true) {
        std::cout << "\n[Browser] > ";
        if (!std::getline(std::cin, cmd)) break;
        if (cmd == "q" || cmd == "quit") break;
        if (cmd == "u") ui.onNavigateUp();
        else if (cmd == "d") ui.onNavigateDown();
        else if (cmd == "x" || cmd == "") ui.onButtonCross();
        else if (cmd == "o") ui.onButtonCircle();
        else if (cmd == "p") { player.togglePause(); }
        else if (cmd == "left") ui.onNavigateLeft();
        else if (cmd == "right") ui.onNavigateRight();
        else if (!cmd.empty()) ui.navigateTo(cmd);

        ui.render();
        player.update();
        std::cout << "State: " << (int)player.getState() << " | Time: " << player.getCurrentTime() << "s / " << player.getDuration() << "s\n";
    }

    std::cout << "Simulator exited.\n";
    return 0;
}
