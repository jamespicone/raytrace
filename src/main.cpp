#include "SDL.h"
#include <stdio.h>

#include "triangle.h"
#include "vector.h"
#include "colour.h"

#include <cfloat>
#include <cmath>
#include <thread>
#include <vector>

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
