//#include <Wire.h>
#include <ESP8266WiFi.h>

#include <menu.h>
#include <menuIO/serialOut.h>
#include <menuIO/chainStream.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/serialIn.h>

#include "badge_display.h"
#include "badgeMenu.h"
#include "badge.h"
#include "qrcode.h"
#include "neocontroller.h"
#include "ProgressConfig.h"
#include "ballhole.h"
#include "slot.h"
#include "cyberSpa.h"
#include "ogrady.h"
#include "roulotto.h"
#include "weightShake.h"

#ifdef MENU_DEBUG
#include "minigame_template.h"
#endif // MENU_DEBUG

#include "registration.h"
#include "badge_status.h"
#include "badgeOneWireMenu.h"
#include "badgeBatteryMenu.h"
#include "badgeHapticMenu.h"
#include "cyberPartnerGame.h"
#include "CPStoreDisplay.h"
#include "CPInventoryDisplay.h"
#include "CPMating.h"

#include "ccserial.h"


// Define the callback function for starting Roulotto / Classful minigames
// It feels _wrong_ to include menu.h anywhere but in badgeMenu.cpp,
// so the wrappers live here
Menu::result startCyberSpaGame(Menu::eventMask e, Menu::navNode& nav, Menu::prompt& item) {
    CYBERSPA::getInstance().doStart();
    return Menu::proceed; // Indicate successful execution
}
Menu::result startRoulottoGame(Menu::eventMask e, Menu::navNode& nav, Menu::prompt& item) {
    ROULOTTO::getInstance().doStart();
    return Menu::proceed; // Indicate successful execution
}
#ifdef MENU_DEBUG
Menu::result startMinigameGame(Menu::eventMask e, Menu::navNode& nav, Menu::prompt& item) {
    MINIGAME::getInstance().doStart();
    return Menu::proceed; // Indicate successful execution
}
#endif // MENU_DEBUG
Menu::result startWeightShakeGame(Menu::eventMask e, Menu::navNode& nav, Menu::prompt& item) {
    WEIGHTSHAKE::getInstance().doStart();
    return Menu::proceed; // Indicate successful execution
}
Menu::result startBallHoleGame(Menu::eventMask e, Menu::navNode& nav, Menu::prompt& item) {
    BH::getInstance().doStart();
    return Menu::proceed;
}
Menu::result startOgradySaysGame(Menu::eventMask e, Menu::navNode& nav, Menu::prompt& item) {
    OGS::getInstance().doStart();
    return Menu::proceed;
}
Menu::result startSlotGame(Menu::eventMask e, Menu::navNode& nav, Menu::prompt& item) {
    SLOT::getInstance().doStart();
    return Menu::proceed;
}


