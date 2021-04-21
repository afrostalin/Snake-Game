// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "GamePlugin.h"

#include "Components/Snake.h"

#include "Utils/LocalizationManager.h"

#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/EnvPackage.h>
#include <CrySchematyc/Utils/SharedString.h>

#include <IGameObjectSystem.h>
#include <IGameObject.h>

// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>

#include <CrySystem/ICryPluginManager.h>

namespace Snake
{
	CGamePlugin::~CGamePlugin()
	{
		// Remove any registered listeners before 'this' becomes invalid
		if (gEnv->pGameFramework != nullptr)
		{
			gEnv->pGameFramework->RemoveNetworkedClientListener(*this);
		}

		if (gEnv->pSystem)
		{
			gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
			gEnv->pSystem->UnregisterWindowMessageHandler(this);
		}

		if (gEnv->pInput)
		{
			gEnv->pInput->RemoveEventListener(this);
		}

		if (gEnv->pSchematyc)
		{
			gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(CGamePlugin::GetCID());
		}

		m_pUserProfile.reset();

		g_pGame->pCore = nullptr;

		SAFE_DELETE(g_pGame->pLocalization);
		
		// Unregister game console variables
		g_pGameCVars->UnregisterCVars();

		SAFE_DELETE(g_pGameCVars);
		SAFE_DELETE(g_pGame);
	}

	bool CGamePlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
	{
		gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CGamePlugin");

		gEnv->pSystem->LoadConfiguration("%USER%/game.cfg", nullptr, ELoadConfigurationType::eLoadConfigInit, ELoadConfigurationFlags::SuppressConfigNotFoundWarning);

		EnableUpdate(EUpdateStep::MainUpdate, true);

		g_pGame->pCore = this;

		gEnv->pSystem->RegisterWindowMessageHandler(this);

		return true;
	}

	void CGamePlugin::MainUpdate(float frameTime)
	{
#if CRY_PLATFORM_WINDOWS
		if (!gEnv->IsEditor() && !gEnv->IsDedicated())
		{
			gEnv->pSystem->PumpWindowMessage(true, 0);
		}
#endif

#if CRY_WATCH_ENABLED
		CryWatch3DTick(frameTime);
#endif 
	}

