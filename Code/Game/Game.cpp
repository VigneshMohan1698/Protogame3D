#include "Game.hpp"
#include "stdlib.h"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Engine/Math/VertexUtils.hpp>
#include "Engine/Input/InputSystem.hpp"
#include <vector>
#include <Engine/Renderer/SimpleTriangleFont.hpp>
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include <Engine/Core/NamedProperties.hpp>

Vertex_PCU vertices[3] =
{
Vertex_PCU(Vec3(600.0f, 400.0f, 0.0f), Rgba8(255, 255, 255, 255), Vec2(0.0f, 0.0f)),
Vertex_PCU(Vec3(800.0f,  600.0f, 0.0f), Rgba8(255, 255, 255, 255), Vec2(0.0f, 0.0f)),
Vertex_PCU(Vec3(1000.0f, 400.0f, 0.0f), Rgba8(255, 255, 255, 255), Vec2(0.0f, 0.0f)),
};

extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInputSystem;
extern AudioSystem* g_theAudioSystem; 
extern EventSystem* g_theEventSystem;
extern Window* g_theWindow;
RandomNumberGenerator rng;


Game::Game()
{
}

void Game::Startup()
{
	InitializeCameras();
	LoadGameSounds();
	m_IsAttractMode = true;
	m_currentGameState = GameStates::EngineLogo;
	g_theRenderer->SetBlendMode(BlendMode::Opaque);
	m_worldCamera.SetOrthoView();
	//m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.0f, 0.1f, 100.0f);
	m_player = new Player(this, Vec3(0.0f,0.0f,0.0f), EulerAngles(0.1f, 0.1f, 0.1f));
	AddCubePropToGame();
	AddGridLinesToGame();
	AddSphereToGame();
	g_theInputSystem->SetMouseMode(false, false, false);
	TextureCreateInfo info;
	info.name = "TempBackBuffer";
	info.memory  = eMemoryHint::GPU;
	info.format  = eTextureFormat::R8G8B8A8_UNORM;
	info.dimensions = g_theRenderer->GetRenderConfig().m_window->GetClientDimensions();
	info.bindFlags = TEXTURE_BIND_RENDER_TARGET_BIT ;
	m_tempRTVTexture = g_theRenderer->CreateTextureFromInfo(info);

	m_effectMode.effects = Vec4();
	m_effectCBO = new ConstantBuffer(sizeof(EffectsBuffer));
	g_theRenderer->CreateConstantBuffer(sizeof(EffectsBuffer), m_effectCBO);

	g_theEventSystem->SubscribeEventCallbackObjectMethod("Test", *g_theApp, &App::Event_Test);
	g_theEventSystem->SubscribeEventCallbackObjectMethod("SpawnActor", *this, &Game::SpawnActor);
}


void Game::Update(float deltaSeconds)
{
	bool windowHasFocus = g_theRenderer->GetRenderConfig().m_window->HasFocus();
	if (!windowHasFocus)
	{
		g_theInputSystem->SetMouseMode(false, false, false);
	}
	else if (windowHasFocus)
	{
		if (g_theConsole->GetMode() == DevConsoleMode::VISIBLE)
		{
			g_theInputSystem->SetMouseMode(false, false, false);
		}
		else
		{
			if (m_IsAttractMode)
			{
				g_theInputSystem->SetMouseMode(false, false, false);
			}
			else
			{
				g_theInputSystem->SetMouseMode(true, true, true);
			}
		}
	}
	switch (m_currentGameState)
	{
	case GameStates::EngineLogo:		 UpdateEngineLogo(deltaSeconds);		break;
	case GameStates::Attract:			 UpdateAttractMode(deltaSeconds);		break;
	case GameStates::GameWorld:			 UpdateGame(deltaSeconds);				break;
	default:																	break;
	}


}
void Game::Render() 
{
	Rgba8 clearScreen = Rgba8(0, 0, 0, 255);
	g_theRenderer->ClearScreen(clearScreen);
	switch (m_currentGameState)
	{
	case GameStates::EngineLogo:		 RenderEngineLogo();							break;
	case GameStates::Attract:			 RenderAttractMode();							break;
	case GameStates::GameWorld:			 RenderGame();									break;
	default:																			break;
	}
}

