#include <tmpl8/surface.hpp>
#include <lodepng/lodepng.hpp>
#include <vector>
#include <algorithm>

namespace
{
	/*
	 * Data for text rendering.
	 * 
	 * Every 64-bit number represents one glyph. It is encoded as a stream from least till
	 * most significant bit.
	 *
	 * The first 12 bits are header data, containing the following 3, 4-bit numbers:
	 *  - Bottom offset. The amount of pixels from the bottom to start rendering at.
	 *  - Width. The amount of horizontal pixels in the glyph, and its "advance".
	 *  - Height. The amount of vertical pixels in the glyph.
	 *
	 * After the header, each bit is a pixel in the glyph ordered in rows of "width" bits
	 * left-to-right, bottom-to-top.
	 *
	 * Example for encoding a 2x3 glyph with a vertical offset of 4 pixels looking like this:
	 * +--+--+
	 * |  |XX|
	 * +--+--+
	 * |XX|XX|
	 * +--+--+
	 * |XX|  |
	 * +--+--+
	 * Row 2  Row 1  Row 0  Height  Width  Bottom
	 * 10     11     01     0011    0010   0100
	 *
	 * Binary literal:      0b101101001100100100
	 * Hexadecimal literal: 0x2d324
	 *
	 * In the table below the data is padded with zeroes in front of it, for better readability.
	 */
	constexpr uint64_t font_data[128 - 33] =
	{
		                           /* ! */0x000000000007d712, /* " */0x000000000002d237, /* # */0x0000295f57d4a752,
		/* $ */0x000013c1741e4752, /* % */0x0000452820a51752, /* & */0x00001144b3536752, /* ' */0x0000000000003217,
		/* ( */0x000000c21112c742, /* ) */0x0000003488843742, /* * */0x0000000000969346, /* + */0x0000000427c84552,
		/* , */0x0000000000007311, /* - */0x000000000001f154, /* . */0x0000000000003212, /* / */0x0000410820841752,
		/* 0 */0x00003a39ace2e752, /* 1 */0x000010c42109f752, /* 2 */0x00003a306083f752, /* 3 */0x00003a306422e752,
		/* 4 */0x000062928fe10752, /* 5 */0x00007c2f8422e752, /* 6 */0x000030417c62e752, /* 7 */0x00007e3041084752,
		/* 8 */0x00003a317462e752, /* 9 */0x00003a31f4106752, /* : */0x0000000000033612, /* ; */0x0000000000067711,
		/* < */0x0000008421248742, /* = */0x00000000f801f453, /* > */0x0000001248421742, /* ? */0x00003a3041004752,
		/* @ */0x001e86db7d07e762, /* A */0x00003a3f8c631752, /* B */0x00003e2f8c62f752, /* C */0x00003a210862e752,
		/* D */0x00003e318c62f752, /* E */0x00007c270843f752, /* F */0x00007c2708421752, /* G */0x0000783d8c62e752,
		/* H */0x0000463f8c631752, /* I */0x00000001d2497732, /* J */0x000042108422e752, /* K */0x000045274c631752,
		/* L */0x000004210843f752, /* M */0x000047758c631752, /* N */0x00004675cc631752, /* O */0x00003a318c62e752,
		/* P */0x00003e2f08421752, /* Q */0x00003a318c536752, /* R */0x00003e2f8c631752, /* S */0x0000782e8422e752,
		/* T */0x00007c8421084752, /* U */0x000046318c62e752, /* V */0x000046318a944752, /* W */0x000046318d771752,
		/* X */0x0000454454631752, /* Y */0x0000454421084752, /* Z */0x00007e082083f752, /* [ */0x00000001c924f732,
		/* \ */0x0000044222110752, /* ] */0x00000001e4927732, /* ^ */0x0000000001151356, /* _ */0x000000000001f151,
		/* ` */0x0000000000006228, /* a */0x0000000e87a3e552, /* b */0x0000042d9c62f752, /* c */0x0000000e8862e552,
		/* d */0x00004216cc63e752, /* e */0x0000000e8fc3e552, /* f */0x000000c2f2222742, /* g */0x000003d18fa0f651,
		/* h */0x0000042d9c631752, /* i */0x000000000005f712, /* j */0x000802108462e851, /* k */0x0000001195359742,
		/* l */0x0000000001556722, /* m */0x0000000bad631552, /* n */0x0000000f8c631552, /* o */0x0000000e8c62e552,
		/* p */0x000001b38bc21651, /* q */0x000002d98fa10651, /* r */0x0000000d98421552, /* s */0x0000001e0ba0f552,
		/* t */0x00000000ba494732, /* u */0x000000118c63e552, /* v */0x000000118c544552, /* w */0x000000118d6be552,
		/* x */0x0000001151151552, /* y */0x000002318fa0f651, /* z */0x0000001f4105f552, /* { */0x000000c22122c742,
		/* | */0x00000000000ff811, /* } */0x0000003448443742, /* ~ */0x0000000000999267,        0x00007af7b4210752,
	};