	void CGamePlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
		case ESYSTEM_EVENT_GAME_FRAMEWORK_INIT_DONE:
		{
			gEnv->pInput->AddEventListener(this);
			g_pGameCVars->RegisterCVars();				

			// Some debug 
			if (gEnv->IsEditor())
			{
				g_pGameCVars->dg_snakeParts = 1;
				g_pGameCVars->dg_borders = 1;
				g_pGameCVars->dg_barriers = 1;
			}
		}
		break;
		case ESYSTEM_EVENT_CRYSYSTEM_INIT_DONE:
		{
			g_pGame->pLocalization = new CGameLocalizationManager();
			m_pUserProfile.reset(new CUserProfile());
		}
		break;
		case ESYSTEM_EVENT_GAME_POST_INIT:
		{
			gEnv->pGameFramework->AddNetworkedClientListener(*this);		

			if (!gEnv->IsEditor())
			{
				gEnv->pConsole->ExecuteString("map level_00", false, true);
			}
		}
		break;
		case ESYSTEM_EVENT_LEVEL_UNLOAD_START:
		{
			g_pGameCVars->g_gameplayStarted = false;

			if (!gEnv->pSystem->IsQuitting())
			{
				gEnv->pAudioSystem->StopAllSounds();
			}
		}
		break;
		case ESYSTEM_EVENT_REGISTER_SCHEMATYC_ENV:
		{
			// Register all components that belong to this plug-in
			auto staticAutoRegisterLambda = [](Schematyc::IEnvRegistrar& registrar)
			{
				// Call all static callback registered with the CRY_STATIC_AUTO_REGISTER_WITH_PARAM
				Detail::CStaticAutoRegistrar<Schematyc::IEnvRegistrar&>::InvokeStaticCallbacks(registrar);
			};

			if (gEnv->pSchematyc)
			{
				gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(
					stl::make_unique<Schematyc::CEnvPackage>(
						CGamePlugin::GetCID(),
						"EntityComponents",
						"Space Raccoon Game Studio",
						"Components",
						staticAutoRegisterLambda
						)
				);
			}
		}
		break;
		case ESYSTEM_EVENT_LEVEL_UNLOAD:
		{
			m_players.clear();
		}
		break;
		case ESYSTEM_EVENT_FULL_SHUTDOWN:
		case ESYSTEM_EVENT_FAST_SHUTDOWN:
		{
			// Shutdown...
		}
		break;
		case ESYSTEM_EVENT_LEVEL_GAMEPLAY_START:
			g_pGameCVars->g_gameplayStarted = true;
			break;
		}
	}

	bool CGamePlugin::OnClientConnectionReceived(int channelId, bool bIsReset)
	{
		SEntitySpawnParams spawnParams;
		spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();

		const string playerName = string().Format("Player%" PRISIZE_T, m_players.size());
		spawnParams.sName = playerName;

		if (m_players.empty() && !gEnv->IsDedicated())
		{
			spawnParams.id = LOCAL_PLAYER_ENTITY_ID;
			spawnParams.nFlags |= ENTITY_FLAG_LOCAL_PLAYER;
		}

		if (IEntity * pPlayerEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams))
		{
			pPlayerEntity->GetNetEntity()->SetChannelId(channelId);

			if (CSnake* pPlayer = pPlayerEntity->GetOrCreateComponentClass<CSnake>())
			{
				if (pPlayer->IsLocalClient())
				{
					pPlayer->RegisterListener(this);
				}

				m_players.emplace(std::make_pair(channelId, pPlayerEntity->GetId()));
			}
		}

		return true;
	}

	bool CGamePlugin::OnClientReadyForGameplay(int channelId, bool bIsReset)
	{
		if (gEnv->IsEditor())
			return true;

		for (const auto& it : m_players)
		{
			if (it.first == channelId)
			{
				IEntity* pPlayerEntity = gEnv->pEntitySystem->GetEntity(it.second);
				CSnake* pPlayer = pPlayerEntity ? pPlayerEntity->GetComponent<CSnake>() : nullptr;

				if (pPlayer)
				{
					pPlayer->OnReadyForGameplayOnServer();
				}
			}
		}

		return true;
	}

	void CGamePlugin::OnClientDisconnected(int channelId, EDisconnectionCause cause, const char* description, bool bKeepClient)
	{
		auto it = m_players.find(channelId);
		if (it != m_players.end())
		{
			gEnv->pEntitySystem->RemoveEntity(it->second);
			m_players.erase(it);
		}
	}

	bool CGamePlugin::OnInputEvent(const SInputEvent& event)
	{
		if (event.keyId == eKI_SYS_ConnectDevice && event.deviceType == EInputDeviceType::eIDT_Gamepad)
		{
			// Gamepad connected
		}
		else if(event.keyId == eKI_SYS_DisconnectDevice && event.deviceType == EInputDeviceType::eIDT_Gamepad)
		{
			// Gamepad disconnected
		}

		return false;
	}

	bool CGamePlugin::HandleMessage(CRY_HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
	{
		switch (uMsg)
		{
		case WM_SYSCHAR: // Prevent ALT + key combinations from creating 'ding' sounds
		{
			const bool bAlt = (lParam & (1 << 29)) != 0;
			if (bAlt && wParam == VK_F4)
			{
				return false; // Pass though ALT+F4
			}

			*pResult = 0;
			return true;
		}
		break;
		case WM_SETFOCUS:
		{
			if (g_pGameCVars)
			{
				g_pGameCVars->g_hasWindowFocus = true;

				if (m_isPausedByLostFocus && gEnv->pGameFramework)
				{
					m_isPausedByLostFocus = false;
					gEnv->pGameFramework->PauseGame(false, false, 0);
					Log("Automatic game resume by active focus");
				}
			}
		}
		break;
		case WM_KILLFOCUS:
		{
			if (g_pGameCVars)
			{
				g_pGameCVars->g_hasWindowFocus = false;

				if (gEnv->pGameFramework && !gEnv->pGameFramework->IsGamePaused() && g_pGameCVars->g_gameplayStarted)
				{
					gEnv->pGameFramework->PauseGame(true, false, 0);
					m_isPausedByLostFocus = true;
					Log("Automatic game pause by lost focus");
				}
			}
		}
		break;
		}
		return false;
	}

	void CGamePlugin::OnLocalPlayerSpawned()
	{
		for (const auto& it : m_localPlayerListeners)
		{
			it->OnLocalPlayerSpawned();
		}
	}

	void CGamePlugin::OnLocalPlayerRevived()
	{
		for (const auto& it : m_localPlayerListeners)
		{
			it->OnLocalPlayerRevived();
		}
	}

	CUserProfile* CGamePlugin::GetUserProfile() const
	{
		return m_pUserProfile.get();
	}

	void CGamePlugin::RegisterLocalPlayerListener(ILocalPlayerListener* pListener)
	{
		if (!stl::find(m_localPlayerListeners, pListener))
			m_localPlayerListeners.push_back(pListener);
	}

	void CGamePlugin::UnregisterLocalPlayerListener(ILocalPlayerListener* pListener)
	{
		stl::find_and_erase(m_localPlayerListeners, pListener);
	}

	CRYREGISTER_SINGLETON_CLASS(CGamePlugin)
}