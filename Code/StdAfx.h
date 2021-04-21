// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryCore/Project/CryModuleDefs.h>

#define eCryModule eCryM_EnginePlugin
#define GAME_API   DLL_EXPORT
#define TITLE "[Snake] "

#include <CryCore/Platform/platform.h>
#include <CryCore/StaticInstanceList.h>

#include <CrySystem/ISystem.h>
#include <Cry3DEngine/I3DEngine.h>
#include <CryNetwork/ISerialize.h>

#include <CrySchematyc/Reflection/TypeDesc.h>
#include <CrySchematyc/Utils/EnumFlags.h>
#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/Elements/EnvFunction.h>
#include <CrySchematyc/Env/Elements/EnvSignal.h>
#include <CrySchematyc/ResourceTypes.h>
#include <CrySchematyc/MathTypes.h>
#include <CrySchematyc/Utils/SharedString.h>

#include "CVars.h"

namespace Snake
{
	class CGameLocalizationManager;
	class CGamePlugin;

	// Game global pointers
	struct SGameEnv
	{
	public:
		SGameEnv() {}
		virtual ~SGameEnv() {}
	public:
		CGameLocalizationManager* pLocalization = nullptr;
		CGamePlugin*              pCore = nullptr;
	};

	extern SGameEnv*   g_pGame;
	extern CGameCVars* g_pGameCVars;

	//! Logging system
	inline void Log(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		char szText[2048];
		cry_vsprintf(szText, format, args);
		va_end(args);

		CryLog("$8%s$1%s", TITLE, szText);
	}

	inline void LogDebug(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		char szText[2048];
		cry_vsprintf(szText, format, args);
		va_end(args);

		CryLog("$8%s$1%s", TITLE, szText);
#if DEBUG_UI
		if (g_pGame && g_pGameCVars && (g_pGame->IsDebugMode() || g_pGame->IsDevMode()) && g_pGameCVars->g_ui_dublicateWarningsErrorsToUI > 0 && g_pGame->pDebugMenu)
		{
			g_pGame->pDebugMenu->ShowDebug(szText);
		}
#endif
	}

	inline void LogAlways(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		char szText[2048];
		cry_vsprintf(szText, format, args);
		va_end(args);

		CryLogAlways("$8%s$1%s", TITLE, szText);
	}

	inline void LogWarning(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		char szText[2048];
		cry_vsprintf(szText, format, args);
		va_end(args);

		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "$8%s$6%s", TITLE, szText);

#if DEBUG_UI
		if (g_pGame && g_pGameCVars && (g_pGame->IsDebugMode() || g_pGame->IsDevMode()) && g_pGameCVars->g_ui_dublicateWarningsErrorsToUI > 0 && g_pGame->pDebugMenu)
		{
			g_pGame->pDebugMenu->ShowWarning(szText);
		}
#endif
	}

	inline void LogError(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		char szText[2048];
		cry_vsprintf(szText, format, args);
		va_end(args);

		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "$8%s$4%s", TITLE, szText);

#if DEBUG_UI
		if (g_pGame && g_pGameCVars && (g_pGame->IsDebugMode() || g_pGame->IsDevMode()) && g_pGameCVars->g_ui_dublicateWarningsErrorsToUI > 0 && g_pGame->pDebugMenu)
		{
			g_pGame->pDebugMenu->ShowError(szText);
		}
#endif
	}

#if !defined(_RELEASE)
#define CRY_WATCH_ENABLED			 (1)
#else
#define CRY_WATCH_ENABLED			 (0)
#endif

#define CryWatch(...) CryWatchFunc(string().Format(__VA_ARGS__))
#define CryWatchLog(...) CryWatchLogFunc(string().Format(__VA_ARGS__))

#if CRY_WATCH_ENABLED

	int CryWatchFunc(const char* message);
	int CryWatchLogFunc(const char* message);
	void CryWatch3DAdd(const char* text, const Vec3& posIn, float lifetime = 2.f, const Vec3 * velocity = nullptr, float gravity = 3.f, const ColorB& color = ColorB());
	void CryWatch3DReset();
	void CryWatch3DTick(float dt);

#else

#define CryWatchFunc(message)          (0)
#define CryWatchLogFunc(message)          (0)
#define CryWatch3DAdd(...)             ((void)0)
#define CryWatch3DReset()              ((void)0)
#define CryWatch3DTick(dt)             ((void)0)

#endif

#ifdef TODO
#undef TODO
#endif
#define STRINGANIZE2(x) # x
#define STRINGANIZE1(x) STRINGANIZE2(x)
#define TODO(y)               __pragma(message(__FILE__ "(" STRINGANIZE1(__LINE__) ") : " "TODO >>> " STRINGANIZE2(y)))

	struct CVarHelper
	{
		static void Unregister(const char* name)
		{
			if (gEnv && gEnv->pConsole)
			{
				gEnv->pConsole->UnregisterVariable(name);
			}
		}

		static void RemoveComand(const char* name)
		{
			if (gEnv && gEnv->pConsole)
			{
				gEnv->pConsole->RemoveCommand(name);
			}
		}
	};

	ILINE static void Interpolate(float& actual, float goal, float speed, float frameTime, float limit = 0.f)
	{
		float delta = goal - actual;

		if (limit > 0.001f)
		{
			delta = max(min(delta, limit), -limit);
		}

		actual += delta * min(frameTime * speed, 1.0f);
	}
}