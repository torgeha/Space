// game.cpp
#include "stdafx.h"
#include "base.h"
#include "sys.h"
#include "core.h"

#define SAFESUB(ARG_BASE, ARG_CHUNK)          (ARG_BASE - ARG_CHUNK > 0        ? ARG_BASE - ARG_CHUNK : 0)
#define SAFEADD(ARG_BASE, ARG_CHUNK, ARG_MAX) (ARG_BASE + ARG_CHUNK <= ARG_MAX ? ARG_BASE + ARG_CHUNK : ARG_MAX)
#define MAX(ARG_A, ARG_B) ((ARG_A)>(ARG_B)?(ARG_A):(ARG_B))
#define MIN(ARG_A, ARG_B) ((ARG_A)<(ARG_B)?(ARG_A):(ARG_B))

#define SPRITE_SCALE 8.f
#define SHADOW_OFFSET 80.f
#define SHADOW_SCALE 0.9f

#define SHIP_W 250.f
#define SHIP_H 270.f
//#define VERTICAL_SHIP_VEL 20.f
//#define HORIZONTAL_SHIP_VEL 10.f
#define SPRITE_SCALE 8.f
#define MAINSHIP_ENTITY 0
#define MAINSHIP_RADIUS 100.f
#define ROCK_RADIUS 100.f
#define CRASH_VEL 20.f
#define ROCK_CRASH_ENERGY_LOSS 30.f
#define MINE_CRASH_ENERGY_LOSS 80.f
#define MAX_ENERGY 100.f
#define MAX_FUEL 100.f
#define ROCKET_SPEED 15.f
#define MIN_TIME_BETWEEN_ROCKETS 1.f
#define MIN_FUEL_FOR_HEAL MAX_FUEL / 2.f
#define FUEL_HEAL_PER_FRAME .2f
#define ENERGY_HEAL_PER_FRAME .1f
#define JUICE_FUEL 30.f

#define MAIN_SHIP g_entities[MAINSHIP_ENTITY]

#define ENERGY_BAR_W 60.f
#define ENERGY_BAR_H 1500.f
#define FUEL_BAR_W 60.f
#define FUEL_BAR_H 1500.f
#define CHUNK_W 40.f
#define CHUNK_H 40.f
#define MAX_CHUNKS 30

#define START_ROCK_CHANCE_PER_PIXEL 1.f/1000.f
#define EXTRA_ROCK_CHANCE_PER_PIXEL 0.f//1.f/2500000.f

#define JUICE_CHANCE_PER_PIXEL 1.f/2500000.f // Took it from rock, no mention in code, should download and have a look

#define SHIP_CRUISE_SPEED 25.f
#define SHIP_START_SPEED 5.f
#define SHIP_INC_SPEED .5f
#define HORIZONTAL_SHIP_VEL 10.f
#define SHIP_TILT_INC .2f
#define SHIP_TILT_FRICTION .1f
#define SHIP_MAX_TILT 1.5.f
#define SHIP_HVEL_FRICTION .1f
#define TILT_FUEL_COST .03f
#define FRAME_FUEL_COST .01f

#define RACE_END 100000.f

#define FPS 60.f
#define FRAMETIME (1.f/FPS)

#define STARTING_TIME 2.f
#define DYING_TIME 2.f
#define VICTORY_TIME = 8.f

#define LOG(ALL_ARGS) printf ALL_ARGS
//#define LOG(ALL_ARGS)

#define FIRST_CHALLANGE 3000.f

float g_current_race_pos = 0.f;
float g_camera_offset = 0.f;
float g_rock_chance = START_ROCK_CHANCE_PER_PIXEL;

//---------------------------------------------------------------------------
// Game state
enum GameState { GS_PLAYING, GS_DYING, GS_STARTING, GS_VICTORY };
GameState g_gs = GS_STARTING;
float g_gs_timer = 0.f;

//---------------------------------------------------------------------------
// Textures
int g_ship_LL, g_ship_L, g_ship_C, g_ship_R, g_ship_RR;
int g_bkg, g_pearl, g_energy, g_fuel, g_star, g_mine, g_drone;
int g_rock[5];

//---------------------------------------------------------------------------
// Entities
enum EType { E_NULL, E_MAIN, E_ROCK, E_STAR, E_MINE, E_DRONE, E_JUICE, E_ROCKET };

#define MAX_ENTITIES 64

