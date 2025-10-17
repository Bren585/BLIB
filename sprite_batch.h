#pragma once
#include "sprite.h"

namespace BLIB {

	class sprite_batch : public sprite {
	private:
		const size_t max_vertices;
		std::vector<vertex> vertices;

	public:
		sprite_batch(const string& filename, size_t max_sprites, flags flags = batch_flags);
		sprite_batch(sprite_batch&& o) noexcept;
		sprite_batch& operator= (sprite_batch&&) = default;
		~sprite_batch() {}

		// maybe make this work again, ya dig?
		virtual void render(float2 pos, float2 size, float2 tpos = 0, float2 tsize = 0, float angle = 0, float2 c = C_CC, color color = { 1, 1, 1 }) override {}

		void prerender(float2 pos, float2 size, float2 tpos = 0, float2 tsize = 0, float angle = 0, float2 c = C_CC, color color = { 1, 1, 1 });
		void begin();
		void end();
	};
}