#include "SDL.h"
#include <stdio.h>

#include "triangle.h"
#include "vector.h"
#include "colour.h"

#include <cfloat>
#include <cmath>
#include <cstring>
#include <thread>
#include <vector>

#if defined(__AVX2__)
#include <immintrin.h>
#endif

#ifndef M_PI
#define M_PI 3.1415926535
#endif

static const int MAX_REFLECT = 5;
static const double MIN_LFACTOR = 0.05;

std::vector<Triangle> geometry;
std::vector<Vector> lights;
Vector move;
int x_accel = 0;
int z_accel = 0;
double theta = 0;
double phi = 0;

void createGeometry()
{
	// Front wall
	geometry.push_back(
		Triangle(
			Vector(-250, -500, 250),
			Vector(-250, 10, 250),
			Vector(250, 10, 250),
			0xFF999900u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(-250, -500, 250),
			Vector(250, -500, 250),
			Vector(250, 10, 250),
			0xFF999900u
		)
	);
	
	// Back wall
	geometry.push_back(
		Triangle(
			Vector(-250, -500, -250),
			Vector(-250, 10, -250),
			Vector(250, 10, -250),
			0xFF999900u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(-250, -500, -250),
			Vector(250, -500, -250),
			Vector(250, 10, -250),
			0xFF999900u
		)
	);
	
	// Left wall
	geometry.push_back(
		Triangle(
			Vector(-250, -500, 250),
			Vector(-250, 10, 250),
			Vector(-250, 10, -250),
			0xFF999900u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(-250, -500, 250),
			Vector(-250, -500, -250),
			Vector(-250, 10, -250),
			0xFF999900u
		)
	);
	
	// Right wall
	geometry.push_back(
		Triangle(
			Vector(250, -500, 250),
			Vector(250, 10, 250),
			Vector(250, 10, -250),
			0xFF999900u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(250, -500, 250),
			Vector(250, -500, -250),
			Vector(250, 10, -250),
			0xFF999900u
		)
	);
	
	// Floor	
	geometry.push_back(
		Triangle(
			Vector(-250, 0, -250),
			Vector(-250, 0, 250),
			Vector(250, 0, 250),
			0x33222200u,
			true,
			0.1f
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(250, 0, 250),
			Vector(250, 0, -250),
			Vector(-250, 0, -250),
			0x33222200u,
			true,
			0.1f
		)
	);
	
	// Ceiling
	geometry.push_back(
		Triangle(
			Vector(-250, -500, -250),
			Vector(-250, -500, 250),
			Vector(250, -500, 250),
			0xFFCCCC00u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(250, -500, 250),
			Vector(250, -500, -250),
			Vector(-250, -500, -250),
			0xFFCCCC00u
		)
	);
	
	// chandelier
	geometry.push_back(
		Triangle(
			Vector(40, -500, 100),
			Vector(0, -480, 140),
			Vector(40, -500, 180),
			0xFFFF9900u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(-40, -500, 100),
			Vector(0, -480, 140),
			Vector(-40, -500, 180),
			0xFFFF9900u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(40, -500, 100),
			Vector(0, -480, 140),
			Vector(-40, -500, 100),
			0xFFFF9900u
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(40, -500, 180),
			Vector(0, -480, 140),
			Vector(-40, -500, 180),
			0xFFFF9900u
		)
	);
	
	// Mirror
	geometry.push_back(
		Triangle(
			Vector(150, -400, 240),
			Vector(150, 0, 240),
			Vector(50, 0, 240),
			0x0000ff00u,
			true,
			0.9f
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(150, -400, 240),
			Vector(50, -400, 240),
			Vector(50, 0, 240),
			0x0000ff00u,
			true,
			0.9f
		)
	);
	
	// Enemy
	geometry.push_back(
		Triangle(
			Vector(100, 0, 100),
			Vector(120, -250, 80),
			Vector(140, -250, 60),
			0x00000000u,
			false,
			0,
			true
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(140, -250, 60),
			Vector(120, 0, 80),
			Vector(100, 0, 100),
			0x00000000u,
			false,
			0,
			true
		)
	);
	
	geometry.push_back(
		Triangle(
			Vector(115.5, -220, 85.5),
			Vector(125.5, -230, 75.5),
			Vector(135.5, -220, 65.5),
			0xff000000u,
			false,
			0,
			true,
			true
		)
	);
	
	lights.push_back(Vector(-240, -400, -220));
}