void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindShader(g_theRenderer->GetDefaultShader());
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	//Camera* camPtr = (Camera*) &m_screenCamera;
	//camPtr->SetColorTarget(nullptr);
	RenderMenuScreenPlayButton();
	std::vector<Vertex_PCU> gameNameText;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Images/SquirrelFixedFont");
	font->AddVertsForText2D(gameNameText, Vec2(600, 600), 75.0f, "PROTOGAME3D", Rgba8::CYAN, 0.6f, 50.0f);
	font->AddVertsForTextInBox2D(gameNameText, m_screenCamera.GetCameraBounds(), 15.0f, "HIT SPACE TO PLAY", Rgba8(255, 0, 0, 255),
		0.6f, Vec2(0.5f,0.5f), TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(int(gameNameText.size()), gameNameText.data());
	gameNameText.clear();
	font = nullptr;
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->EndCamera(m_screenCamera);
}
void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED((void) deltaSeconds);
	//UpdateMenuScreenPlayButton(deltaSeconds);
	//---------------------------CHECKING FOR PLAYER INPUT AND SWITCHING TO GAME SCREEN-----------------------------------------------

}

void Game::RenderGame() 
{
	g_theRenderer->BeginCamera(m_worldCamera);
	Shader* shader = g_theRenderer->GetDefaultShader();
	g_theRenderer->BindShader(shader);
	g_theRenderer->SetModalMatrix(Mat44());
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	if (m_cubeProp)
	{
		m_cubeProp->Render();
	}
	Texture* sphereTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	for (int i = 0; i < 200; i++)
	{
		m_gridLines[i]->Render();
	}
	g_theRenderer->BindTexture(sphereTexture);
	if (m_sphere)
	{
		m_sphere->Render();
	}
	
	g_theRenderer->EndCamera(m_worldCamera);
	Shader* effectShader = g_theRenderer->CreateOrGetShader("Data/Shaders/Effect");
	g_theRenderer->CopyCPUToGPU(&m_effectCBO, sizeof(EffectsBuffer), m_effectCBO);
 	g_theRenderer->ApplySpecialEffects(effectShader, m_effectCBO, m_effectMode);
}
void Game::UpdateGame(float deltaSeconds)
{
	m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.0f, 0.1f, 100.0f);

	Vec3 ibasis, jbasis, kbasis;
	if (!m_quadUpdateLocked)
	{
		m_worldCamera.GetCameraOrientation().GetAsVectors_XFwd_YLeft_ZUp(ibasis, jbasis, kbasis);
		AABB2 actorQuad;
		actorQuad.m_mins.x =-2.4f;
		actorQuad.m_maxs.x = 2.4f;
		actorQuad.m_mins.y = -1.2f;
		actorQuad.m_maxs.y = 1.2f;

		topleft = m_worldCamera.m_position + (jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_mins.y) + ibasis * 2.0f;
		bottomleft = m_worldCamera.m_position + (jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_maxs.y) + ibasis * 2.0f;
		topRight = m_worldCamera.m_position + (-jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_mins.y) + ibasis * 2.0f;
		bottomRight = m_worldCamera.m_position + (-jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_maxs.y) + ibasis * 2.0f;
		AddVertsForRoundedQuad3D(actorVerts, topleft, bottomleft, bottomRight, topRight, Rgba8::WHITE, AABB2::ZERO_TO_ONE);
	}
	if (g_theInputSystem->WasKeyJustPressed('T'))
	{
		std::string filePath = "Data/Definitions/SimpleCommands.xml";
		g_theConsole->ExecuteXmlCommandScriptFile(filePath);
		g_theEventSystem->FireEvent("Test");
	}
	GetInputAndMovePlayer(deltaSeconds);
	GetInputAndDrawDebugScreenObjects(deltaSeconds);
	DrawDebugScreenTexts(deltaSeconds);
	if (m_sphere)
	{
		m_sphere->Update(deltaSeconds);
	}
	if (m_cubeProp)
	{
		m_cubeProp->Update(deltaSeconds);
	}

}
void Game::GetInputAndDrawDebugScreenObjects(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	Mat44 playerMatrix = m_player->GetModalMatrix();
	if (g_theInputSystem->WasKeyJustPressed('1'))
	{
		m_effectMode.effects.x == 0 ? m_effectMode.effects.x = 1 : m_effectMode.effects.x = 0;
	}
	if (g_theInputSystem->WasKeyJustPressed('2'))
	{
		m_effectMode.effects.y == 0 ? m_effectMode.effects.y = 1 : m_effectMode.effects.y = 0;
	}
	if (g_theInputSystem->WasKeyJustPressed('3'))
	{
		m_effectMode.effects.z == 0 ? m_effectMode.effects.z = 1 : m_effectMode.effects.z = 0;
	}
	if (g_theInputSystem->WasKeyJustPressed('4'))
	{
		//Spawn Billboard
		Vec3 textPosition = m_player->m_position + playerMatrix.GetIBasis3D() * 1.0f;
		EulerAngles playerOrientation = m_player->m_orientationDegrees;
		Vec3 playerPosition = m_player->m_position;
		std::string playerText = Stringf("Position:%.2f,%.2f,%.2f Orientation:%.2f,%.2f,%.2f", playerPosition.x, playerPosition.y, playerPosition.z,playerOrientation.m_yawDegrees, playerOrientation.m_pitchDegrees, playerOrientation.m_rollDegrees);
		DebugAddWorldBillboardText(playerText, textPosition, 0.2f, Vec2(), -1.0f, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USEDEPTH);
	}
	if (g_theInputSystem->WasKeyJustPressed('5'))
	{
		//Spawn Cylinder
		Vec3 position = m_player->m_position;
		Vec3 top = Vec3(position.x, position.y, position.z+0.5f);
		Vec3 base = Vec3(position.x, position.y, position.z);
		DebugAddWorldWireCylinder(base, top, position, 0.25f, 10.0f, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USEDEPTH);
	}
	if (g_theInputSystem->WasKeyJustPressed('6'))
	{
		//Add Screen message with camera orientation
		EulerAngles cameraOrientation = m_player->m_orientationDegrees;
		std::string cameraOrientationText = Stringf("Camera Orientation: %.2f, %.2f,%.2f", cameraOrientation.m_yawDegrees, cameraOrientation.m_pitchDegrees, cameraOrientation.m_rollDegrees);
		DebugAddMessage(cameraOrientationText, 5.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInputSystem->WasKeyJustPressed('9'))
	{
		float timeDilationValue = (float)DebugRenderGetClock()->GetTimeDilation() - 0.1f;
		DebugRenderClockSetTimeDilation(timeDilationValue);
	}
	if (g_theInputSystem->WasKeyJustPressed('0'))
	{
		float timeDilationValue = (float)DebugRenderGetClock()->GetTimeDilation() + 0.1f;
		DebugRenderClockSetTimeDilation(timeDilationValue);
	}
}
void Game::EndFrame()
{
	if ((g_theInputSystem->WasKeyJustPressed(' ') ||
		g_theInputSystem->WasKeyJustPressed('N') ||
		g_theInputSystem->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_A) ||
		g_theInputSystem->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START)) && m_currentGameState == GameStates::Attract)
	{
		SwitchToGame(0.01f);
	}
	if (g_theInputSystem->WasKeyJustPressed('F') )
	{
		m_quadUpdateLocked = !m_quadUpdateLocked;
	}
	if (!m_quadUpdateLocked)
	{
		actorVerts.clear();
	}

}
void Game::DrawDebugScreenTexts(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	float scaleDebugClock = (float)DebugRenderGetClock()->GetTimeDilation();
	float scaleGameClock = (float)g_theApp->m_gameClock.GetTimeDilation();
	std::string playerPositionText = Stringf("Player Position: %.2f, %.2f,%.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z);
	DebugAddScreenText(playerPositionText, Vec2(), 2.0f, Vec2(0.0f, 1.0f), 1.0f, Rgba8::WHITE, Rgba8::RED);
	std::string devConsoleClockText = Stringf("Dev Console    | Time: %.2f  Fps: %.2f  Scale: %.2f", g_theConsole->m_clock.GetTotalTime(), 1.0f / g_theConsole->m_clock.GetDeltaTime(), 1.0f);
	DebugAddScreenText(devConsoleClockText, Vec2(), -1.0f, Vec2(1.0f, 1.0f), 1.0f, Rgba8::WHITE, Rgba8::RED);
	std::string debugClockText = Stringf("Debug Render    | Time: %.2f  Fps: %.2f  Scale: %.2f",DebugRenderGetClock()->GetTotalTime(), 1.0f / DebugRenderGetClock()->GetDeltaTime(), scaleDebugClock);
	DebugAddScreenText(debugClockText, Vec2(), -1.0f, Vec2(1.0f, 0.98f), 1.0f, Rgba8::WHITE, Rgba8::RED);
	std::string gameClockText = Stringf("Game    | Time: %.2f  Fps: %.2f  Scale: %.2f", g_theApp->m_gameClock.GetTotalTime(), 1.0f / g_theApp->m_gameClock.GetDeltaTime(), scaleGameClock);
	DebugAddScreenText(gameClockText, Vec2(), -1.0f, Vec2(1.0f, 0.96f), 1.0f, Rgba8::WHITE, Rgba8::RED);
}
void Game::PauseGame(bool pauseGame)
{
	m_isGamePaused = pauseGame;
	m_masterVolume = 0.0f;
	if (!(m_currentGameState == GameStates::Attract) && !(m_currentGameState == GameStates::EngineLogo))
	{
		if (m_isGamePaused)
		{
		}
		else
		{
			m_masterVolume = 1.0f;
		}
	}
}
void Game::DebugRender() const
{

}
void Game::ShutDown()
{
	/*m_screenCamera.DestroyTextures();
	m_worldCamera.DestroyTextures();*/
	g_theRenderer->DestroyTexture(m_tempRTVTexture);
	g_theRenderer->ReleaseConstantBuffer(m_effectCBO);
}

void Game::AddCubePropToGame()
{
	if (m_cubeProp)
	{
		delete m_cubeProp;
	}
	std::vector<Vertex_PCU> verts;
	Vec3 BottomLeft;
	Vec3 BottomRight;
	Vec3 TopLeft;
	Vec3 TopRight;
	BottomLeft = Vec3(-0.5f, 0.5f, -0.5f);
	BottomRight = Vec3(-0.5f, -0.5f, -0.5f);
	TopLeft = Vec3(-0.5f, .5f, .5f);
	TopRight = Vec3(-0.5f, -0.5f, 0.5f);
	AddVertsForQuad3D(verts, TopLeft, BottomLeft, BottomRight, TopRight, Rgba8::CYAN, AABB2::ZERO_TO_ONE);

	BottomLeft = Vec3(0.5f, -0.5f, -0.5f);
	BottomRight = Vec3(0.5f, 0.5f, -0.5f);
	TopLeft = Vec3(0.5f, -.5f, .5f);
	TopRight = Vec3(0.5f, 0.5f, 0.5f);
	AddVertsForQuad3D(verts, TopLeft, BottomLeft, BottomRight, TopRight, Rgba8::RED, AABB2::ZERO_TO_ONE);

	BottomLeft = Vec3(-0.5f, -0.5f, -0.5f);
	BottomRight = Vec3(0.5f, -0.5f, -0.5f);
	TopLeft = Vec3(-0.5f, -.5f, .5f);
	TopRight = Vec3(0.5f, -0.5f, 0.5f);
	AddVertsForQuad3D(verts, TopLeft, BottomLeft, BottomRight, TopRight, Rgba8::MAGENTA, AABB2::ZERO_TO_ONE);

	BottomLeft = Vec3(0.5f, 0.5f, -0.5f);
	BottomRight = Vec3(-0.5f, 0.5f, -0.5f);
	TopLeft = Vec3(0.5f, .5f, .5f);
	TopRight = Vec3(-0.5f, 0.5f, 0.5f);
	AddVertsForQuad3D(verts, TopLeft, BottomLeft, BottomRight, TopRight, Rgba8::GREEN, AABB2::ZERO_TO_ONE);

	BottomLeft = Vec3(0.5f, -0.5f, 0.5f);
	BottomRight = Vec3(0.5f, 0.5f, 0.5f);
	TopLeft = Vec3(-0.5f, -.5f, .5f);
	TopRight = Vec3(-0.5f, 0.5f, 0.5f);
	AddVertsForQuad3D(verts, TopLeft, BottomLeft, BottomRight, TopRight, Rgba8::BLUE, AABB2::ZERO_TO_ONE);

	BottomLeft = Vec3(0.5f, 0.5f, -0.5f);
	BottomRight = Vec3(0.5f, -0.5f, -0.5f);
	TopLeft = Vec3(-0.5f, .5f, -.5f);
	TopRight = Vec3(-0.5f, -0.5f, -0.5f);
	AddVertsForQuad3D(verts, TopLeft, BottomLeft, BottomRight, TopRight, Rgba8::YELLOW, AABB2::ZERO_TO_ONE);

	m_cubeProp = new Props(this, Vec3(-2.0f, 2.0f, 0.0f), EulerAngles(30.0f, 0.0f, 0.0f), verts);
	m_cubeProp->m_colorChange = 5.0f;
	verts.clear();
}
void Game::AddGridLinesToGame()
{
	int m_position = -50;
	std::vector<Vertex_PCU> redverts;
	std::vector<Vertex_PCU> greenverts;
	for (int i = 0; i < 100; i++)
	{
		float thickness = 0.03f;
		Rgba8 color;
		AABB3 redLines = AABB3(Vec3(-50.0f, 0.0f,0.0f), Vec3(49.0f, thickness, thickness));
		color = Rgba8(150,150,150,255);
		if (m_position % 5 == 0.0f)
		{
			thickness = 0.06f;
			color = Rgba8::RED;
		}
		if (m_position == 0)
		{
			color = Rgba8::WHITE;
		}
		AddVertsForAABB3D(redverts, redLines, color, AABB2::ZERO_TO_ONE);
		m_gridLines[i] = new Props(this, Vec3(0.0f, (float)m_position, 0.0f), EulerAngles(0.0f, 0.0f, 0.0f), redverts);
		redverts.clear();
		m_position++;
	}
	m_position = -50;
	for (int i = 100; i < 200; i++)
	{
		float thickness = 0.03f;
		Rgba8 color;

		AABB3 greenLines = AABB3(Vec3(0.0f, -50.f, 0.0f), Vec3(thickness, 49.0f, thickness));
		color = Rgba8(150, 150, 150, 255);
		if (m_position % 5 == 0.0f)
		{
			thickness = 0.06f;
			color = Rgba8::GREEN;
		}
		if (m_position == 0)
		{
			color = Rgba8::WHITE;
		}
		AddVertsForAABB3D(greenverts, greenLines, color, AABB2::ZERO_TO_ONE);
		m_gridLines[i] = new Props(this, Vec3((float)m_position, 0.0f, 0.0f), EulerAngles(0.0f, 0.0f, 0.0f), greenverts);
		greenverts.clear();
		m_position++;
	}
	
}
void Game::AddSphereToGame()
{
	if (m_sphere)
	{
		delete m_sphere;
	}
	std::vector<Vertex_PCU> verts;
	AddVertsForSphere3D(verts, 1.0f, m_sphere->m_position, AABB2::ZERO_TO_ONE, Rgba8::WHITE);
	m_sphere = new Props(this, Vec3(10.0f,-5.0,1.0f), EulerAngles(30.0f,30.0f, 0.0f), verts);
	verts.clear();
}
void Game::GetInputAndMovePlayer(float deltaSeconds)
{
	XBoxController const& controller = g_theInputSystem->GetController(0);
	if (g_theInputSystem->IsKeyDown(16) || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
	{
		m_playerVelocity = 10.0f;
	}
	else
	{
		m_playerVelocity = 1.0f;
	}
	if (g_theInputSystem->IsKeyDown('H') || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_START))
	{
		m_player->m_position = Vec3(0.0f, 0.0f, 0.0f);
		m_player->m_orientationDegrees = EulerAngles(0.0f, 0.0f, 0.0f);
	}
	Mat44 playerMatrix = m_player->GetModalMatrix();
	if (g_theInputSystem->IsKeyDown('W'))
	{
		m_player->m_position += playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('S'))
	{
		m_player->m_position -= playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('D'))
	{
		m_player->m_position -= playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('A'))
	{
		m_player->m_position += playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('Z'))
	{
		m_player->m_position += playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('C'))
	{
		m_player->m_position -= playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
	}
	Vec2 mouseDelta = g_theInputSystem->GetMouseClientDelta();
	m_player->m_orientationDegrees.m_yawDegrees += (mouseDelta.x * m_mouseSensitivity.x);
	m_player->m_orientationDegrees.m_pitchDegrees -= (mouseDelta.y * m_mouseSensitivity.y);

	if (g_theInputSystem->IsKeyDown('Q'))
	{
		m_player->m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 6.0f);
	}
	if (g_theInputSystem->IsKeyDown('E'))
	{
		m_player->m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 6.0f);
	}

	
	if (controller.IsConnected())
	{
		float leftStickOrientation;
		float rightStickOrientation;
		Vec2 leftStick;
		Vec2 rightStick;
		if (controller.GetLeftStick().GetMagnitude() > 0.0f)
		{
			leftStickOrientation = controller.GetLeftStick().GetOrientationDegrees();
			leftStick = Vec2::MakeFromPolarDegrees(leftStickOrientation);
		}
		if (controller.GetRightStick().GetMagnitude() > 0.0f)
		{
			rightStickOrientation = controller.GetRightStick().GetOrientationDegrees();
			rightStick = Vec2::MakeFromPolarDegrees(rightStickOrientation);
		}

		if (leftStick.y > 0.2f)
		{
			m_player->m_position += playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (leftStick.y <-0.2f)
		{
			m_player->m_position -= playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (leftStick.x > 0.2f)
		{
			m_player->m_position -= playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (leftStick.x < -0.2f)
		{
			m_player->m_position += playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
		}

		if (rightStick.y > 0.2f)
		{
			m_player->m_orientationDegrees.m_pitchDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
		}
		if (rightStick.y < -0.2f)
		{
			m_player->m_orientationDegrees.m_pitchDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
		}
		if (rightStick.x > 0.2f)
		{
			m_player->m_orientationDegrees.m_yawDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
		}
		if (rightStick.x < -0.2f)
		{
			m_player->m_orientationDegrees.m_yawDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
		}
		if (g_theInputSystem->GetController(0).GetLeftTrigger() > 0.15f)
		{
			m_player->m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 10.0f);
		}
		if (g_theInputSystem->GetController(0).GetRightTrigger() > 0.15f)
		{
			m_player->m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 10.0f);
		}
		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER))
		{
			m_player->m_position -= playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_LEFTSHOULDER))
		{
			m_player->m_position += playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
		}
	}

	m_player->Update(deltaSeconds);
}

