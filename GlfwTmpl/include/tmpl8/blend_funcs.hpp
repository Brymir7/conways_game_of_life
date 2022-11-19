#pragma once

#include <tmpl8/integers.hpp>

namespace tmpl8
{
	template <typename T> constexpr T blend_min(const T& a, const T& b) { return a < b ? a : b; }
	template <typename T> constexpr T blend_max(const T& a, const T& b) { return a > b ? a : b; }

	struct blend_none
	{
		constexpr pixel operator()(pixel /*dst*/, pixel src) const
		{
			return src;
		}
	};
	struct blend_add
	{
		constexpr pixel operator()(pixel dst, pixel src) const
		{
			return blend_min<pixel>((dst & 0x000000ff) + (src & 0x000000ff), 0x000000ff)  // b
				 | blend_min<pixel>((dst & 0x0000ff00) + (src & 0x0000ff00), 0x0000ff00)  // g
				 | blend_min<pixel>((dst & 0x00ff0000) + (src & 0x00ff0000), 0x00ff0000)  // r
				 | blend_min<pixel>((dst & 0xff000000) + (src & 0xff000000), 0xff000000); // a
		}
	};
	struct blend_alpha
	{
		constexpr pixel operator()(pixel dst, pixel src) const
		{
			uint8_t src_alpha = (src >> 24) & 0xff;
			uint8_t inv_src_alpha = 255 - src_alpha;

			uint32_t r = ((src & 0x00ff0000) * src_alpha + (dst & 0x00ff0000) * inv_src_alpha) / 255 & 0x00ff0000;
			uint32_t g = ((src & 0x0000ff00) * src_alpha + (dst & 0x0000ff00) * inv_src_alpha) / 255 & 0x0000ff00;
			uint32_t b = ((src & 0x000000ff) * src_alpha + (dst & 0x000000ff) * inv_src_alpha) / 255 & 0x000000ff;
			return r | g | b;
		}
	};
}