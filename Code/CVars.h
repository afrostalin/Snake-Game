// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

namespace Snake
{
	class CGameCVars
	{
	public:
		CGameCVars() {}
		~CGameCVars() {}
	public:
		void RegisterCVars();
		void UnregisterCVars();
	public:
		float g_s_music_volume = 1.f;
		float g_s_sfx_volume = 1.f;
		float g_s_master_volume = 1.f;
	public:
		float g_meters_per_step = 2.f;
		float g_snakeSpawnOffsetZ = 0.f;
		float g_fruitSpawnDistance = 15.f;
		int   g_watch_enabled = 0;

		int   g_ui_managerLogs = 0;
		int   g_ui_use_job_queue = 1;
		int   g_ui_dublicateWarningsErrorsToUI = 1;
		float g_ui_notificationShowTime = 5.f;
		int   g_ui_hudHide = 0;

		int   g_controlType = 0;
		int   g_automaticSelectWindowResolution = 0;

		bool  g_hasWindowFocus = false;
		bool  g_gameplayStarted = false;
	public:
		float pl_default_speed = 7.f;
		int   pl_default_size = 4;
		float pl_cameraInterpolationSpeed = 5.f;
		float pl_cameraDistanceFromPlayer = 20.f;
		float pl_cameraDistanceFromPlayerTOP = 50.f;
		float pl_cameraYoffset = -5.f;
		float pl_cameraXoffset = 0.f;
		float pl_cameraSlope = -70.f;
		int   pl_autoReviveAfterDeath = 1;
		int   pl_cameraModeChangingType = 0;
		int   pl_cameraDefaultMode = 0;
		float pl_maxDistForFastChangeDir = 0.5f;
	public:
		// Debug
		int dg_field = 0;
		int dg_snakeDeath = 0;
		int dg_snakeSpawn = 0;
		int dg_borders = 0;
		int dg_barriers = 0;
		int dg_spawnPoints = 0;
		int dg_availableSpawnPoints = 0;
		int dg_snakeCoordsOnField = 0;
		int dg_snakeParts = 0;
	protected:
		void RegisterAudioCVars();
		void UnregisterAudioCVars();
	protected:
		void RegisterGameCVars();
		void UnregisterGameCVars();
	protected:
		void RegisterPlayerCVars();
		void UnregisterPlayerCVars();
	};
}