void updateGeometry()
{
	move += Vector(
		static_cast<float>(x_accel * cos(theta) - z_accel * sin(theta)),
		0,
		static_cast<float>(x_accel * sin(theta) + z_accel * cos(theta))
	) * 5;
}

// --- SIMD intersection ------------------------------------------------------
// Möller-Trumbore is scalar per triangle, but the per-ray work is dominated by
// scanning every triangle. Lay the triangle data out struct-of-arrays so we can
// test 8 triangles at once with AVX2 (matched by the /arch:AVX2 build flag).
struct GeometrySoA
{
	// One entry per padded lane (real triangle count rounded up to a multiple
	// of 8). Padding lanes hold zeroed edges, so their determinant is 0 and the
	// epsilon test discards them.
	std::vector<float> v0x, v0y, v0z;   // base vertex
	std::vector<float> e1x, e1y, e1z;   // v1 - v0
	std::vector<float> e2x, e2y, e2z;   // v2 - v0
	std::vector<int>   visible;         // all-ones if a real, non-invisible triangle
	size_t groups = 0;                  // number of 8-wide groups
};

GeometrySoA g_soa;

void buildGeometrySoA()
{
	size_t n = geometry.size();
	size_t groups = (n + 7) / 8;
	size_t padded = groups * 8;

	g_soa.groups = groups;
	g_soa.v0x.assign(padded, 0.0f); g_soa.v0y.assign(padded, 0.0f); g_soa.v0z.assign(padded, 0.0f);
	g_soa.e1x.assign(padded, 0.0f); g_soa.e1y.assign(padded, 0.0f); g_soa.e1z.assign(padded, 0.0f);
	g_soa.e2x.assign(padded, 0.0f); g_soa.e2y.assign(padded, 0.0f); g_soa.e2z.assign(padded, 0.0f);
	g_soa.visible.assign(padded, 0);

	for (size_t i = 0; i < n; ++i)
	{
		const Triangle& tri = geometry[i];
		Vector e1 = tri.v1 - tri.v0;
		Vector e2 = tri.v2 - tri.v0;

		g_soa.v0x[i] = tri.v0.x; g_soa.v0y[i] = tri.v0.y; g_soa.v0z[i] = tri.v0.z;
		g_soa.e1x[i] = e1.x;     g_soa.e1y[i] = e1.y;     g_soa.e1z[i] = e1.z;
		g_soa.e2x[i] = e2.x;     g_soa.e2y[i] = e2.y;     g_soa.e2z[i] = e2.z;
		g_soa.visible[i] = tri.invisible ? 0 : -1;
	}
}

#if defined(__AVX2__)
static const bool kHaveSimd = true;

static inline __m256 dot3(__m256 ax, __m256 ay, __m256 az, __m256 bx, __m256 by, __m256 bz)
{
	return _mm256_add_ps(_mm256_mul_ps(ax, bx), _mm256_add_ps(_mm256_mul_ps(ay, by), _mm256_mul_ps(az, bz)));
}

