#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <tmpl8/integers.hpp>
#include <tmpl8/blend_funcs.hpp>

namespace tmpl8
{
	class surface final
	{
	public:
		surface() noexcept;
		surface           (const surface& other);
		surface& operator=(const surface& other);
		surface           (surface&& other) noexcept;
		surface& operator=(surface&& other) noexcept;

		explicit surface(const char* file_path);
		explicit surface(int32_t width, int32_t height);
		explicit surface(int32_t width, int32_t height, int32_t pitch);
		explicit surface(int32_t width, int32_t height, std::unique_ptr<pixel[]> buffer, int32_t pitch);

		~surface() = default;

		explicit operator bool() const noexcept;

		      pixel* buffer()       { return buffer_.get(); }
		const pixel* buffer() const { return buffer_.get(); }
		int32_t       width() const { return width_;  }
		int32_t      height() const { return height_; }
		int32_t       pitch() const { return pitch_;  }

		pixel operator()(int32_t x, int32_t y) const;

		void save_to_file(const char* file_path) const;

		void clear(pixel clear_color);

		void plot(int32_t x, int32_t y, pixel color);

		void print(const std::string& string, int32_t x, int32_t y, pixel color);
		void print(const char* string,        int32_t x, int32_t y, pixel color);

		void draw      (const surface& image, int32_t x, int32_t y);
		void draw      (const surface& image, int32_t x, int32_t y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height);
		template <typename t_blend_func = blend_add>
		void draw_blend(const surface& image, int32_t x, int32_t y, t_blend_func blend_func = t_blend_func());
		template <typename t_blend_func = blend_add>
		void draw_blend(const surface& image, int32_t x, int32_t y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, t_blend_func blend_func = t_blend_func());

		void line(float   x1, float   y1, float   x2, float   y2, pixel color);
		void box (int32_t x1, int32_t y1, int32_t x2, int32_t y2, pixel color);
		void bar (int32_t x1, int32_t y1, int32_t x2, int32_t y2, pixel color);
		
		surface resize(int32_t new_width, int32_t new_height) const;
		void    resize_from(const surface& target);

	private:
		std::unique_ptr<pixel[]> buffer_;
		int32_t                  width_;
		int32_t                  height_;
		int32_t                  pitch_;
	};

	template <typename t_blend_func>
	void surface::draw_blend(const surface& image, int32_t x, int32_t y, t_blend_func blend_func)
	{
		assert(image);
		draw_blend<t_blend_func>(image, x, y, 0, 0, image.width_, image.height_, std::forward<t_blend_func>(blend_func));
	}

	template <typename t_blend_func>
	void surface::draw_blend(const surface& image, int32_t x, int32_t y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, t_blend_func blend_func)
	{
		assert(image);
		assert(*this);

		// Clip left and bottom edge
		if (x < 0) { src_x -= x; src_width += x; x = 0; }
		if (y < 0) { src_y -= y; src_height += y; y = 0; }
		// Clip right and top edge
		if (x + src_width > width_) src_width -= x + src_width - width_;
		if (y + src_height > height_) src_height -= y + src_height - height_;

		// Ensure boundaries
		assert(src_x + src_width <= image.width_);
		assert(src_y + src_height <= image.height_);

		if (src_width <= 0 || src_height <= 0) return;

		pixel* dst = buffer_.get() + x + y * pitch_;
		pixel* src = image.buffer_.get() + src_x + src_y * image.pitch_;
		size_t advDst = pitch_ - src_width;
		size_t advSrc = image.pitch_ - src_width;
		for (size_t iy = 0; iy < src_height; iy++, dst += advDst, src += advSrc)
		{
			for (size_t ix = 0; ix < src_width; ix++, dst++, src++)
				*dst = blend_func(*dst, *src);
		}
	}
}
