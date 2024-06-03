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
 * along with OpenXcom.  If not, see <http:///www.gnu.org/licenses/>.
 */

#include <SDL_stdinc.h>

#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleItem.h"

namespace OpenXcom
{

class InteractiveSurface;
class Text;
class Bar;
class NumberText;
class BattleUnit;
class SavedBattleGame;

class VisibleUnitButton
{
  public:
	static const int VISIBLE_MAX = 10;
	static const int SPOTTED_MAX = 10;
	static const int TOTAL_MAX = 10;

	VisibleUnitButton(int const i, int const battleviewX, int const battleviewY, Element const * modElement, SDLKey const key);

	void Enable(int const i, BattleUnit * target, Element const * modElement) noexcept;
	static void Recenter(int screenWidth, int count, VisibleUnitButton **const buttons, Element const * modElement) noexcept;
	void Reset() noexcept;

	InteractiveSurface * GetButton() const noexcept;
	NumberText * GetCaption() const noexcept;
	BattleUnit * GetUnit() const noexcept;
	int GetBackgroundColor() const noexcept;
	int GetBorderColor() const noexcept;
	bool UseShortCycle() const noexcept;
	BattleUnit * GetSpotter(std::vector<BattleUnit *> * units, BattleUnit * selected);

	static bool TargetIsEligible(BattleUnit const * target, VisibleUnitButton ** const visibleUnitButtons, VisibleUnitButton ** const spottedUnitButtons);

	static bool ShareButtonSpace(Element const * a, Element const * b) noexcept;

	static Element ApplyDefaults(Element const * modElement);


  private:
	InteractiveSurface *button_;
	NumberText *caption_;
	BattleUnit *unit_;
	Uint8 textColor_;
	Uint8 backgroundColor_;
	Uint8 borderColor_;
	bool shortCycle_;
};

}