// Computes the Möller-Trumbore validity mask and hit distance t for 8 triangles
// starting at group `g`. Lanes that miss get t = FLT_MAX in `t_out`.
static inline __m256 intersectGroup(size_t g, bool includeInvisible,
	__m256 ox, __m256 oy, __m256 oz, __m256 dx, __m256 dy, __m256 dz, __m256& t_out)
{
	const __m256 eps  = _mm256_set1_ps(FLT_EPSILON);
	const __m256 zero = _mm256_setzero_ps();
	const __m256 one  = _mm256_set1_ps(1.0f);
	const __m256 sign = _mm256_set1_ps(-0.0f);
	size_t b = g * 8;

	__m256 e1x = _mm256_loadu_ps(&g_soa.e1x[b]), e1y = _mm256_loadu_ps(&g_soa.e1y[b]), e1z = _mm256_loadu_ps(&g_soa.e1z[b]);
	__m256 e2x = _mm256_loadu_ps(&g_soa.e2x[b]), e2y = _mm256_loadu_ps(&g_soa.e2y[b]), e2z = _mm256_loadu_ps(&g_soa.e2z[b]);
	__m256 v0x = _mm256_loadu_ps(&g_soa.v0x[b]), v0y = _mm256_loadu_ps(&g_soa.v0y[b]), v0z = _mm256_loadu_ps(&g_soa.v0z[b]);

	// h = dir x e2
	__m256 hx = _mm256_sub_ps(_mm256_mul_ps(dy, e2z), _mm256_mul_ps(dz, e2y));
	__m256 hy = _mm256_sub_ps(_mm256_mul_ps(dz, e2x), _mm256_mul_ps(dx, e2z));
	__m256 hz = _mm256_sub_ps(_mm256_mul_ps(dx, e2y), _mm256_mul_ps(dy, e2x));

	__m256 a = dot3(e1x, e1y, e1z, hx, hy, hz);
	__m256 mask = _mm256_cmp_ps(_mm256_andnot_ps(sign, a), eps, _CMP_GE_OQ);   // |a| >= eps
	__m256 f = _mm256_div_ps(one, a);

	// s = origin - v0
	__m256 sx = _mm256_sub_ps(ox, v0x), sy = _mm256_sub_ps(oy, v0y), sz = _mm256_sub_ps(oz, v0z);
	__m256 u = _mm256_mul_ps(f, dot3(sx, sy, sz, hx, hy, hz));
	mask = _mm256_and_ps(mask, _mm256_and_ps(_mm256_cmp_ps(u, zero, _CMP_GE_OQ), _mm256_cmp_ps(u, one, _CMP_LE_OQ)));

	// q = s x e1
	__m256 qx = _mm256_sub_ps(_mm256_mul_ps(sy, e1z), _mm256_mul_ps(sz, e1y));
	__m256 qy = _mm256_sub_ps(_mm256_mul_ps(sz, e1x), _mm256_mul_ps(sx, e1z));
	__m256 qz = _mm256_sub_ps(_mm256_mul_ps(sx, e1y), _mm256_mul_ps(sy, e1x));
	__m256 v = _mm256_mul_ps(f, dot3(dx, dy, dz, qx, qy, qz));
	mask = _mm256_and_ps(mask, _mm256_and_ps(_mm256_cmp_ps(v, zero, _CMP_GE_OQ), _mm256_cmp_ps(_mm256_add_ps(u, v), one, _CMP_LE_OQ)));

	__m256 t = _mm256_mul_ps(f, dot3(e2x, e2y, e2z, qx, qy, qz));
	mask = _mm256_and_ps(mask, _mm256_cmp_ps(t, eps, _CMP_GT_OQ));

	if (!includeInvisible)
	{
		mask = _mm256_and_ps(mask, _mm256_loadu_ps((const float*)&g_soa.visible[b]));
	}

	t_out = _mm256_blendv_ps(_mm256_set1_ps(FLT_MAX), t, mask);
	return mask;
}

// Closest hit across all triangles. Returns the triangle index, or -1 on miss.
static int closestHitSimd(const Ray& ray, bool includeInvisible, float& out_t)
{
	const __m256 ox = _mm256_set1_ps(ray.start.x), oy = _mm256_set1_ps(ray.start.y), oz = _mm256_set1_ps(ray.start.z);
	const __m256 dx = _mm256_set1_ps(ray.direction.x), dy = _mm256_set1_ps(ray.direction.y), dz = _mm256_set1_ps(ray.direction.z);
	const __m256i lane = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);

	__m256  best_t   = _mm256_set1_ps(FLT_MAX);
	__m256i best_idx = _mm256_set1_epi32(-1);

	for (size_t g = 0; g < g_soa.groups; ++g)
	{
		__m256 t;
		intersectGroup(g, includeInvisible, ox, oy, oz, dx, dy, dz, t);

		__m256 better = _mm256_cmp_ps(t, best_t, _CMP_LT_OQ);
		best_t = _mm256_blendv_ps(best_t, t, better);
		__m256i idx = _mm256_add_epi32(_mm256_set1_epi32((int)(g * 8)), lane);
		best_idx = _mm256_castps_si256(_mm256_blendv_ps(_mm256_castsi256_ps(best_idx), _mm256_castsi256_ps(idx), better));
	}

	float ts[8]; int is[8];
	_mm256_storeu_ps(ts, best_t);
	_mm256_storeu_si256((__m256i*)is, best_idx);

	float bt = FLT_MAX; int bi = -1;
	for (int l = 0; l < 8; ++l)
	{
		if (is[l] >= 0 && ts[l] < bt) { bt = ts[l]; bi = is[l]; }
	}

	out_t = bt;
	return bi;
}

