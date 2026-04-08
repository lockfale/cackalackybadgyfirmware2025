/**
 * @file CyberPartnerSprite.cpp
 * @brief Implements the CyberPartnerSprite class functions.
 *
 * This file contains the implementation of CyberPartnerSprite, which handles
 * character sprite updates, movement, and rendering on the display.
 *
 * It uses `badge_display.h` for drawing functions and references sprite data from
 * `CyberPartner_bitmaps.h`.
 */
#include "CyberPartnerSprite.h"

const uint8_t* birth_death_sprites_sizes[] = {
    egg1_size, egg2_size, death_size,
	age_size, weight_size,
	heart1_size, heart1_large_size, heart2_size, heart2_large_size,
	poop1_size, poop1_large_size, poop2_size, poop2_large_size,
	illness1_size, illness1_large_size, illness2_size, illness2_large_size, illness3_size, illness3_large_size,
};

const unsigned char* birth_death_sprites_images[] PROGMEM = {
    egg1, egg2, death,
	age, weight,
	heart1, heart1_large, heart2, heart2_large,
	poop1, poop1_large, poop2, poop2_large,
	illness1, illness1_large, illness2, illness2_large, illness3_size, illness3_large,
};

const uint8_t* partner_sprites_sizes[][NUM_ACTIONS] = {
	{
		baby_knobby_general1_size, baby_knobby_general2_size,
		baby_knobby_eating1_size, baby_knobby_eating2_size
	},
	{
		toddler_knobby_general1_size, toddler_knobby_general2_size,
		toddler_knobby_eating1_size, toddler_knobby_eating2_size
	},
	{
		child_knobby_general1_size, child_knobby_general2_size,
		child_knobby_eating1_size, child_knobby_eating2_size
	},
	{
		teen_knobby_general1_size, teen_knobby_general2_size,
		teen_knobby_eating1_size, teen_knobby_eating2_size
	},
	{
		adult_knobby_general1_size, adult_knobby_general2_size,
		adult_knobby_eating1_size, adult_knobby_eating2_size
	},
	{
		senior_knobby_general1_size, senior_knobby_general2_size,
		senior_knobby_eating1_size, senior_knobby_eating2_size
	},
	{
		my_rock_0_size, my_rock_180_size,
		my_rock_0_size, my_rock_180_size
	},
};

const unsigned char* partner_sprites_images[][NUM_ACTIONS] PROGMEM = {
	{
        baby_knobby_general1, baby_knobby_general2,
		baby_knobby_eating1, baby_knobby_eating2
	},
	{
		toddler_knobby_general1, toddler_knobby_general2,
		toddler_knobby_eating1, toddler_knobby_eating2
	},
	{
		child_knobby_general1, child_knobby_general2,
		child_knobby_eating1, child_knobby_eating2
	},
	{
		teen_knobby_general1, teen_knobby_general2,
		teen_knobby_eating1, teen_knobby_eating2
	},
	{
		adult_knobby_general1, adult_knobby_general2,
		adult_knobby_eating1, adult_knobby_eating2
	},
	{
		senior_knobby_general1, senior_knobby_general2,
		senior_knobby_eating1, senior_knobby_eating2
	},
	{
		my_rock_0, my_rock_180,
		my_rock_0, my_rock_180

	},
};

const uint8_t* const (*CyberPartnerSprite::sprite_sizes)[NUM_ACTIONS] = partner_sprites_sizes;
const unsigned char* const (*CyberPartnerSprite::sprite_data)[NUM_ACTIONS] = partner_sprites_images;

/**
 * @brief Constructs a CyberPartnerSprite instance.
 *
 * This constructor initializes a `CyberPartnerSprite` object with the provided parameters.
 * It sets up the sprite's size, data, character, action, and position. Additionally, it
 * initializes the sprite's action by calling the `setAction` method.
 *
 * @param initialCharacter The initial character to associate with the sprite.
 * @param initialAction The initial action to associate with the sprite.
 * @param startX The starting X-coordinate of the sprite.
 * @param startBaseY The starting base Y-coordinate of the sprite.
 */
CyberPartnerSprite::CyberPartnerSprite(Character initialCharacter, Action initialAction,
										int16_t startX, int16_t startBaseY)
{
    // Initialize other members
    character = initialCharacter;
    x = startX;
    baseY = startBaseY;
    action = initialAction;

	// Initialize sprite dimensions based on the initial action
	width = pgm_read_byte(sprite_sizes[static_cast<size_t>(character)][static_cast<size_t>(action)]);
	height = pgm_read_byte(sprite_sizes[static_cast<size_t>(character)][static_cast<size_t>(action)] + 1);

    setAction(initialAction);
}

/**
 * @brief Sets the character for the CyberPartnerSprite instance.
 *
 * This function updates the `character` attribute of the `CyberPartnerSprite` instance
 * with the provided `newCharacter` value. After updating the character, it calls the
 * `setAction` method to ensure the sprite's size is updated correctly based on the
 * current action.
 *
 * @param newCharacter The new character to set for the sprite.
 */
void CyberPartnerSprite::setCharacter(Character newCharacter) {
    character = newCharacter;
    setAction(action); // Ensure size updates correctly
}

/**
 * @brief Retrieves the character associated with the CyberPartnerSprite instance.
 *
 * This function returns the current value of the `character` attribute
 * of the `CyberPartnerSprite` instance.
 *
 * @return The current character of the CyberPartnerSprite instance.
 */
Character CyberPartnerSprite::getCharacter() {
    return character;
}

