// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
// Most shapes rely on Inigo Quilez's approach

#include "as_sdf_operators.glsl"

float dot2( in vec2 v ) { vec3 v3 = vec3(v.x, v.y, 0.); return dot(v3,v3); }
float dot2( in vec3 v ) { return dot(v,v); }
float ndot( in vec2 a, in vec2 b ) { return a.x*b.x - a.y*b.y; }

// Sphere - exact
float sd_sphere(vec3 p, float s) {
    return length(p) - s;
}

// Box - exact
float sd_box(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// Round Box - exact
float sd_round_box(vec3 p, vec3 b, float r) {
    vec3 q = abs(p) - b;
    return length(max(q, vec3(0.0))) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

// Box Frame - exact
float sd_box_frame(vec3 p, vec3 b, float e) {
    p = abs(p) - b;
    vec3 q = abs(p + e) - e;
    return min(min(
        length(max(vec3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
        length(max(vec3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
        length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

// Torus - exact
float sd_torus(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

// Capped Torus - exact
float sd_capped_torus(vec3 p, vec2 sc, float ra, float rb) {
    p.x = abs(p.x);
    float k = (sc.y * p.x > sc.x * p.y) ? dot(p.xy, sc) : length(p.xy);
    return sqrt(dot(p, p) + ra * ra - 2.0 * ra * k) - rb;
}

// Link - exact
float sd_link(vec3 p, float le, float r1, float r2) {
    vec3 q = vec3(p.x, max(abs(p.y) - le, 0.0), p.z);
    return length(vec2(length(q.xy) - r1, q.z)) - r2;
}

// Infinite Cylinder - exact
float sd_cylinder(vec3 p, vec3 c) {
    return length(p.xz - c.xy) - c.z;
}

// Cone - exact
float sd_cone(vec3 p, vec2 c, float h) {
    vec2 q = h * vec2(c.x / c.y, -1.0);
    vec2 w = vec2(length(p.xz), p.y);
    vec2 a = w - q * clamp(dot(w, q) / dot(q, q), 0.0, 1.0);
    vec2 b = w - q * vec2(clamp(w.x / q.x, 0.0, 1.0), 1.0);
    float k = sign(q.y);
    float d = min(dot(a, a), dot(b, b));
    float s = max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
    return sqrt(d) * sign(s);
}

// Cone - bound (not exact!)
float sd_cone_bound(vec3 p, vec2 c, float h) {
    float q = length(p.xz);
    return max(dot(c.xy, vec2(q, p.y)), -h - p.y);
}

// Infinite Cone - exact
float sd_infinite_cone(vec3 p, vec2 c) {
    vec2 q = vec2(length(p.xz), -p.y);
    float d = length(q - c * max(dot(q, c), 0.0));
    return d * ((q.x * c.y - q.y * c.x < 0.0) ? -1.0 : 1.0);
}

// Plane - exact
float sd_plane(vec3 p, vec3 n, float h) {
    return dot(p, n) + h;
}

// Hexagonal Prism - exact
float sd_hex_prism(vec3 p, vec2 h) {
    const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
    p = abs(p);
    p.xy -= 2.0 * min(dot(k.xy, p.xy), 0.0) * k.xy;
    vec2 d = vec2(
        length(p.xy - vec2(clamp(p.x, -k.z * h.x, k.z * h.x), h.x)) * sign(p.y - h.x),
        p.z - h.y);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

// Triangular Prism - bound
float sd_tri_prism(vec3 p, vec2 h) {
    vec3 q = abs(p);
    return max(q.z - h.y, max(q.x * 0.866025 + p.y * 0.5, -p.y) - h.x * 0.5);
}

// Capsule / Line - exact
float sd_capsule(vec3 p, vec3 a, vec3 b, float r) {
    vec3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

// Capsule / Line - exact
float sd_vertical_capsule(vec3 p, float h, float r) {
    p.y -= clamp(p.y, 0.0, h);
    return length(p) - r;
}

// Vertical Capped Cylinder - exact
float sd_capped_cylinder(vec3 p, float h, float r) {
    vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(r, h);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

// Arbitrary Capped Cylinder - exact
float sd_arbitrary_capped_cylinder(vec3 p, vec3 a, vec3 b, float r) {
    vec3 ba = b - a;
    vec3 pa = p - a;
    float baba = dot(ba, ba);
    float paba = dot(pa, ba);
    float x = length(pa * baba - ba * paba) - r * baba;
    float y = abs(paba - baba * 0.5) - baba * 0.5;
    float x2 = x * x;
    float y2 = y * y * baba;
    float d = (max(x, y) < 0.0) ? -min(x2, y2) : (((x > 0.0) ? x2 : 0.0) + ((y > 0.0) ? y2 : 0.0));
    return sign(d) * sqrt(abs(d)) / baba;
}

// Rounded Cylinder - exact
float sd_rounded_cylinder(vec3 p, float ra, float rb, float h) {
    vec2 d = vec2(length(p.xz) - 2.0 * ra + rb, abs(p.y) - h);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rb;
}

// Capped Cone - exact
float sd_capped_cone(vec3 p, float h, float r1, float r2) {
    vec2 q = vec2(length(p.xz), p.y);
    vec2 k1 = vec2(r2, h);
    vec2 k2 = vec2(r2 - r1, 2.0 * h);
    vec2 ca = vec2(q.x - min(q.x, (q.y < 0.0) ? r1 : r2), abs(q.y) - h);
    vec2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot(k2, k2), 0.0, 1.0);
    float s = (cb.x < 0.0 && ca.y < 0.0) ? -1.0 : 1.0;
    return s * sqrt(min(dot(ca, ca), dot(cb, cb)));
}

// Capped Cone - exact
float sd_capped_cone_exact(vec3 p, vec3 a, vec3 b, float ra, float rb) {
    float rba = rb - ra;
    float baba = dot(b - a, b - a);
    float papa = dot(p - a, p - a);
    float paba = dot(p - a, b - a) / baba;
    float x = sqrt(papa - paba * paba * baba);
    float cax = max(0.0, x - ((paba < 0.5) ? ra : rb));
    float cay = abs(paba - 0.5) - 0.5;
    float k = rba * rba + baba;
    float f = clamp((rba * (x - ra) + paba * baba) / k, 0.0, 1.0);
    float cbx = x - ra - f * rba;
    float cby = paba - f;
    float s = (cbx < 0.0 && cay < 0.0) ? -1.0 : 1.0;
    return s * sqrt(min(cax * cax + cay * cay * baba, cbx * cbx + cby * cby * baba));
}

// Solid Angle - exact
float sd_solid_angle(vec3 p, vec2 c, float ra) {
    vec2 q = vec2(length(p.xz), p.y);
    float l = length(q) - ra;
    float m = length(q - c * clamp(dot(q, c), 0.0, ra));
    return max(l, m * sign(c.y * q.x - c.x * q.y));
}

// Cut Sphere - exact
float sd_cut_sphere(vec3 p, float r, float h) {
    float w = sqrt(r * r - h * h);
    vec2 q = vec2(length(p.xz), p.y);
    float s = max((h - r) * q.x * q.x + w * w * (h + r - 2.0 * q.y), h * q.x - w * q.y);
    return (s < 0.0) ? length(q) - r : ((q.x < w) ? h - q.y : length(q - vec2(w, h)));
}

// Cut Hollow Sphere - exact
float sd_cut_hollow_sphere(vec3 p, float r, float h, float t) {
    float w = sqrt(r * r - h * h);
    vec2 q = vec2(length(p.xz), p.y);
    return ((h * q.x < w * q.y) ? length(q - vec2(w, h)) : abs(length(q) - r)) - t;
}

// Death Star - exact
float sd_death_star(vec3 p2, float ra, float rb, float d) {
    float a = (ra * ra - rb * rb + d * d) / (2.0 * d);
    float b = sqrt(max(ra * ra - a * a, 0.0));
    vec2 p = vec2(p2.x, length(p2.yz));
    if (p.x * b - p.y * a > d * max(b - p.y, 0.0))
        return length(p - vec2(a, b));
    else
        return max((length(p) - ra), -(length(p - vec2(d, 0.0)) - rb));
}

// Round cone - exact
float sd_round_cone(vec3 p, float r1, float r2, float h) {
    float b = (r1 - r2) / h;
    float a = sqrt(1.0 - b * b);
    vec2 q = vec2(length(p.xz), p.y);
    float k = dot(q, vec2(-b, a));
    if (k < 0.0) return length(q) - r1;
    if (k > a * h) return length(q - vec2(0.0, h)) - r2;
    return dot(q, vec2(a, b)) - r1;
}

// Round Cone - exact
float sd_round_cone_ab(vec3 p, vec3 a, vec3 b, float r1, float r2) {
    vec3 ba = b - a;
    float l2 = dot(ba, ba);
    float rr = r1 - r2;
    float a2 = l2 - rr * rr;
    float il2 = 1.0 / l2;
    vec3 pa = p - a;
    float y = dot(pa, ba);
    float z = y - l2;
    float x2 = dot2(pa * l2 - ba * y);
    float y2 = y * y * l2;
    float z2 = z * z * l2;
    float k = sign(rr) * rr * rr * x2;
    if (sign(z) * a2 * z2 > k) return sqrt(x2 + z2) * il2 - r2;
    if (sign(y) * a2 * y2 < k) return sqrt(x2 + y2) * il2 - r1;
    return (sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
}

// ellipsoid - bound (not exact!)
float sd_ellipsoid(vec3 p, vec3 r) {
    float k0 = length(p / r);
    float k1 = length(p / (r * r));
    return k0 * (k0 - 1.0) / k1;
}

// revolved_vesica - exact
float sd_revolved_vesica_segment(vec3 p, vec3 a, vec3 b, float w) {
    vec3 c = (a + b) * 0.5;
    float l = length(b - a);
    vec3 v = (b - a) / l;
    float y = dot(p - c, v);
    vec2 q = vec2(length(p - c - y * v), abs(y));

    float r = 0.5 * l;
    float d = 0.5 * (r * r - w * w) / w;
    vec3 h = (r * q.x < d * (q.y - r)) ? vec3(0.0, r, 0.0) : vec3(-d, 0.0, d + w);

    return length(q - h.xy) - h.z;
}

// rhombus - exact
float sd_rhombus(vec3 p, float la, float lb, float h, float ra) {
    p = abs(p);
    vec2 b = vec2(la, lb);
    float f = clamp((dot(b, b - 2.0 * p.xz)) / dot(b, b), -1.0, 1.0);
    vec2 q = vec2(length(p.xz - 0.5 * b * vec2(1.0 - f, 1.0 + f)) * sign(p.x * b.y + p.z * b.x - b.x * b.y) - ra, p.y - h);
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0));
}

// octahedron - exact
float sd_octahedron_exact(vec3 p, float s) {
    p = abs(p);
    float m = p.x + p.y + p.z - s;
    vec3 q;
    if (3.0 * p.x < m) q = p.xyz;
    else if (3.0 * p.y < m) q = p.yzx;
    else if (3.0 * p.z < m) q = p.zxy;
    else return m * 0.57735027;

    float k = clamp(0.5 * (q.z - q.y + s), 0.0, s);
    return length(vec3(q.x, q.y - s + k, q.z - k));
}

//octahedron - bound (not exact)
float sd_octahedron(vec3 p, float s) {
    p = abs(p);
    return (p.x + p.y + p.z - s) * 0.57735027;
}

// pyramid - exact
float sd_pyramid(vec3 p, float h) {
    float m2 = h * h + 0.25;

    p.xz = abs(p.xz);
    p.xz = (p.z > p.x) ? p.zx : p.xz;
    p.xz -= 0.5;

    vec3 q = vec3(p.z, h * p.y - 0.5 * p.x, h * p.x + 0.5 * p.y);

    float s = max(-q.x, 0.0);
    float t = clamp((q.y - 0.5 * p.z) / (m2 + 0.25), 0.0, 1.0);

    float a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
    float b = m2 * (q.x + 0.5 * t) * (q.x + 0.5 * t) + (q.y - m2 * t) * (q.y - m2 * t);

    float d2 = min(q.y, -q.x * m2 - q.y * 0.5) > 0.0 ? 0.0 : min(a, b);

    return sqrt((d2 + q.z * q.z) / m2) * sign(max(q.z, -p.y));
}

// // triangle - exact
// float ud_triangle(vec3 p, vec3 a, vec3 b, vec3 c) {
//     vec3 ba = b - a; vec3 pa = p - a;
//     vec3 cb = c - b; vec3 pb = p - b;
//     vec3 ac = a - c; vec3 pc = p - c;
//     vec3 nor = cross(ba, ac);

//     return sqrt(
//         (sign(dot(cross(ba, nor), pa)) +
//          sign(dot(cross(cb, nor), pb)) +
//          sign(dot(cross(ac, nor), pc)) < 2.0)
//             ?
//             min(min(
//                 dot(ba * clamp(dot(ba, pa) / dot2(ba), 0.0, 1.0) - pa),
//                 dot(cb * clamp(dot(cb, pb) / dot2(cb), 0.0, 1.0) - pb)),
//                 dot(ac * clamp(dot(ac, pc) / dot2(ac), 0.0, 1.0) - pc))
//             :
//             dot(nor, pa) * dot(nor, pa) / dot2(nor));
// }

// // quad - exact
// float ud_quad(vec3 p, vec3 a, vec3 b, vec3 c, vec3 d) {
//     vec3 ba = b - a; vec3 pa = p - a;
//     vec3 cb = c - b; vec3 pb = p - b;
//     vec3 dc = d - c; vec3 pc = p - c;
//     vec3 ad = a - d; vec3 pd = p - d;
//     vec3 nor = cross(ba, ad);

//     return sqrt(
//         (sign(dot(cross(ba, nor), pa)) +
//          sign(dot(cross(cb, nor), pb)) +
//          sign(dot(cross(dc, nor), pc)) +
//          sign(dot(cross(ad, nor), pd)) < 3.0)
//             ?
//             min(min(min(
//                 dot(ba * clamp(dot(ba, pa) / dot2(ba), 0.0, 1.0) - pa),
//                 dot(cb * clamp(dot(cb, pb) / dot2(cb), 0.0, 1.0) - pb)),
//                 dot(dc * clamp(dot(dc, pc) / dot2(dc), 0.0, 1.0) - pc)),
//                 dot(ad * clamp(dot(ad, pd) / dot2(ad), 0.0, 1.0) - pd))
//             :
//             dot(nor, pa) * dot(nor, pa) / dot2(nor));
// }

//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////













