/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <fstream>
#include <sstream>

#include "link.h"
#include "menu.h"
#include "selector.h"
#include "debug.h"
#include "fonthelper.h"

Link::Link(Esoteric *app, LinkAction action)
	: Button(app->ts, true)
	, app(app)
{
	this->action = action;
	this->edited = false;
	this->iconPath = app->skin->getSkinFilePath("icons/generic.png");
	this->padding = 4;
	this->displayTitle = "";
}

void Link::run() {
	TRACE("calling action");
	this->action();
	TRACE("action called");
}

const std::string &Link::getTitle() {
	return this->title;
}

const std::string &Link::getDisplayTitle() {
	return this->displayTitle;
}

const std::string &Link::getDisplayTitleLine2() {
	return this->displayTitleLine2;
}

void Link::setTitle(const std::string &title) {
	if (title != this->title) {
		this->title = title;
		this->edited = true;

		// Reduce title length to fit the link width
		// TODO :: maybe move to a format function, called after loading in LinkApp
		// and called again after skin column change etc
		std::string temp = std::string(title);
		//temp = StringUtils::strReplace(temp, "-", " "); // Why ban the use of hyphen here??
		std::string::size_type pos = temp.find( "  ", 0 );
		while (pos != std::string::npos) {
			temp = StringUtils::strReplace(temp, "  ", " ");
			pos = temp.find( "  ", 0 );
		};
		int maxWidth = (app->linkWidth);

		// Wrap the title onto up to two lines, breaking only at word
		// boundaries so we never cut a word in half. If it still doesn't
		// fit after two lines, fall back to truncating the second line
		// with ".." like before.
		this->displayTitle = "";
		this->displayTitleLine2 = "";

		if ((int)app->font->getLineWidthSafe(temp) <= maxWidth) {
			// fits on a single line, nothing more to do
			this->displayTitle = temp;
		} else {
			std::vector<std::string> words;
			std::istringstream iss(temp);
			std::string word;
			while (iss >> word) {
				words.push_back(word);
			}

			std::string line1, line2;
			size_t idx = 0;

			// pack as many whole words as possible onto the first line
			while (idx < words.size()) {
				std::string candidate = line1.empty() ? words[idx] : line1 + " " + words[idx];
				if ((int)app->font->getLineWidthSafe(candidate) <= maxWidth) {
					line1 = candidate;
					idx++;
				} else {
					break;
				}
			}

			// edge case: even a single word doesn't fit on an empty line;
			// character-truncate just that word so we never overflow
			if (line1.empty() && idx < words.size()) {
				std::string forced = words[idx];
				while (!forced.empty() && (int)app->font->getLineWidthSafe(forced + "..") > maxWidth) {
					forced = forced.substr(0, forced.length() - 1);
				}
				line1 = forced + "..";
				idx++;
			}

			// pack whatever remains onto the second line
			while (idx < words.size()) {
				std::string candidate = line2.empty() ? words[idx] : line2 + " " + words[idx];
				if ((int)app->font->getLineWidthSafe(candidate) <= maxWidth) {
					line2 = candidate;
					idx++;
				} else {
					break;
				}
			}

			// still more words left over than fit in two lines: truncate
			// the second line with ".." to signal there's more text
			if (idx < words.size()) {
				while (!line2.empty() && (int)app->font->getLineWidthSafe(line2 + "..") > maxWidth) {
					std::string::size_type lastSpace = line2.find_last_of(' ');
					if (lastSpace != std::string::npos) {
						line2 = line2.substr(0, lastSpace);
					} else {
						line2 = line2.substr(0, line2.length() - 1);
					}
				}
				line2 += "..";
			}

			this->displayTitle = line1;
			this->displayTitleLine2 = line2;
		}
	}
}

const std::string &Link::getDescription() {
	return description;
}

void Link::setDescription(const std::string &description) {
	this->description = description;
	edited = true;
}

const std::string &Link::getIcon() {
	return icon;
}

void Link::setIcon(const std::string &icon) {
	this->icon = icon;

	if (icon.compare(0, 5, "skin:") == 0)
		this->iconPath = app->skin->getSkinFilePath(icon.substr(5, std::string::npos));
	else
		this->iconPath = icon;

	edited = true;
}

const std::string &Link::searchIcon() {
	TRACE("enter");
	if (this->iconPath.empty()) {
		this->iconPath = app->skin->getSkinFilePath("icons/generic.png");
	} else if (!FileUtils::fileExists(this->iconPath)) {
		this->iconPath = app->skin->getSkinFilePath("icons/generic.png");
	} else if (!app->skin->getSkinFilePath(this->iconPath).empty()) {
		this->iconPath = app->skin->getSkinFilePath(this->iconPath);
	} else
		this->iconPath = app->skin->getSkinFilePath("icons/generic.png");
	TRACE("exit : %s", this->iconPath.c_str());
	return this->iconPath;
}

const std::string &Link::getIconPath() {
	if (iconPath.empty()) searchIcon();
	return iconPath;
}

void Link::setIconPath(const std::string &icon) {
	if (FileUtils::fileExists(icon))
		iconPath = icon;
	else
		iconPath = app->skin->getSkinFilePath("icons/generic.png");
}