void Game::InitializeCameras()
{
	m_worldCamera = Camera(-1.0f, -1.0f, CAMERA_WORLD_SIZEX, CAMERA_WORLD_SIZEY);
	m_screenCamera = Camera(0.0f, 0.0f, CAMERA_SCREEN_SIZEX, CAMERA_SCREEN_SIZEY);

	Vec3 ibasis = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 jbasis = Vec3(-1.0f, 0.0f, 0.0f);
	Vec3 kbasis = Vec3(0.0f, 1.0f, 0.0f);
	m_worldCamera.SetViewToRenderTransform(ibasis,jbasis,kbasis);
}
void Game::LoadGameSounds()
{
	m_gameSounds[(int)GameSounds::GamePause] = g_theAudioSystem->CreateOrGetSound("Data/Audio/Pause.mp3");
	m_gameSounds[(int)GameSounds::GameVictory] = g_theAudioSystem->CreateOrGetSound("Data/Audio/Victory.mp3");
}
SoundPlaybackID Game::PlaySounds(std::string soundPath)
{
	SoundID NewLevel = g_theAudioSystem->CreateOrGetSound("Data/Audio/NewLevel.mp3");
	return g_theAudioSystem->StartSound(NewLevel);
}
void Game::StopSounds(SoundPlaybackID soundInstance)
{
	g_theAudioSystem->StopSound(soundInstance);
}

