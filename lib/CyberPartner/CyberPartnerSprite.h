/**
 * @file CyberPartnerSprite.h
 * @brief Defines the CyberPartnerSprite class for handling character sprite rendering.
 *
 * This class provides functionality for managing, animating, and displaying character
 * sprites. It supports different characters, actions, and movement mechanics.
 *
 * Dependencies:
 * - badge_display.h (for rendering bitmaps)
 * - CyberPartner_bitmaps.h (for sprite data)
 */
#pragma once

#include "badge_display.h"
#include "CyberPartner_bitmaps.h"

extern const uint8_t* birth_death_sprites_sizes[] PROGMEM;
extern const unsigned char* birth_death_sprites_images[] PROGMEM;

constexpr uint8_t NUM_CHARACTERS = 9;
constexpr uint8_t NUM_ACTIONS = 10;

extern const uint8_t* partner_sprites_sizes[][NUM_ACTIONS] PROGMEM;
extern const unsigned char* partner_sprites_images[][NUM_ACTIONS] PROGMEM;

/**
 * @enum Direction
 * @brief Represents possible movement directions for the sprite.
 */
enum class Direction {
    Left,  /**< Moving left */
    Right, /**< Moving right */
    None   /**< No movement */
};

/**
 * @enum Character
 * @brief Enumerates different character types that can be displayed as sprites.
 */
enum class Character {
    babyKnobby,
    toddlerKnobby,
    childKnobby,
    teenKnobby,
    adultKnobby,
    seniorKnobby,
    rock,
    COUNT
};

/**
 * @enum Action
 * @brief Enumerates possible actions a sprite can perform.
 */
enum class Action {
    General1,
    General2,
    Eating1,
    Eating2,
};

/**
 * @class CyberPartnerSprite
 * @brief Represents a sprite for a cyber partner character.
 */
class CyberPartnerSprite {
    public:
        /**
         * @brief Constructs a CyberPartnerSprite with specified parameters.
         * @param initialCharacter The initial character type.
         * @param initialAction The initial action state.
         * @param startX The initial x-coordinate of the sprite.
         * @param startBaseY The initial base y-coordinate of the sprite.
         */
        CyberPartnerSprite(Character initialCharacter, Action initialAction,
                           int16_t startX, int16_t startBaseY);

        /**
         * @brief Sets the character of the sprite.
         * @param newCharacter The new character type to set.
         */
        void setCharacter(Character newCharacter);

        /**
         * @brief Gets the current character of the sprite.
         * @return The character type of the sprite.
         */
        Character getCharacter();

        /**
         * @brief Sets the action of the sprite and updates its size accordingly.
         * @param newAction The new action state to set.
         */
        void setAction(Action newAction);

        /**
         * @brief Sets the direction in which the sprite is facing.
         * @param newDirection The new direction (Left, Right, None).
         */
        void setDirection(Direction newDirection);

        /**
         * @brief Sets the location of the sprite.
         * @param startX The new x-coordinate.
         * @param startBaseY The new base y-coordinate.
         */
        void setLocation(int16_t startX, int16_t startBaseY);

        /**
         * @brief Retrieves the current x-coordinate of the sprite.
         * @return The x-coordinate of the sprite.
         */
        uint16_t getXLocation();

        /**
         * @brief Moves the sprite based on its direction and updates its animation state.
         */
        void move(uint8_t minX);

        /**
         * @brief Draws the sprite at its current position.
         */
        void draw();

        /**
         * @brief Erases the sprite from the screen.
         */
        void erase();

    private:
        static const uint8_t* const (*sprite_sizes)[NUM_ACTIONS];  /**< Array of sprite size data */
        static const unsigned char* const (*sprite_data)[NUM_ACTIONS];  /**< Array of sprite image data */
        uint8_t width = 0;  /**< Width of the current sprite */
        uint8_t height = 0; /**< Height of the current sprite */
        int16_t x = 0;      /**< X-coordinate of the sprite */
        int16_t baseY = 0;  /**< Baseline Y-coordinate of the sprite */
        Character character = Character::babyKnobby; /**< Current character */
        Action action  = Action::General1;      /**< Current action */
        Direction direction = Direction::None;  /**< Current movement direction */

        void renderSprite(uint16_t color);
    };
