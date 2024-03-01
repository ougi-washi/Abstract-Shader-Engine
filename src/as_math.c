// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_math.h"

f32 as_radians(const f32 degrees)
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

as_vec3 as_vec3_get_normal(const as_vec3* v)
{
    as_vec3 result;
    f32 length = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

    if (length != 0.0f)
    {
        result.x = v->x / length;
        result.y = v->y / length;
        result.z = v->z / length;
    }
    else
    {
        result = *v;
    }

    return result;
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

as_vec3 as_vec3_cross(const as_vec3* a, const as_vec3* b)
{
	as_vec3 result;

	result.x = a->y * b->z - a->z * b->y;
	result.y = a->z * b->x - a->x * b->z;
	result.z = a->x * b->y - a->y * b->x;

	return result;
}

f32 as_vec3_length(const as_vec3* v) 
{
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

f32 as_vec3_squared_distance(const as_vec3* a, const as_vec3* b)
{
	f32 dx = a->x - b->x;
	f32 dy = a->y - b->y;
	f32 dz = a->z - b->z;
	return dx * dx + dy * dy + dz * dz;
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

as_mat4 as_mat4_multiply(const as_mat4* a, const as_mat4* b)
{
	as_mat4 result;

	for (size_t i = 0; i < 4; ++i)
	{
		for (size_t j = 0; j < 4; ++j)
		{
			result.m[i][j] = a->m[i][0] * b->m[0][j] +
				a->m[i][1] * b->m[1][j] +
				a->m[i][2] * b->m[2][j] +
				a->m[i][3] * b->m[3][j];
		}
	}

	return result;
}

as_vec3 as_mat4_get_translation(const as_mat4* m)
{
	return AS_VEC(as_vec3, m->m[3][0], m->m[3][1], m->m[3][2]);
}

void as_mat4_translate(as_mat4* m, const as_vec3* translation)
{
	m->m[3][0] += translation->x;
	m->m[3][1] += translation->y;
	m->m[3][2] += translation->z;
}

void as_mat4_set_translation(as_mat4* m, const as_vec3* translation)
{
	m->m[3][0] = translation->x;
	m->m[3][1] = translation->y;
	m->m[3][2] = translation->z;
}

extern as_quat as_mat4_get_rotation(const as_mat4* m)
{
	as_quat quat;

	float trace = m->m[0][0] + m->m[1][1] + m->m[2][2];

	if (trace > 0.0f)
	{
		float s = 0.5f / sqrtf(trace + 1.0f);
		quat.w = 0.25f / s;
		quat.x = (m->m[2][1] - m->m[1][2]) * s;
		quat.y = (m->m[0][2] - m->m[2][0]) * s;
		quat.z = (m->m[1][0] - m->m[0][1]) * s;
	}
	else
	{
		if (m->m[0][0] > m->m[1][1] && m->m[0][0] > m->m[2][2])
		{
			float s = 2.0f * sqrtf(1.0f + m->m[0][0] - m->m[1][1] - m->m[2][2]);
			quat.w = (m->m[2][1] - m->m[1][2]) / s;
			quat.x = 0.25f * s;
			quat.y = (m->m[0][1] + m->m[1][0]) / s;
			quat.z = (m->m[0][2] + m->m[2][0]) / s;
		}
		else if (m->m[1][1] > m->m[2][2])
		{
			float s = 2.0f * sqrtf(1.0f + m->m[1][1] - m->m[0][0] - m->m[2][2]);
			quat.w = (m->m[0][2] - m->m[2][0]) / s;
			quat.x = (m->m[0][1] + m->m[1][0]) / s;
			quat.y = 0.25f * s;
			quat.z = (m->m[1][2] + m->m[2][1]) / s;
		}
		else
		{
			float s = 2.0f * sqrtf(1.0f + m->m[2][2] - m->m[0][0] - m->m[1][1]);
			quat.w = (m->m[1][0] - m->m[0][1]) / s;
			quat.x = (m->m[0][2] + m->m[2][0]) / s;
			quat.y = (m->m[1][2] + m->m[2][1]) / s;
			quat.z = 0.25f * s;
		}
	}

	return quat;
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

void as_mat4_rotate(as_mat4* m, const f32 angle, const as_vec3* axis)
{
	f32 const a = angle;
	f32 const c = cosf(a);
	f32 const s = sinf(a);

	as_vec3 axis_norm = *axis;
	as_vec3_normalize(&axis_norm);
	as_vec3 temp = { (1.0f - c) * axis_norm.x, (1.0f - c) * axis_norm.y, (1.0f - c) * axis_norm.z };

	as_mat4 rotate = 
	{ {
		{c + temp.x * axis_norm.x, temp.x * axis_norm.y + s * axis_norm.z, temp.x * axis_norm.z - s * axis_norm.y, 0.0f},
		{temp.y * axis_norm.x - s * axis_norm.z, c + temp.y * axis_norm.y, temp.y * axis_norm.z + s * axis_norm.x, 0.0f},
		{temp.z * axis_norm.x + s * axis_norm.y, temp.z * axis_norm.y - s * axis_norm.x, c + temp.z * axis_norm.z, 0.0f},
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

	*m = result;
}

void as_mat4_rotate_around_pivot(as_mat4* m, const f32 angle, const as_vec3* axis, const as_vec3* pivot)
{
	// still WIP
	as_mat4 translation_matrix = AS_MAT4_TRANSLATION(pivot->x, pivot->y, pivot->z);
	as_mat4 rotation_matrix = { 0 };
	as_mat4_rotate(&rotation_matrix, angle, axis);
	as_mat4 translation_back_matrix = AS_MAT4_TRANSLATION(-pivot->x, -pivot->y, -pivot->z);

	as_vec3 p = as_mat4_get_translation(m);
	as_vec3 p1 = as_mat4_multiply_vec3(&translation_matrix, &p);
	as_vec3 p2 = as_mat4_multiply_vec3(&rotation_matrix, &p1);
	as_vec3 p3 = as_mat4_multiply_vec3(&translation_back_matrix, &p2);

	as_mat4_set_translation(m, &p3);
}

void as_mat4_rotate_with_quat(as_mat4* m, const as_quat* q)
{
	as_mat4 rotation_matrix = {
		.m = {
			{1.0f - 2.0f * (q->y * q->y + q->z * q->z), 2.0f * (q->x * q->y - q->w * q->z),     2.0f * (q->x * q->z + q->w * q->y),     0.0f},
			{2.0f * (q->x * q->y + q->w * q->z),     1.0f - 2.0f * (q->x * q->x + q->z * q->z), 2.0f * (q->y * q->z - q->w * q->x),     0.0f},
			{2.0f * (q->x * q->z - q->w * q->y),     2.0f * (q->y * q->z + q->w * q->x),     1.0f - 2.0f * (q->x * q->x + q->y * q->y), 0.0f},
			{0.0f,                                 0.0f,                                 0.0f,                                 1.0f}
		}
	};

	*m = as_mat4_multiply(m, &rotation_matrix);
}

void as_mat4_set_rotation(as_mat4* m, const as_vec3* rotation)
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

as_vec3 as_mat4_multiply_vec3(const as_mat4* m, const as_vec3* v)
{
	as_vec4 result = as_mat4_multiply_vec4(m, AS_VEC_PTR(as_vec4, v->x, v->y, v->z, 1.f));
	return AS_VEC(as_vec3, result.x, result.y, result.z);
}

as_vec4 as_mat4_multiply_vec4(const as_mat4* m, const as_vec4* v)
{
	as_vec4 result;

	result.x = m->m[0][0] * v->x + m->m[1][0] * v->y + m->m[2][0] * v->z + m->m[3][0] * v->w;
	result.y = m->m[0][1] * v->x + m->m[1][1] * v->y + m->m[2][1] * v->z + m->m[3][1] * v->w;
	result.z = m->m[0][2] * v->x + m->m[1][2] * v->y + m->m[2][2] * v->z + m->m[3][2] * v->w;
	result.w = m->m[0][3] * v->x + m->m[1][3] * v->y + m->m[2][3] * v->z + m->m[3][3] * v->w;

	// diff approach later, to optimize
	//result.x = dot(row[0], v);
	//result.y = dot(row[1], v);
	//result.z = dot(row[2], v);
	//result.w = dot(row[3], v);

	return result;
}

extern as_vec3 as_mat4_get_scale(const as_mat4* m)
{
	as_vec3 scale;
	scale.x = sqrtf(m->m[0][0] * m->m[0][0] + m->m[0][1] * m->m[0][1] + m->m[0][2] * m->m[0][2]);
	scale.y = sqrtf(m->m[1][0] * m->m[1][0] + m->m[1][1] * m->m[1][1] + m->m[1][2] * m->m[1][2]);
	scale.z = sqrtf(m->m[2][0] * m->m[2][0] + m->m[2][1] * m->m[2][1] + m->m[2][2] * m->m[2][2]);

	return scale;
}

void as_mat4_set_scale(as_mat4* m, const as_vec3* scale)
{
	m->m[0][0] = scale->x;
	m->m[1][1] = scale->y;
	m->m[2][2] = scale->z;
}

as_mat4 as_mat4_look_at(const as_vec3* eye, const as_vec3* center, const as_vec3* up)
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

as_mat4 as_mat4_perspective(const f32 fov, const f32 aspect, const f32 near_plane, const f32 far_plane)
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

as_quat as_vec3_to_quat(const as_vec3* v)
{
	float c1 = cosf(v->x / 2.0f);
	float s1 = sinf(v->x / 2.0f);
	float c2 = cosf(v->y / 2.0f);
	float s2 = sinf(v->y / 2.0f);
	float c3 = cosf(v->z / 2.0f);
	float s3 = sinf(v->z / 2.0f);

	as_quat quat;
	quat.w = c1 * c2 * c3 - s1 * s2 * s3;
	quat.x = s1 * s2 * c3 + c1 * c2 * s3;
	quat.y = s1 * c2 * c3 + c1 * s2 * s3;
	quat.z = c1 * s2 * c3 - s1 * c2 * s3;

	return quat;
}

as_vec3 as_quat_to_vec3(const as_quat* q)
{
	float roll, pitch, yaw;

	// roll (x-axis rotation)
	float sinr_cosp = 2.0f * (q->w * q->x + q->y * q->z);
	float cosr_cosp = 1.0f - 2.0f * (q->x * q->x + q->y * q->y);
	roll = atan2f(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	float sinp = 2.0f * (q->w * q->y - q->z * q->x);
	if (fabs(sinp) >= 1)
		pitch = copysignf(AS_PI / 2, sinp); // use 90 degrees if out of range
	else
		pitch = asinf(sinp);

	// yaw (z-axis rotation)
	float siny_cosp = 2.0f * (q->w * q->z + q->x * q->y);
	float cosy_cosp = 1.0f - 2.0f * (q->y * q->y + q->z * q->z);
	yaw = atan2f(siny_cosp, cosy_cosp);

	return (as_vec3) { roll, pitch, yaw };
}

void as_quat_rotate(as_quat* q, const f32 angle, const as_vec3* axis)
{
	f32 const a = angle / 2.0f;
	f32 const s = sinf(a);

	as_quat rotation_quat;
	rotation_quat.w = cosf(a);
	rotation_quat.x = axis->x * s;
	rotation_quat.y = axis->y * s;
	rotation_quat.z = axis->z * s;

	as_quat_mul(q, &rotation_quat, q);
}

as_quat as_mat4_to_quat(const as_mat4* m)
{
	as_quat quat;

	float trace = m->m[0][0] + m->m[1][1] + m->m[2][2];

	if (trace > 0.0f)
	{
		float s = 0.5f / sqrtf(trace + 1.0f);
		quat.w = 0.25f / s;
		quat.x = (m->m[2][1] - m->m[1][2]) * s;
		quat.y = (m->m[0][2] - m->m[2][0]) * s;
		quat.z = (m->m[1][0] - m->m[0][1]) * s;
	}
	else
	{
		if (m->m[0][0] > m->m[1][1] && m->m[0][0] > m->m[2][2])
		{
			float s = 2.0f * sqrtf(1.0f + m->m[0][0] - m->m[1][1] - m->m[2][2]);
			quat.w = (m->m[2][1] - m->m[1][2]) / s;
			quat.x = 0.25f * s;
			quat.y = (m->m[0][1] + m->m[1][0]) / s;
			quat.z = (m->m[0][2] + m->m[2][0]) / s;
		}
		else if (m->m[1][1] > m->m[2][2])
		{
			float s = 2.0f * sqrtf(1.0f + m->m[1][1] - m->m[0][0] - m->m[2][2]);
			quat.w = (m->m[0][2] - m->m[2][0]) / s;
			quat.x = (m->m[0][1] + m->m[1][0]) / s;
			quat.y = 0.25f * s;
			quat.z = (m->m[1][2] + m->m[2][1]) / s;
		}
		else
		{
			float s = 2.0f * sqrtf(1.0f + m->m[2][2] - m->m[0][0] - m->m[1][1]);
			quat.w = (m->m[1][0] - m->m[0][1]) / s;
			quat.x = (m->m[0][2] + m->m[2][0]) / s;
			quat.y = (m->m[1][2] + m->m[2][1]) / s;
			quat.z = 0.25f * s;
		}
	}

	return quat;
}