void Game::SpawnActor(EventArgs& args)
{
	std::string defaultValue = "";

	if (args.GetValue("type", defaultValue) == "Cube")
	{
		AddCubePropToGame();
		std::string  position = args.GetValue("pos", defaultValue);
		Vec3 value;
		value.SetFromText(position.c_str());
		m_cubeProp->m_position = value;
	}
	
	else if (args.GetValue("type", defaultValue) == "Sphere")
	{
		AddSphereToGame();
		std::string  position = args.GetValue("pos", defaultValue);
		Vec3 value;
		value.SetFromText(position.c_str());
		m_sphere->m_position = value;
	}

}

void Game::RenderEngineLogo() const
{
	Texture* logoFire;
	Texture* logo;

	AABB2 engineLogoBounds;
	engineLogoBounds.m_mins = m_screenCamera.GetCameraBounds().m_mins + Vec2(400.0f, 200.0f);
	engineLogoBounds.m_maxs = m_screenCamera.GetCameraBounds().m_maxs + Vec2(-400.0f, -200.0f);

	std::vector<Vertex_PCU> screenVerts;
	float engineLogoAlpha = RangeMap(m_engineLogoSeconds - m_currentEngineLogoSeconds, 0.0f, m_engineLogoSeconds, 20.0f, 254.0f);
	int engineLogoAlphaInt = (int)engineLogoAlpha;
	AddVertsForAABB2D(screenVerts, engineLogoBounds, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), Rgba8(255, 255, 255, (unsigned char)engineLogoAlphaInt));

	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindShader(g_theRenderer->GetDefaultShader());
	logo = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EngineLogo.png");
	g_theRenderer->BindTexture(logo);
	g_theRenderer->DrawVertexArray((int)screenVerts.size(), screenVerts.data());
	screenVerts.clear();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->EndCamera(m_screenCamera);

	if (m_engineLogoFireSeconds >= 0.0f)
	{
		logoFire = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
		SpriteSheet* m_explosionSpriteSheet = new SpriteSheet(*logoFire, IntVec2(5, 5));
		std::vector<Vertex_PCU> explosionVerts;
		SpriteAnimDefinition animation = SpriteAnimDefinition(*m_explosionSpriteSheet, 0, 20, 2.5f, SpriteAnimPlaybackType::ONCE);

		AABB2 explosionAABB2A;
		explosionAABB2A.m_mins = Vec2(6.5f, 4.0f);
		explosionAABB2A.m_maxs = Vec2(7.0f, 4.5f);

		AABB2 explosionAABB2B;
		explosionAABB2B.m_mins = Vec2(9.5f, 4.0f);
		explosionAABB2B.m_maxs = Vec2(10.0f, 4.5f);
		SpriteDefinition currentExplosionSpriteDef = animation.GetSpriteDefAtTime(m_currentEngineLogoSeconds);
		AddVertsForAABB2D(explosionVerts, explosionAABB2A, currentExplosionSpriteDef.GetUVs(), Rgba8(255, 255, 255, 255));
		AddVertsForAABB2D(explosionVerts, explosionAABB2B, currentExplosionSpriteDef.GetUVs(), Rgba8(255, 255, 255, 255));

		g_theRenderer->BeginCamera(m_worldCamera);
		g_theRenderer->BindShader(g_theRenderer->GetDefaultShader());
		g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
		g_theRenderer->BindTexture(logoFire);
		g_theRenderer->DrawVertexArray((int)explosionVerts.size(), explosionVerts.data());
		explosionVerts.clear();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->EndCamera(m_worldCamera);
	}
}
void Game::UpdateEngineLogo(float deltaSeconds)
{
	g_theApp->m_isDebugDraw = false;
	m_currentEngineLogoSeconds += deltaSeconds;
	m_engineLogoFireSeconds -= deltaSeconds;
	if (m_currentEngineLogoSeconds >= m_engineLogoSeconds)
	{
		SwitchGameModes(GameStates::Attract);
	}
	if (g_theInputSystem->WasKeyJustPressed(' '))
	{
		m_currentGameState = GameStates::Attract;
	}
}