struct Entity
{
	EType	type;
	vec2	pos;
	vec2	vel;
	float	radius;
	float	energy;
	float	fuel;
	float	tilt;
	float	gfxscale;
	int		gfx;
	bool	gfxadditive;
	rgba	color;
	bool	has_shadow;
};
Entity g_entities[MAX_ENTITIES];

//-----------------------------------------------------------------------------
void InsertEntity(EType type, vec2 pos, vec2 vel, float radius, int gfx, bool has_shadow, bool additive = false)
{
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		if (g_entities[i].type == E_NULL)
		{
			g_entities[i].type =		type;
			g_entities[i].pos =			pos;
			g_entities[i].vel =			vel;
			g_entities[i].radius =		radius;
			g_entities[i].gfx =			gfx;
			g_entities[i].energy =		MAX_ENERGY;
			g_entities[i].fuel =		MAX_FUEL;
			g_entities[i].tilt =		0.f;
			g_entities[i].gfxscale =	1.f;
			g_entities[i].gfxadditive = additive;
			g_entities[i].color =		vmake(1.f, 1.f, 1.f, 1.f);
			g_entities[i].has_shadow =	has_shadow;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void loadResources()
{
	// Resources
	g_ship_LL = CORE_LoadBmp("data/ShipLL.bmp", false);
	g_ship_L = CORE_LoadBmp("data/ShipL.bmp", false);
	g_ship_C = CORE_LoadBmp("data/ShipC.bmp", false);
	g_ship_R = CORE_LoadBmp("data/ShipR.bmp", false);
	g_ship_RR = CORE_LoadBmp("data/ShipRR.bmp", false);
	g_bkg = CORE_LoadBmp("data/bkg0.bmp", false);
	g_rock[0] = CORE_LoadBmp("data/Rock0.bmp", false);
	g_rock[1] = CORE_LoadBmp("data/Rock1.bmp", false);
	g_rock[2] = CORE_LoadBmp("data/Rock2.bmp", false);
	g_rock[3] = CORE_LoadBmp("data/Rock3.bmp", false);
	g_rock[4] = CORE_LoadBmp("data/Rock4.bmp", false);
	g_pearl = CORE_LoadBmp("data/Pearl.bmp", false);
	g_energy = CORE_LoadBmp("data/Energy.bmp", false);
	g_fuel = CORE_LoadBmp("data/Fuel.bmp", false);
	g_star = CORE_LoadBmp("data/Star.bmp", false);
}

//----------------------------------------------------------------------------
void UnloadResources()
{
	CORE_UnloadBmp(g_ship_LL);
	CORE_UnloadBmp(g_ship_L);
	CORE_UnloadBmp(g_ship_C);
	CORE_UnloadBmp(g_ship_R);
	CORE_UnloadBmp(g_ship_RR);
	CORE_UnloadBmp(g_bkg);
	CORE_UnloadBmp(g_rock[0]);
	CORE_UnloadBmp(g_rock[1]);
	CORE_UnloadBmp(g_rock[2]);
	CORE_UnloadBmp(g_rock[3]);
	CORE_UnloadBmp(g_rock[4]);
	CORE_UnloadBmp(g_pearl);
	CORE_UnloadBmp(g_energy);
	CORE_UnloadBmp(g_fuel);
	CORE_UnloadBmp(g_star);
}

//----------------------------------------------------------------------------------
// Level generation
float g_next_challange_area = FIRST_CHALLANGE;
vec2 g_last_conditioned = vmake(0.f, 0.f);
#define PATH_TWIST_RATIO .5f // This means about 30 degrees maximum
#define PATH_WIDTH (2.f * MAINSHIP_RADIUS)

void GenNextElements()
{
	// Called every game loop, but only does work when close to next challange area
	if (g_current_race_pos + 2 * G_HEIGHT > g_next_challange_area)
	{
		float current_y = g_next_challange_area;
		LOG(("Current: %f\n", g_next_challange_area));

		// Choose how many layers of rocks
		int nlayers = (int)CORE_URand(1, 20);
		LOG(("nlayers: %d\n", nlayers));
		for (int i = 0; i < nlayers; i++)
		{
			LOG(("Where: %f\n", current_y));

			// Choose pass point
			float displace = (current_y - g_last_conditioned.y) * PATH_TWIST_RATIO;
			float bracket_left = g_last_conditioned.x - displace;
			float bracket_right = g_last_conditioned.x + displace;
			bracket_left = MAX(bracket_left, 2.f * MAINSHIP_RADIUS);
			bracket_right = MIN(bracket_right, G_WIDTH - 2.f * MAINSHIP_RADIUS);
			g_last_conditioned.y = current_y;
			g_last_conditioned.x = CORE_FRand(bracket_left, bracket_right);

			// Choose how many rocks
			int nrocks = (int)CORE_URand(0, 3);
			LOG(("nrocks: %d\n", nrocks));

			// Generate rocks
			for (int i = 0; i < nrocks; i++)
			{
				// Find a valid position
				vec2 rockpos;
				for (;;)
				{
					rockpos = vmake(CORE_FRand(0.f, G_WIDTH), current_y);
					if (rockpos.x + ROCK_RADIUS < g_last_conditioned.x - PATH_WIDTH
						|| rockpos.x - ROCK_RADIUS > g_last_conditioned.x + PATH_WIDTH)
						break;
				}
				// Insert obstacle
				EType t = E_ROCK;
				int gfx = g_rock[1/*CORE_URand(0,4)*/];
				if (CORE_RandChance(0.1f)) { t = E_MINE; gfx = g_mine; } // Mine?
				else if (CORE_RandChance(0.1f)) { t = E_DRONE; gfx = g_drone; } // Drone?
				vec2 vel = vmake(CORE_FRand(-0.5f, 0.5f), CORE_FRand(-0.5f, 0.5f)); // velocity
				InsertEntity(t, rockpos, vel, ROCK_RADIUS, gfx, true); // Insert the chosen entity

			}
			current_y += CORE_FRand(300.f, 600.f);
		}
		g_next_challange_area = current_y + CORE_FRand(.5f * G_HEIGHT, 1.5f * G_HEIGHT);
	}
}

void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	// Render background, only tiles within camera view
	int first_tile = (int)floorf(g_camera_offset / G_HEIGHT);
	for (int i = first_tile; i < first_tile + 2; i++)
	{
		vec2 pos = vsub(vadd(vmake(G_WIDTH / 2.0f, G_HEIGHT / 2.0f), vmake(0.f, (float)i * G_HEIGHT)), vmake(0.f, g_camera_offset)); // split up, what does it?
		vec2 size = vmake(G_WIDTH, G_HEIGHT);
		CORE_RenderCenteredSprite(pos, size, g_bkg);
	}

	// Draw entities
	for (int i = MAX_ENTITIES - 1; i >= 0; i--)
	{
		if (g_entities[i].type != E_NULL)
		{
			ivec2 size = CORE_GetBmpSize(g_entities[i].gfx);
			vec2 pos = g_entities[i].pos;
			pos.x = (float)((int)pos.x);
			pos.y = (float)((int)pos.y);

			if (g_entities[i].has_shadow) // renders shadows first
			{
				vec2 s_pos = vadd(vsub(pos, vmake(0.f, g_camera_offset)), vmake(0.f, -SHADOW_OFFSET));
				vec2 s_size = vmake(size.x * SPRITE_SCALE * g_entities[i].gfxscale * SHADOW_SCALE, size.y * SPRITE_SCALE * g_entities[i].gfxscale * SHADOW_SCALE);
				int s_texture = g_entities[i].gfx;
				rgba s_color = vmake(0.f, 0.f, 0.f, 0.4f);
				bool s_additive = g_entities[i].gfxadditive;
				CORE_RenderCenteredSprite(s_pos, s_size, s_texture, s_color, s_additive);
			}

			// Render not shadows
			vec2 offset_pos = vsub(pos, vmake(0.f, g_camera_offset)); // Adjust to where camera is at
			vec2 scaled_size = vmake(size.x * SPRITE_SCALE * g_entities[i].gfxscale, size.y * SPRITE_SCALE * g_entities[i].gfxscale);
			int texture = g_entities[i].gfx;
			rgba color = g_entities[i].color;
			bool additive = g_entities[i].gfxadditive;
			CORE_RenderCenteredSprite(offset_pos, scaled_size, texture, color, additive);
		}
	}

	if (g_gs != GS_VICTORY) // if not yet won
	{
		// Draw UI
		// The energy bar
		float energy_ratio = MAIN_SHIP.energy / MAX_ENERGY;
		vec2 e_bar_pos = vmake(ENERGY_BAR_W / 2.f, energy_ratio * ENERGY_BAR_H / 2.f);
		vec2 e_bar_size = vmake(ENERGY_BAR_W, ENERGY_BAR_H * energy_ratio);
		CORE_RenderCenteredSprite(e_bar_pos, e_bar_size, g_energy, COLOR_WHITE, true);

		// The fuel bar
		float fuel_ratio = MAIN_SHIP.fuel / MAX_FUEL;
		vec2 f_bar_pos = vmake(G_WIDTH - FUEL_BAR_W / 2.f, fuel_ratio * FUEL_BAR_H / 2.f);
		vec2 f_bar_size = vmake(FUEL_BAR_W, FUEL_BAR_H * fuel_ratio);
		CORE_RenderCenteredSprite(f_bar_pos, f_bar_size, g_fuel, COLOR_WHITE, true);

		// Draw how long you have lasted
		int num_chunks = (int)((g_current_race_pos / RACE_END) * MAX_CHUNKS);
		for (int i = 0; i < num_chunks; i++)
		{
			vec2 c_pos = vmake(G_WIDTH - 100.f, 50.f + i * 50.f);
			vec2 c_size = vmake(CHUNK_W, CHUNK_H);
			CORE_RenderCenteredSprite(c_pos, c_size, g_pearl);
		}
	}
}

//----------------------------------------------------------------------------
void ResetNewGame()
{
	// Reset everything for a new game
	g_next_challange_area = FIRST_CHALLANGE;
	g_last_conditioned = vmake(0.5f * G_WIDTH, 0.f);
	g_current_race_pos = 0.f;
	g_camera_offset = 0.f;
	g_rock_chance = START_ROCK_CHANCE_PER_PIXEL;
	g_gs = GS_STARTING;
	g_gs_timer = 0.f;

	// Start logic
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		g_entities[i].type = E_NULL;
	}
	// Insert main ship
	InsertEntity(E_MAIN, vmake(G_WIDTH / 2.0, G_HEIGHT / 8.f), vmake(0.f, SHIP_START_SPEED), MAINSHIP_RADIUS, g_ship_C, true);
}

