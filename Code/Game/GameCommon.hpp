#pragma once
struct Vec2;
struct Rgba8;

#define  UNUSED(x) void(x);
//#define  UNUSED(x) float(x);
	//Const expressions For Game
	static constexpr int NUM_STARTING_ASTEROIDS = 6;
	static constexpr int MAX_ASTEROIDS = 20;
	static constexpr int MAX_BULLETS = 50;
	static constexpr float WORLD_SIZE_X = 200.f;
	static constexpr float WORLD_SIZE_Y = 100.f;
	static constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
	static constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
	static constexpr float HEALTH_POINT_X =5.f;
	static constexpr float HEALTH_POINT_Y = 100.f;
	static constexpr float ASTEROID_SPEED = 20.f;
	static constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
	static constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
	static constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
	static constexpr float BULLET_SPEED = 80.f;
	static constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
	static constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
	static constexpr float BEETLE_SPEED = 24.f;
	static constexpr float BEETLE_PHYSICS_RADIUS = 1.2f;
	static constexpr float BEETLE_COSMETIC_RADIUS = 1.8f;
	static constexpr int BEETLE_HEALTH = 3;
	static constexpr float DEBRIS_PHYSICS_RADIUS = 0.5f;
	static constexpr float DEBRIS_COSMETIC_RADIUS = 1.0f;
	static constexpr int WASP_HEALTH = 5;
	static constexpr float WASP_SPEED = 10.f;
	static constexpr float WASP_ACCELERATION = 300.0f;
	static constexpr float WASP_PHYSICS_RADIUS = 1.0f;
	static constexpr float WASP_COSMETIC_RADIUS = 1.5f;
	static constexpr float PLAYER_SHIP_ACCELERATION = 50.f;
	static constexpr float PLAYER_SHIP_HEALTH = 3.0f;
	static constexpr float PLAYER_SHIP_TURN_SPEED = 500.f;
	static constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
	static constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
	
	static constexpr int   PLAYER_SHIP_VERTS = 18;
	static constexpr int   ASTEROID_VERTS = 48;
	static constexpr int   BULLET_VERTS = 6;
	static constexpr int   BEETLE_VERTS = 6;
	static constexpr int   WASPS_VERTS = 6;


	enum class GameEntities
	{
		PlayerShip = 0,
		Asteroid,
		Bullet,
		Beetle,
		Wasp,
		Debris
	};

	//CAMERA OPTIONS
	static constexpr float CAMERA_WORLD_SIZEX = 1.0f;
	static constexpr float CAMERA_WORLD_SIZEY = 1.0f;
	static constexpr float CAMERA_SCREEN_SIZEX = 1600.0f;
	static constexpr float CAMERA_SCREEN_SIZEY = 800.0f;
	//Functions
	void DrawDebugLine(Vec2 start, Vec2 end, Rgba8 color, float thickness);
	void DrawDebugDisk(Vec2 center, float radius, Rgba8 color, float thickness);
	void DrawEllipse(Vec2 center, float radius, Rgba8 color, float thickness);

	