void Game::UpdateGameState()
{
	if (m_PlayerVictory)
		SwitchGameModes(GameStates::Victory);
	if (m_isGameOver)
		SwitchGameModes(GameStates::PlayerDead);
}
void Game::SwitchGameModes(GameStates gameState)
{
	if (m_currentGameState != gameState)
	{
		m_currentGameState = gameState;
		if (m_currentGameState == GameStates::Victory)
		{
			PlaySoundSfx(m_gameSounds[(int)GameSounds::GameVictory]);
		}
		if (m_currentGameState != GameStates::Victory)
		{
			m_PlayerVictory = false;
		}
	}

}
void Game::PlaySoundSfx(SoundPlaybackID soundID)
{
	if (soundID != MISSING_SOUND_ID)
		g_theAudioSystem->StartSound(soundID, false, m_masterVolume);

	else return;

}
void Game::SwitchToGame(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.0f, 0.1f, 1000.0f);
	g_theInputSystem->SetCursorAtCenter();
	m_IsAttractMode = false;
	m_currentGameState = GameStates::GameWorld;
}
bool Game::GetIsMenuScreen()
{
	return m_IsAttractMode;
}
void Game::UpdateMenuScreenPlayButton(float deltaSeconds)
{
	m_playButtonAlpha -= deltaSeconds * 100.0f;

	if (m_playButtonAlpha <= 0)
	{
		m_playButtonAlpha = 255.0f;
	}
}
void Game::RenderMenuScreenPlayButton() const
{
	Vertex_PCU tempVertexArrays[3];
	tempVertexArrays[0] = Vertex_PCU(Vec3(5.0f, 0.0f, 0.f), Rgba8(0, 255, 0, (unsigned char)m_playButtonAlpha), Vec2(0.f, 0.f));
	tempVertexArrays[1] = Vertex_PCU(Vec3(-5.0f, -5.0f, 0.f), Rgba8(0, 255, 0, (unsigned char)m_playButtonAlpha), Vec2(0.f, 0.f));
	tempVertexArrays[2] = Vertex_PCU(Vec3(-5.0f, 5.0f, 0.f), Rgba8(0, 255, 0, (unsigned char)m_playButtonAlpha), Vec2(0.f, 0.f));
	TransformVertexArray3D(3, tempVertexArrays, 8.0f, 0.0f, Vec3(WORLD_CENTER_X * 8, WORLD_CENTER_Y * 8, 0.0f));
	g_theRenderer->DrawVertexArray(3, tempVertexArrays);

}

