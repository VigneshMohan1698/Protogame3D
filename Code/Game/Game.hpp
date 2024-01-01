
#pragma once
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Player.hpp"
#include "Game/Props.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Texture;
class ConstantBuffer;
enum class GameStates
{
	EngineLogo = 0,
	Attract,
	GameWorld,
	GamePaused,
	PlayerDead,
	Victory
};
enum class GameSounds
{
	GamePause = 0,
	GameVictory,
	GameOver,
	TotalSounds
};

class Game
{
public:
	Game();
	~Game() {}
	//------------------------------MAIN FUNCTIONS------------------------------------------------------
	void				Startup();
	void				Update(float deltaSeconds);
	void				Render();
	void				ShutDown();
	void				DebugRender() const;

	//------------------------------MENU SCREEN FUNCTIONS-----------------------------------------------
	void				RenderAttractMode() const;
	void				UpdateAttractMode(float deltaSeconds);
	bool				GetIsMenuScreen();
	void				UpdateMenuScreenPlayButton(float deltaSeconds);
	void				RenderMenuScreenPlayButton() const;

	//------------------------------GAME FUNCTIONS-------------------------------------------------------
	void				UpdateGameState();
	void				SwitchGameModes(GameStates gameState);
	void				UpdateGame(float deltaSeconds);
	void				RenderGame() ;
	void				CreateWorldTextAndBasis();
	void				TestNameProperties();

	void				AddCubePropToGame();
	void				AddGridLinesToGame();
	void				AddSphereToGame();

	void				GetInputAndMovePlayer(float deltaSeconds);
	void				InitializeCameras();
	void				PlaySoundSfx(SoundPlaybackID soundID);
	void				UpdateEngineLogo(float deltaSeconds);
	void				RenderEngineLogo() const;
	void				LoadGameSounds();
	SoundPlaybackID		PlaySounds(std::string soundPath);
	void				StopSounds(SoundPlaybackID soundInstance);
	void				SpawnActor(EventArgs& args);
	void				SwitchToGame(float deltaSeconds);
	void				GetInputAndDrawDebugScreenObjects(float deltaSeconds);
	void EndFrame();
	void				DrawDebugScreenTexts(float deltaSeconds);
	void				PauseGame(bool pauseGame);
	Camera				m_worldCamera;
	Camera				m_screenCamera;
	ConstantBuffer*		m_effectCBO = nullptr;
	static	void		Test();
public:
	EffectsBuffer		m_effectMode;
	Vec3 topleft, bottomleft, topRight, bottomRight;
	bool				m_quadUpdateLocked = false;
	Player*				m_player;
	Props*				m_cubeProp;
	Props*				m_gridLines[200];
	Props*				m_grid;
	Props*				m_sphere;
	GameStates			m_currentGameState = GameStates::Attract;
	SoundID				m_gameSounds[(int)GameSounds::TotalSounds] = {};
	bool				m_PlayerVictory = false;
	bool				m_isGameOver = false;
	bool				m_isGamePaused = false;
	float				m_masterVolume = 1.0f;
	float				m_currentEngineLogoSeconds = 0.0f;
	float				m_engineLogoFireSeconds = 2.0f;
	float				m_engineLogoSeconds = 3.5f;
	Vec2				m_mouseSensitivity  = Vec2(0.1f, 0.1f);
	float				m_playerVelocity = 1.0f;
	Texture*			m_tempRTVTexture;
private:
	bool				m_IsAttractMode = false;
	float			    m_playButtonAlpha = 255.0f;
	Vec2				m_cursorPosition;			
	std::string			m_AttractScreenMusic;
	std::string			m_GameMusic;

	std::vector<Vertex_PNCU> actorVerts;
};