class HelloWorldPrompt : public prompt {
public:
    HelloWorldPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isHelloWorldUnlocked()) {
            return out.printRaw("Hello World!", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class SUChallengePrompt : public prompt {
public:
    SUChallengePrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isSuUnlocked()) {
            return out.printRaw("Super User, did", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotDroppedPrompt : public prompt {
public:
    GotDroppedPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotDroppedUnlocked()) {
            return out.printRaw("Had a great fall", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotStarvedPrompt : public prompt {
public:
    GotStarvedPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotStarvedUnlocked()) {
            return out.printRaw("Starved to Death", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotThirstyPrompt : public prompt {
public:
    GotThirstyPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotThirstyUnlocked()) {
            return out.printRaw("Died of Thirst", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotDepressedPrompt : public prompt {
public:
    GotDepressedPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotDepressedUnlocked()) {
            return out.printRaw("Got too Sad", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotOldPrompt : public prompt {
public:
    GotOldPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotOldUnlocked()) {
            return out.printRaw("Got too Old", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotHeavyPrompt : public prompt {
public:
    GotHeavyPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotHeavyUnlocked()) {
            return out.printRaw("Food made you fat", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotSkinnyPrompt : public prompt {
public:
    GotSkinnyPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotSkinnyUnlocked()) {
            return out.printRaw("Lost too much weight", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotChillyPrompt : public prompt {
public:
    GotChillyPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotChillyUnlocked()) {
            return out.printRaw("Got too Cold", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class GotSweatyPrompt : public prompt {
public:
    GotSweatyPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isGotSweatyUnlocked()) {
            return out.printRaw("Got too Hot", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class BeatOGradyPrompt : public prompt {
public:
    BeatOGradyPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isBeatOgradyUnlocked()) {
            return out.printRaw("Nenorized O'Grady", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class BeatRoulottoPrompt : public prompt {
public:
    BeatRoulottoPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isBeatRoulottoUnlocked()) {
            return out.printRaw("Lucky Roulotto Spin", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class TonyHawkPrompt : public prompt {
public:
    TonyHawkPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isTonyHawkUnlocked()) {
            return out.printRaw("Spoofed OneWire", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

class tonyHawkPrompt : public prompt {
public:
    tonyHawkPrompt(constMEM promptShadow& p) : prompt(p) {}

    Used printTo(navRoot& root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t panelNr) override {
        if (ProgressConfig::getInstance().isTonyHawkUnlocked()) {
            return out.printRaw("Did A 900", len);
        } else {
            return out.printRaw("[!] Locked", len);
        }
    }
};

namespace BMenu {
    using namespace Menu;

    #define MAX_DEPTH 2

    typedef enum {
        MENU_DISABLED = 0,
        MENU_ENABLED = 1
    } MENU_STATE_t;
    static MENU_STATE_t menu_state;

    MENU(activityMenu, "Activities", doNothing, Menu::noEvent, Menu::wrapStyle
        ,OP("Play BallHole",startBallHoleGame,enterEvent)
        ,OP("Play Cyber Spa",startCyberSpaGame,enterEvent)
        ,OP("Play O'Grady Says",startOgradySaysGame,enterEvent)
        ,OP("Play Roulotto",startRoulottoGame,enterEvent)
        ,OP("Play Slot Game",startSlotGame,enterEvent)
#ifdef MENU_DEBUG
        ,OP("Play Minigame_Template",startMinigameGame,enterEvent)
#endif // MENU_DEBUG
        ,OP("Play Weight Shake",startWeightShakeGame,enterEvent)
        ,EXIT("<Back\n")
    );

    MENU(achievementsMenu, "Achievements", doNothing, Menu::noEvent, Menu::wrapStyle
        ,altOP(HelloWorldPrompt, "", doNothing, enterEvent)
        ,altOP(SUChallengePrompt, "", doNothing, enterEvent)
        ,altOP(GotDroppedPrompt, "", doNothing, enterEvent)
        ,altOP(GotStarvedPrompt, "", doNothing, enterEvent)
        ,altOP(GotThirstyPrompt, "", doNothing, enterEvent)
        ,altOP(GotDepressedPrompt, "", doNothing, enterEvent)
        ,altOP(GotOldPrompt, "", doNothing, enterEvent)
        ,altOP(GotHeavyPrompt, "", doNothing, enterEvent)
        ,altOP(GotSkinnyPrompt, "", doNothing, enterEvent)
        ,altOP(GotChillyPrompt, "", doNothing, enterEvent)
        ,altOP(GotSweatyPrompt, "", doNothing, enterEvent)
        ,altOP(BeatOGradyPrompt, "", doNothing, enterEvent)
        ,altOP(BeatRoulottoPrompt, "", doNothing, enterEvent)
        ,altOP(TonyHawkPrompt, "", doNothing, enterEvent)
        ,altOP(tonyHawkPrompt, "", doNothing, enterEvent)
        ,EXIT("<Back\n")
    );

    #ifdef ENABLE_KIOSK_MODE
    MENU(onewireMenu, "iButton", doNothing, Menu::noEvent, Menu::wrapStyle
    ,OP("Get ID",                   BadgeOneWireMenu::scanForDevice,enterEvent)
    ,OP("Read Mem",                 BadgeOneWireMenu::readMemory,enterEvent)
    ,OP("Read Scratch",             BadgeOneWireMenu::readScratchpad,enterEvent)
    ,OP("Read App Reg",             BadgeOneWireMenu::readApplicationRegister,enterEvent)
    ,OP("Read Status",              BadgeOneWireMenu::readStatusRegister,enterEvent)
    ,OP("[Kiosk] Setup iButton]",   BadgeOneWireMenu::kioskSetupIButton,enterEvent)
    ,EXIT("<Back\n")
    );
    #endif // ENABLE_KIOSK_MODE

    MENU(aboutMenu, "About", doNothing, Menu::noEvent, Menu::wrapStyle
        ,OP("@melvin2001",          QRCode::createQRCodeMelvin,         enterEvent)
        ,OP("@pandatrax",           QRCode::createQRCodePandatrax,      enterEvent)
        ,OP("@persinac",            QRCode::createQRCodePersinac,       enterEvent)
        ,OP("@working-as-designed", QRCode::createQRCodeStress,         enterEvent)
        ,OP("@clarkehackworth",     QRCode::createQRCodeClarkeHackworth,enterEvent)
        ,EXIT("<Back\n")
    );

    MENU(mainMenu, greeting_str, doNothing, Menu::noEvent, Menu::wrapStyle
        #ifndef ENABLE_KIOSK_MODE
        ,SUBMENU(activityMenu)
        ,OP("CP Store",         CPStoreDisplay::getInstance().doStart,        enterEvent)
        ,OP("CP Inventory",     CPInventoryDisplay::getInstance().doStart,    enterEvent)
        ,OP("CP Play",          CPMating::getInstance().doStart,              enterEvent)
        ,SUBMENU(achievementsMenu)
        ,OP("Discord",          Registration::discordRegistration,              enterEvent)
        #endif
        #ifdef ENABLE_KIOSK_MODE
        ,SUBMENU(onewireMenu)
        #endif
        ,SUBMENU(aboutMenu)
        ,OP("Badge Status",     BADGE_STATUS::doStart,          enterEvent)
    );

    // define menu colors --------------------------------------------------------
    //  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
    //monochromatic color table
    const colorDef<uint16_t> colors[6] MEMMODE={
        {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//bgColor
        {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//fgColor
        {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//valColor
        {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//unitColor
        {{WHITE,BLACK},{BLACK,BLACK,BLACK}},//cursorColor
        {{WHITE,BLACK},{BLACK,WHITE,WHITE}},//titleColor
    };

    //serialIn serial(Serial);
    //MENU_INPUTS(in,&serial);
    chainStream<0> in(NULL);

    #define fontX 6
    #define fontY 9

    MENU_OUTPUTS(out,MAX_DEPTH
        ,ADAGFX_OUT(BDISPLAY::display,colors,fontX,fontY,{0,1,BDISPLAY::SCREEN_WIDTH/fontX,(BDISPLAY::SCREEN_HEIGHT/fontY)-1})
        ,NONE
        //,SERIAL_OUT(Serial)
    );

    NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

    void enableMenu( void ) {
        BDISPLAY::clearAndDisplayBatteryPercentage();
        BDISPLAY::display.setTextSize(1);

        menu_state = MENU_ENABLED;
        nav.showTitle=false;
        nav.idleOff();
        BDISPLAY::display.display();
        //#ifndef ENABLE_KIOSK_MODE
        NeoController::getInstance().changeChase();
        //#endif // ENABLE_KIOSK_MODE
    }

    void disableMenu( void ) {
        nav.idleOn();
        BDISPLAY::clearAndDisplayBatteryPercentage();
        BDISPLAY::display.display();
        menu_state = MENU_DISABLED;
    }

    bool isMenuRunning( void ) {
        if(menu_state == MENU_DISABLED){
            return false;
        } else {
            return true;
        }
    }

    void moveMenuDown( void ) {
        if(isMenuRunning()){
            nav.doNav(upCmd);
        }
    }

    void moveMenuUp( void ) {
        if(isMenuRunning()){
            nav.doNav(downCmd);
        }
    }

    void enterMenu( void ) {
        if(isMenuRunning()){
            nav.doNav(enterCmd);
        }
    }

    void escMenu( void ) {
        if(isMenuRunning()){
            nav.doNav(escCmd);
        }
    }

    void updateMenu( void ) {
        if(menu_state == MENU_ENABLED){
            nav.poll();
            BDISPLAY::displayBatteryPercentage();
            BDISPLAY::display.display();

            #ifndef ENABLE_KIOSK_MODE
            if(nav.sleepTask) {
                disableMenu();
                CyberPartnerGame::getInstance().enable();
            }
            #endif // ENABLE_KIOSK_MODE
        }

    }

}
