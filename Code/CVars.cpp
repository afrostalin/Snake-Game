// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "CVars.h"

#include <CrySystem/IConsole.h>
#include <CrySystem/ConsoleRegistration.h>

#include "Components/Snake.h"

namespace Snake
{
	void CGameCVars::RegisterCVars()
	{
		RegisterGameCVars();
		RegisterAudioCVars();
		RegisterPlayerCVars();
	}

	void CGameCVars::UnregisterCVars()
	{
		UnregisterGameCVars();
		UnregisterAudioCVars();
		UnregisterPlayerCVars();
	}

	// Audio
	static void CMD_AudioVolumeChange(ICVar* pCVar)
	{
		CryAudio::ControlId parameter = CryAudio::InvalidControlId;

		if (pCVar->GetName() == string("g_s_musicVolume"))
		{
			parameter = CryAudio::StringToId("music_volume");
		}
		else if (pCVar->GetName() == string("g_s_sfxVolume"))
		{
			parameter = CryAudio::StringToId("sfx_volume");
		}
		else if (pCVar->GetName() == string("g_s_masterVolume"))
		{
			parameter = CryAudio::StringToId("master_volume");
		}

		const float value = crymath::clamp(pCVar->GetFVal(), 0.f, 1.f);

		gEnv->pAudioSystem->SetParameterGlobally(parameter, value);
	}

