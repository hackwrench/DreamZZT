/* bomb.cpp - Bomb object
 * Copyright (C) 2000 - 2007 Sam Steele
 *
 * This file is part of DreamZZT.
 *
 * DreamZZT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * DreamZZT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <Tiki/tiki.h>
#include <Tiki/hid.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Tiki;
using namespace Tiki::GL;
using namespace Tiki::Hid;
using namespace Tiki::Audio;

#include "board.h"
#include "object.h"
#include "sound.h"

extern ZZTMusicStream *zm;

extern struct board_info_node *currentbrd;
extern struct world_header world;

void Bomb::message(ZZTObject *them, std::string message) {
	if(them->type()==ZZT_PLAYER && message == "touch" && m_shape==0x0B) {
		m_shape='9';
		m_cycle=10;
		m_flags|=F_PUSHABLE;
		draw();
		if(zm!=NULL)
			zm->setTune("tcf+cf+c");
		if(zm!=NULL)
			zm->start();
	}
}

void Bomb::update() {
	int x,y;
	double a,b;
	ZZTObject *them, *o;

	if(m_shape <= '9' && m_shape >= '1') {
		m_shape--;
		m_cycle=10;
		m_counter=4;
		draw();
		if(m_shape<'9') {
			if(zm!=NULL)
				zm->setTune((m_shape%2==0)?"t5":"t8");
			if(zm!=NULL)
				zm->start();
		}
	}

	if(m_shape=='0' && m_counter>0) {
		m_counter--;
		m_cycle=1;

		if(m_counter==3) {
			if(zm!=NULL)
				zm->setTune("t+++c-c-c-c-c-c");
			if(zm!=NULL)
				zm->start();
		}

		for(y = (int)m_position.y - 4; y <= (int)m_position.y + 4; y++) {
			for(x = (int)m_position.x - 7; x <= (int)m_position.x + 7; x++) {
				a=((x-m_position.x)*(x-m_position.x))/2.0f;
				b=(y-m_position.y)*(y-m_position.y);

				if((int)sqrt(a+b) == 4-m_counter) {
					if(x>=0 && y >= 0 && x<BOARD_X && y<BOARD_Y) {
						them=currentbrd->board[x][y].obj;
						if(them->type()==ZZT_BREAKABLE || ::is_empty(currentbrd,x,y)) {
							o=::create_object(ZZT_EXPLOSION);
							put(o,x,y);
						} else if(them!=NULL) {
							them->message(this,"bombed");
						}
					}
				}
			}
		}
	}
	if(m_counter<=0) {
		remove_from_board(currentbrd,this);
	}
}

void Bomb::create() {
	m_counter=4;
	if(m_start > 0)
		m_shape=m_start + '0';
	else
		m_shape=0x0B;
}

void Explosion::create() {
	m_counter=4;
	m_shape=176;
	m_fg=7;
	m_bg=0;
	m_cycle=1;
	currentbrd->objects.push_back(this);
}

void Explosion::update() {
	switch(m_counter) {
	case 4:
		m_fg=7;
		m_bg=0;
		break;
	case 3:
		m_fg=15;
		m_bg=0;
		break;
	case 2:
		m_fg=14;
		m_bg=0;
		break;
	case 1:
		m_fg=12;
		m_bg=0;
		break;
	case 0:
		m_fg=0;
		m_bg=0;
		remove_from_board(currentbrd,this);
		break;
	}
	m_counter--;
	draw();
}