// Any hit with eps < t < 1 (shadow rays only need existence, not the closest).
static bool anyHitSimd(const Ray& ray, bool includeInvisible)
{
	const __m256 ox = _mm256_set1_ps(ray.start.x), oy = _mm256_set1_ps(ray.start.y), oz = _mm256_set1_ps(ray.start.z);
	const __m256 dx = _mm256_set1_ps(ray.direction.x), dy = _mm256_set1_ps(ray.direction.y), dz = _mm256_set1_ps(ray.direction.z);
	const __m256 one = _mm256_set1_ps(1.0f);

	for (size_t g = 0; g < g_soa.groups; ++g)
	{
		__m256 t;
		__m256 mask = intersectGroup(g, includeInvisible, ox, oy, oz, dx, dy, dz, t);
		mask = _mm256_and_ps(mask, _mm256_cmp_ps(t, one, _CMP_LT_OQ));
		if (_mm256_movemask_ps(mask)) { return true; }
	}
	return false;
}
#else
static const bool kHaveSimd = false;
static int  closestHitSimd(const Ray&, bool, float&) { return -1; }
static bool anyHitSimd(const Ray&, bool) { return false; }
#endif

bool g_use_simd = kHaveSimd;

double getLighting(const Vector& loc, const Vector& normal, bool reflected = false)
{
	Ray ray = Ray(loc, Vector(0, 0, 0));
	
	double ret = 0;
	Vector temp;
	Vector normal_vec;
	
	for (auto i = lights.begin(); i != lights.end(); ++i)
	{
		ray.direction = *i - loc;
		normal_vec = ray.direction; normal_vec.normalise();
		ray.start = loc + normal_vec;

		bool hit = false;

		if (g_use_simd)
		{
			hit = anyHitSimd(ray, reflected);
		}
		else
		{
			float t;
			Colour colour;

			for (auto i = geometry.begin(); i != geometry.end(); ++i)
			{
				if (!i->invisible || reflected)
				{
					if (i->getIntersection(ray, temp, t, colour))
					{
						if (t < 1) {hit = true; break;}
					}
				}
			}
		}

		if (!hit) {ret += fabs(normal_vec.dotprod(normal));}
	}

	return ret;
}

bool getClosestIntersect(const Ray& ray, Colour& colour, int reflectcount = 0, double l_factor = 1)
{
	float closest_t = FLT_MAX;
	Vector loc;
	Vector new_loc;
	Vector normal;
	float t;
	Colour new_colour;
	bool ret = false;
	bool reflective = false;
	float reflectivity = 0;
	bool glows = false;
	
	if (g_use_simd)
	{
		int idx = closestHitSimd(ray, reflectcount > 0, t);
		if (idx >= 0)
		{
			const Triangle& tri = geometry[idx];
			closest_t = t;
			colour = tri.colour;
			loc = ray.start + ray.direction * t;
			normal = tri.getNormal();
			ret = true;
			reflective = tri.reflective;
			reflectivity = tri.reflectivity;
			glows = tri.glows;
		}
	}
	else
	{
		for (auto i = geometry.begin(); i != geometry.end(); ++i)
		{
			if (!i->invisible || reflectcount > 0)
			{
				if (i->getIntersection(ray, new_loc, t, new_colour))
				{
					if (t < closest_t)
					{
						closest_t = t;
						colour = new_colour;
						loc = new_loc;
						normal = i->getNormal();
						ret = true;
						reflective = i->reflective;
						reflectivity = i->reflectivity;
						glows = i->glows;
					}
				}
			}
		}
	}
	
	if (ret)
	{
		if (reflective && reflectcount < MAX_REFLECT && l_factor > MIN_LFACTOR)
		{
			double l = 1;
			if (!glows) {l = 0.2 + 0.8 * getLighting(loc, normal, reflectcount > 0);}

			l *= l_factor;
			colour *= l;
			
			Vector v = ray.direction; v.normalise();
			Ray reflect_ray = Ray(loc, v -  normal * 2 * v.dotprod(normal));
			reflect_ray.start += reflect_ray.direction;
			
			getClosestIntersect(reflect_ray, new_colour, reflectcount + 1, reflectivity * l_factor);
			colour *= 1 - reflectivity;
			colour += new_colour;
		}
		else
		{
			double l = 1;
			if (!glows) {l = 0.2 + 0.8 * getLighting(loc, normal, reflectcount > 0);}
			
			l *= l_factor;
			colour *= l;
		}
	}
	
	return ret;
}

