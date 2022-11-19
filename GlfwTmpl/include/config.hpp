#pragma once

#include <tmpl8/integers.hpp>

namespace config
{
	/** The width of the game area in pixels. */
	constexpr int32_t  screen_width      = 100;
	/** The height of the game area in pixels. */
	constexpr int32_t  screen_height     = 100;

	/** True for vsync, false for uncapped. */
	constexpr bool     use_vsync         = true;
	/** The amount to scale the game up with. 2^n. */
	constexpr size_t   scale_shift       = 4;
	/** True to interpolate the pixels when scaling. (Smoother & blurrier, vs hard-edges.) */
	constexpr bool     scale_interpolate = false;

	/** The title of the window. */
	constexpr char const*    screen_title      = "Template";
	/** True to exit the game when escape is pressed, otherwise false. */
	constexpr bool     exit_on_escape    = true;
}
