#include <game.hpp>
#include <config.hpp>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#ifdef _WIN32
#include <windows.h>

void sleep(unsigned milliseconds)
{
	Sleep(milliseconds);
}
#else
#include <unistd.h>

void sleep(unsigned milliseconds)
{
	usleep(milliseconds * 1000); // takes microseconds
}
#endif

using namespace tmpl8;
using namespace config;

typedef struct Grid {
	size_t width_;
	size_t height_;
	bool* cells_;
	bool* cells_buffer_;
} Grid;


int32_t mouse_x, mouse_y;
float timer = 0;
bool start = false;
Grid grid;

Grid grid_init(size_t width, size_t height) {
	size_t cell_count = width * height;
	auto memory = static_cast<bool*>(calloc(cell_count * 2, sizeof(bool)));;
	assert(memory);
	Grid grid = {
		.width_ = width,
		.height_ = height,
		.cells_ = memory,
		.cells_buffer_ = memory + cell_count,
	};
	return grid;
}
void grid_free(Grid* grid) {
	free(grid->cells_ < grid->cells_buffer_ ? grid->cells_ : grid->cells_buffer_);
}
bool get_cell(Grid* grid, size_t x, size_t y) {
	assert(x < grid->width_);
	assert(y < grid->height_);
	return grid->cells_[y * grid->width_ + x];
}

size_t alive_neighbours(Grid* grid, size_t x, size_t y) {
	size_t min_x = x == 0 ? 0 : x - 1;
	size_t max_x = x == grid->width_ - 1 ? grid->width_ - 1 : x + 1;
	size_t min_y = y == 0 ? 0 : y - 1;
	size_t max_y = y == grid->height_ - 1 ? grid->height_ - 1 : y + 1;
	size_t alive_neighbour_count = 0;
	for (size_t current_y = min_y; current_y <= max_y; ++current_y) {
		for (size_t current_x = min_x; current_x <= max_x; ++current_x) {
			if (get_cell(grid, current_x, current_y)) {
				++alive_neighbour_count;
			}
		}
	}
	if (get_cell(grid, x, y)) {
		--alive_neighbour_count;
	}
	return alive_neighbour_count;
}
void write_cell(Grid* grid, size_t x, size_t  y, bool new_value) {
	assert(x < grid->width_);
	assert(y < grid->height_);
	grid->cells_[y * grid->width_ + x] = new_value;
}
void write_cell_cells_buffer(Grid* grid, size_t x, size_t y, bool new_value) {
	assert(x < grid->width_);
	assert(y < grid->height_);
	grid->cells_buffer_[y * grid->width_ + x] = new_value;
}
void grid_next_generation(Grid* grid) {
	for (size_t y = 0; y < grid->height_; ++y) {
		for (size_t x = 0; x < grid->width_; ++x){
			size_t alive_neighbours_count = alive_neighbours(grid, x, y);
			bool new_state = alive_neighbours_count == 3 ||
				(alive_neighbours_count == 2 && get_cell(grid, x, y));
			write_cell_cells_buffer(grid, x, y, new_state);
		}
	}
	bool* temp = grid->cells_;
	grid->cells_ = grid->cells_buffer_;
	grid->cells_buffer_ = temp;
}

void grid_print(Grid* grid, surface& screen) {
	assert(grid->height_ <= screen_height);
	assert(grid->width_ <= screen_width);
	for (size_t y = 0; y < grid->height_; ++y) {
		for (size_t x = 0; x < grid->width_; ++x) {
			if (get_cell(grid, x, y)) {
				screen.plot(x, y, 0xffffffff);
			}
		}
	}
}


game::game(surface& screen) : screen_(screen)
{	
	grid = grid_init(screen.width(), screen.height());
}


game::~game()
{
	grid_free(&grid);
}

void game::tick(float delta_time)
{
	if (start) {
		timer += delta_time;
		if (timer >= 0.1f) {
			timer -= 0.1f;
			grid_next_generation(&grid);
	}
		screen_.clear(0x000000);
		grid_print(&grid, screen_);
	}
}


void game::mouse_down(mouse_button button, modifiers modifiers)
{	
	get_cell(&grid, mouse_x, mouse_y) ? write_cell(&grid, mouse_x, mouse_y, false) : write_cell(&grid, mouse_x, mouse_y, true);
	grid_print(&grid, screen_);
}

void game::mouse_up(mouse_button button, modifiers modifiers)
{
	
	
}

void game::mouse_move(int32_t x, int32_t y)
{
	mouse_x = x;
	mouse_y = y;
}


void game::key_down(key key, modifiers modifiers)
{
	switch (key) {
	case key::w:
		if (start != true)
			start = true;
		else
			start = false;
		break;
	case key::a:
		if (start != true) {
			grid_print(&grid, screen_);
		}
	}
	
}

void game::key_up(key key, modifiers modifiers)
{

}

void game::key_repeat(key key, modifiers modifiers)
{
}

void game::key_char(uint32_t letter)
{

}