void Game::CreateWorldTextAndBasis()
{
	Vec3 IBasis = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 JBasis = Vec3(0.0f, 1.0f, 0.0f);
	Vec3 KBasis = Vec3(0.0f, 0.0f, 1.0f);
	Mat44 basisMatrix = Mat44(IBasis, JBasis, KBasis, Vec3());
	DebugAddWorldBasis(basisMatrix, -1.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);

	std::string xbasisText = Stringf("x -forward");
	Mat44 xForwardMatrix = Mat44::CreateXRotationDegrees(90.0f);
	DebugAddWorldText(xbasisText, xForwardMatrix, 0.2f, Vec2(0.0f, 0.0f), -1.0f, Rgba8::RED, Rgba8::RED, DebugRenderMode::ALWAYS);

	std::string ybasisText = Stringf("y -left");
	Mat44 yLeftMatrix = Mat44::CreateZRotationDegrees(90.0f);
	DebugAddWorldText(ybasisText, yLeftMatrix, 0.2f, Vec2(0.0f, 0.0f), -1.0f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::ALWAYS);

	std::string zbasisText = Stringf("z -up");
	Mat44 zUpMatrix = Mat44::CreateYRotationDegrees(-90.0f);
	DebugAddWorldText(zbasisText, zUpMatrix, 0.2f, Vec2(0.0f, 0.0f), -1.0f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::ALWAYS);
}
// -------------------------NAMED PROPERTIES---------------------------------------
void Game::TestNameProperties()
{
	std::string lastName ("Eiserloh");

//	NamedProperties employmentInfoProperties;
	// ...

	NamedProperties p;
	float a = 1.93f;
	const char* m_firstName = "FirstName";
	const char* m_lastName = "Squirrel";
	//p.SetValue("FirstName", "Squirrel"); 	// Setting as c-string (const char*) data...
	//p.SetValue("LastName", lastName);	// Setting as std::string data...
	p.SetValue("Height", a);
	/*p.SetValue("Age", 48);
	p.SetValue("IsMarried", true);
	p.SetValue("Position", Vec2(3.5f, 6.2f));
	p.SetValue("EyeColor", Rgba8(77, 38, 23));*/
	//p.SetValue("PlayerInfo", employmentInfoProperties);

	// Note the subtleties in the Set, Get, and return types for each of the following examples:
	/*std::string first_name = p.GetValue("FirstName", "UNKNOWN");
	std::string last_name = p.GetValue("LastName", "UNKNOWN");*/

	m_firstName = p.GetValue("FirstName", m_firstName);
	m_lastName = p.GetValue("LastName", m_lastName);


}