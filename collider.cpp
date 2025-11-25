#include "pch.h"
#include "collider.h"
#include "object.h"
#include "entity.h"

namespace BLIB::collision {
#define OBJ_2D (flat::object*)
#define OBJ_3D (full::object*)

	collision_type resolve(collider_type a, collider_type b) {
		switch (a) {
		case none:			return no_move;
		case bully:
			switch (b) {
			case none:		return no_move;
			case bully:		return no_move;
			case loser:		return b_moves;
			case swing:		return b_moves;
			default:		return no_move;
			}
		case loser:
			switch (b) {
			case none:		return no_move;
			case bully:		return a_moves;
			case loser:		return compromise;
			case swing:		return a_moves;
			default:		return no_move;
			}
		case swing:
			switch (b) {
			case none:		return no_move;
			case bully:		return a_moves;
			case loser:		return b_moves;
			case swing:		return compromise;
			default:		return no_move;
			}
		default:			return no_move;
		}
	}

	// CHECK

	constexpr float vertical_cutoff = 0.966f;

	flat::collision check(flat::object* a, flat::object* b, bool do_move) {
		flat::collision result = check(a->peek_collider(), b->peek_collider(), !do_move);
		if (!result || result.depth == NO_HIT) { return false; }
		if (!do_move) { return result; }

		switch (resolve(a->peek_collider()->get_type(), b->peek_collider()->get_type())) {
		case no_move:									break;
		case a_moves:		move(a, b, result);			break;
		case b_moves:		move(b, a, -result);		break;
		case compromise:	move(a, b, result, true);	break;
		}

		return result;
	}

	flat::collision check(flat::entity* a, flat::object* b, bool do_move) {
		flat::collision result = check(a->peek_collider(), b->peek_collider(), !do_move);
		if (!result || result.depth == NO_HIT) { return false; }
		if (!do_move) { return result; }

		switch (resolve(a->peek_collider()->get_type(), b->peek_collider()->get_type())) {
		case no_move:									break;
		case a_moves:		move(a, b, result);			break;
		case b_moves:		move(b, OBJ_2D a, -result);	break;
		case compromise:	move(a, b, result, true);	break;
		}

		return result;
	}

	flat::collision check(flat::entity* a, flat::entity* b, bool do_move) {
		flat::collision result = check(a->peek_collider(), b->peek_collider(), !do_move);
		if (!result || result.depth == NO_HIT) { return false; }
		if (!do_move) { return result; }

		switch (resolve(a->peek_collider()->get_type(), b->peek_collider()->get_type())) {
		case no_move:									break;
		case a_moves:		move(a, b, result);			break;
		case b_moves:		move(b, a, -result);		break;
		case compromise:	move(a, b, result, true);	break;
		}

		return result;
	}

	flat::collision check(flat::object* a, flat::entity* b, bool do_move) { return check(b, a, do_move); }

	full::collision check(full::object* a, full::object* b, bool do_move) {
		full::collision result = check(a->peek_collider(), b->peek_collider(), !do_move);
		if (!result || result.depth == NO_HIT) { return false; }
		if (!do_move) { return result; }

		switch (resolve(a->peek_collider()->get_type(), b->peek_collider()->get_type())) {
		case no_move:									break;
		case a_moves:		move(a, b, result);			break;
		case b_moves:		move(b, a, -result);		break;
		case compromise:	move(a, b, result, true);	break;
		}

		return result;
	}

	full::collision check(full::entity* a, full::object* b, bool do_move) {
		full::collision result = check(a->peek_collider(), b->peek_collider(), !do_move);
		if (!result || result.depth == NO_HIT) { return false; }
		if (!do_move) { return result; }

		switch (resolve(a->peek_collider()->get_type(), b->peek_collider()->get_type())) {
		case no_move:									break;
		case a_moves:		move(a, b, result);			break;
		case b_moves:		move(b, OBJ_3D a, -result);	break;
		case compromise:	move(a, b, result, true);	break;
		}

		return result;
	}

	full::collision check(full::entity* a, full::entity* b, bool do_move) {
		full::collision result = check(a->peek_collider(), b->peek_collider(), !do_move);
		if (!result || result.depth == NO_HIT) { return false; }
		if (!do_move) { return result; }

		switch (resolve(a->peek_collider()->get_type(), b->peek_collider()->get_type())) {
		case no_move:									break;
		case a_moves:		move(a, b, result);			break;
		case b_moves:		move(b, a, -result);		break;
		case compromise:	move(a, b, result, true);	break;
		}

		return result;
	}

	full::collision check(full::object* a, full::entity* b, bool do_move) { return check(b, a, do_move); }

#undef OBJ_2D
#undef OBJ_3D

	// MOVE

	void move(flat::object* obj, flat::object* mover, flat::collision penetration, bool both) {
		if (both) penetration.depth *= 0.5f;
		obj->pos += penetration.normal * penetration.depth;
		if (both) mover->pos -= penetration.normal * penetration.depth;
	}

	void move(flat::entity* obj, flat::object* mover, flat::collision penetration, bool both) {
		if (both) penetration.depth *= 0.5f;

		obj->add_pos(penetration.normal * penetration.depth);
		obj->add_vel(-penetration.normal * dot(penetration.normal, obj->get_vel()));

		if (both) { mover->pos -= penetration.normal * penetration.depth; }
	}

	void move(flat::entity* obj, flat::entity* mover, flat::collision penetration, bool both) {
		float ma = obj->get_wgt();
		float mb = mover->get_wgt();
		float mass_ratio = both ? ma / (ma + mb) : 1.0f;

		obj->add_pos(penetration.normal * penetration.depth * mass_ratio);
		if (both) mover->add_pos(-penetration.normal * penetration.depth * (1 - mass_ratio));

		float2 relative_vel = obj->get_vel() - mover->get_vel();
		float vn = dot(relative_vel, penetration.normal);

		if (vn >= 0) return;

		float2 tangent = { -penetration.normal.y, penetration.normal.x };
		float vt = dot(relative_vel, tangent);

		float jn = -(1.0f + RESTITUTION) * vn / (mb + ma);
		float jt = -vt / (1.0f / ma + 1.0f / mb);
		jt = clamp(-fabsf(jn) * FRICTION, jt, fabsf(jn) * FRICTION);

		obj->add_vel((jn * penetration.normal * 2 * mb) + (jt * tangent / ma));
		if (both) { mover->add_vel((-jn * penetration.normal * 2 * ma) - (jt * tangent / mb)); }
	}

	void move(full::object* obj, full::object* mover, full::collision penetration, bool both) {
		if (both) penetration.depth *= 0.5f;
		obj->add_pos(penetration.normal * penetration.depth);
		if (both) mover->add_pos(-penetration.normal * penetration.depth);
	}

	void move(full::entity* obj, full::object* mover, full::collision penetration, bool both) {
		if (both) penetration.depth *= 0.5f;

		obj->add_pos(penetration.normal * penetration.depth);
		obj->add_vel(-penetration.normal * dot(penetration.normal, obj->get_vel()));

		if (both) { mover->add_pos(-penetration.normal * penetration.depth); }
	}

	void move(full::entity* obj, full::entity* mover, full::collision penetration, bool both) {
		float ma = obj->get_wgt();
		float mb = mover->get_wgt();
		float mass_ratio = both ? ma / (ma + mb) : 1.0f;

		obj->add_pos(penetration.normal * penetration.depth * mass_ratio);
		if (both) mover->add_pos(-penetration.normal * penetration.depth * (1 - mass_ratio));

		float3 relative_vel = obj->get_vel() - mover->get_vel();
		float vn = dot(relative_vel, penetration.normal);

		if (vn >= 0) return;

		float3 tangent = relative_vel - dot(relative_vel, penetration.normal) * penetration.normal;
		if ((tangent).mag() > 0) tangent.norm();
		else tangent = float3(0, 0, 0);
		float vt = dot(relative_vel, tangent);

		float jn = -(1.0f + RESTITUTION) * vn / (mb + ma);
		float jt = -vt / (1.0f / ma + 1.0f / mb);
		jt = clamp(-fabsf(jn) * FRICTION, jt, fabsf(jn) * FRICTION);

		obj->add_vel((jn * penetration.normal * 2 * mb) + (jt * tangent / ma));
		if (both) { mover->add_vel((-jn * penetration.normal * 2 * ma) - (jt * tangent / mb)); }
	}

	// LAND

	void land(flat::entity* obj, flat::collision penetration) {
		if		(penetration.normal.y >= vertical_cutoff)	{ obj->floor(); }
		else if (penetration.normal.y <= -vertical_cutoff)	{ obj->ceil(); }
		else												{ obj->wall(); }
	}

	void land(full::entity* obj, full::collision penetration) {
		if		(penetration.normal.y >= vertical_cutoff)	{ obj->floor(); }
		else if (penetration.normal.y <= -vertical_cutoff)	{ obj->ceil(); }
		else												{ obj->wall(); }
	}

	// RAY PICK

	bool ray_pick(const flat::object* obj, const float2& ray_origin, const float2& ray_direction, float2* out_position, float2* out_normal) {
		float dist = obj->peek_collider()->ray_pick(ray_origin, ray_direction, out_position, out_normal);
		return dist > 0 && dist != NO_HIT;
	}

	bool ray_pick(const flat::entity* obj, const float2& ray_origin, const float2& ray_direction, float2* out_position, float2* out_normal) {
		float dist = obj->peek_collider()->ray_pick(ray_origin, ray_direction, out_position, out_normal);
		return dist > 0 && dist != NO_HIT;
	}