	// The last character (ASCII 0x7f or DEL) is the paragraph symbol, and is used
	// to indicate an invalid printed character.
	// All control characters (ASCII < 0x20) except DEL are treated as a space.
}

namespace tmpl8
{
	surface::surface() noexcept :
		buffer_(nullptr),
		width_(0), height_(0), pitch_(0)
	{
	}

	surface::surface(const surface& other) :
		width_ (other.width_),
		height_(other.height_),
		pitch_ (other.pitch_)
	{
		buffer_ = std::make_unique<pixel[]>(height_ * pitch_);
		std::memcpy(buffer_.get(), other.buffer_.get(), sizeof(pixel) * height_ * pitch_);
	}

	surface& surface::operator=(const surface& other)
	{
		if (this == &other) return *this;
		if (width_  != other.width_  ||
			height_ != other.height_ ||
			pitch_  != other.pitch_)
		{
			width_  = other.width_;
			height_ = other.height_;
			pitch_  = other.pitch_;
			buffer_ = width_ == 0
				? nullptr
				: std::make_unique<pixel[]>(pitch_ * height_);
		}

		std::memcpy(buffer_.get(), other.buffer_.get(), sizeof(pixel) * pitch_ * height_);

		return *this;
	}

	surface::surface(surface&& other) noexcept :
		buffer_(std::move(other.buffer_)),
		width_ (other.width_),
		height_(other.height_),
		pitch_ (other.pitch_)
	{
		other.width_  = 0;
		other.height_ = 0;
		other.pitch_  = 0;
	}

	surface& surface::operator=(surface&& other) noexcept
	{
		width_  = other.width_;
		height_ = other.height_;
		pitch_  = other.pitch_;
		buffer_ = std::move(other.buffer_);

		other.width_  = 0;
		other.height_ = 0;
		other.pitch_  = 0;

		return *this;
	}

	surface::surface(const char* file_path) : surface()
	{
		std::vector<uint8_t> data;
		uint32_t w, h;
		lodepng::decode(data, w, h, std::string(file_path));

		width_ = w;
		height_ = h;
		pitch_ = w;

		buffer_ = std::make_unique<pixel[]>(w * h);

		for (uint32_t y = 0; y < h; ++y) {
			for (uint32_t x = 0; x < w; ++x) {
				uint8_t r = data[(y * w + x) * 4 + 0];
				uint8_t g = data[(y * w + x) * 4 + 1];
				uint8_t b = data[(y * w + x) * 4 + 2];
				uint8_t a = data[(y * w + x) * 4 + 3];
				buffer_[(h - y - 1) * w + x] = a << 24 | r << 16 | g << 8 | b;
			}
		}
	}

	surface::surface(int32_t width, int32_t height) : surface(width, height, width) { }
	surface::surface(int32_t width, int32_t height, int32_t pitch) :
		width_ (width),
		height_(height),
		pitch_ (pitch)
	{
		assert(width_ > 0 && height_ > 0 && pitch_ >= width_);
		buffer_ = std::make_unique<pixel[]>(pitch_ * height_);
	}

	surface::surface(int32_t width, int32_t height, std::unique_ptr<pixel[]> buffer, int32_t pitch) :
		buffer_(std::move(buffer)),
		width_ (width),
		height_(height),
		pitch_ (pitch)
	{
		assert(width_ > 0 && height_ > 0 && pitch_ >= width_);
		assert(buffer_ != nullptr);
	}

	surface::operator bool() const noexcept
	{
		return buffer_ != nullptr;
	}

	pixel surface::operator()(int32_t x, int32_t y) const
	{
		return buffer_[x + y * pitch_];
	}

	void surface::save_to_file(const char* file_path) const
	{
		std::vector<unsigned char> saveData(width_ * height_ * sizeof(pixel));

		for (size_t y = 0; y < height_; y++)
		{
			memcpy(saveData.data() + y * width_ * sizeof(pixel), buffer_.get() + (height_ - 1 - y) * pitch_, sizeof(pixel) * width_);
		}
		lodepng::encode(file_path, saveData, width_, height_);
	}

	void surface::clear(pixel clear_color)
	{
		assert(*this);
		
		pixel clear_hue = clear_color & 0xff;
		if (clear_hue == ((clear_color >>  8) & 0xff) &&
			clear_hue == ((clear_color >> 16) & 0xff) &&
			clear_hue == ((clear_color >> 24) & 0xff))
		{
			std::memset(buffer_.get(), clear_hue, sizeof(pixel) * height_ * pitch_);
			return;
		}

		pixel* dst = buffer_.get(); // Destination
		size_t adv = pitch_ - width_; // Advance after each row
		for (size_t y = 0; y < height_; y++, dst += adv)
		{
			for (size_t x = 0; x < width_; x++, dst++) *dst = clear_color;
		}
	}

	void surface::plot(int32_t x, int32_t y, pixel color)
	{
		assert(*this);
		assert(x >= 0 && y >= 0 && x < width_ && y < height_);
		buffer_[x + y * pitch_] = color;
	}