void changeDisplay(SDL_Surface* screen, unsigned int width, unsigned int height)
{
	float dist = 3;
	Vector origin = Vector(0, -250, -240) + move;
	Ray ray = Ray(origin, Vector(0, 0, 0));
	Vector forward = Vector(0, 0, dist);
	Vector up = Vector(0, 6, 0);
	Vector right = Vector(8, 0, 0);
	
	forward.rotateAroundX(phi);
	up.rotateAroundX(phi);
	right.rotateAroundX(phi);
	
	forward.rotateAroundY(theta);
	up.rotateAroundY(theta);
	right.rotateAroundY(theta);
	
	if (SDL_MUSTLOCK(screen)) {SDL_LockSurface(screen);}
	uint32_t* pixels = (uint32_t*) screen->pixels;

	// Rendering each pixel is independent, so split the rows across threads.
	auto renderRows = [&](unsigned int y0, unsigned int y1)
	{
		Ray local_ray = ray;
		Colour colour;

		for (unsigned int i = y0; i < y1; ++i)
		{
			uint32_t* row = pixels + i * width;
			for (unsigned int j = 0; j < width; ++j)
			{
				local_ray.direction = right * (j / float(width - 1) - 0.5f) + up * (i / float(height - 1) - 0.5f) + forward;
				local_ray.direction.normalise();

				if (!getClosestIntersect(local_ray, colour)) {colour = 0x00000000u;}

				row[j] = colour.convertTo32();
			}
		}
	};

	unsigned int num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0) {num_threads = 1;}
	if (num_threads > height) {num_threads = height;}

	std::vector<std::thread> threads;
	threads.reserve(num_threads - 1);

	unsigned int rows_per = height / num_threads;
	unsigned int extra = height % num_threads;
	unsigned int y = 0;

	for (unsigned int t = 0; t < num_threads; ++t)
	{
		unsigned int rows = rows_per + (t < extra ? 1 : 0);
		unsigned int y0 = y;
		unsigned int y1 = y + rows;
		y = y1;

		// Run the final chunk on the calling thread to avoid an idle wait.
		if (t + 1 == num_threads) {renderRows(y0, y1);}
		else {threads.emplace_back(renderRows, y0, y1);}
	}

	for (auto& th : threads) {th.join();}

	if (SDL_MUSTLOCK(screen)) {SDL_UnlockSurface(screen);}
}