//-----------------------------------------------------------------------------
void RunGame()
{
	// Control main ship
	if (g_gs == GS_VICTORY || g_gs == GS_STARTING)
	{
		if (MAIN_SHIP.vel.y < SHIP_CRUISE_SPEED)
		{
			MAIN_SHIP.vel.y = SAFEADD(MAIN_SHIP.vel.y, SHIP_INC_SPEED, SHIP_CRUISE_SPEED);
		}
		MAIN_SHIP.fuel = SAFESUB(MAIN_SHIP.fuel, FRAME_FUEL_COST);
	}

	// Heal main ship
	if (g_gs != GS_DYING)
	{
		if (MAIN_SHIP.energy < MAX_ENERGY && MAIN_SHIP.fuel > MIN_FUEL_FOR_HEAL)
		{
			MAIN_SHIP.energy = SAFEADD(MAIN_SHIP.energy, ENERGY_HEAL_PER_FRAME, MAX_ENERGY);
			MAIN_SHIP.fuel = SAFESUB(MAIN_SHIP.fuel, FUEL_HEAL_PER_FRAME);
			LOG(("- energy: %f, fuel: %f\n", MAIN_SHIP.energy, MAIN_SHIP.fuel));
		}
	}

	// Move entities
	for (int i = MAX_ENTITIES - 1; i >= 0; i--)
	{
		if (g_entities[i].type != E_NULL)
		{
			g_entities[i].pos = vadd(g_entities[i].pos, g_entities[i].vel);

			// Remove entities that fell off screen
			if (g_entities[i].pos.y < g_camera_offset - G_HEIGHT)
				g_entities[i].type = E_NULL;
		}
	}

	// Advance "stars"
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		if (g_entities[i].type == E_STAR)
			g_entities[i].gfxscale *= 1.008f;
	}

	// Dont let steering off the screen
	if (MAIN_SHIP.pos.x < MAINSHIP_RADIUS)
		MAIN_SHIP.pos.x = MAINSHIP_RADIUS;
	if (MAIN_SHIP.pos.x > G_WIDTH - MAINSHIP_RADIUS)
		MAIN_SHIP.pos.x = G_WIDTH - MAINSHIP_RADIUS;

	// Check collisions
	if (g_gs == GS_PLAYING)
	{
		// Check everything against ship
		for (int i = 1; i < MAX_ENTITIES; i++)
		{
			// Should check against ship?
			if (g_entities[i].type == E_ROCK
				|| g_entities[i].type == E_JUICE
				|| g_entities[i].type == E_MINE
				|| g_entities[i].type == E_DRONE)
			{
				float distance = vlen2(vsub(g_entities[i].pos, MAIN_SHIP.pos)); // Distance from object to ship
				float crash_distance = CORE_FSquare(g_entities[i].radius + MAIN_SHIP.radius); // Minimum allowed distance before crash
				if (distance < crash_distance)
				{
					switch (g_entities[i].type)
					{
					case E_ROCK:
						if (g_entities[i].energy > 0)
						{
							MAIN_SHIP.energy = SAFESUB(MAIN_SHIP.energy, ROCK_CRASH_ENERGY_LOSS);
							MAIN_SHIP.vel.y = SHIP_START_SPEED;
							
							// Set rock velocity
							vec2 vel_direction = vsub(g_entities[i].pos, MAIN_SHIP.pos); // direction of rock velocity, away from ship
							vec2 normalized_vel_direction = vunit(vel_direction); // normalize
							vec2 vel = vscale(normalized_vel_direction, CRASH_VEL); // Scale, ie give the rock correct speed.
							g_entities[i].vel = vel;
							g_entities[i].energy = 0;
						}
						break;

					case E_JUICE:
						MAIN_SHIP.fuel = SAFEADD(MAIN_SHIP.fuel, JUICE_FUEL, MAX_FUEL);
						g_entities[i].type = E_NULL;
						break;

					case E_MINE:
						MAIN_SHIP.energy = SAFESUB(MAIN_SHIP.energy, MINE_CRASH_ENERGY_LOSS);
						MAIN_SHIP.vel.y = SHIP_START_SPEED;
						g_entities[i].type = E_NULL;
						break;

					case E_DRONE:
						MAIN_SHIP.energy = SAFESUB(MAIN_SHIP.energy, MINE_CRASH_ENERGY_LOSS);
						MAIN_SHIP.vel.y = SHIP_START_SPEED;
						g_entities[i].type = E_NULL;
						break;

					default:
						break;
					}
				}
			}
			else if (g_entities[i].type == E_ROCKET)
			{
				// Check all hit-able objects against this rocket
				for (int j = 1; i < MAX_ENTITIES; j++) 
				{
					// Should check against rocket?
					if (g_entities[j].type == E_ROCK
						|| g_entities[j].type == E_MINE
						|| g_entities[j].type == E_DRONE)
					{
						float distance = vlen2(vsub(g_entities[i].pos, g_entities[j].pos));
						float crash_distance = CORE_FSquare(g_entities[i].radius + g_entities[j].radius);
						if (distance < crash_distance)
						{
							// Impact!
							g_entities[i].type = E_NULL;
							g_entities[j].type = E_NULL;

							break;
						}
					}
				}
			}
		}
	}


	// INE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111

	// Possibly insert new juice
	if (g_gs == GS_PLAYING)
	{
		float trench = MAIN_SHIP.pos.y - g_current_race_pos; // How much advanced from previous frame
		if (CORE_RandChance(trench * JUICE_CHANCE_PER_PIXEL))
		{

			// FROM HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11

			vec2 pos = vmake(CORE_FRand(0.f, G_WIDTH), g_camera_offset + G_HEIGHT + ); // Random x, insert 400y above window
			vec2 vel = vmake(CORE_FRand(-1.f, +1.f), CORE_FRand(-1.f, +1.f)); // Random small velocity to make rocks "float"
			InsertEntity(E_ROCK, pos, vel, ROCK_RADIUS, g_rock[CORE_URand(0,4)], true);
		}
		// Advance difficulty in level
		g_rock_chance += (trench * EXTRA_ROCK_CHANCE_PER_PIXEL);
	}

	// Generate new level elements as we advance
	GenNextElements();

	// Set camera to follow the main ship
	g_camera_offset = MAIN_SHIP.pos.y - G_HEIGHT / 8.f;

	if (g_gs == GS_PLAYING)
	{
		g_current_race_pos = MAIN_SHIP.pos.y; // Advance in level
		if (g_current_race_pos >= RACE_END) // Check if victory
		{
			g_gs = GS_VICTORY;
			g_gs_timer = 0.f;
			MAIN_SHIP.gfxadditive = true;
		}
	}

	// Advance game mode
	g_gs_timer += FRAMETIME;
	switch (g_gs)
	{
	case GS_STARTING:
		if (g_gs_timer >= STARTING_TIME) // Start delay before starting to play
		{
			g_gs = GS_PLAYING;
			g_gs_timer = 0.f;
		}
		break;
	case GS_DYING:
		if (g_gs_timer >= DYING_TIME)
		{
			ResetNewGame();
		}
		break;
	case GS_PLAYING:
		if (MAIN_SHIP.energy <= 0.f || MAIN_SHIP.energy <= 0.f) // No energy or fuel --> die
		{
			g_gs = GS_DYING;
			g_gs_timer = 0.f;
		}
		break;
	case GS_VICTORY:
		if (CORE_RandChance(1.f / 10.f))
		{
			InsertEntity(E_STAR, MAIN_SHIP.pos, vadd(MAIN_SHIP.vel, vmake(CORE_FRand(-5.f, 5.f), CORE_FRand(-5.f, 5.f))), 0, g_star, false, true);
		}
		
		if (g_gs_timer >= 8.f) // Should use VICTORY_TIME, but stupid VS dont want me to...
		{
			ResetNewGame();
		}
		break;
	}
}