	bool ray_pick(const full::object* obj, const float3& ray_origin, const float3& ray_direction, float3* out_position, float3* out_normal) {
		float dist = obj->peek_collider()->ray_pick(ray_origin, ray_direction, out_position, out_normal);
		return dist > 0 && dist != NO_HIT;
	}

	bool ray_pick(const full::entity* obj, const float3& ray_origin, const float3& ray_direction, float3* out_position, float3* out_normal) {
		float dist = obj->peek_collider()->ray_pick(ray_origin, ray_direction, out_position, out_normal);
		return dist > 0 && dist != NO_HIT;
	}

}

// 2D Colliders ***************************************************************************************************************

BLIB::flat::collision BLIB::collision::check(const flat::collider* a, const flat::collider* b, bool any) {
	if (!a || !b) return false;
	BLIB::flat::collision result(NO_HIT, {0, 0});
	BLIB::flat::collision temp_result(false);

	temp_result = a->collide(b);
	if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }

	for (const auto& a_child : a->get_children()) {
		temp_result = check(static_cast<flat::collider*>(a_child.get()), b, any);
		if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }
	}

	for (const auto& b_child : b->get_children()) {
		temp_result = check(a, static_cast<flat::collider*>(b_child.get()), any);
		if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }
	}

	for (const auto& a_child : a->get_children()) {
		for (const auto& b_child : b->get_children()) {
			temp_result = check(static_cast<flat::collider*>(a_child.get()), static_cast<flat::collider*>(b_child.get()), any);
			if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }
		}
	}

//#ifdef _DEBUG
//	float2 delta = a->get_pos() - b->get_pos();
//	if (result && result.depth != NO_HIT && delta.mag_sq() > EPS * EPS) _ASSERT_EXPR_A(dot(result.normal, delta) > 0, L"Normal does not follow B->A convention.");
//#endif

	return result;
}

namespace BLIB::flat {

	// HELPERS SHOULD ALWAYS RETURN B -> A FACING NORMALS

	float2 project_rect(const float2& pos, const float2& size, const float2 axes[2], const float2& axis) {
		float center = dot(pos, axis);
		float r = fabsf(size.x * dot(axes[0], axis)) + fabsf(size.y * dot(axes[1], axis));
		return { center - r, center + r };
	}

	collision SAT_rect_rect(const float2& a_pos, const float2& a_size, const float2 a_axes[2], const float2& b_pos, const float2& b_size, const float2 b_axes[2]) {
		float2 axes[4]{ a_axes[0], a_axes[1], b_axes[0], b_axes[1] };

		collision min_collision(NO_HIT, {0, 0});

		for (int i = 0; i < 4; i++) {
			float2 a_p = project_rect(a_pos, a_size, a_axes, axes[i]);
			float2 b_p = project_rect(b_pos, b_size, b_axes, axes[i]);
			if (a_p.y < b_p.x || a_p.x > b_p.y) { return false; }

			float depth = minim(a_p.y, b_p.y) - maxim(a_p.x, b_p.x);

			if (depth < min_collision.depth) {
				min_collision.depth = depth;
				min_collision.normal = (dot(a_pos - b_pos, axes[i]) < 0) ? -axes[i] : axes[i];
			}
		}

		return min_collision;
	}

	collision aligned_rect_rect(const aligned_rect_collider* a, const rect_collider* b) {
		float2 a_pos = a->get_pos();
		float2 a_size = a->get_size();
		float2 a_axes[2] = { {1, 0}, {0, 1} };

		float2 b_pos = b->get_pos();
		float2 b_size = b->get_size();
		float b_ang = b->get_ang();
		float2 b_axes[2] = { {cosf(b_ang), sinf(b_ang)}, {-sinf(b_ang), cosf(b_ang)} };

		return SAT_rect_rect(a_pos, a_size, a_axes, b_pos, b_size, b_axes);
	}

	collision aligned_rect_circle(const aligned_rect_collider* a, const circle_collider* b) {
		float2 a_pos = a->get_pos();
		float2 a_size = a->get_size();
		float2 a_min = a_pos - a_size;
		float2 a_max = a_pos + a_size;

		float2 b_pos = b->get_pos();
		float r = b->get_r();

		float2 closest = clamp(a_min, b_pos, a_max);
		float2 direction = (closest - b_pos);
		float dist_sq = direction.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float2 normal = (dist > EPS) ? (direction / dist) : float2{ 1, 0 };

		return { r - dist, normal };
	}

	collision rect_circle(const rect_collider* a, const circle_collider* b) {
		float2 a_pos = a->get_pos();
		float2 a_size = a->get_size();
		float a_ang = a->get_ang();

		float2 b_pos = b->get_pos();
		float r = b->get_r();

		float2 x_axis = { cosf(a_ang), sinf(a_ang) };
		float2 y_axis = { -sinf(a_ang), cosf(a_ang) };

		float2 local = { dot(b_pos - a_pos, x_axis), dot(b_pos - a_pos, y_axis) };
		float2 closest = clamp(-a_size, local, a_size);

		float2 direction = (closest - local);
		float dist_sq = direction.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float2 normal = (dist > 0.0f) ? (x_axis * direction.x + y_axis * direction.y) / dist : float2{ -1, 0 };

		return { r - dist, normal };
	}

	// DOUBLE DISPATCH REQUIRES THAT O = A, THIS = B
	// ENFORCE B -> A BY INVERSION IF NECESSARY

	// aligned rect
	collision aligned_rect_collider::collide_with(const aligned_rect_collider*	o) const {
		float2 b_pos = parent_position;
		float2 b_size = get_size();
		float2 b_min = b_pos - b_size;
		float2 b_max = b_pos + b_size;

		float2 a_pos = o->get_pos();
		float2 a_size = o->get_size();
		float2 a_min = a_pos - a_size;
		float2 a_max = a_pos + a_size;

		float2 overlap = {
			minim(a_max.x, b_max.x) - maxim(a_min.x, b_min.x),
			minim(a_max.y, b_max.y) - maxim(a_min.y, b_min.y)
		};

		if (overlap.x < EPS || overlap.y < EPS) { return false; }

		if (overlap.x < overlap.y) {
			float2 normal = (a_pos.x < b_pos.x) ? float2{ -1, 0 } : float2{ 1, 0 };
			return { overlap.x, normal };
		}
		else {
			float2 normal = (a_pos.y < b_pos.y) ? float2{ 0, -1 } : float2{ 0, 1 };
			return { overlap.y, normal };
		}
	}

	collision aligned_rect_collider::collide_with(const rect_collider*			o) const { return -aligned_rect_rect	(this, o); }
	collision aligned_rect_collider::collide_with(const circle_collider*		o) const { return -aligned_rect_circle	(this, o); }

	// rect
	collision rect_collider::collide_with(const rect_collider*			o) const {
		float2 b_pos = get_pos();
		float2 b_size = get_size();
		float  b_ang = get_ang();
		float2 b_axes[2] = { {cosf(b_ang), sinf(b_ang)}, {-sinf(b_ang), cosf(b_ang)} };

		float2 a_pos = o->get_pos();
		float2 a_size = o->get_size();
		float  a_ang = o->get_ang();
		float2 a_axes[2] = { {cosf(a_ang), sinf(a_ang)}, {-sinf(a_ang), cosf(a_ang)} };

		return SAT_rect_rect(a_pos, a_size, a_axes, b_pos, b_size, b_axes);
	}

	collision rect_collider::collide_with(const aligned_rect_collider*	o) const { return aligned_rect_rect	(o, this); }
	collision rect_collider::collide_with(const circle_collider*		o) const { return -rect_circle		(this, o); }

	// circle 
	collision circle_collider::collide_with(const circle_collider*			o) const {
		float rs = get_r() + o->get_r();
		float2 d = o->get_pos() - get_pos();
		float dist_sq = d.mag_sq();

		if (dist_sq > rs * rs) { return false; }

		float dist = sqrtf(dist_sq);
		float2 normal = (dist > EPS) ? d / dist : float2{ -1, 0 };
		return { rs - dist, normal };
	}

	collision circle_collider::collide_with(const aligned_rect_collider*	o) const { return aligned_rect_circle	(o, this); }
	collision circle_collider::collide_with(const rect_collider*			o) const { return rect_circle			(o, this); }

	// Ray Picking

