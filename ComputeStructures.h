#pragma once
struct alignas(16) vec3 {
	float x, y, z, padding;

	vec3() : x(0), y(0), z(0), padding(0) {}
	vec3(float x, float y, float z) : x(x), y(y), z(z), padding(0) {}
};

struct vec4 {
	float x, y, z, a;

	vec4() : x(0), y(0), z(0), a(0) {}
	vec4(float x, float y, float z, float a) : x(x), y(y), z(z), a(0) {}
};

struct alignas(16) Material {
	vec3 emmisionStrength;
	vec3 emmisionColor;
	vec3 diffuseColor;
	vec3 smoothness;
};

struct alignas(16) TraceCircle {
	Material material;
	vec3 position;
	float radius;
};

struct alignas(16) CameraSettings {
	vec3 position;
	vec3 direction;
	float fov;
};