	void print_helper(const char* str_buffer, size_t str_length, int32_t x, int32_t y, pixel color, pixel* buffer, int32_t pitch)
	{
		for (size_t i = 0; i < str_length; i++)
		{
			size_t c = str_buffer[i];
			if (c < 0x21) { x += 4; continue; }
			if (c > 0x7f) c = 0x7f;

			uint64_t data = font_data[c - 0x21];
			auto bottom = static_cast<int32_t>(data & 0xf); data >>= 4;
			auto width  = static_cast<int32_t>(data & 0xf); data >>= 4;
			auto height = static_cast<int32_t>(data & 0xf); data >>= 4;

			pixel* dst = buffer + x + (y + bottom) * pitch;
			size_t adv = pitch - width;
			for (int32_t cy = 0; cy < height; cy++, dst += adv)
			{
				for (int32_t cx = 0; cx < width; cx++, dst++)
				{
					if (data & 1) *dst = color;
					data >>= 1;
				}
			}

			x += width + 1;
		}
	}

	void surface::print(const std::string& string, int32_t x, int32_t y, pixel color)
	{
		assert(*this);

		print_helper(string.data(), string.length(), x, y, color, buffer_.get(), pitch_);
	}

	void surface::print(const char* string, int32_t x, int32_t y, pixel color)
	{
		assert(*this);

		size_t len = strlen(string);
		print_helper(string, len, x, y, color, buffer_.get(), pitch_);
	}

	void surface::draw(const surface& image, int32_t x, int32_t y)
	{
		assert(image);
		draw(image, x, y, 0, 0, image.width_, image.height_);
	}

	void surface::draw(const surface& image, int32_t x, int32_t y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height)
	{
		assert(image);
		assert(*this);
		draw_blend<blend_none>(image, x, y, src_x, src_y, src_width, src_height);
	}

	void surface::line(float x1, float y1, float x2, float y2, pixel color)
	{
		assert(*this);
		if (x1 < 0 || y1 < 0 || x1 >= width_ || y1 >= height_ ||
			x2 < 0 || y2 < 0 || x2 >= width_ || y2 >= height_)
			return;

		float delta_x = x2 - x1;
		float delta_y = y2 - y1;
		float line_length = std::max(std::abs(delta_x), std::abs(delta_y));

		size_t step_count = static_cast<size_t>(line_length);
		float step_x = delta_x / line_length;
		float step_y = delta_y / line_length;
		for (size_t i = 0; i <= step_count; i++)
		{
			buffer_[static_cast<int>(x1) + static_cast<int>(y1) * pitch_] = color;
			x1 += step_x, y1 += step_y;
		}
	}

	void surface::box(int32_t x1, int32_t y1, int32_t x2, int32_t y2, pixel color)
	{
		assert(*this);

		if (x1 > x2) std::swap(x1, x2);
		if (y1 > y2) std::swap(y1, y2);

		if (x2 >= 0 && x1 < width_)
		{
			// Bottom
			size_t x_start = std::max(x1, 0);
			size_t x_end   = std::min(x2, width_ - 1);
			if (y1 >= 0)
			{
				for (size_t x = x_start; x <= x_end; x++)
					buffer_[y1 * pitch_ + x] = color;
			}
			// Top
			if (y1 != y2 && y2 < height_)
			{
				for (size_t x = x_start; x <= x_end; x++)
					buffer_[y2 * pitch_ + x] = color;
			}
		}
		if (y2 >= 0 && y1 < height_)
		{
			// Left
			size_t y_start = std::max(y1, 0);
			size_t y_end   = std::min(y2, height_ - 1);
			if (x1 >= 0)
			{
				for (size_t y = y_start; y <= y_end; y++)
					buffer_[y * pitch_ + x1] = color;
			}
			// Right
			if (x1 != x2 && x2 < width_)
			{
				for (size_t y = y_start; y <= y_end; y++)
					buffer_[y * pitch_ + x2] = color;
			}
		}
	}

	void surface::bar(int32_t x1, int32_t y1, int32_t x2, int32_t y2, pixel color)
	{
		assert(*this);

		if (x1 > x2) std::swap(x1, x2);
		if (y1 > y2) std::swap(y1, y2);
		x1 = std::max(x1, 0);
		y1 = std::max(y1, 0);
		x2 = std::min(x2, width_  - 1);
		y2 = std::min(y2, height_ - 1);

		pixel* dst = buffer_.get() + y1 * pitch_ + x1;
		size_t adv = pitch_ - (x2 + 1 - x1);
		for (size_t y = y1; y <= y2; y++, dst += adv)
		{
			for (size_t x = x1; x <= x2; x++, dst++)
				*dst = color;
		}
	}

	surface surface::resize(int32_t new_width, int32_t new_height) const
	{
		assert(*this);
		surface s(new_width, new_height);
		s.resize_from(*this);
		return s;
	}

	void surface::resize_from(const surface& /*target*/)
	{
		assert(*this);
		assert(!"Not implemented.");
	}
}