	float collider::ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position, float2* out_normal) const {
		float2 ray_norm = ray_direction.norm();

		float min_hit = NO_HIT;
		float2 min_position;
		float2 min_normal;

		float hit = _ray_pick(ray_origin, ray_norm, (out_position || out_normal) ? &min_position : nullptr, out_normal ? &min_normal : nullptr);
		if (hit < min_hit) { min_hit = hit; }

		for (const auto& child : children) {
			float2 position;
			float2 normal;
			const collider* child_ptr = static_cast<collider*>(child.get());
			float hit = child_ptr->ray_pick(ray_origin, ray_norm, (out_position || out_normal) ? &position : nullptr, out_normal ? &normal : nullptr);
			if (!(out_position || out_normal)) { return hit; }
			else if (hit < min_hit) {
				min_position = position;
				min_normal = normal;
				min_hit = hit;
			}
		}

		if (out_position) { *out_position = min_position; }
		if (out_normal) { *out_normal = min_normal; }

		return min_hit;
	}

	float ray_pick_edge(float2 ray_origin, float2 ray_direction, float2 edge_start, float2 edge_end, float2* out_position, float2* out_normal) {
		float2 edge_direction = edge_end - edge_start;
		float denom = edge_direction % ray_direction;
		if (fabsf(denom) > EPS) {
			float2 delta = edge_start - ray_origin;
			float t = (edge_direction % delta) / denom;
			float u = (ray_direction  % delta) / denom;

			if (t > EPS && u >= 0 && u <= 1) {
				if (out_position)	{ *out_position = ray_origin + ray_direction * t; }
				if (out_normal)		{ *out_normal = float2{ edge_direction.y, -edge_direction.x }.norm(); }
				return t;
			}
		}
		return NO_HIT;
	}

	float ray_pick_polygon(float2 ray_origin, float2 ray_direction, float2 vertices[], int vertex_count, float2* out_position, float2* out_normal) {
		float2 min_position, min_normal;
		float min_hit = NO_HIT;

		for (int i = 0; i < vertex_count; i++) {
			float2 position, normal;
			float hit = ray_pick_edge(ray_origin, ray_direction, vertices[i], vertices[(i + 1) % vertex_count], (out_position || out_normal) ? &position : nullptr, out_normal ? &normal : nullptr);
			if (hit < min_hit) {
				if (!(out_position || out_normal)) { return hit; }
				min_position = position;
				min_normal = normal;
				min_hit = hit;
			}
		}

		if (out_position) { *out_position = min_position; }
		if (out_normal) { *out_normal = min_normal; }

		return min_hit;
	}

	float aligned_rect_collider::_ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position, float2* out_normal) const {
		float2 pos = get_pos();
		float2 size = get_size();

		float2 vertices[4] = {
			{ pos.x + size.x, pos.y + size.y },
			{ pos.x - size.x, pos.y + size.y },
			{ pos.x - size.x, pos.y - size.y },
			{ pos.x + size.x, pos.y - size.y }
		};

		return ray_pick_polygon(ray_origin, ray_direction, vertices, 4, out_position, out_normal);
	}

	float rect_collider::_ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position, float2* out_normal) const {
		float2 pos = get_pos();
		float2 size = get_size();
		float angle = get_ang();

		float2 corners[4] = {
			{ +size.x, +size.y },
			{ -size.x, +size.y },
			{ -size.x, -size.y },
			{ +size.x, -size.y }
		};

		float2 vertices[4];
		for (int i = 0; i < 4; i++) {
			vertices[i] = pos + corners[i].rotate(angle);
		}

		return ray_pick_polygon(ray_origin, ray_direction, vertices, 4, out_position, out_normal);
	}

	float circle_collider::_ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position, float2* out_normal) const {
		float2 pos = get_pos();
		float2 d = ray_origin - pos;
		float proj = dot(d, ray_direction);
		float dist_sq = d.mag_sq();
		float gap = (r * r) - dist_sq;
		
		if (gap < 0.0f && proj > 0.0f) { return NO_HIT; }

		float discrim = proj * proj + gap;
		if (discrim < 0) { return NO_HIT; }

		float t = -proj - sqrtf(discrim);
		if (t < 0) { t = 0; }

		float2 hit_pos;
		if ((out_position || out_normal)) { hit_pos = ray_origin + ray_direction * t; }

		if (out_position) { *out_position = hit_pos; }
		if (out_normal) { *out_normal = (hit_pos - pos).norm(); }

		return t;
	}

	// Debug Rendering

	void aligned_rect_collider::_render_debug(render_settings rs) const {
		float2 pos = get_pos();
		float2 size = get_size();
		float2 axes[2] = { {1, 0}, {0, 1} };
		debug::draw::circle(pos, 1.0f, WHITE);
		debug::draw::line(pos, pos + axes[0] * size.x, RED);
		debug::draw::line(pos, pos + axes[1] * size.y, PINK);
		debug::draw::rect(pos, size, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void rect_collider::_render_debug(render_settings rs) const {
		float2 pos = get_pos();
		float2 size = get_size();
		float ang = get_ang();
		float2 axes[2] = { {cosf(ang), sinf(ang)}, {-sinf(ang), cosf(ang)} };
		debug::draw::circle(pos, 1.0f, WHITE);
		debug::draw::line(pos, pos + axes[0] * size.x, RED);
		debug::draw::line(pos, pos + axes[1] * size.y, PINK);
		debug::draw::rect(pos, size, WHITE, ang);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void circle_collider::_render_debug(render_settings rs) const {
		float2 pos = get_pos();
		float r = get_r();
		debug::draw::circle(pos, 1.0f, WHITE);
		debug::draw::circle(pos, r, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

}

// 3D Colliders ***************************************************************************************************************

bool pre_check(const BLIB::full::aabb_collider& a, const BLIB::full::aabb_collider& b) {

	const float3 a_pos = a.get_trans().get_pos();
	const float3 b_pos = b.get_trans().get_pos();

	const float3 a_size = a.get_size();
	const float3 b_size = b.get_size();

	for (int i = 0; i < 3; i++) {
		if (fabsf(a_pos[i] - b_pos[i]) > (a_size[i] + b_size[i])) { return false; }
	}
	return true;
}

BLIB::full::collision BLIB::collision::check(const full::collider* a, const full::collider* b, bool any) {
	if (!a || !b) { return false; }
	BLIB::full::collision result(NO_HIT, {0, 0, 0});
	BLIB::full::collision temp_result(false);

	if (pre_check(static_cast<const BLIB::full::aabb_collider>(*a), static_cast<const BLIB::full::aabb_collider>(*b))) {
		temp_result = a->collide(b);
		if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }
	}

	for (const auto& a_child : a->get_children()) {
		temp_result = check(static_cast<full::collider*>(a_child.get()), b, any);
		if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }
	}

	for (const auto& b_child : b->get_children()) {
		temp_result = check(a, static_cast<full::collider*>(b_child.get()), any);
		if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }
	}

	for (const auto& a_child : a->get_children()) {
		for (const auto& b_child : b->get_children()) {
			temp_result = check(static_cast<full::collider*>(a_child.get()), static_cast<full::collider*>(b_child.get()), any);
			if (temp_result.depth < result.depth) { if (any) { return temp_result; } else { result = temp_result; } }
		}
	}

//#ifdef _DEBUG
//	float3 delta = a->get_trans().get_pos() - b->get_trans().get_pos();
//	if (result && result.depth != NO_HIT && delta.mag_sq() > EPS * EPS) _ASSERT_EXPR_A(dot(result.normal, delta) > 0, L"Normal does not follow B->A convention.");
//#endif

	return result;
}

namespace BLIB::full {

	// The Great Assumption of all of these functions is that you've already passed pre-check, so the circumscribed aabb is already overlapping
	// HELPERS SHOULD ALWAYS RETURN B -> A FACING NORMALS

	collision SAT_triangle_box(const triangle& t, const float3& b_pos, const float3& b_size, const float3x3& b_axes, const float3& axis) {
		if (axis.mag_sq() < EPS * EPS) { return false; }
	
		float3 t_ps = { dot(t.A, axis), dot(t.B, axis), dot(t.C, axis) };
		float t_p_max = maxim(maxim(t_ps[0], t_ps[1]), t_ps[2]);
		float t_p_min = minim(minim(t_ps[0], t_ps[1]), t_ps[2]);

		float b_p_min = FLT_MAX;
		float b_p_max = -FLT_MAX;
		for (int x = -1; x <= 1; x += 2) {
			for (int y = -1; y <= 1; y += 2) {
				for (int z = -1; z <= 1; z += 2) {
					float3 c = float3{ (float)x, (float)y, (float)z } * b_size;
					float3 c_pos = b_pos;
					for (int i = 0; i < 3; i++) {
						c_pos += b_axes[i] * c[i];
					}

					float c_n = dot(c_pos, axis);
					b_p_min = minim(b_p_min, c_n);
					b_p_max = maxim(b_p_max, c_n);
				}
			}
		}

		float depth = minim(t_p_max, b_p_max) - maxim(t_p_min, b_p_min);
		if (depth < 0) { return false; }

		float flip = (dot(t.centroid() - b_pos, axis) > 0) ? 1.0f : -1.0f;

		return { depth, axis.norm() * flip };
	}

	collision SAT_triangle_triangle(const triangle& a, const triangle& b, const float3& axis) {
		if (axis.mag_sq() < EPS * EPS) { return { FLT_MAX, {0, 0, 0} }; }

		float a_min =  FLT_MAX;
		float a_max = -FLT_MAX;
		float b_min =  FLT_MAX;
		float b_max = -FLT_MAX;

		for (int i = 0; i < 3; i++) {
			float a_d = dot(a[i], axis);
			float b_d = dot(b[i], axis);

			a_min = minim(a_min, a_d);
			a_max = maxim(a_max, a_d);
			b_min = minim(b_min, b_d);
			b_max = maxim(b_max, b_d);
		}

		float depth = minim(a_max, b_max) - maxim(a_min, b_min);
		if (depth < 0) { return false; }

		float flip = (dot(a.centroid() - b.centroid(), axis) > 0) ? 1.0f : -1.0f;

		return { depth, axis.norm() * flip };
	}

	collision SAT_box_box_axis(const float3& a_pos, const float3& a_size, const float3x3& a_axes, const float3& b_pos, const float3& b_size, const float3x3& b_axes, const float3& axis) {
		if (axis.mag_sq() < EPS * EPS) { return collision(NO_HIT, { 0, 0, 0 }); }

		float3 L = axis.norm();
		float3 T = a_pos - b_pos;

		float ra = 0, rb = 0;
		for (int i = 0; i < 3; i++) {
			ra += fabsf(dot(a_axes[i], L)) * a_size[i];
			rb += fabsf(dot(b_axes[i], L)) * b_size[i];
		}

		float TdL = dot(T, L);
		float dist = fabsf(TdL);
		float depth = ra + rb - dist;

		if (depth < 0) { return false; }

		// normal direction can be unstable here, so I'm going to forgo correcting every one
		return { depth, L };
	}

	collision SAT_box_box(const float3& a_pos, const float3& a_size, const float3x3& a_axes, const float3& b_pos, const float3& b_size, const float3x3& b_axes) {
		collision min_collision(NO_HIT, { 0, 0, 0 }), temp(false);

		for (int i = 0; i < 3; i++) {
			if (!(temp = SAT_box_box_axis(a_pos, a_size, a_axes, b_pos, b_size, b_axes, a_axes[i]))) { return false; }
			if (temp.depth < min_collision.depth) { min_collision = temp; }
			if (!(temp = SAT_box_box_axis(a_pos, a_size, a_axes, b_pos, b_size, b_axes, b_axes[i]))) { return false; }
			if (temp.depth < min_collision.depth) { min_collision = temp; }
		}

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (!(temp = SAT_box_box_axis(a_pos, a_size, a_axes, b_pos, b_size, b_axes, a_axes[i] % b_axes[j]))) { return false; }
				if (temp.depth < min_collision.depth) { min_collision = temp; }
			}
		}

		// enforcing a <- b here instead of in SAT_box_box_axis
		if (min_collision.depth != NO_HIT) {
			float3 T = a_pos - b_pos;
			if (dot(min_collision.normal, T) < 0) { min_collision.normal = -min_collision.normal; }
		}
		return min_collision;
	}

	collision triangle_triangle(const triangle& a, const triangle& b) {
		collision min_collision(FLT_MAX, { 0, 0, 0 }), temp(false);

		if (!(temp = SAT_triangle_triangle(a, b, a.norm()))) { return false; }
		if (temp.depth < min_collision.depth) { min_collision = temp; }

		if (!(temp = SAT_triangle_triangle(a, b, b.norm()))) { return false; }
		if (temp.depth < min_collision.depth) { min_collision = temp; }

		float3 a_e[3] = { a.B - a.A, a.C - a.B, a.A - a.C };
		float3 b_e[3] = { b.B - b.A, b.C - b.B, b.A - b.C };
		for (auto& ea : a_e) {
			for (auto& eb : b_e) {
				if (!(temp = SAT_triangle_triangle(a, b, ea % eb))) { return false; }
				if (temp.depth < min_collision.depth) { min_collision = temp; }
			}
		}

		return min_collision;
	}

	collision point_in_mesh(const float3& p, const mesh_collider* mesh) {
		const BLIB::model* model = mesh->get_mesh();
		float3 pos = mesh->get_trans().get_pos();
		float3 ray = { 1 - EPS * 2, EPS, EPS };

		uint32_t hit_count = model->ray_collision(mesh->get_trans(), p, ray, nullptr, nullptr);

		if (hit_count % 2) {
			collision min_collision(FLT_MAX, { 0, 0, 0 });

			for (const triangle& t : model->peek_triangles()) {
				float3 t_n = ((t.B - t.A) % (t.C - t.A)).norm();
				float dist = dot(p - t.A, t_n);

				float abs_dist = fabsf(dist);
				if (abs_dist < min_collision.depth) { min_collision = { abs_dist, (dist > 0) ? t_n : -t_n }; }
			}

			return { min_collision };
		}
		return false;
	}
	
	collision point_over_plane(const float3& p, const float3& plane_p, const float3& plane_n) {
		return { dot(plane_p - p, plane_n), plane_n };
	}

	collision aabb_box(const aabb_collider* a, const box_collider* b) {
		float3 a_pos = a->get_trans().get_pos();
		float3 a_size = a->get_size();
		float3x3 a_axes = float3x3::identity();

		float3 b_pos = b->get_trans().get_pos();
		float3 b_size = b->get_size();
		float3x3 b_axes(b->get_quaternion());

		return SAT_box_box(a_pos, a_size, a_axes, b_pos, b_size, b_axes);
	}

	collision aabb_sphere(const aabb_collider* a, const sphere_collider* b) {
		float3 a_pos = a->get_trans().get_pos();
		float3 a_size = a->get_size();
		float3 b_pos = b->get_trans().get_pos();

		float3 closest = clamp(a_pos - a_size, b_pos, a_pos + a_size);
	
		float3 d = (closest - b_pos);
		float r = b->get_r();
		float dist_sq = d.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float depth = r - dist;
		float3 normal;
		if (dist > EPS) { normal = d / dist; }
		else			{ normal = (a_pos - b_pos).norm(); }

		return { depth, normal };
	}

	collision aabb_cylinder(const aabb_collider* a, const cylinder_collider* b) {
		float3 a_pos = a->get_trans().get_pos();
		float3 a_size = a->get_size();
		float2 a_min = a_pos.xz() - a_size.xz();
		float2 a_max = a_pos.xz() + a_size.xz();

		float3 b_pos3 = b->get_trans().get_pos();
		float2 b_pos = b_pos3.xz();

		float2 closest = clamp(a_min, b_pos, a_max);
		float2 d = closest - b_pos;
		float r = b->get_r();
		float dist_sq = d.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float over_xz = r - dist;

		float a_min_y = a_pos.y - a_size.y;
		float a_max_y = a_pos.y + a_size.y;
		float b_min_y = b_pos3.y - b->get_h();
		float b_max_y = b_pos3.y + b->get_h();
		float over_y = minim(a_max_y, b_max_y) - maxim(a_min_y, b_min_y);

		if (over_y < over_xz) {
			float3 normal = (a_pos.y < b_pos3.y) ? float3(0, -1, 0) : float3(0, 1, 0);
			return { over_y, normal };
		}
		else {
			float2 normal = (dist > EPS) ? d / dist : (a_pos.xz() - b_pos).norm();
			return { over_xz, float3(normal.x, 0, normal.y) };
		}
	}

	collision aabb_capsule(const aabb_collider* a, const capsule_collider* b) {
		const float3 up(0, 1, 0);

		float3 a_pos = a->get_trans().get_pos();
		float3 a_size = a->get_size();
		float3 a_min = a_pos - a_size;
		float3 a_max = a_pos + a_size;

		float3 b_pos = b->get_trans().get_pos();
		float b_h = b->get_h();
		float3 b_0 = b_pos - up * b_h;
		float3 b_1 = b_pos + up * b_h;
		float r = b->get_r();

		float3 closest = clamp(a_min, b_pos, a_max);
	
		float3 sphere_pos = b_pos;
		sphere_pos.y = clamp(b_0.y, closest.y, b_1.y);

		float3 d = closest - sphere_pos;
		float dist_sq = d.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float depth = r - dist;

		float3 normal = (dist > EPS) ? d / dist : float3(1, 0, 0);

		return { depth, normal };
	}

	collision aabb_mesh(const aabb_collider* a, const mesh_collider* b) {
		return NO_COLLISION_3;
		//float3 a_pos = a->get_trans().get_pos();
		//float3 a_size = a->get_size();
		//float3x3 a_axes(true);
		//float3 a_min = a_pos - a_size;
		//float3 a_max = a_pos + a_size;
		//
		//collision min_collision(FLT_MAX, 0), temp(false);
		//for (const triangle& t : b->get_mesh()->peek_triangles()) {
		//	float3 t_min = minimize(minimize(t.A, t.B), t.C);
		//	float3 t_max = maximize(maximize(t.A, t.B), t.C);
		//
		//	if (t_max.x < a_min.x || t_min.x > a_max.x) { min_collision = false; break; }
		//	if (t_max.y < a_min.y || t_min.y > a_max.y) { min_collision = false; break; }
		//	if (t_max.z < a_min.z || t_min.z > a_max.z) { min_collision = false; break; }
		//
		//	float3 t_n = (((t.B - t.A) % (t.C - t.A))).norm();
		//
		//	if (!(temp = SAT_triangle_box(t, a_pos, a_size, a_axes, t_n))) { min_collision = false; break; }
		//	if (temp.depth < min_collision.depth) { min_collision = temp; }
		//
		//	float3x3 t_e{t.B - t.A, t.C - t.B, t.A - t.C};
		//
		//	for (int i = 0; i < 3 && min_collision; i++) {
		//		for (int j = 0; j < 3 && min_collision; j++) {
		//			if (!(temp = SAT_triangle_box(t, a_pos, a_size, a_axes, a_axes[i] % t_e[j]))) { min_collision = false; break; }
		//			if (temp.depth < min_collision.depth) { min_collision = temp; }
		//		}
		//	}
		//	
		//}
		//if (min_collision) { return min_collision; }
		//
		//return point_in_mesh(a_pos, b);
	}

	collision aabb_plane(const aabb_collider* a, const plane_collider* b) {
		float3 n = b->get_normal();
		float3x3 axes = float3x3::identity();
		float3 corner = a->get_trans().get_pos();
		float3 size = a->get_size();

		for (int i = 0; i < 3; i++) {
			if (dot(axes[i], n) < 0) { axes[i] *= -1; }
			corner += size[i] * axes[i];
		}

		return point_over_plane(corner, b->get_trans().get_pos(), n);
	}

	collision box_sphere(const box_collider* a, const sphere_collider* b) {
		float3 a_pos = a->get_trans().get_pos();
		float3 a_size = a->get_size();
		float3x3 a_axes(a->get_quaternion());

		float3 b_pos = b->get_trans().get_pos();

		float3 local = a_axes.rotate(b_pos - a_pos);
		float3 closest = clamp(-a_size, local, a_size);
		float3 d = closest - local;
		float r = b->get_r();
		float dist_sq = d.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float depth = r - dist;
		float3 normal;

		if (dist > EPS) { normal = a_axes.inv_rotate(d / dist); }
		else			{ normal = (a_pos - b_pos).norm(); }

		return { depth, normal };
	}

	collision box_cylinder(const box_collider* a, const cylinder_collider* b) {
		float3 a_pos3 = a->get_trans().get_pos();
		float3 a_size = a->get_size();
		float3x3 a_axes(a->get_quaternion());

		float3 b_pos3 = b->get_trans().get_pos();
		float3 local = a_axes.rotate(b_pos3 - a_pos3);
		float2 b_pos = local.xz();

		float2 a_min = -a_size.xz();
		float2 a_max = a_size.xz();
		float2 closest = clamp(a_min, b_pos, a_max);

		float2 d = closest - b_pos;
		float r = b->get_r();
		float dist_sq = d.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float over_xz = r - dist;

		float world_height = dot(a_axes.y().abs(), a_size);
		float a_min_y = a_pos3.y - world_height;
		float a_max_y = a_pos3.y + world_height;
		float b_min_y = b_pos3.y - b->get_h();
		float b_max_y = b_pos3.y + b->get_h();
		float over_y = minim(a_max_y, b_max_y) - maxim(a_min_y, b_min_y);

		if (over_y < over_xz) { return { over_y, (a_pos3.y < b_pos3.y) ? float3(0, -1, 0) : float3(0, 1, 0) }; }
		else {
			float3 normal_xz;
			if (dist > EPS) { normal_xz = d / dist; }
			else { normal_xz = b_pos.mag_sq() > EPS * EPS ? (-b_pos).norm() : float2(1, 0); }
			return { over_xz, a_axes.inv_rotate({ normal_xz.x, 0, normal_xz.y }).norm() };
		}
	}

	collision box_capsule(const box_collider* a, const capsule_collider* b) {
		const float3 up(0, 1, 0);

		float3 a_pos = a->get_trans().get_pos();
		float3 a_size = a->get_size();
		float3x3 a_axes(a->get_quaternion());

		float3 b_pos = b->get_trans().get_pos();
		float b_h = b->get_h();
		float3 b_0 = b_pos - up * b_h;
		float3 b_1 = b_pos + up * b_h;
		float r = b->get_r();

		float3 local = a_axes.rotate(b_pos - a_pos);
		float3 closest = clamp(-a_size, local, a_size);
		float3 world_closest = a_pos + a_axes.inv_rotate(closest);

		float3 sphere_pos = b_pos;
		sphere_pos.y = clamp(b_0.y, world_closest.y, b_1.y);

		float3 d = world_closest - sphere_pos;
		float dist_sq = d.mag_sq();

		if (dist_sq > r * r) { return false; }

		float dist = sqrtf(dist_sq);
		float3 normal = (dist > EPS) ? d / dist : float3(1, 0, 0);
		return { r - dist, normal };
	} 

	collision box_mesh(const box_collider* a, const mesh_collider* b) {
		return NO_COLLISION_3;
		//float3 a_pos = a->get_trans().get_pos();
		//float3 a_size = a->get_size();
		//float3x3 a_axes(a->get_rotation());
		//float3 a_min = a_pos - a_size;
		//float3 a_max = a_pos + a_size;
		//
		//collision min_collision(FLT_MAX, 0), temp(false);
		//for (const triangle& t : b->get_mesh()->peek_triangles()) {
		//	for (int i = 0; i < 3 && min_collision; i++) {
		//		if (!(temp = SAT_triangle_box(t, a_pos, a_size, a_axes, a_axes[i]))) { min_collision = false; break; }
		//		if (temp.depth < min_collision.depth) { min_collision = temp; }
		//	}
		//	if (!min_collision) { break; }
		//
		//	float3 t_n = (((t.B - t.A) % (t.C - t.A))).norm();
		//
		//	if (!(temp = SAT_triangle_box(t, a_pos, a_size, a_axes, t_n))) { min_collision = false; break; }
		//	if (temp.depth < min_collision.depth) { min_collision = temp; }
		//
		//	float3x3 t_e{ t.B - t.A, t.C - t.B, t.A - t.C };
		//
		//	for (int i = 0; i < 3 && min_collision; i++) {
		//		for (int j = 0; j < 3 && min_collision; j++) {
		//			if (!(temp = SAT_triangle_box(t, a_pos, a_size, a_axes, a_axes[i] % t_e[j]))) { min_collision = false; break; }
		//			if (temp.depth < min_collision.depth) { min_collision = temp; }
		//		}
		//	}
		//}
		//if (min_collision) { return min_collision; }
		//
		//return point_in_mesh(a_pos, b);
	}

	collision box_plane(const box_collider* a, const plane_collider* b) {
		float3 n = b->get_normal();
		float3x3 axes(a->get_quaternion());
		float3 corner = a->get_trans().get_pos();
		float3 size = a->get_size();

		for (int i = 0; i < 3; i++) {
			if (dot(axes[i], n) < 0) { axes[i] *= -1; }
			corner += size[i] * axes[i];
		}

		return point_over_plane(corner, b->get_trans().get_pos(), n);
	}

	collision sphere_cylinder(const sphere_collider* a, const cylinder_collider* b) {
		float3 a_pos = a->get_trans().get_pos();
		float a_r = a->get_r();

		float3 b_pos = b->get_trans().get_pos();
		float b_r = b->get_r();
		float b_h = b->get_h();

		float2 d_xz = a_pos.xz() - b_pos.xz();
		float rs = a_r + b_r;
		if (d_xz.mag_sq() > rs * rs) { return false; }

		float min_y = b_pos.y - b_h;
		float max_y = b_pos.y + b_h;

		float3 d = { d_xz.x, 0, d_xz.y };
		if (a_pos.y < min_y) { d.y = min_y - a_pos.y; }
		else if (a_pos.y > max_y) { d.y = max_y - a_pos.y; }

		float dist_sq = d.mag_sq();

		if (dist_sq > rs * rs) { return false; }

		float dist = sqrtf(dist_sq);
		float3 normal = (dist > EPS) ? d / dist : float3(1, 0, 0);
		return { rs - dist, normal };
	}

	collision sphere_capsule(const sphere_collider* a, const capsule_collider* b) {
		const float3 up(0, 1, 0);

		float3 a_pos = a->get_trans().get_pos();

		float3 b_pos = b->get_trans().get_pos();
		float  b_h = b->get_h();
		float3 b_bottom = b_pos - up * b_h;

		float rs = a->get_r() + b->get_r();

		float t = dot(a_pos - b_bottom, up);
		t = clamp(0.0f, t, 1.0f);
		
		float3 b_sphere = b_bottom + up * t;

		float3 d = a_pos - b_sphere;
		float dist_sq = d.mag_sq();
		if (dist_sq > rs * rs) { return false; }

		float dist = sqrtf(dist_sq);
		float3 normal = (dist > EPS) ? d / dist : float3(1, 0, 0);
		return { rs - dist, normal };
	}

	collision sphere_mesh(const sphere_collider* a, const mesh_collider* b) {
		return NO_COLLISION_3;
		//float r = a->get_r();
		//float r_sq = r * r;
		//float3 a_pos = a->get_trans().get_pos();
		//
		//for (const triangle& t : b->get_mesh()->peek_triangles()) {
		//	for (int i = 0; i < 3; i++) {
		//		float3 d = (t[i] - a_pos);
		//		float dist_sq = d.mag_sq();
		//		if (dist_sq < r_sq) { 
		//			float dist = sqrtf(dist_sq);
		//			float3 normal = (dist > EPS) ? d / dist : float3(1, 0, 0);
		//			return { r - dist, normal };
		//		}
		//	}
		//}
		//
		//return point_in_mesh(a_pos, b);
	}

	collision sphere_plane(const sphere_collider* a, const plane_collider* b) {
		float3 n = b->get_normal();
		return point_over_plane(a->get_trans().get_pos() - n * a->get_r(), b->get_trans().get_pos(), n);
	}

	collision cylinder_capsule(const cylinder_collider* a, const capsule_collider* b) {
		const float3 up(0, 1, 0);

		float3 a_pos = a->get_trans().get_pos();
		float a_h = a->get_h();
		float3 a_0 = a_pos - up * a_h;
		float3 a_1 = a_pos + up * a_h;
		float a_r = a->get_r();

		float3 b_pos = b->get_trans().get_pos();
		float b_h = b->get_h();
		float3 b_0 = b_pos - up * b_h;
		float3 b_1 = b_pos + up * b_h;
		float b_r = b->get_r();

		// Basically, find the closest sphere in the capsule, and do sphere_cylinder
		float3 sphere_pos = b_pos;
		sphere_pos.y = clamp(b_0.y, a_pos.y, b_1.y);

		float2 d_xz = a_pos.xz() - sphere_pos.xz();
		float rs = a_r + b_r;
		if (d_xz.mag_sq() > rs * rs) { return false; }

		float min_y = b_0.y;
		float max_y = b_1.y;

		float3 d = { d_xz.x, 0, d_xz.y };
		if		(a_pos.y < min_y) { d.y = min_y - a_pos.y; }
		else if (a_pos.y > max_y) { d.y = max_y - a_pos.y; }

		float dist_sq = d.mag_sq();

		if (dist_sq > rs * rs) { return false; }

		float dist = sqrtf(dist_sq);
		float3 normal = (dist > EPS) ? -d / dist : float3(1, 0, 0);
		return { rs - dist, normal };
	}

	collision cylinder_mesh(const cylinder_collider* a, const mesh_collider* b) {
		return NO_COLLISION_3;
		// unfinished
		//const float3 up(0, 1, 0);
		//
		//float r = a->get_r();
		//float r_sq = r * r;
		//float3 a_pos = a->get_trans().get_pos();
		//float a_h = a->get_h();
		//float3 a_0 = a_pos - up * a_h;
		//float3 a_1 = a_pos + up * a_h;
		//
		//auto check_point = [&](const float3& p) -> bool {
		//	if (p.y > a_0.y && p.y < a_1.y) {
		//		float2 p_xz = { p.x, p.z };
		//		float2 c_xz = { a_pos.x, a_pos.z };
		//		return (c_xz - p_xz).mag_sq() < r_sq;
		//	}
		//	return false;
		//};
		//
		//for (const triangle& t : b->get_mesh()->peek_triangles()) {
		//	float3x3 edge_centers{ (t.A + t.B) * 0.5f, (t.B + t.C) * 0.5f, (t.C + t.A) * 0.5f };
		//
		//	for (int i = 0; i < 3; i++) {
		//		if (check_point(t[i])) { return true; }
		//		if (check_point(edge_centers[i])) { return true; }
		//	}
		//
		//	if (check_point(t.centroid())) { return true; }
		//}
		//
		//return point_in_mesh(a_pos, b);
	}

	collision cylinder_plane(const cylinder_collider* a, const plane_collider* b) {
		const float3 up(0, 1, 0);
		float3 n = b->get_normal();
		float3 c = (dot(up, n) < 0) ? -up : up;
		float3 c_pos = a->get_trans().get_pos() + c * a->get_h();

		float3 n_proj = n - up * dot(up, n);

		if (fabsf(n_proj.mag_sq()) < EPS * EPS) { return point_over_plane(c_pos, b->get_trans().get_pos(), n); }
		return point_over_plane(c_pos - a->get_r() * n_proj.norm(), b->get_trans().get_pos(), n);
	}

	collision capsule_mesh(const capsule_collider* a, const mesh_collider* b) {
		return NO_COLLISION_3;
		// unfinished
		//const float3 up(0, 1, 0);
		//
		//float r = a->get_r();
		//float r_sq = r * r;
		//float3 a_pos = a->get_trans().get_pos();
		//float a_h = a->get_h();
		//float3 a_0 = a_pos - up * a_h;
		//float3 a_1 = a_pos + up * a_h;
		//
		//for (const triangle& t : b->get_mesh()->peek_triangles()) {
		//	float3 sphere_pos = a_pos;
		//
		//	float3x3 edge_centers{ (t.A + t.B) * 0.5f, (t.B + t.C) * 0.5f, (t.C + t.A) * 0.5f };
		//
		//	for (int i = 0; i < 3; i++) {
		//		sphere_pos.y = clamp(a_0.y, t[i].y, a_1.y);
		//		if ((sphere_pos - t[i]).mag_sq() < r_sq) { return true; }
		//
		//		sphere_pos.y = clamp(a_0.y, edge_centers[i].y, a_1.y);
		//		if ((sphere_pos - edge_centers[i]).mag_sq() < r_sq) { return true; }
		//	}
		//
		//	float3 centroid = t.centroid();
		//	sphere_pos.y = clamp(a_0.y, centroid.y, a_1.y);
		//	if ((sphere_pos - centroid).mag_sq() < r_sq) { return true; }
		//}
		//
		//return point_in_mesh(a_pos, b);
	}

	collision capsule_plane(const capsule_collider* a, const plane_collider* b) {
		const float3 up(0, 1, 0);
		float3 n = b->get_normal();
		return point_over_plane(a->get_trans().get_pos() + a->get_h() * ((dot(up, n) < 0) ? -up : up) - n * a->get_r(), b->get_trans().get_pos(), n);
	}

	collision mesh_plane(const mesh_collider* a, const plane_collider* b) {
		return NO_COLLISION_3;
		// Iterate to find the "lowest point" on the normal, and collide that with plane
	}

	// DOUBLE DISPATCH REQUIRES THAT O = A, THIS = B
	// ENFORCE B -> A BY INVERSION IF NECESSARY

	// AABB

	collision aabb_collider::collide_with(const aabb_collider* o) const {
		const float3 a_pos = o->get_trans().get_pos();
		const float3 b_pos = get_trans().get_pos();

		const float3 a_size = o->get_size();
		const float3 b_size = get_size();

		float3 delta = a_pos - b_pos;
		float3 overlap = (a_size + b_size) - delta.abs();

		collision result(overlap.x, { 0, 0, 0 });
		int axis = 0;

		if (overlap.y < result.depth) { result.depth = overlap.y; axis = 1; }
		if (overlap.z < result.depth) { result.depth = overlap.z; axis = 2; }

		result.normal[axis] = (delta[axis] > 0) ? 1.0f : -1.0f;

		return result;
	}

	collision aabb_collider::collide_with(const box_collider*		o) const { return -aabb_box		(this, o);	}	
	collision aabb_collider::collide_with(const sphere_collider*	o) const { return -aabb_sphere	(this, o);	}
	collision aabb_collider::collide_with(const cylinder_collider*	o) const { return -aabb_cylinder(this, o);	}
	collision aabb_collider::collide_with(const capsule_collider*	o) const { return -aabb_capsule	(this, o);	}
	collision aabb_collider::collide_with(const mesh_collider*		o) const { return NO_COLLISION_3;			}//{ return aabb_mesh		(this, o); }
	collision aabb_collider::collide_with(const plane_collider*		o) const { return -aabb_plane	(this, o);	}

	// BOX

	box_collider::operator aabb_collider() const {
		float3x3 R = quat;

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				R[i][i] = fabsf(R[i][j]);
			}
		}

		aabb_collider out( 
			nullptr,
			{ 
				dot(R[0], size),
				dot(R[1], size),
				dot(R[2], size)
			}
		);

		out.set_off(offset);
		out.set_scl(scale);

		return out;
	}

	collision box_collider::collide_with(const box_collider*		o) const {
		float3 b_pos = trans.get_pos();
		float3 b_size = get_size();
		float3x3 b_axes(get_quaternion());

		float3 a_pos = o->get_trans().get_pos();
		float3 a_size = o->get_size();
		float3x3 a_axes(o->get_quaternion());

		return SAT_box_box(a_pos, a_size, a_axes, b_pos, b_size, b_axes);
	}

	collision box_collider::collide_with(const aabb_collider*		o) const { return aabb_box		(o, this);	}
	collision box_collider::collide_with(const sphere_collider*		o) const { return -box_sphere	(this, o);	}
	collision box_collider::collide_with(const cylinder_collider*	o) const { return -box_cylinder	(this, o);	}
	collision box_collider::collide_with(const capsule_collider*	o) const { return -box_capsule	(this, o);	}
	collision box_collider::collide_with(const mesh_collider*		o) const { return NO_COLLISION_3;			}//{ return box_mesh		(this, o); }
	collision box_collider::collide_with(const plane_collider*		o) const { return -box_plane	(this, o);	}

	// SPHERE 

	sphere_collider::operator aabb_collider() const {
		aabb_collider out(nullptr, float3{ r });

		out.set_off(offset);
		out.set_scl(scale);

		return out;
	}

	collision sphere_collider::collide_with(const sphere_collider*		o) const {
		float3 d = o->get_trans().get_pos() - trans.get_pos();
		float rs = get_r() + o->get_r();

		float dist_sq = d.mag_sq();

		if (dist_sq > rs * rs) { return false; }

		float dist = sqrtf(dist_sq);
		float3 normal = (dist > EPS) ? d / dist : float3(1, 0, 0);
		return { rs - dist, normal };
	}

	collision sphere_collider::collide_with(const aabb_collider*		o) const { return aabb_sphere		(o, this);	}
	collision sphere_collider::collide_with(const box_collider*			o) const { return box_sphere		(o, this);	}
	collision sphere_collider::collide_with(const cylinder_collider*	o) const { return -sphere_cylinder	(this, o);	}
	collision sphere_collider::collide_with(const capsule_collider*		o) const { return -sphere_capsule	(this, o);	}
	collision sphere_collider::collide_with(const mesh_collider*		o) const { return NO_COLLISION_3;			    }//{ return sphere_mesh		(this, o); }
	collision sphere_collider::collide_with(const plane_collider*		o) const { return -sphere_plane		(this, o);	}

	// CYLINDER

	cylinder_collider::operator aabb_collider() const {
		aabb_collider out(nullptr, { r, h, r });

		out.set_off(offset);
		out.set_scl(scale);

		return out;
	}

	collision cylinder_collider::collide_with(const cylinder_collider*	o) const {
		float3 a_pos3 = o->get_trans().get_pos();
		float3 b_pos3 = trans.get_pos();

		float2 a_pos = a_pos3.xz();
		float2 b_pos = b_pos3.xz();

		const float2 d = a_pos - b_pos;
		const float rs = get_r() + o->get_r();
		float dist_sq = d.mag_sq();

		if (dist_sq > rs * rs) { return false; }

		float dist = sqrtf(dist_sq);
		float over_xz = rs - dist;

		float a_h = o->get_h();
		float b_h = get_h();

		float a_min_y = a_pos3.y - a_h;
		float a_max_y = a_pos3.y + a_h;
		float b_min_y = b_pos3.y - b_h;
		float b_max_y = b_pos3.y + b_h;
		float over_y = minim(a_max_y, b_max_y) - maxim(a_min_y, b_min_y);

		if (over_y < over_xz) {
			float3 normal = (a_pos.y < b_pos3.y) ? float3(0, -1, 0) : float3(0, 1, 0);
			return { over_y, normal };
		}
		else {
			float2 normal;
			if (dist > EPS) { normal = d / dist; }
			else {
				if (fabsf(d.x) > fabsf(d.y))	{ normal.x = (d.x > 0) ? 1.0f : -1.0f; }
				else							{ normal.y = (d.y > 0) ? 1.0f : -1.0f; }
			}
			return { over_xz, float3(normal.x, 0, normal.y) };
		}
	}

	collision cylinder_collider::collide_with(const aabb_collider*		o) const { return aabb_cylinder		(o, this);	}
	collision cylinder_collider::collide_with(const box_collider*		o) const { return box_cylinder		(o, this);	}
	collision cylinder_collider::collide_with(const sphere_collider*	o) const { return sphere_cylinder	(o, this);	}
	collision cylinder_collider::collide_with(const capsule_collider*	o) const { return -cylinder_capsule	(this, o);	}
	collision cylinder_collider::collide_with(const mesh_collider*		o) const { return NO_COLLISION_3;			    }//{ return cylinder_mesh		(this, o); }
	collision cylinder_collider::collide_with(const plane_collider*		o) const { return -cylinder_plane	(this, o);	}

	// CAPSULE

	capsule_collider::operator aabb_collider() const {
		aabb_collider out(nullptr, { r, h + r, r });

		out.set_off(offset);
		out.set_scl(scale);

		return out;
	}

	collision capsule_collider::collide_with(const capsule_collider*	o) const {
		const float3 up( 0, 1, 0 );

		float3 b_pos = trans.get_pos();
		float  b_h = get_h();
		float3 b_0 = b_pos - up * b_h;
		float3 b_1 = b_pos + up * b_h;

		float3 a_pos = o->get_trans().get_pos();
		float  a_h = o->get_h();
		float3 a_0 = a_pos - up * a_h;
		float3 a_1 = a_pos + up * a_h;
	
		float3 d = a_pos - b_pos;
		float rs = get_r() + o->get_r();
		if ((d.x * d.x + d.z * d.z) > rs * rs) { return false; } // Early exit

		if (a_1.y < b_0.y) { d.y = b_0.y - a_1.y; }
		else if (b_1.y < a_0.y) { d.y = b_1.y - a_0.y; }
		else { d.y = 0; }

		float dist_sq = d.mag_sq();

		if (dist_sq > rs * rs) { return false; }

		float dist = sqrtf(dist_sq);
		float3 normal = (dist > EPS) ? d / dist : float3(1, 0, 0);
		return { rs - dist, normal };
	}

	collision capsule_collider::collide_with(const aabb_collider*		o) const { return aabb_capsule		(o, this);	}
	collision capsule_collider::collide_with(const box_collider*		o) const { return box_capsule		(o, this);	}
	collision capsule_collider::collide_with(const sphere_collider*		o) const { return sphere_capsule	(o, this);	}
	collision capsule_collider::collide_with(const cylinder_collider*	o) const { return cylinder_capsule	(o, this);	}
	collision capsule_collider::collide_with(const mesh_collider*		o) const { return NO_COLLISION_3;			    }//{ return capsule_mesh		(this, o); }
	collision capsule_collider::collide_with(const plane_collider*		o) const { return -capsule_plane	(this, o);	}

	// MESH

	mesh_collider::operator aabb_collider() const {
		aabb_collider out(nullptr, model_ptr->get_size() * 0.5f);
		return out;
	}

	collision mesh_collider::collide_with(const mesh_collider*		o) const {
		return NO_COLLISION_3;
		//for (const triangle& ta : get_mesh()->peek_triangles()) {
		//	for (const triangle& tb : o->get_mesh()->peek_triangles()) {
		//		if (triangle_triangle(ta, tb)) { return true; }
		//	}
		//}
		//
		//if (point_in_mesh(get_trans().get_pos(), o)) { return true; }
		//if (point_in_mesh(o->get_trans().get_pos(), this)) { return true; }
		//
		//return false;
	}

	collision mesh_collider::collide_with(const aabb_collider*		o) const { return NO_COLLISION_3; }//{ return aabb_mesh		(o, this); }
	collision mesh_collider::collide_with(const box_collider*		o) const { return NO_COLLISION_3; }//{ return box_mesh		(o, this); }
	collision mesh_collider::collide_with(const sphere_collider*	o) const { return NO_COLLISION_3; }//{ return sphere_mesh		(o, this); }
	collision mesh_collider::collide_with(const cylinder_collider*	o) const { return NO_COLLISION_3; }//{ return cylinder_mesh	(o, this); }
	collision mesh_collider::collide_with(const capsule_collider*	o) const { return NO_COLLISION_3; }//{ return capsule_mesh	(o, this); }
	collision mesh_collider::collide_with(const plane_collider*		o) const { return NO_COLLISION_3; }//{ return -mesh_plane		(this, o); }

	// PLANE

	collision plane_collider::collide_with(const plane_collider* o) const { return NO_COLLISION_3; } // A plane x plane collision should never be used.

	collision plane_collider::collide_with(const aabb_collider*		o) const { return aabb_plane		(o, this); }
	collision plane_collider::collide_with(const box_collider*		o) const { return box_plane			(o, this); }
	collision plane_collider::collide_with(const sphere_collider*	o) const { return sphere_plane		(o, this); }
	collision plane_collider::collide_with(const cylinder_collider*	o) const { return cylinder_plane	(o, this); }
	collision plane_collider::collide_with(const capsule_collider*	o) const { return capsule_plane		(o, this); }
	collision plane_collider::collide_with(const mesh_collider*		o) const { return mesh_plane		(o, this); }

	// Ray Picking

	float collider::ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		float3 ray_norm = ray_direction.norm();

		float min_hit = NO_HIT;
		float3 min_position;
		float3 min_normal;

		float hit = _ray_pick(ray_origin, ray_norm, (out_position || out_normal) ? &min_position : nullptr, out_normal ? &min_normal : nullptr);
		if (hit < min_hit) { min_hit = hit; }

		for (const auto& child : children) {
			float3 position;
			float3 normal;
			const collider* child_ptr = static_cast<collider*>(child.get());
			float hit = child_ptr->ray_pick(ray_origin, ray_norm, (out_position || out_normal) ? &position : nullptr, out_normal ? &normal : nullptr);
			if (!(out_position || out_normal)) { return hit; }
			else if (hit < min_hit) {
				min_position = position;
				min_normal = normal;
				min_hit = hit;
			}
		}

		if (out_position) { *out_position = min_position; }
		if (out_normal) { *out_normal = min_normal; }

		return min_hit;

	}

	float ray_pick_plane(float3 ray_origin, float3 ray_direction, float3 plane_origin, float3 plane_normal, float3* out_position, float3* out_normal) {
		float denom = dot(ray_direction, plane_normal);

		if (fabsf(denom) < EPS) { return NO_HIT; }

		float t = dot((plane_origin - ray_origin), plane_normal) / denom;

		if (t < EPS) { return NO_HIT; }

		if (out_position) { *out_position = ray_origin + ray_direction * t; }
		if (out_normal) { *out_normal = plane_normal; }

		return t;
	}

	float ray_pick_rect_face(float3 ray_origin, float3 ray_direction, float3 vertex[4], float3* out_position, float3* out_normal) {
		float3 hit_pos;
		float t = ray_pick_plane(ray_origin, ray_direction, vertex[0], triangle{ vertex[2], vertex[1], vertex[0] }.norm(), &hit_pos, out_normal);

		if (t == NO_HIT) { return t; }

		float3 u_dir = vertex[1] - vertex[0];
		float3 v_dir = vertex[3] - vertex[0];

		float u_len = u_dir.mag();
		float v_len = v_dir.mag();

		float3 U = u_dir / u_len;
		float3 V = v_dir / v_len;

		float3 local = hit_pos - vertex[0];
		float u = dot(local, U);
		float v = dot(local, V);

		if (u < 0.0f || u > u_len || v < 0.0f || v > v_len) { return NO_HIT; }

		if (out_position) { *out_position = hit_pos; }
		return t;
	}

	float aabb_collider::_ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		float3 pos = get_trans().get_pos();
		float3 size = get_size();

		float3 vertices[8] = {
			{ pos.x + size.x, pos.y + size.y, pos.z + size.z },
			{ pos.x + size.x, pos.y + size.y, pos.z - size.z },
			{ pos.x + size.x, pos.y - size.y, pos.z + size.z },
			{ pos.x + size.x, pos.y - size.y, pos.z - size.z },
			{ pos.x - size.x, pos.y + size.y, pos.z + size.z },
			{ pos.x - size.x, pos.y + size.y, pos.z - size.z },
			{ pos.x - size.x, pos.y - size.y, pos.z + size.z },
			{ pos.x - size.x, pos.y - size.y, pos.z - size.z }
		};

		int indices[6][4] = {
			{ 1, 5, 7, 3 },
			{ 0, 2, 6, 4 },
			{ 0, 1, 3, 2 },
			{ 5, 4, 6, 7 },
			{ 1, 0, 4, 5 },
			{ 2, 3, 7, 6 }
		};

		float min_hit = NO_HIT;
		float3 min_position;
		float3 min_normal;

		for (int i = 0; i < 6; i++) {
			float3 position;
			float3 normal;
			float3 face_vertices[4]{vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]] };
			float hit = ray_pick_rect_face(ray_origin, ray_direction, face_vertices, (out_position || out_normal) ? &position : nullptr, out_normal ? &normal : nullptr);
			if (!(out_position || out_normal)) { return hit; }
			else if (hit < min_hit) {
				min_position = position;
				min_normal = normal;
				min_hit = hit;
			}
		}

		if (out_position) { *out_position = min_position; }
		if (out_normal) { *out_normal = min_normal; }
		return min_hit;
	}

	float box_collider::_ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		float3 pos = get_trans().get_pos();
		float3x3 axes(get_trans().get_qtn());
		float3 size = get_size();

		float3x3 rotated_size;
		for (int i = 0; i < 3; i++) {
			float3 temp(0);
			temp[i] = size[i];
			rotated_size[i] = axes.rotate(temp);
		}

		float3 vertices[8] = {
			{ pos + rotated_size[0] + rotated_size[1] + rotated_size[2] },
			{ pos + rotated_size[0] + rotated_size[1] - rotated_size[2] },
			{ pos + rotated_size[0] - rotated_size[1] + rotated_size[2] },
			{ pos + rotated_size[0] - rotated_size[1] - rotated_size[2] },
			{ pos - rotated_size[0] + rotated_size[1] + rotated_size[2] },
			{ pos - rotated_size[0] + rotated_size[1] - rotated_size[2] },
			{ pos - rotated_size[0] - rotated_size[1] + rotated_size[2] },
			{ pos - rotated_size[0] - rotated_size[1] - rotated_size[2] }
		};

		int indices[6][4] = {
			{ 1, 5, 7, 3 },
			{ 0, 2, 6, 4 },
			{ 0, 1, 3, 2 },
			{ 5, 4, 6, 7 },
			{ 1, 0, 4, 5 },
			{ 2, 3, 7, 6 }
		};

		float min_hit = NO_HIT;
		float3 min_position;
		float3 min_normal;

		for (int i = 0; i < 6; i++) {
			float3 position;
			float3 normal;
			float3 face_vertices[4]{ vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]] };
			float hit = ray_pick_rect_face(ray_origin, ray_direction, face_vertices, (out_position || out_normal) ? &position : nullptr, out_normal ? &normal : nullptr);
			if (!(out_position || out_normal)) { return hit; }
			else if (hit < min_hit) {
				min_position = position;
				min_normal = normal;
				min_hit = hit;
			}
		}

		if (out_position) { *out_position = min_position; }
		if (out_normal) { *out_normal = min_normal; }
		return min_hit;
	}

	float sphere_collider::_ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		float3 pos = get_trans().get_pos();
		float3 d = ray_origin - pos;
		float proj = dot(d, ray_direction);
		float dist_sq = d.mag_sq();
		float gap = (r * r) - dist_sq;

		if (gap < 0.0f && proj > 0.0f) { return NO_HIT; }

		float discrim = proj * proj + gap;
		if (discrim < 0) { return NO_HIT; }

		float t = -proj - sqrtf(discrim);
		if (t < 0) { t = 0; }

		float3 hit_pos;
		if ((out_position || out_normal)) { hit_pos = ray_origin + ray_direction * t; }

		if (out_position) { *out_position = hit_pos; }
		if (out_normal) { *out_normal = (hit_pos - pos).norm(); }

		return t;
	}

	float cylinder_collider::_ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		float3 up = { 0, 1, 0 };
		
		float3 pos = get_trans().get_pos();
		float2 pos_xz = pos.xz();
		float h = get_h();
		float r = get_r();

		float min_hit = NO_HIT;

		float3 min_position, min_normal;

		// Check top and bottom
		for (int i = -1; i < 2; i += 2) {
			float3 position, normal;
			float hit = ray_pick_plane(ray_origin, ray_direction, pos + up * h * i, up * i, out_position ? &position : nullptr, out_normal ? &normal : nullptr);

			if (hit == NO_HIT) continue;

			float2 hit_xz = position.xz();
			float2 dist_xz = hit_xz - pos_xz;
			if (dist_xz.mag_sq() > r * r) continue;

			if (hit < min_hit) {
				if (!(out_position || out_normal)) { return hit; }
				min_position = position;
				min_normal = normal;
				min_hit = hit;
			}
		}

		float2 ray_origin_xz = ray_origin.xz();
		float2 ray_direction_xz = ray_direction.xz();

		// Check sides
		flat::circle_collider circle(nullptr, r);
		circle.sync(pos_xz, float2{ 1 }, 0);

		float2 position_xz, normal_xz;

		float hit = circle.ray_pick(ray_origin_xz, ray_direction_xz, &position_xz, out_normal ? &normal_xz : nullptr);

		if (IS_HIT(hit)) {
			float3 position = ray_origin + ray_direction * hit;

			if (position.y > pos.y - h && position.y < pos.y + h) {
				min_position = position;
				min_normal = { normal_xz.x, 0, normal_xz.y };
				min_hit = hit;
			}
		}

		if (out_position) { *out_position = min_position; }
		if (out_normal) { *out_normal = min_normal; }

		return min_hit;
	}

	float capsule_collider::_ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		cylinder_collider body(nullptr, r, h);
		body.set_scl(scale);
		body.sync(get_trans());

		float3 min_position;
		float3 min_normal;
		float min_hit = body.ray_pick(ray_origin, ray_direction, out_position ? &min_position : nullptr, out_normal ? &min_normal : nullptr);
		if (!(out_position || out_normal) && IS_HIT(min_hit)) { return min_hit; }

		sphere_collider cap[2] = { {nullptr, r}, {nullptr, r} };
		for (int i = 0; i < 2; i++) {
			float flip = i ? -1.0f : 1.0f;
			cap[i].set_off({ 0, get_h() * flip, 0 });
			cap[i].set_scl(scale);
			cap[i].sync(get_trans());

			float3 position;
			float3 normal;
			float hit = cap[i].ray_pick(ray_origin, ray_direction, out_position ? &position : nullptr, out_normal ? &normal : nullptr);
			if (hit < min_hit) {
				if (!(out_position || out_normal)) { return hit; }
				min_position = position;
				min_normal = normal;
				min_hit = hit;
			}
		} 

		if (IS_HIT(min_hit)) {
			if (out_position) { *out_position = min_position; }
			if (out_normal)   { *out_normal =  min_normal; }
		}

		return min_hit;

	}

	float mesh_collider::_ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		float3 position;
		get_mesh()->ray_collision(get_trans(), ray_origin, ray_direction, &position, out_normal, (!(out_position || out_normal)));
		if (!(out_position || out_normal)) { return ANY_HIT; }
		return (ray_origin - position).mag();
	}

	float plane_collider::_ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const {
		return ray_pick_plane(ray_origin, ray_direction, trans.get_pos(), get_normal(), out_position, out_normal);
	}

	// Debug Rendering

	void aabb_collider::_render_debug(render_settings rs) const {
		if (!debug_model) { debug_model.reset(create_cube(-size, size)); }
		transform collider_trans;
		collider_trans.set_qtn(quaternion::identity());
		collider_trans.set_scl(get_scl());
		collider_trans.set_pos(trans.get_pos());
		( rs & render_settings(rasterize::WIRE, stencil::SURFACE_NONE, pixel_shader{ "default_full" }) & debug_model->default_rs()).set();
		debug_model->render(collider_trans, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void box_collider::_render_debug(render_settings rs) const {
		if (!debug_model) { debug_model.reset(create_cube(-size, size)); }
		transform collider_trans;
		collider_trans.set_qtn(get_quaternion());
		collider_trans.set_scl(get_scl());
		collider_trans.set_pos(trans.get_pos());
		( rs & render_settings(rasterize::WIRE, stencil::SURFACE_NONE, pixel_shader{ "default_full" }) & debug_model->default_rs()).set();
		debug_model->render(collider_trans, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void sphere_collider::_render_debug(render_settings rs) const {
		if (!debug_model) { debug_model.reset(create_sphere()); }
		transform collider_trans;
		collider_trans.set_qtn(quaternion::identity());
		collider_trans.set_scl(float3{ get_r() });
		collider_trans.set_pos(trans.get_pos());
		( rs & render_settings(rasterize::WIRE, stencil::SURFACE_NONE, pixel_shader{ "default_full" }) & debug_model->default_rs()).set();
		debug_model->render(trans, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void cylinder_collider::_render_debug(render_settings rs) const {
		if (!debug_model) { debug_model.reset(create_cylinder()); }
		transform collider_trans;
		collider_trans.set_qtn(quaternion::identity());
		collider_trans.set_scl(float3{ get_r(), get_h(), get_r() } * 2);
		collider_trans.set_pos(trans.get_pos());
		( rs & render_settings(rasterize::WIRE, stencil::SURFACE_NONE, pixel_shader{ "default_full" }) & debug_model->default_rs()).set();
		debug_model->render(collider_trans, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void capsule_collider::_render_debug(render_settings rs) const {
		if (!debug_model) { debug_model.reset(create_capsule()); }
		transform collider_trans;
		collider_trans.set_qtn(quaternion::identity());
		collider_trans.set_scl(float3{ get_r(), get_h(), get_r() } * 2);
		collider_trans.set_pos(trans.get_pos());
		( rs & render_settings(rasterize::WIRE, stencil::SURFACE_NONE, pixel_shader{ "default_full" }) & debug_model->default_rs()).set();
		debug_model->render(collider_trans, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void mesh_collider::_render_debug(render_settings rs) const {
		( rs & render_settings(rasterize::WIRE, stencil::SURFACE_NONE, pixel_shader{ "default_full" }) & debug_model->default_rs()).set();
		transform collider_trans;
		collider_trans.set_qtn(quaternion::identity());
		collider_trans.set_scl(get_scl());
		collider_trans.set_pos(trans.get_pos());
		model_ptr->render(collider_trans, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

	void plane_collider::_render_debug(render_settings rs) const {
		if (!debug_model) { debug_model.reset(create_quad()); }
		( rs & render_settings(rasterize::WIRE, stencil::SURFACE_NONE, pixel_shader{ "default_full" }) & debug_model->default_rs()).set();
		transform collider_trans;
		collider_trans.set_qtn(face_to(get_normal()));
		collider_trans.set_scl(get_scl());
		collider_trans.set_pos(trans.get_pos());
		debug_model->render(collider_trans, WHITE);
		for (auto& child : get_children()) { child->render_debug(rs); }
	}

}