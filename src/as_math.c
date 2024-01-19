// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_math.h"
#include <math.h>

f32 as_radians(f32 degrees)
{
	return degrees * (AS_PI / 180.0f);
}

void as_vec3_normalize(as_vec3* v)
{
	f32 length = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

	if (length != 0.0f)
	{
		v->x /= length;
		v->y /= length;
		v->z /= length;
	}
}

f32 as_vec3_dot(const as_vec3* a, const as_vec3* b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

as_vec3 as_vec3_unit_z()
{
	as_vec3 result = { 0.0f, 0.0f, 1.0f };
	return result;
}

as_mat4 as_mat4_identity()
{
	as_mat4 result = { 0 };
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

void as_mat4_set_identity(as_mat4* m)
{
	for (size_t i = 0; i < 4; ++i) 
	{
		for (size_t j = 0; j < 4; ++j) 
		{
			m->m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
}

void as_translate(as_mat4* m, const as_vec3* translation)
{
	m->m[0][3] += translation->x;
	m->m[1][3] += translation->y;
	m->m[2][3] += translation->z;
}

void as_set_translation(as_mat4* m, const as_vec3* translation)
{
	m->m[3][0] = translation->x;
	m->m[3][1] = translation->y;
	m->m[3][2] = translation->z;
}

//as_mat4 as_rotate(const as_vec3* axis, const f32 angle)
//{
//	f32 c = cosf(angle);
//	f32 s = sinf(angle);
//	f32 t = 1.0f - c;
//
//	as_vec3 normalized_axis = { axis->x, axis->y, axis->z };
//	as_vec3_normalize(&normalized_axis);
//
//	as_mat4 result = {
//		.M = {
//			{t * normalized_axis.x * normalized_axis.x + c, t * normalized_axis.x * normalized_axis.y + s * normalized_axis.z, t * normalized_axis.x * normalized_axis.z - s * normalized_axis.y, 0.0f},
//			{t * normalized_axis.x * normalized_axis.y - s * normalized_axis.z, t * normalized_axis.y * normalized_axis.y + c, t * normalized_axis.y * normalized_axis.z + s * normalized_axis.x, 0.0f},
//			{t * normalized_axis.x * normalized_axis.z + s * normalized_axis.y, t * normalized_axis.y * normalized_axis.z - s * normalized_axis.x, t * normalized_axis.z * normalized_axis.z + c, 0.0f},
//			{0.0f, 0.0f, 0.0f, 1.0f},
//		}
//	};
//
//	return result;
//}

as_mat4 as_rotate(const as_mat4* m, const f32 angle, const as_vec3* v)
{
	f32 const a = angle;
	f32 const c = cosf(a);
	f32 const s = sinf(a);

	as_vec3 axis = *v;
	as_vec3_normalize(&axis);
	as_vec3 temp = { (1.0f - c) * axis.x, (1.0f - c) * axis.y, (1.0f - c) * axis.z };

	as_mat4 rotate = 
	{ {
		{c + temp.x * axis.x, temp.x * axis.y + s * axis.z, temp.x * axis.z - s * axis.y, 0.0f},
		{temp.y * axis.x - s * axis.z, c + temp.y * axis.y, temp.y * axis.z + s * axis.x, 0.0f},
		{temp.z * axis.x + s * axis.y, temp.z * axis.y - s * axis.x, c + temp.z * axis.z, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f},
	} };

	as_mat4 result = { {
		{m->m[0][0] * rotate.m[0][0] + m->m[1][0] * rotate.m[0][1] + m->m[2][0] * rotate.m[0][2],
		 m->m[0][1] * rotate.m[0][0] + m->m[1][1] * rotate.m[0][1] + m->m[2][1] * rotate.m[0][2],
		 m->m[0][2] * rotate.m[0][0] + m->m[1][2] * rotate.m[0][1] + m->m[2][2] * rotate.m[0][2],
		 m->m[0][3] * rotate.m[0][0] + m->m[1][3] * rotate.m[0][1] + m->m[2][3] * rotate.m[0][2]},

		{m->m[0][0] * rotate.m[1][0] + m->m[1][0] * rotate.m[1][1] + m->m[2][0] * rotate.m[1][2],
		 m->m[0][1] * rotate.m[1][0] + m->m[1][1] * rotate.m[1][1] + m->m[2][1] * rotate.m[1][2],
		 m->m[0][2] * rotate.m[1][0] + m->m[1][2] * rotate.m[1][1] + m->m[2][2] * rotate.m[1][2],
		 m->m[0][3] * rotate.m[1][0] + m->m[1][3] * rotate.m[1][1] + m->m[2][3] * rotate.m[1][2]},

		{m->m[0][0] * rotate.m[2][0] + m->m[1][0] * rotate.m[2][1] + m->m[2][0] * rotate.m[2][2],
		 m->m[0][1] * rotate.m[2][0] + m->m[1][1] * rotate.m[2][1] + m->m[2][1] * rotate.m[2][2],
		 m->m[0][2] * rotate.m[2][0] + m->m[1][2] * rotate.m[2][1] + m->m[2][2] * rotate.m[2][2],
		 m->m[0][3] * rotate.m[2][0] + m->m[1][3] * rotate.m[2][1] + m->m[2][3] * rotate.m[2][2]},

		{m->m[3][0], m->m[3][1], m->m[3][2], m->m[3][3]},
	} };

	return result;
}

void as_set_rotation(as_mat4* m, const as_vec3* rotation)
{
	as_mat4_set_identity(m);

	float cosX = cosf(rotation->x);
	float sinX = sinf(rotation->x);
	float cosY = cosf(rotation->y);
	float sinY = sinf(rotation->y);
	float cosZ = cosf(rotation->z);
	float sinZ = sinf(rotation->z);

	m->m[0][0] = cosY * cosZ;
	m->m[0][1] = cosX * sinZ + sinX * sinY * cosZ;
	m->m[0][2] = sinX * sinZ - cosX * sinY * cosZ;

	m->m[1][0] = -cosY * sinZ;
	m->m[1][1] = cosX * cosZ - sinX * sinY * sinZ;
	m->m[1][2] = sinX * cosZ + cosX * sinY * sinZ;

	m->m[2][0] = sinY;
	m->m[2][1] = -sinX * cosY;
	m->m[2][2] = cosX * cosY;
}

as_mat4 as_look_at(const as_vec3* eye, const as_vec3* center, const as_vec3* up)
{
	as_vec3 f, r, u;
	f.x = center->x - eye->x;
	f.y = center->y - eye->y;
	f.z = center->z - eye->z;
	as_vec3_normalize(&f);

	r.x = up->y * f.z - up->z * f.y;
	r.y = up->z * f.x - up->x * f.z;
	r.z = up->x * f.y - up->y * f.x;
	as_vec3_normalize(&r);

	u.x = f.y * r.z - f.z * r.y;
	u.y = f.z * r.x - f.x * r.z;
	u.z = f.x * r.y - f.y * r.x;
	as_vec3_normalize(&u);

	as_mat4 result = {
		.m = {
			{r.x, u.x, -f.x, 0.0f},
			{r.y, u.y, -f.y, 0.0f},
			{r.z, u.z, -f.z, 0.0f},
			{-as_vec3_dot(&r, eye), -as_vec3_dot(&u, eye), as_vec3_dot(&f, eye), 1.0f},
		}
	};
	return result;
}

as_mat4 as_perspective(const f32 fov, const f32 aspect, const f32 near_plane, const f32 far_plane)
{
	f32 tan_half_fov = tanf(fov / 2.0f);
	as_mat4 result = 
	{ {
		{1.0f / (aspect * tan_half_fov),	0.0f,					0.0f,													0.0f},
		{0.0f,								1.0f / tan_half_fov,	0.0f,													0.0f},
		{0.0f,								0.0f,					(far_plane + near_plane) / (near_plane - far_plane),	-1.0f},
		{0.0f,								0.0f,					(2.0f * far_plane * near_plane) / (near - far_plane),	0.0f},
	} };
	return result;
}

void as_set_scale(as_mat4* m, const as_vec3* scale)
{
	m->m[0][0] = scale->x;
	m->m[1][1] = scale->y;
	m->m[2][2] = scale->z;
}