	// Player
	static void CMD_PlayerRevive(IConsoleCmdArgs* pArgs)
	{
		IEntity* pLocalPlayerEntity = gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID);
		if (pLocalPlayerEntity)
		{
			pLocalPlayerEntity->GetComponent<CSnake>()->Revive(ZERO);
		}
	}

	static void CMD_PlayerKill(IConsoleCmdArgs* pArgs)
	{
		IEntity* pLocalPlayerEntity = gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID);
		if (pLocalPlayerEntity)
		{
			pLocalPlayerEntity->GetComponent<CSnake>()->KillSnake(ESnakeDeathType::Invalid);
		}
	}

	static void CMD_PlayerCameraModeChange(ICVar* pCVar)
	{
		IEntity* pLocalPlayerEntity = gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID);
		if (pLocalPlayerEntity)
		{
			ECameraMode mode = ECameraMode::Default;

			if (pCVar->GetIVal() == 1)
				mode = ECameraMode::Top;

			pLocalPlayerEntity->GetComponent<CSnake>()->ChangeCameraMode(mode);
		}
	}

	void CGameCVars::RegisterAudioCVars()
	{
		ConsoleRegistrationHelper::Register("g_s_musicVolume", &g_s_music_volume, 1.f, VF_NULL, "Music volume", &CMD_AudioVolumeChange);
		ConsoleRegistrationHelper::Register("g_s_sfxVolume", &g_s_sfx_volume, 1.f, VF_NULL, "SFX volume", &CMD_AudioVolumeChange);
		ConsoleRegistrationHelper::Register("g_s_masterVolume", &g_s_master_volume, 1.f, VF_NULL, "Master volume", &CMD_AudioVolumeChange);
	}

	void CGameCVars::UnregisterAudioCVars()
	{
		CVarHelper::Unregister("g_s_musicVolume");
		CVarHelper::Unregister("g_s_sfxVolume");
		CVarHelper::Unregister("g_s_masterVolume");
	}

	void CGameCVars::RegisterGameCVars()
	{
		ConsoleRegistrationHelper::Register("g_mps", &g_meters_per_step, 2.f, VF_CHEAT, "Meters per step");
		ConsoleRegistrationHelper::Register("g_snakeSpawnOffsetZ", &g_snakeSpawnOffsetZ, 0.5f, VF_CHEAT, "Snake spawn offset Z");
		ConsoleRegistrationHelper::Register("g_fruitSpawnDistance", &g_fruitSpawnDistance, 15.f, VF_CHEAT, "Fruit spawn distance");
		ConsoleRegistrationHelper::Register("g_watch_enabled", &g_watch_enabled, 1, VF_CHEAT, "Enable/disable CryWatch");

		ConsoleRegistrationHelper::Register("g_controlType", &g_controlType, 0, VF_NULL, "Control type (0 - keyboard, 1 - gamepad)");
		ConsoleRegistrationHelper::Register("g_automaticSelectWindowResolution", &g_automaticSelectWindowResolution, 0, VF_NULL, "Automatic window resolution selecting based on current UI resolution");
	}

	void CGameCVars::UnregisterGameCVars()
	{
		CVarHelper::Unregister("g_mps");
		CVarHelper::Unregister("g_snakeSpawnOffsetZ");
		CVarHelper::Unregister("g_fruitSpawnDistance");
		CVarHelper::Unregister("g_watch_enabled");

		CVarHelper::Unregister("g_controlType");
		CVarHelper::Unregister("g_automaticSelectWindowResolution");
	}

	void CGameCVars::RegisterPlayerCVars()
	{
		ConsoleRegistrationHelper::Register("pl_speed_default", &pl_default_speed, 5.f, VF_CHEAT, "Default snake speed");

		ConsoleRegistrationHelper::Register("pl_cameraInterpolationSpeed", &pl_cameraInterpolationSpeed, 5.f, VF_CHEAT, "Camera interpolation speed");
		ConsoleRegistrationHelper::Register("pl_cameraDistanceFromPlayer", &pl_cameraDistanceFromPlayer, 17.f, VF_CHEAT, "Camera distance from player");
		ConsoleRegistrationHelper::Register("pl_cameraDistanceFromPlayerTOP", &pl_cameraDistanceFromPlayerTOP, 50.f, VF_CHEAT, "Camera distance from in TOP mode");
		ConsoleRegistrationHelper::Register("pl_cameraYoffset", &pl_cameraYoffset, -5.f, VF_CHEAT, "Camera offset Y");
		ConsoleRegistrationHelper::Register("pl_cameraXoffset", &pl_cameraXoffset, 0.f, VF_CHEAT, "Camera offset X");
		ConsoleRegistrationHelper::Register("pl_cameraSlope", &pl_cameraSlope, -70.f, VF_CHEAT, "Camera slope");
		ConsoleRegistrationHelper::Register("pl_cameraModeChangingType", &pl_cameraModeChangingType, 0, VF_NULL, "Camera changing mode type (0 - holding, 1 - pressing)");
		ConsoleRegistrationHelper::Register("pl_cameraDefaultMode", &pl_cameraDefaultMode, 0, VF_NULL, "Default camera mode (0 - Default, 1 - TOP)", &CMD_PlayerCameraModeChange);
		ConsoleRegistrationHelper::Register("pl_maxDistForFastChangeDir", &pl_maxDistForFastChangeDir, 0.3f, VF_CHEAT, "Maximum distance for fast change direction");

		ConsoleRegistrationHelper::Register("pl_autoReviveAfterDeath", &pl_autoReviveAfterDeath, 1, VF_CHEAT, "Auto revive after death");

		ConsoleRegistrationHelper::AddCommand("pl_revive", &CMD_PlayerRevive, VF_CHEAT);
		ConsoleRegistrationHelper::AddCommand("pl_kill", &CMD_PlayerKill, VF_CHEAT);
	}

	void CGameCVars::UnregisterPlayerCVars()
	{
		CVarHelper::Unregister("pl_speed_default");

		CVarHelper::Unregister("pl_cameraInterpolationSpeed");
		CVarHelper::Unregister("pl_cameraDistanceFromPlayer");
		CVarHelper::Unregister("pl_cameraDistanceFromPlayerTOP");
		CVarHelper::Unregister("pl_cameraYoffset");
		CVarHelper::Unregister("pl_cameraXoffset");
		CVarHelper::Unregister("pl_cameraSlope");
		CVarHelper::Unregister("pl_cameraModeChangingType");
		CVarHelper::Unregister("pl_cameraDefaultMode");
		CVarHelper::Unregister("pl_cameraDefaultMode");

		CVarHelper::Unregister("pl_navigationHelper");
		CVarHelper::Unregister("pl_maxDistForFastChangeDir");

		CVarHelper::RemoveComand("pl_revive");
		CVarHelper::RemoveComand("pl_kill");
	}
}