#pragma once
#include "sprite.h"

namespace BLIB {

	class sprite_batch : public sprite {
	private:
		const size_t max_vertices;
		std::vector<vertex> vertices;

		void begin(color color, float2 tile_size);
		void end();

	protected:

		void prerender(float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_index, float2 tile_size = float2{0});
		virtual void render_all(float2 pos, float2 scale, float2 pivot, float rotation) = 0;

	public:
		sprite_batch(const string& filename, size_t max_sprites, flags flags = batch_flags);
		sprite_batch(sprite_batch&& o) noexcept;
		sprite_batch& operator= (sprite_batch&&) = default;
		~sprite_batch() {}

		void render(float2 pos, float2 scale, float2 pivot, float rotation, color color, float2 tile_index, float2 tile_size) override {
			begin(color, tile_size);
			render_all(pos, scale, pivot, rotation);
			end();
		}
	};
}