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
#include "../Engine/Game.h"
#include "../Basescape/CraftInfoState.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleCraftWeapon.h"
#include "../Mod/RuleInterface.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Savegame/CraftWeapon.h"
#include "../Savegame/Transfer.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/SavedGame.h"
#include "../Engine/Options.h"
#include "Globe.h"
#include "../Basescape/BasescapeState.h"
#include "GeoscapeState.h"
#include "GeoscapeCraftState.h"

#include "CraftLiveStatus.h"

namespace OpenXcom
{
	namespace
	{
		Text * initLabel(GeoscapeState * state, Language * language, Font * bigFont, Font * smallFont, Element const * style, std::string const & text)
		{
			if (style && style->w)
			{
				auto element = new Text(style->w, style->h, style->x, style->y);
				state->add(element);
				element->initText(bigFont, smallFont, language);
				element->setSmall();
				element->setColor(style->color);
				element->setText(text);
				return element;
			}
			return new Text(0,0,0,0);
		}

		bool hasCraftLoadOutProblems(Craft * craft)
		{
			if (craft->getNumSoldiers() && craft->getNumEquipment()) return false;
			if (craft->getNumVehicles()) return false;
			auto weaponCount = craft->getNumWeapons();
			auto const w1 = (weaponCount > 0) ? craft->getWeapons()->at(0) : nullptr;
			auto const w2 = (weaponCount > 1) ? craft->getWeapons()->at(1) : nullptr;
			if (w1 && w1->getAmmo()) return false;
			if (w2 && w2->getAmmo()) return false;
			return true;
		}

		std::string getCraftLoadOutProblems(Craft * craft, GeoscapeState * state)
		{
			std::ostringstream ss;
			if (craft->getRules()->getSoldiers())
			{
				if (!craft->getNumSoldiers() && !craft->getNumVehicles())
				{
					ss << state->tr("STR_NO") << " " << state->tr("STR_CREW");
				}
				else if (!craft->getNumEquipment())
				{
					ss << state->tr("STR_NO") << " " << state->tr("STR_EQUIPMENT_UC");
				}
			}
			if (craft->getRules()->getWeapons())
			{
				if (ss.tellp() != std::streampos(0))
				{
					ss << ", ";
				}
				ss << state->tr("STR_NO") << " " << state->tr("STR_AMMO");
			}
			return ss.str();
		}
	}

	CraftLiveStatus::CraftLiveStatus(GeoscapeState * state)
	{
		auto const mod = state->_game->getMod();
		RuleInterface * interface = mod->getInterface("craftLiveStatus");
		if (!interface) return;
		modCraft = interface->getElement("textCraft");
		modFuel = interface->getElement("textFuel");
		modDamage = interface->getElement("textDamage");
		modReady = interface->getElement("textReady");
		modStatus = interface->getElement("textStatus");
		modList = interface->getElement("list");

		if (modStatus->x + modStatus->w > (Options::baseXGeoscape - 63))
		{
			modStatus->w = Options::baseXGeoscape - 63 - modStatus->x;
		}

		if (modList->x + modList->w > (Options::baseXGeoscape - 63))
		{
			modList->w = Options::baseXGeoscape - 63 - modList->x;
		}

		auto const bigFont = mod->getFont("FONT_GEO_BIG");
		auto const smallFont = mod->getFont("FONT_GEO_SMALL");
		auto const language = state->_game->getLanguage();

		std::string fuel = state->tr("STR_FUEL");
		size_t pos = fuel.find('>');
		if (pos != std::string::npos)
		{
			fuel = fuel.substr(0, pos);
		}

		btnToggle_ = new InteractiveSurface(modCraft->w, modCraft->h, modCraft->x, modCraft->y);
		state->add(btnToggle_);
		btnToggle_->onMouseClick((ActionHandler)&GeoscapeState::LiveCraftToggleLeftClick);

		txtName_ = initLabel(state, language, bigFont, smallFont, modCraft, state->tr("STR_CRAFT"));
		txtFuel_ = initLabel(state, language, bigFont, smallFont, modFuel, fuel);
		txtDamage_ = initLabel(state, language, bigFont, smallFont, modDamage, state->tr("STR_DAMAGE_UC"));
		txtReady_ = initLabel(state, language, bigFont, smallFont, modReady, state->tr("STR_READY"));
		txtStatus_ = initLabel(state, language, bigFont, smallFont, modStatus, state->tr("STR_STATUS"));

		lstCraft_ = new TextList(modList->w, modList->h, modList->x, modList->y);
		state->add(lstCraft_);
		lstCraft_->setColor(modList->color);
		lstCraft_->initText(bigFont, smallFont, language);
		lstCraft_->setSmall();
		lstCraft_->setColumns(5, modCraft->w, modFuel->w, modDamage->w, modReady->w, modStatus->w);
		lstCraft_->setSelectable(true);
		lstCraft_->setMargin(modList->value);
		lstCraft_->onMouseClick((ActionHandler)&GeoscapeState::LiveCraftStatusLeftClick);
		lstCraft_->onMouseClick((ActionHandler)&GeoscapeState::LiveCraftStatusRightClick, SDL_BUTTON_RIGHT);
	}

