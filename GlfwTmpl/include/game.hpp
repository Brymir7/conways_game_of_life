#pragma once

#include <tmpl8/integers.hpp>
#include <tmpl8/mouse_button.hpp>
#include <tmpl8/key.hpp>
#include <tmpl8/modifiers.hpp>
#include <tmpl8/surface.hpp>

class game
{
public:
	/** @brief  Creates the variables for your game. */
	explicit game(tmpl8::surface& screen);
	/** @brief  Destroys the variables for your game. */
	~game();
	
	/**
	 * @brief  Updates and renders your game.
	 * @param  delta_time  The time in seconds since the last frame.
	 */
	void tick(float delta_time);
	
	/**
	 * @brief  Called when a mouse button is pressed.
	 * @param  button     Which mouse button is pressed.
	 * @param  modifiers  The modifier keys that are being held.
	 */
	void mouse_down(tmpl8::mouse_button button, tmpl8::modifiers modifiers);
	/**
	 * @brief  Called when a mouse button is released.
	 * @param  button     Which mouse button is released.
	 * @param  modifiers  The modifier keys that are being held.
	 */
	void mouse_up  (tmpl8::mouse_button button, tmpl8::modifiers modifiers);
	/**
	 * @brief  Called when the mouse is moved over the screen.
	 * @param  x  The x coordinate of the mouse in screen space.
	 * @param  y  The y coordinate of the mouse in screen space.
	 */
	void mouse_move(int32_t x, int32_t y);

	/**
	 * @brief  Called when a keyboard button is pressed.
	 * @param  key        Which key is pressed.
	 * @param  modifiers  The modifier keys that are being held.
	 */
	void key_up    (tmpl8::key key, tmpl8::modifiers modifiers);
	/**
	 * @brief  Called when a keyboard button is released.
	 * @param  key        Which key is released.
	 * @param  modifiers  The modifier keys that are being held.
	 */
	void key_down  (tmpl8::key key, tmpl8::modifiers modifiers);
	/**
	 * @brief  Called when a keyboard button is repeated (held for 
	 *         an extended period of time).
	 * @param  key        Which key is repeated.
	 * @param  modifiers  The modifier keys that are being held.
	 */
	void key_repeat(tmpl8::key key, tmpl8::modifiers modifiers);
	/**
	 * @brief  Called when a character is typed.
	 * @param  letter  Which character is typed (Unicode).
	 */
	void key_char  (uint32_t   letter);

	// Deletes the default constructor, copy constructor and
	// copy assignment operator.
	game           ()            = delete;
	game           (const game&) = delete;
	game& operator=(const game&) = delete;

private:
	// You can add your own variables here.
	tmpl8::surface& screen_;
};