int main(int argc, char** argv)
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Surface* backbuf;

	bool selftest = false;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--scalar") == 0) {g_use_simd = false;}
		else if (strcmp(argv[i], "--selftest") == 0) {selftest = true;}
	}

    /* Initialize the SDL library */
    if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr,
                "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    unsigned int width = 1600;
	unsigned int height = 900;

	unsigned int backbuf_width = 640;
	unsigned int backbuf_height = 360;

	// Headless check that the SIMD path produces the same image as the scalar
	// reference. Renders one frame each way and reports the pixel difference.
	if (selftest)
	{
		createGeometry();
		buildGeometrySoA();

		SDL_Surface* sa = SDL_CreateRGBSurface(0, backbuf_width, backbuf_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0);
		SDL_Surface* sb = SDL_CreateRGBSurface(0, backbuf_width, backbuf_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0);

		g_use_simd = false; changeDisplay(sa, backbuf_width, backbuf_height);
		g_use_simd = true;  changeDisplay(sb, backbuf_width, backbuf_height);

		uint32_t* pa = (uint32_t*)sa->pixels;
		uint32_t* pb = (uint32_t*)sb->pixels;
		unsigned long long total = (unsigned long long)backbuf_width * backbuf_height;
		unsigned long long diff = 0;
		int max_chan = 0;
		for (unsigned long long i = 0; i < total; ++i)
		{
			if (pa[i] != pb[i])
			{
				++diff;
				for (int s = 8; s < 32; s += 8)
				{
					int d = int((pa[i] >> s) & 0xff) - int((pb[i] >> s) & 0xff);
					if (d < 0) {d = -d;}
					if (d > max_chan) {max_chan = d;}
				}
			}
		}
		printf("selftest: simd=%s, %llu/%llu pixels differ, max channel delta %d\n",
			kHaveSimd ? "available" : "unavailable", diff, total, max_chan);

		SDL_FreeSurface(sa);
		SDL_FreeSurface(sb);
		SDL_Quit();
		return 0;
	}

	window = SDL_CreateWindow("mirrorhouse",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, 0);
    if ( window == NULL ) {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        exit(1);
    }
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if ( renderer == NULL ) {
        fprintf(stderr, "Couldn't create renderer: %s\n", SDL_GetError());
        exit(1);
    }

	// The texture format must match the backbuf surface so the per-frame
	// upload is a straight copy with no conversion.
	Uint32 pixel_format = SDL_MasksToPixelFormatEnum(32, 0xff000000, 0x00ff0000, 0x0000ff00, 0);
	texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, backbuf_width, backbuf_height);
	backbuf = SDL_CreateRGBSurface(0, backbuf_width, backbuf_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0);
    if ( texture == NULL || backbuf == NULL ) {
        fprintf(stderr, "Couldn't create render texture: %s\n", SDL_GetError());
        exit(1);
    }

	SDL_ShowCursor(0);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
	createGeometry();
	buildGeometrySoA();

	bool quit = false;

	Uint64 perf_freq = SDL_GetPerformanceFrequency();
	Uint64 fps_timer = SDL_GetPerformanceCounter();
	unsigned int fps_frames = 0;

	while (!quit)
	{
		SDL_Event event;
		
		{
			while(SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE) {quit = true;}
					if (event.key.keysym.sym == SDLK_a) {--x_accel;}
					if (event.key.keysym.sym == SDLK_d) {++x_accel;}
					if (event.key.keysym.sym == SDLK_w) {++z_accel;}
					if (event.key.keysym.sym == SDLK_s)	{--z_accel;}
				}
				
				if (event.type == SDL_KEYUP)
				{
					if (event.key.keysym.sym == SDLK_a)	{++x_accel;}
					if (event.key.keysym.sym == SDLK_d)	{--x_accel;}
					if (event.key.keysym.sym == SDLK_w) {--z_accel;}
					if (event.key.keysym.sym == SDLK_s)	{++z_accel;}
				}
				
				if (event.type == SDL_QUIT) {quit = true;}
				
				if (event.type == SDL_MOUSEMOTION)
				{
					theta -= event.motion.xrel / 1000.0;
					phi -= event.motion.yrel / 1000.0;
					if (phi > M_PI / 2.0) {phi = M_PI / 2.0;}
					if (phi < -M_PI / 2.0) {phi = -M_PI / 2.0;}
				}
			}
		}
	
		updateGeometry();
		changeDisplay(backbuf, backbuf_width, backbuf_height);

		// Upload the low-res frame and let the GPU scale it to the window.
		SDL_UpdateTexture(texture, NULL, backbuf->pixels, backbuf->pitch);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		// Show the frame rate in the title bar, refreshed about twice a second.
		++fps_frames;
		Uint64 now = SDL_GetPerformanceCounter();
		double elapsed = double(now - fps_timer) / perf_freq;
		if (elapsed >= 0.5)
		{
			char title[64];
			snprintf(title, sizeof(title), "mirrorhouse - %.1f FPS", fps_frames / elapsed);
			SDL_SetWindowTitle(window, title);
			fps_frames = 0;
			fps_timer = now;
		}
	}

	SDL_FreeSurface(backbuf);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
