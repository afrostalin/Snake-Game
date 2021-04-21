// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

namespace Snake
{
	class CGameLocalizationManager
	{
	public:
		CGameLocalizationManager();
		virtual ~CGameLocalizationManager();

		void SetGameType();
		void SetCredits(bool enable);

#if !defined(_RELEASE)
		static void LocalizationDumpLoadedTags(IConsoleCmdArgs* pArgs);
#endif

	protected:
		enum ELocalizationTag
		{
			eLT_Init,
			eLT_GameType,
			eLT_Credits,
			eLT_Num
		};

		void LoadLocalizationData();
		void LegacyLoadLocalizationData();

		void LoadTag(ELocalizationTag tag);
		void LoadTagInternal(ELocalizationTag tag, const char* pTag);
		void UnloadTag(ELocalizationTag tag);

		string m_loadedTag[eLT_Num];
	};
}