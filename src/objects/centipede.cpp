/* centipede.cpp - Centipede objects
 * Copyright (c) 2000-2006 Sam Steele
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
#include <stdio.h>
#include <stdlib.h>

using namespace Tiki;
using namespace Tiki::Hid;
using namespace Tiki::Audio;

#include "object.h"
#include "board.h"
#include "status.h"
#include "sound.h"

extern ZZTMusicStream *zm;

extern struct board_info_node *currentbrd;

char cgun[5] = {'0',27,26,24,25};

void Centipede::create() {
	m_discovery = true;
}

void Centipede::update() {
	if(m_type==ZZT_CENTIPEDE_HEAD) {
		if(m_discovery) {
			discover(NULL);
			if(m_next!=NULL) {
				m_heading = m_nextHeading = opposite(toward(m_next));
			} else {
				m_heading = m_nextHeading = direction((rand() % 4) + 1);
			}
		}
		doMove(m_heading);
	} else {
		if(!m_discovery && m_prev==NULL) { //become a head
			m_type = ZZT_CENTIPEDE_HEAD;
			m_heading = m_nextHeading = direction((rand() % 4) + 1);
			m_shape = ZZT_CENTIPEDE_HEAD_SHAPE;
		}
	}
	m_discovery = false;
	
	//m_shape = cgun[(int)m_nextHeading];
}

void Centipede::discover(Centipede *prev) {
	int x,y;
	
	if(prev!=NULL) m_heading=m_nextHeading=toward(prev);
	m_prev=prev;
					
	for(y=m_position.y-1; y<=m_position.y+1; y++) {
		for(x=m_position.x-1; x<=m_position.x+1; x++) {
			if((x==m_position.x || y==m_position.y) && currentbrd->board[x][y].obj != NULL && currentbrd->board[x][y].obj != prev && currentbrd->board[x][y].obj != this && currentbrd->board[x][y].obj->getType() == ZZT_CENTIPEDE_BODY && !((Centipede *)currentbrd->board[x][y].obj)->isDiscovered()) {
				m_next=(Centipede *)(currentbrd->board[x][y].obj);
				((Centipede *)currentbrd->board[x][y].obj)->discover(this);
				return;
			}
		}
	}
}

void Centipede::doMove(direction d) {
	if(move(m_nextHeading)) {
		m_heading = m_nextHeading;
		m_nextHeading = d;
		if(m_next!=NULL && m_next->isValid()) m_next->doMove(m_heading);
	}
	
	if(m_next != NULL && !m_next->isValid()) {
		m_next = NULL;
	}
	
	if(m_prev != NULL && !m_prev->isValid()) {
		m_prev = NULL;
	}
}

void Centipede::message(ZZTObject *them, std::string msg) {
	if(m_type == ZZT_CENTIPEDE_HEAD && msg=="thud") {
		m_heading = m_nextHeading = clockwise(m_heading);
		if(rand() % 2 == 1) {
			m_heading = m_nextHeading = opposite(m_heading);
		}
		if(!is_empty(m_heading) && is_empty(opposite(m_heading))) {
			m_heading = m_nextHeading = opposite(m_heading);
		}			
		if(is_empty(m_heading)) {
			doMove(m_heading);
		} else {
			reverse();
			m_type = ZZT_CENTIPEDE_BODY;
		}
	} else if(msg == "thud" || msg == "shot" || msg == "bombed") {
		if(m_prev != NULL) m_prev->unlinkNext();
		if(m_next != NULL) m_next->unlinkPrev();
		Enemy::message(them,msg);
	}
}