#pragma once

#include <tmpl8/enum_class_flags.hpp>

namespace tmpl8
{
	enum class modifiers
	{
		none    = 0x0,
		shift   = 0x1,
		control = 0x2,
		alt     = 0x4,
		super   = 0x8
	};

	template <> struct is_enum_class_flags<modifiers> : std::true_type { };
}