/**
 * @brief Sets the action for the CyberPartnerSprite instance.
 *
 * This function updates the `action` attribute of the `CyberPartnerSprite` instance
 * with the provided `newAction` value. It also adjusts the sprite's `width` and `height`
 * attributes by reading the corresponding size values from the `sprite_sizes` array
 * based on the current character and action.
 *
 * @param newAction The new action to set for the sprite.
 */
void CyberPartnerSprite::setAction(Action newAction) {
	if ((uint8_t)character >= NUM_CHARACTERS || (uint8_t)newAction >= NUM_ACTIONS) {
        Serial.println(F("Error: Invalid character or action index."));
        return;
    }

    action = newAction;
    width = pgm_read_byte(&sprite_sizes[(uint8_t) character][(uint8_t) action][0]);
    height = pgm_read_byte(&sprite_sizes[(uint8_t) character][(uint8_t) action][1]);
}

/**
 * @brief Sets the direction for the CyberPartnerSprite instance.
 *
 * This function updates the `direction` attribute of the `CyberPartnerSprite` instance
 * with the provided `newDirection` value.
 *
 * @param newDirection The new direction to set for the sprite.
 */
void CyberPartnerSprite::setDirection(Direction newDirection) {
    direction = newDirection;
}

/**
 * @brief Sets the location of the CyberPartnerSprite instance.
 *
 * This function updates the `x` and `baseY` attributes of the `CyberPartnerSprite` instance
 * to set its position on the screen.
 *
 * @param startX The new X-coordinate of the sprite.
 * @param startBaseY The new base Y-coordinate of the sprite.
 */
void CyberPartnerSprite::setLocation(int16_t startX, int16_t startBaseY) {
    x = startX;
    baseY = startBaseY;
}

/**
 * @brief Retrieves the X-coordinate of the CyberPartnerSprite instance.
 *
 * This function returns the current value of the `x` attribute, which represents
 * the horizontal position of the sprite on the screen.
 *
 * @return The X-coordinate of the CyberPartnerSprite instance as an unsigned 16-bit integer.
 */
uint16_t CyberPartnerSprite::getXLocation() {
    return x;
}

/**
 * @brief Moves the CyberPartnerSprite instance and updates its action and direction.
 *
 * This function adjusts the position of the `CyberPartnerSprite` instance based on its
 * current direction and randomly changes its action between `Action::General1` and
 * `Action::General2`. It also ensures the sprite stays within the screen boundaries.
 *
 * The function performs the following steps:
 * - Randomly changes the sprite's action after a certain number of frames.
 * - Updates the X-coordinate (`x`) based on the current direction.
 * - Randomly determines a new direction based on a delta value.
 * - Ensures the sprite does not move outside the screen boundaries.
 *
 * @note The screen boundaries are defined by `BDISPLAY::SCREEN_WIDTH`.
 */
void CyberPartnerSprite::move(uint8_t minX) {
    static uint8_t prevActionChange = 0;

    // Change action randomly within a certain range
    if (prevActionChange == 0) {
        prevActionChange = random(5, 10);
        setAction(action == Action::General1 ? Action::General2 : Action::General1);
    } else {
        prevActionChange--;
    }

    // Adjust movement based on direction
    x += (direction == Direction::Left ? -1 : 1);
    int8_t deltaX = random(direction == Direction::Left ? -30 : -5, direction == Direction::Left ? 5 : 30);

    // Randomly determine a new direction
    direction = (deltaX > 0) ? Direction::Right : Direction::Left;

    // Ensure the sprite stays within the screen boundaries
    if (x < minX) {
        direction = Direction::Right;
        x = minX;
    } else if (x + width > BDISPLAY::SCREEN_WIDTH) {
        direction = Direction::Left;
        x = BDISPLAY::SCREEN_WIDTH - width;
    }
}

/**
 * @brief Helper function to render the sprite on the display.
 *
 * This function handles both drawing and erasing the sprite by specifying the color.
 *
 * @param color The color to use for rendering (e.g., WHITE for drawing, BLACK for erasing).
 */
void CyberPartnerSprite::renderSprite(uint16_t color) {
    int16_t y = baseY - height;
    if (direction == Direction::Right) {
        BDISPLAY::drawBitmapFlippedH(x, y, sprite_data[(uint8_t)character][(uint8_t)action], width, height);
    } else {
        BDISPLAY::display.drawBitmap(x, y, sprite_data[(uint8_t)character][(uint8_t)action], width, height, color);
    }
}

/**
 * @brief Draws the CyberPartnerSprite instance on the display.
 *
 * This function renders the sprite on the screen based on its current position,
 * direction, and action. If the sprite's direction is `Direction::Right`, the
 * sprite is drawn with a horizontally flipped bitmap. Otherwise, it is drawn
 * normally.
 *
 * @note The sprite's position is determined by the `x` and `baseY` attributes,
 *       and the bitmap data is retrieved from the `sprite_data` array.
 */
void CyberPartnerSprite::draw() {
    renderSprite(WHITE);
}

/**
 * @brief Erases the CyberPartnerSprite instance from the display.
 *
 * This function removes the sprite from the screen by redrawing its bitmap
 * in black. If the sprite's direction is `Direction::Right`, the bitmap is
 * horizontally flipped before being erased. Otherwise, it is erased normally.
 *
 * @note The position of the sprite is determined by the `x` and `baseY` attributes,
 *       and the bitmap data is retrieved from the `sprite_data` array.
 */
void CyberPartnerSprite::erase() {
    renderSprite(BLACK);
}
