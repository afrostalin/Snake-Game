// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

namespace Snake
{
	class CUserProfile
	{
	public:
		CUserProfile();
		~CUserProfile();
	public:
		void SaveToGameCFG();
	private:
		std::vector<string> m_cvarsForSave;
	};
}