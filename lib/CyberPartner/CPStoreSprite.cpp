/**
 * @file CPStoreSprite.cpp
 * @brief Implements the CPStoreSprite class functions.
 *
 * This file contains the implementation of CPStoreSprite, which handles
 * store sprite rendering on the display.
 *
 * It uses `badge_display.h` for drawing functions and references sprite data from
 * `CPStore_bitmaps.h`.
 */
#include <Arduino.h>
#include "CPStoreSprite.h"
#include "badge_display.h"

namespace {
    const uint8_t* meal_sprites_sizes[][NUM_STAGES] = {
        { apple1_size, apple2_size, apple3_size },
        { bread1_size, bread2_size, bread3_size },
        { cereal1_size, cereal2_size, cereal3_size },
        { cone1_size, cone2_size, cone3_size },
        { pudding1_size, pudding2_size, pudding3_size },
        { scone1_size, scone2_size, scone3_size },
        { sushi1_size, sushi2_size, sushi3_size },
        { tart1_size, tart2_size, tart3_size },
        { monster_drink1_size, monster_drink2_size, monster_drink2_size },
        { malort1_size, malort2_size, malort2_size },
        { milk1_size, milk2_size, milk2_size },
        { soda1_size, soda2_size, soda2_size },

    };

    const unsigned char* meal_sprites_images[][NUM_STAGES] PROGMEM = {
        { apple1, apple2, apple3 },
        { bread1, bread2, bread3 },
        { cereal1, cereal2, cereal3 },
        { cone1, cone2, cone3 },
        { pudding1, pudding2, pudding3 },
        { scone1, scone2, scone3 },
        { sushi1, sushi2, sushi3 },
        { tart1, tart2, tart3 },
        { monster_drink1, monster_drink2, monster_drink2 },
        { malort1, malort2, malort2 },
        { milk1, milk2, milk2 },
        { soda1, soda2, soda2 },
    };

}

const uint8_t* const (*CPStoreSprite::sprite_sizes)[NUM_STAGES] = meal_sprites_sizes;
const unsigned char* const (*CPStoreSprite::sprite_data)[NUM_STAGES] = meal_sprites_images;

/**
 * @brief Constructs a CyberPartnerSprite instance.
 *
 * This constructor initializes a `CyberPartnerSprite` object with the provided parameters.
 * It sets up the sprite's size, data, character, action, and position. Additionally, it
 * initializes the sprite's action by calling the `setAction` method.
 *
 * @param initialMeal The initial meal to associate with the sprite.
 * @param initialStage The initial stage to associate with the sprite.
 * @param startX The starting X-coordinate of the sprite.
 * @param startBaseY The starting base Y-coordinate of the sprite.
 */
CPStoreSprite::CPStoreSprite(CPStoreMeals initialMeal, int16_t startX, int16_t startBaseY)
{
    // Initialize other members
    meal = initialMeal;
    setLocation(startX, startBaseY);
    stage = CPMealsStages::STAGE_FULL;

    // Initialize sprite dimensions based on the initial action
    width = pgm_read_byte(sprite_sizes[static_cast<size_t>(meal)][static_cast<size_t>(0)]);
    height = pgm_read_byte(sprite_sizes[static_cast<size_t>(meal)][static_cast<size_t>(0)] + 1);
}

/**
 * @brief Sets the location of the CPStoreSprite instance.
 *
 * This function updates the `x` and `baseY` attributes of the `CPStoreSprite` instance
 * to set its position on the screen.
 *
 * @param startX The new X-coordinate of the sprite.
 * @param startBaseY The new base Y-coordinate of the sprite.
 */
void CPStoreSprite::setLocation(int16_t startX, int16_t startBaseY) {
    x = startX;
    baseY = startBaseY;
}

/**
 * @brief Sets the meal type of the CPStoreSprite instance.
 *
 * This function updates the `meal` attribute of the `CPStoreSprite` instance
 * to change its associated meal type. It also updates the sprite dimensions
 * based on the new meal type.
 *
 * @param newMeal The new meal type to set.
 */
void CPStoreSprite::setMealStage(CPStoreMeals newMeal, CPMealsStages newStage) {
    meal = newMeal;
    stage = newStage;

    // Update sprite dimensions based on the new meal type
    width = pgm_read_byte(sprite_sizes[static_cast<size_t>(meal)][static_cast<size_t>(0)]);
    height = pgm_read_byte(sprite_sizes[static_cast<size_t>(meal)][static_cast<size_t>(0)] + 1);
}

/**
 * @brief Draws the CPStoreSprite instance on the display.
 *
 * This function renders the sprite on the screen based on its current position,
 * direction, and action. If the sprite's direction is `Direction::Right`, the
 * sprite is drawn with a horizontally flipped bitmap. Otherwise, it is drawn
 * normally.
 *
 * @note The sprite's position is determined by the `x` and `baseY` attributes,
 *       and the bitmap data is retrieved from the `sprite_data` array.
 */
void CPStoreSprite::draw() {
    int16_t y = baseY - height;
    BDISPLAY::display.drawBitmap(x, y, sprite_data[(uint8_t)meal][(uint8_t)stage], width, height, WHITE);
}

/**
 * @brief Erases the CPStoreSprite instance from the display.
 *
 * This function removes the sprite from the screen by redrawing its bitmap
 * in black. If the sprite's direction is `Direction::Right`, the bitmap is
 * horizontally flipped before being erased. Otherwise, it is erased normally.
 *
 * @note The position of the sprite is determined by the `x` and `baseY` attributes,
 *       and the bitmap data is retrieved from the `sprite_data` array.
 */
void CPStoreSprite::erase() {
    int16_t y = baseY - height;
    BDISPLAY::display.drawBitmap(x, y, sprite_data[(uint8_t)meal][(uint8_t)stage], width, height, BLACK);
}