	void CraftLiveStatus::TryAddCraft(Craft * craft, GeoscapeState * state, Transfer const * transfer)
	{
		if (modCraft->value > 0 && crafts_.size() >= modCraft->value) return; // Craft limit

		std::string status = craft->getStatus();

		std::string statusText = GeoscapeCraftState::GetDetailedStatusMessage(craft, state);
		int hours = craft->getHoursUntilReady();

		bool craftUnarmed = false;

		if (transfer)
		{
			hours += transfer->getHours();
			statusText = state->tr("STR_TRANSFER_UC");
		}
		else if (status == "STR_OUT")
		{
			hours += (int)(craft->getBase()->getDistance(craft) / MovingTarget::calculateRadianSpeed(craft->getRules()->getMaxSpeed())) / 3600;
		}
		else if (status == "STR_READY")
		{
			if (modStatus->value != 2) return; // Status filter
			craftUnarmed = hasCraftLoadOutProblems(craft);
            statusText = craftUnarmed
				? getCraftLoadOutProblems(craft, state)
				: (std::string)(state->tr(status));
		}
		else
		{
			if (modStatus->value == 0) return; // Status filter
			statusText = state->tr(status);
		}

		crafts_.push_back(craft);

		std::ostringstream ssHours;
		ssHours << hours << "H";

		lstCraft_->addRow(5,
			craft->getName(state->_game->getLanguage()).c_str(),
			(modFuel->w) ? Unicode::formatNumber(craft->getFuelPercentage()).c_str() : "",
			(modDamage->w && craft->getDamage() > 0) ? Unicode::formatNumber(craft->getDamagePercentage()).c_str() : "",
			(modReady->w && status != "STR_READY") ? ssHours.str().c_str() : "",
			(modStatus->w) ? statusText.c_str() : ""
		);

		auto const row = crafts_.size() - 1;

		if (craftUnarmed)
		{
			lstCraft_->setCellColor(row, 4, modStatus->border);
		}
		else if (status == "STR_READY")
		{
			lstCraft_->setCellColor(row, 4, modStatus->color2);
		}

		if (craft->getLowFuel())
		{
			lstCraft_->setCellColor(row, 1, modFuel->color2);
		}
		else if (craft->getFuel() < craft->getFuelLimit(craft->getBase(), craft->getDestination()))
		{
			lstCraft_->setCellColor(row, 1, modFuel->border);
		}

		auto const damage = craft->getDamagePercentage();
		if (damage > 0)
		{
			auto const color = damage >= modDamage->value ?
				modDamage->color2 : modDamage->border;
			lstCraft_->setCellColor(row, 0, color);
			lstCraft_->setCellColor(row, 2, color);
		}
	}

	void CraftLiveStatus::Think(GeoscapeState * state)
	{
		if (!txtName_) return; // Not inited yet

		crafts_.clear();
		lstCraft_->clearList();
		for (std::vector<Base *>::iterator i = state->_game->getSavedGame()->getBases()->begin(); i != state->_game->getSavedGame()->getBases()->end(); ++i)
		{
			auto base = *i;

			for (std::vector<Craft *>::iterator j = base->getCrafts()->begin(); j != base->getCrafts()->end(); ++j)
			{
				TryAddCraft(*j, state);
			}

			for (std::vector<Transfer *>::iterator j = base->getTransfers()->begin(); j != base->getTransfers()->end(); ++j)
			{
				auto transfer = *j;
				if (transfer->getType() != TransferType::TRANSFER_CRAFT) continue;
				auto craft = transfer->getCraft();
				TryAddCraft(craft, state, transfer);
			}
		}
	}

	void CraftLiveStatus::HandleLeftClick(GeoscapeState * state)
	{
		if (state->buttonsDisabled()) return;
		auto row = lstCraft_->getSelectedRow();
		if (row == -1) return;
		Craft * c = crafts_[row];
		if (c->getStatus() == "STR_READY" || ((c->getStatus() == "STR_OUT" || Options::craftLaunchAlways) && !c->getLowFuel() && !c->getMissionComplete()))
		{
			state->_game->pushState(new GeoscapeCraftState(c, state->_globe, 0));
		}
	}

	void CraftLiveStatus::HandleRightClick(GeoscapeState * state)
	{
		if (state->buttonsDisabled()) return;
		auto row = lstCraft_->getSelectedRow();
		if (row == -1) return;
		Craft * c = crafts_[row];
		if (c->getStatus() == "STR_OUT")
		{
			state->_globe->center(c->getLongitude(), c->getLatitude());
		}
		else
		{
			Base * b = c->getBase();
			state->_globe->center(b->getLongitude(), b->getLatitude());
		}
	}

	void CraftLiveStatus::Toggle(GeoscapeState * state)
	{
		if (state->buttonsDisabled()) return;
		auto visible = !lstCraft_->getVisible();
		lstCraft_->setVisible(visible);
		txtFuel_->setVisible(visible);
		txtDamage_->setVisible(visible);
		txtReady_->setVisible(visible);
		txtStatus_->setVisible(visible);
	}

}
