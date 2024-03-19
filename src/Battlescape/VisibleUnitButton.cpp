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

#include "BattlescapeState.h"
#include "../Engine/InteractiveSurface.h"
#include "../Interface/NumberText.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/BattleUnit.h"

#include "VisibleUnitButton.h"

namespace OpenXcom
{
	Element VisibleUnitButton::ApplyDefaults(Element const * modElement)
	{
		return Element(300, -16, 15, 12, 16, 32, 15, false, 270, modElement);
	}

	VisibleUnitButton::VisibleUnitButton(int const index, int const battleviewX, int const battleviewY, Element const * modElement, SDLKey const key)
	{
		// battleviewX is the left edge (always 0?)
		// battleviewY is the bottom edge (144?)

		Element style = ApplyDefaults(modElement);

		static int const margin = 1;

		int thisX = 0;
		int thisY = 0;
		switch (style.value) // Direction of placement of subsequent buttons
		{
		case -1: // Centered, Left-to-right
		case 0: // East / Left-to-right
			thisX = battleviewX + style.x + (index * (style.w + margin));
			thisY = battleviewY + style.y;
			break;
		case 90: // South / Top-to-bottom
			thisX = battleviewX + style.x;
			thisY = battleviewY + style.y + (index * (style.h + margin));
			break;
		case 180: // West / Right-to-left
			thisX = battleviewX + style.x - (index * (style.w + margin));
			thisY = battleviewY + style.y;
			break;
		case 270: // North / Bottom-to-top
		default:
			thisX = battleviewX + style.x;
			thisY = battleviewY + style.y - (index * (style.h + margin));
			break;
		}

		button_ = new InteractiveSurface(style.w, style.h, thisX, thisY);
		caption_ = new NumberText(style.w, style.h, thisX + 6, thisY + 4);
		unit_ = nullptr;

		button_->onMouseClick((ActionHandler)&BattlescapeState::btnVisibleUnitClick);
		button_->onKeyboardPress((ActionHandler)&BattlescapeState::btnVisibleUnitClick, key);
		button_->onMouseIn((ActionHandler)&BattlescapeState::txtTooltipIn);
		button_->onMouseOut((ActionHandler)&BattlescapeState::txtTooltipOut);
	}

	void VisibleUnitButton::Enable(int const index, BattleUnit * target, Element const * modElement) noexcept
	{
		Element style = ApplyDefaults(modElement);

		std::ostringstream tooltip;
		tooltip << "STR_CENTER_ON_ENEMY_" << index;
		button_->setTooltip(tooltip.str());
		caption_->setValue(index);
		caption_->setX(button_->getX() + ((index < 10) ? 6 : 4)); // Center digits

		button_->setVisible(true);
		caption_->setVisible(true);
		unit_ = target;
		caption_->setColor(style.color);
		backgroundColor_ = style.color2;
		borderColor_ = style.border;
		shortCycle_ = style.TFTDMode;
	}

	void VisibleUnitButton::Reset() noexcept
	{
		button_->setVisible(false);
		caption_->setVisible(false);
		unit_ = nullptr;
	}

	void VisibleUnitButton::Recenter(int screenWidth, int count, VisibleUnitButton ** const buttons, Element const * modElement) noexcept
	{
		Element style = ApplyDefaults(modElement);

		if (style.value != -1) return;

		static int const margin = 1;

		int buttonBarWidth = (count * style.w) + ((count - 1) * margin);
		int x = (screenWidth / 2) - (buttonBarWidth / 2);

		for (int index = 0; index < count; ++index)
		{
			VisibleUnitButton * button = buttons[index];
			button->GetButton()->setX(x + (index * (style.w + margin)));
			button->GetCaption()->setX(button->GetButton()->getX() + ((button->GetCaption()->getValue() < 10) ? 6 : 4));
		}
	}

	InteractiveSurface * VisibleUnitButton::GetButton() const noexcept
	{
		return button_;
	}

	NumberText * VisibleUnitButton::GetCaption() const noexcept
	{
		return caption_;
	}

	BattleUnit * VisibleUnitButton::GetUnit() const noexcept
	{
		return unit_;
	}

	int VisibleUnitButton::GetBackgroundColor() const noexcept
	{
		return backgroundColor_;
	}

	int VisibleUnitButton::GetBorderColor() const noexcept
	{
		return borderColor_;
	}

	bool VisibleUnitButton::UseShortCycle() const noexcept
	{
		return shortCycle_;
	}

	bool VisibleUnitButton::TargetIsEligible(BattleUnit const * target, VisibleUnitButton ** const visibleUnitButtons, VisibleUnitButton ** const spottedUnitButtons)
	{
		if (target->isOut() ||                         // Must be alive & awake
			target->getFaction() != FACTION_HOSTILE || // Must be an enemy
			!target->getVisible())                     // Must have been revealed to player
		{
			return false;
		}

		// Must not already have a button
		for (int i = 0; i < VISIBLE_MAX; ++i) if (visibleUnitButtons[i]->GetUnit() == target) return false;
		for (int i = 0; i < SPOTTED_MAX; ++i) if (spottedUnitButtons[i]->GetUnit() == target) return false;

		return true;
	}

	bool VisibleUnitButton::ShareButtonSpace(Element const * a, Element const * b) noexcept
	{
		Element aa = ApplyDefaults(a);
		Element bb = ApplyDefaults(b);
		return aa.x == bb.x && aa.y == bb.y;
	}

}
