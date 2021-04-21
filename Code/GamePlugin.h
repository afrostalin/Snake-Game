// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CrySystem/ICryPlugin.h>
#include <CryGame/IGameFramework.h>
#include <CryEntitySystem/IEntityClass.h>
#include <CryNetwork/INetwork.h>
#include <CryInput/IInput.h>
#include <CrySystem/IWindowMessageHandler.h>

#include "Utils/UserProfile.h"
#include "Components/Snake.h"

namespace Snake
{
	class CGamePlugin
		: public Cry::IEnginePlugin
		, public ISystemEventListener
		, public INetworkedClientListener
		, public IInputEventListener
		, public IWindowMessageHandler
		, public ILocalPlayerListener
	{
	public:
		CRYINTERFACE_SIMPLE(Cry::IEnginePlugin)
		CRYGENERATE_SINGLETONCLASS_GUID(CGamePlugin, "Snake", "{BCC7B624-C27D-4F45-A578-A00BB040B37C}"_cry_guid)

		virtual ~CGamePlugin();

		// Cry::IEnginePlugin
		virtual const char* GetCategory() const override { return "Game"; }
		virtual bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
		virtual void MainUpdate(float frameTime) override;
		// ~Cry::IEnginePlugin

		// ISystemEventListener
		virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
		// ~ISystemEventListener

		// INetworkedClientListener
		virtual void OnLocalClientDisconnected(EDisconnectionCause cause, const char* description) override {}
		virtual bool OnClientConnectionReceived(int channelId, bool bIsReset) override;
		virtual bool OnClientReadyForGameplay(int channelId, bool bIsReset) override;
		virtual void OnClientDisconnected(int channelId, EDisconnectionCause cause, const char* description, bool bKeepClient) override;
		virtual bool OnClientTimingOut(int channelId, EDisconnectionCause cause, const char* description) override { return true; }
		// ~INetworkedClientListener

		// IInputEventListener
		virtual bool OnInputEvent(const SInputEvent& event) override;
		// ~IInputEventListener

		// IWindowMessageHandler
		virtual bool HandleMessage(CRY_HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
		// ~IWindowMessageHandler

		// ILocalPlayerListener
		virtual void OnLocalPlayerSpawned() override;
		virtual void OnLocalPlayerRevived() override;
		//~ILocalPlayerListener

		// Helper function to get the CGamePlugin instance
		// Note that CGamePlugin is declared as a singleton, so the CreateClassInstance will always return the same pointer
		static CGamePlugin* GetInstance()
		{
			return cryinterface_cast<CGamePlugin>(CGamePlugin::s_factory.CreateClassInstance().get());
		}
	public:
		CUserProfile* GetUserProfile() const;

		void RegisterLocalPlayerListener(ILocalPlayerListener* pListener);
		void UnregisterLocalPlayerListener(ILocalPlayerListener* pListener);
	protected:
		std::unordered_map<int, EntityId>  m_players;
		std::unique_ptr<CUserProfile>      m_pUserProfile = nullptr;
		std::vector<ILocalPlayerListener*> m_localPlayerListeners;
	private:
		bool                               m_isPausedByLostFocus = false;
	};
}