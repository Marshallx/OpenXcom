#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <vector>
#include "../Engine/State.h"

namespace OpenXcom
{

class Text;
class TextList;
class Base;
class Craft;

class CraftLiveStatus
{
public:
	CraftLiveStatus(GeoscapeState* state);
	void TryAddCraft(Craft * craft, GeoscapeState * state, Transfer const * transfer = nullptr);
	void Think(GeoscapeState* state);
	void HandleLeftClick(GeoscapeState * state);
	void HandleRightClick(GeoscapeState * state);
	void Toggle(GeoscapeState * state);
private:
	Element const * modCraft = nullptr;
	Element const * modFuel = nullptr;
	Element const * modDamage = nullptr;
	Element const * modReady = nullptr;
	Element * modStatus = nullptr;
	Element * modList = nullptr;
	InteractiveSurface * btnToggle_ = nullptr;
	Text * txtName_ = nullptr;
	Text * txtFuel_ = nullptr;
	Text * txtDamage_ = nullptr;
	Text * txtReady_ = nullptr;
	Text * txtStatus_ = nullptr;
	TextList * lstCraft_ = nullptr;
	std::vector<Craft *> crafts_ = {};
};

}
