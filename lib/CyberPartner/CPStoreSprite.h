/**
 * @file CPStoreSprite.h
 * @brief Defines the CyberPartnerSprite class for handling character sprite rendering.
 *
 * This class provides functionality for managing, animating, and displaying character
 * sprites. It supports different characters, actions, and movement mechanics.
 *
 * Dependencies:
 * - badge_display.h (for rendering bitmaps)
 * - CPStore_bitmaps.h (for sprite data)
 */
#pragma once

#include "CPStore_bitmaps.h"

constexpr uint8_t NUM_STAGES = 3;

enum class CPStoreMeals {
    MEAL_APPLE,
    MEAL_BREAD,
    MEAL_CEREAL,
    MEAL_CONE,
    MEAL_PUDDING,
    MEAL_SCONE,
    MEAL_SUSHI,
    MEAL_TART,
    DRINK_MONSTER,
    DRINK_MALORT,
    DRINK_MILK,
    DRINK_SODA,
    MEAL_COUNT
};

enum class CPMealsStages {
    STAGE_FULL,
    STAGE_HALF,
    STAGE_GONE,
    STAGE_COUNT
};

/**
 * @class CPStoreSprite
 * @brief Handles the rendering of store sprites on the display.
 *
 * This class manages the display of store sprites, including their animations and
 * interactions with the user.
 */
class CPStoreSprite {
    public:
        /**
         * @brief Constructs a CPStoreSprite object.
         * @param initialMeal The initial meal type.
         * @param startX The initial x-coordinate of the sprite.
         * @param startBaseY The initial base y-coordinate of the sprite.
         * */
        CPStoreSprite(CPStoreMeals initialMeal, int16_t startX, int16_t startBaseY);

        /**
         * @brief Sets the location of the sprite.
         * @param startX The new x-coordinate.
         * @param startBaseY The new base y-coordinate.
         */
        void setLocation(int16_t startX, int16_t startBaseY);

        /**
         * @brief Sets the meal type and stage of the sprite.
         * @param newMeal The new meal type.
         * @param newStage The new stage of the meal.
         */
        void setMealStage(CPStoreMeals newMeal, CPMealsStages newStage);

        /**
         * @brief Draws the sprite at its current position.
         */
        void draw();

        /**
         * @brief Erases the sprite from the screen.
         */
        void erase();

    private:
        static const uint8_t* const (*sprite_sizes)[NUM_STAGES];  /**< Array of sprite size data */
        static const unsigned char* const (*sprite_data)[NUM_STAGES];  /**< Array of sprite image data */
        CPStoreMeals meal;
        CPMealsStages stage;
        int16_t x;
        int16_t baseY;
        uint8_t width;
        uint8_t height;
};