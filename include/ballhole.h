#pragma once
#include <Arduino.h>
#include "game_timer.h"

class BH {
public:
    static BH& getInstance();
    BH(const BH&) = delete;
    BH& operator=(const BH&) = delete;
    BH(BH&&) = delete;
    BH& operator=(BH&&) = delete;

    void setup();
    void update();
    void doStart();
    void doStop();
    bool isRunning();

protected:
    ~BH() = default;

private:
    BH() = default;

    void finishGame(bool result);
    void playGame();
    void runBallHole();

    typedef enum {
        BH_DISABLED = 0,
        BH_BANNER   = 1,
        BH_RUNNING  = 2,
        BH_INSTRUCT = 3,
        BH_RESULT   = 4
    } GAME_STATE_t;

    typedef enum {
        GR_DISABLED = 0,
        GR_LOST     = 1,
        GR_WIN      = 2
    } GAME_RESULT_t;

    static constexpr int8_t center_x = 64; // SCREEN_WIDTH / 2
    static constexpr int8_t center_y = 32; // SCREEN_HEIGHT / 2

    int16_t ballx = 0;
    int16_t bally = 0;
    float   ballDeltaBias = 0.10f;
    int16_t ballxDelta = 0;
    int16_t ballyDelta = 0;
    int8_t  level = 1;
    int8_t  highlevel = 1;
    uint16_t banner_delay_ticker   = 0;
    uint16_t instruct_delay_ticker = 0;
    uint16_t result_delay_ticker   = 0;
    GameTimer    gameTimer;
    GAME_STATE_t  game_state  = BH_DISABLED;
    GAME_RESULT_t game_result = GR_DISABLED;
};
