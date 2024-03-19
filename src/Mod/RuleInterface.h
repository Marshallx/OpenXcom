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
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

struct Element
{
	/// basic rect info, and 3 colors.
	int x, y, w, h, color, color2, border;
	/// defines inversion behaviour
	bool TFTDMode;
	// general purpose value
	int value;

	// Create an element with the specified values and (optionally) overwrite
	// the values with those from the supplied other element iff the other
	// element's values are set.
	Element(int x = 0, int y = 0, int w = 0, int h = 0, int color = 0,
		int color2 = 0, int border = 0, bool TFTDMode = false, int value = 0,
		Element const * other = nullptr);
};

class RuleInterface
{
private:
	std::string _type;
	std::string _palette;
	std::string _parent;
	std::string _music;

	std::map <std::string, Element> _elements;
public:
	/// Constructor.
	RuleInterface(const std::string & type);
	/// Destructor.
	~RuleInterface();
	/// Load from YAML.
	void load(const YAML::Node& node);
	/// Get an element.
	Element *getElement(const std::string &id);
	/// Get palette.
	const std::string &getPalette() const;
	/// Get parent interface rule.
	const std::string &getParent() const;
	/// Get music.
	const std::string &getMusic() const;
};

}
