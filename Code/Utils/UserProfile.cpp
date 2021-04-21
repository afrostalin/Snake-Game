// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "UserProfile.h"

namespace Snake
{
	CUserProfile::CUserProfile()
	{
		// Window settings
		m_cvarsForSave.push_back("r_width");
		m_cvarsForSave.push_back("r_height");
		m_cvarsForSave.push_back("r_windowtype");
		m_cvarsForSave.push_back("r_displayinfo");
		m_cvarsForSave.push_back("r_vsync");

		// Graphics settings
		m_cvarsForSave.push_back("sys_spec_full");

		// Audio settings
		m_cvarsForSave.push_back("g_s_masterVolume");
		m_cvarsForSave.push_back("g_s_musicVolume");
		m_cvarsForSave.push_back("g_s_sfxVolume");

		// Game settings
		m_cvarsForSave.push_back("g_controlType");
		m_cvarsForSave.push_back("g_automaticSelectWindowResolution");

		// Player settings
		m_cvarsForSave.push_back("pl_cameraModeChangingType");
		m_cvarsForSave.push_back("pl_cameraDefaultMode");

		// Auto-resolution switching
		if (g_pGameCVars->g_automaticSelectWindowResolution)
		{
			HMONITOR m_activeMonitor = MonitorFromWindow((HWND)gEnv->pSystem->GetHWND(), MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo;
			monitorInfo.cbSize = sizeof(monitorInfo);
			GetMonitorInfo(m_activeMonitor, &monitorInfo);

			// Match monitor resolution in borderless full screen mode
			const int displayWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			const int displayHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

			if (displayWidth && displayHeight)
			{
				gEnv->pConsole->GetCVar("r_width")->Set(displayWidth);
				gEnv->pConsole->GetCVar("r_height")->Set(displayHeight);
			}
		}
	}

	CUserProfile::~CUserProfile()
	{
	}

	void CUserProfile::SaveToGameCFG()
	{
		const string path = PathUtil::Make(gEnv->pCryPak->GetAlias("%USER%"), "game.cfg");
		Log("Saving current user settings to <%s>", path.c_str());

		if (gEnv->pConsole == nullptr)
			return;

		FILE* pFile = fxopen("%USER%/game.cfg", "wb");
		if (pFile == nullptr)
		{
			LogWarning("[UserProfile] Can't save game.cfg - can't open file");
			return;
		}

		fputs("-- [Game-Configuration]\r\n", pFile);
		fputs("-- Attention: This file is re-generated by the system! Editing is not recommended! \r\n\r\n", pFile);

		for (const auto &it : m_cvarsForSave)
		{
			if (it.IsEmpty())
				continue;

			ICVar* pCvar = gEnv->pConsole->GetCVar(it.c_str());
			if (pCvar)
			{
				string lineForSaving = string().Format("%s = %s \r\n", it.c_str(), pCvar->GetString());
				if (!lineForSaving.IsEmpty())
				{
					fputs(lineForSaving.c_str(), pFile);
				}
			}
		}

		fclose(pFile);
	}
}