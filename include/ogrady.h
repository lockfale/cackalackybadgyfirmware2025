#pragma once
#include <Arduino.h>
#include "game_timer.h"

class OGS {
public:
    static OGS& getInstance();
    OGS(const OGS&) = delete;
    OGS& operator=(const OGS&) = delete;
    OGS(OGS&&) = delete;
    OGS& operator=(OGS&&) = delete;

    void setup();
    void update();
    void doStart();
    void doStop();
    bool isRunning();

    void clickYellowButton();
    void clickBlueButton();
    void clickRedButton();
    void clickGreenButton();

protected:
    ~OGS() = default;

private:
    OGS() = default;

    void populateRoundArrays();
    void runOgradySays();
    void finishGame(bool result);
    bool isRoundWinner();

    typedef enum {
        OGS_DISABLED    = 0,
        OGS_TITLEBANNER = 1,
        OGS_INSTRUCT    = 2,
        OGS_PATTERNSHOW = 3,
        OGS_GET_INPUT   = 4,
        OGS_RESULT      = 5
    } GAME_STATE_t;

    typedef enum {
        GR_DISABLED = 0,
        GR_LOST     = 1,
        GR_WIN      = 2
    } GAME_RESULT_t;

    typedef enum {
        Y = 1,
        R = 2,
        G = 3,
        B = 4
    } ogsColor_t;

    unsigned long ogsPatternSeed       = 0;
    int* dynamicRoundArray             = nullptr;
    int* dynamicInputArray             = nullptr;
    int  roundWinCounter               = 0;
    int  roundArraySize                = 0;
    int  roundInputCounter             = 0;
    int  roundShowPatternCounter       = 0;
    int  roundScalar                   = 0;
    uint16_t titleBannerTicker         = 0;
    uint16_t instructBannerTicker      = 0;
    uint16_t patternBannerTicker       = 0;
    uint16_t inputBannerTicker         = 0;
    uint16_t resultBannerTicker        = 0;
    uint16_t inputBannerLimit          = 0;
    GameTimer    gameTimer;
    GAME_STATE_t  game_state  = OGS_DISABLED;
    GAME_RESULT_t game_result = GR_DISABLED;

    const String intToColor(ogsColor_t color);
    String getRoundArrayAsInts(int* arr, int size);
    String getRoundArrayAsColors(int* arr, int size);
};