//-----------------------------------------------------------------------------
void ProcessInput()
{
/*	//old
	if (SYS_KeyPressed(SYS_KEY_LEFT))
		g_entities[MAINSHIP_ENTITY].vel.x = -HORIZONTAL_SHIP_VEL;
	else if (SYS_KeyPressed(SYS_KEY_RIGHT))
		g_entities[MAINSHIP_ENTITY].vel.x = HORIZONTAL_SHIP_VEL;
	else
		g_entities[MAINSHIP_ENTITY].vel.x = 0.f;*/

	bool left = SYS_KeyPressed(SYS_KEY_LEFT);
	bool right = SYS_KeyPressed(SYS_KEY_RIGHT);
	if (left && !right)
	{
		MAIN_SHIP.fuel = SAFESUB(MAIN_SHIP.fuel, TILT_FUEL_COST); // Steering consumes more fuel
		MAIN_SHIP.tilt -= SHIP_TILT_INC; // For sprite animation
	}
	if (right && !left)
	{
		MAIN_SHIP.fuel = SAFESUB(MAIN_SHIP.fuel, TILT_FUEL_COST);
		MAIN_SHIP.tilt += SHIP_TILT_INC;
	}
	if (!right && !left)
	{
		MAIN_SHIP.tilt *= (1.f - SHIP_TILT_FRICTION);
	}

	if (MAIN_SHIP.tilt <= -SHIP_MAX_TILT) MAIN_SHIP.tilt = -SHIP_MAX_TILT;
	if (MAIN_SHIP.tilt >= SHIP_MAX_TILT) MAIN_SHIP.tilt = SHIP_MAX_TILT;

	MAIN_SHIP.vel.x += MAIN_SHIP.tilt;
	MAIN_SHIP.vel.x *= (1.f - SHIP_HVEL_FRICTION);
	
	// Tilt animation
	float tilt = MAIN_SHIP.tilt;
	if (tilt < -0.6f * SHIP_MAX_TILT) MAIN_SHIP.gfx = g_ship_LL;
	else if (tilt < -0.2f * SHIP_MAX_TILT) MAIN_SHIP.gfx = g_ship_L;
	else if (tilt < 0.2f * SHIP_MAX_TILT) MAIN_SHIP.gfx = g_ship_C;
	else if (tilt < 0.6f * SHIP_MAX_TILT) MAIN_SHIP.gfx = g_ship_R;
	else MAIN_SHIP.gfx = g_ship_RR;
}

//-----------------------------------------------------------------------------
// Game state (apart from entities & other stand-alone modules)
float g_time = 0.f;

//-----------------------------------------------------------------------------
// Main
int Main(void)
{
	// Start things up & load resources ---------------------------------------------------
	loadResources();
	ResetNewGame();

	// Set up rendering ---------------------------------------------------------------------
	glViewport(0, 0, SYS_WIDTH, SYS_HEIGHT);
	glClearColor(0.0f, 0.1f, 0.3f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, G_WIDTH, 0.0, G_HEIGHT, 0.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Main game loop! ======================================================================
	while (!SYS_GottaQuit())
	{
		Render();
		SYS_Show();
		ProcessInput();
		RunGame();
		SYS_Pump();
		SYS_Sleep(16); // Cant be static if game should run equally on different hardware!
		g_time += FRAMETIME;
	}
	UnloadResources();

	return 0;
}