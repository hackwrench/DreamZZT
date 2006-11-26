/* main.cpp - DreamZZT: A reimplementation of the ZZT engine
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
#include <Tiki/plxcompat.h>
#include <Tiki/gl.h>
#include <Tiki/hid.h>
#include <Tiki/eventCollector.h>
#include <Tiki/tikitime.h>
#include <Tiki/thread.h>
#include <string.h>
#include <time.h>

#include <Tiki/drawables/console.h>
#include <Tiki/oggvorbis.h>

using namespace Tiki;
using namespace Tiki::GL;
using namespace Tiki::Hid;
using namespace Tiki::Audio;
using namespace Tiki::Thread;

#include "window.h"
#include "sound.h"
#include "board.h"
#include "status.h"
#include "debug.h"
#include "editor.h"
#include "os.h"

Mutex zzt_screen_mutex;

extern struct world_header world;

#define MAIN_MENU "$Welcome to DreamZZT 3.0\r\r\
Please select an option from the\r\
menu below:\r\
\r\
!new;Start a New Game\r\
!restore;Restore a Saved Game\r\
!tutorial;DreamZZT Tutorial\r\
!credits;Credits\r\
!quit;Quit DreamZZT\r\
\r\
(C) 2006 Sam Steele\r"

#define CREDITS "$Programming\r\r\
Sam Steele\r\
http://www.c99.org/dc/\r\r\
$Original ZZT\r\r\
Tim Sweeny - Epic Games\r\
http://www.epicgames.com/\r\r\
$ZZT file format specs\r\r\
Kev Vance\r\
http://www.kvance.com\r\r\
$Tiki\r\r\
Cryptic Allusion, LLC\r\
http://www.cadcdev.com/\r\r\
$Simple DirectMedia Layer - Linux\r\r\
This software may be dynamically linked\r\
to libSDL, an LGPL licensed library.\r\
http://www.libsdl.org/\r\r\
$Testing, Special Thanks and Shoutouts\r\r\
Jason Costa - Necrocosm Software\r\
http://www.necrocosm.com\r\r\
Chris 'Kilokahn' Haslage\r\
http://www.kkwow.net\r\r\
Brian Pinney\r\r\
$DreamZZT is (C) 2006 Sam Steele\r\
$For more information, please visit\r\
$http://www.c99.org/dc/dzzt/\r"

void play_zzt(const char *filename);
void net_menu();

ConsoleText *ct;
extern ConsoleText *dt;
extern struct board_info_node *board_list;
int switchbrd=-1;
extern Player *player;

void menu_background() {
	int x,y;
	ct->color(1,0);
	for(y=0;y<BOARD_Y;y++) {
		for(x=0;x<BOARD_X;x++) {
			ct->locate(x,y);
			ct->printf("%c",177);
		}
	}
}	

#if TIKI_PLAT == TIKI_DC
//extern uint8 romdisk[];
//KOS_INIT_ROMDISK(romdisk);

pvr_init_params_t params = {
	{ PVR_BINSIZE_16, PVR_BINSIZE_8, PVR_BINSIZE_16, PVR_BINSIZE_8, PVR_BINSIZE_0 },
	512*1024,0,0
};
#endif

ZZTMusicStream *zm = NULL;
Tiki::Thread::Thread *render_thread;
Texture *zzt_font;

void render() {
#if 0
	int x,y,w,h;
	if(player!=NULL) {
		w=640;
		h=480;
		x=player->getPosition().x*8*-1 + (256/2) - 4;
		y=player->getPosition().y*20 - (192*2) - 8;
	}
#endif	
	zzt_screen_mutex.lock();
	Frame::begin();
	Frame::transEnable();
#if 0
	if(player!=NULL) {
		zzt_font->select();
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glViewport(x,y,w,h);
		ct->draw(Drawable::Opaque);
		ct->draw(Drawable::Trans);
	}		
	zzt_font->select();
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glViewport(0,192,256/*+74*/,192);
#endif
	ct->draw(Drawable::Opaque);
	dt->draw(Drawable::Opaque);
	ct->draw(Drawable::Trans);
	dt->draw(Drawable::Trans);
	Frame::finish();
	zzt_screen_mutex.unlock();
}

extern "C" int tiki_main(int argc, char **argv) {
	TUIWindow *t, *c;
	srand(time(NULL));
		
	// Init Tiki
	Tiki::init(argc, argv);
	Tiki::setName("DreamZZT", NULL);
	//Hid::callbackReg(tkCallback, NULL);
	
#if TIKI_PLAT == TIKI_DC
	fs_chdir("/pc/users/sam/projects/dreamzzt/resources");
	
	pvr_init(&params);
#endif

	zm = new ZZTMusicStream;
	//zm->setTune("cdefgab+c");
	//zm->start();
	zm->setVolume(0.2);
	
	//initialize the screen		
	zzt_font = new Texture("zzt-ascii.png", true);
	ct = new ConsoleText(80,25, zzt_font);
	ct->setSize(640,480);
	ct->translate(Vector(320,240,0));
	
	debug_init();
	ct->color(15,1);
	ct->clear();
	dzzt_logo();
	menu_background();
	
	//new_world();
	//edit_zzt();
	
	if(argc > 1) {
		play_zzt(argv[argc-1]);
	}
	
	while(switchbrd != -2) {
		ct->color(15,1);
		ct->clear();
		dzzt_logo();
		menu_background();

		t = new TUIWindow("Main Menu");
		t->buildFromString(MAIN_MENU);
		t->doMenu(ct);
		
		if(t->getLabel() == "quit" || t->getLabel() =="\0") {
			break;
		} else if(t->getLabel() == "new") {
			std::string s = os_select_file("Select a game","zzt");
			if(s!="")	play_zzt(s.c_str());
		} else if(t->getLabel() == "tutorial") {
			play_zzt("tutorial.zzt");
		} else if(t->getLabel() == "restore") {
			std::string s = os_select_file("Select a saved game","sav");
			if(s!="")	play_zzt(s.c_str());
		/*} else if(!strcmp(tmp,"net")) {
			net_menu();*/
		} else if(t->getLabel() == "credits") {
			c = new TUIWindow("Credits");
			c->buildFromString(CREDITS);
			c->doMenu(ct);
			
			delete c;
		}
		
		delete t;
	}

	if(zm!=NULL && zm->isPlaying()) zm->stop();
	Tiki::shutdown();
  return 0;
}

extern Player *player;
extern struct board_info_node *currentbrd;
bool gameFrozen;
extern bool playerInputActive;

void titleHidCallback(const Event & evt, void * data) {
	if (evt.type == Hid::Event::EvtQuit) {
		switchbrd = -2;
	}
	if (evt.type == Hid::Event::EvtKeypress && evt.key == 13) {
		switchbrd = world.start;
	}
}	

void play_zzt(const char *filename) {
	int start;
	char tmp[50];
	int hidCookie = Hid::callbackReg(titleHidCallback, NULL);

	gameFrozen = false;
	playerInputActive = true;
	
	switchbrd=-1;
  if(load_zzt(filename,1)==-1) {
		TUIWindow t("Error");
		t.buildFromString("Unable to load world\r\r!ok;Ok");
		t.doMenu(ct);
		return;
	}
	start=world.start;
	switch_board(0);
	remove_from_board(currentbrd,player);
	player=NULL;
	ct->locate(BOARD_X+2,7);
  ct->color(14,1);
	ct->printf("   World: ");
	ct->color(15,1);
	ct->printf("%s",world.title.string);
	ct->locate(BOARD_X+2,9);
#ifdef DREAMCAST
	ct->printf("   Press Start");
#else
	ct->printf("   Press Enter");
#endif
	ct->locate(BOARD_X+2,10);
	ct->printf("    to begin!");
	draw_board();
	do {
		if(!gameFrozen) update_brd();
		draw_board();
		draw_msg();
		render();
		Time::sleep(60000);
	} while(world.saved==0 && switchbrd==-1);
	Hid::callbackUnreg(hidCookie);
	if(switchbrd==-2) return;
	switchbrd=-1;
	free_world();
	load_zzt(filename,0);
	start=world.start;
	ct->color(15,1);
	ct->clear();
	dzzt_logo();
  draw_hud_ingame();
  switch_board(start);
  srand(time(0));
  draw_board();
  player->setFlag(F_SLEEPING);
  player->update();
  while(1) {
    if(!gameFrozen) update_brd();
		draw_board();
    draw_msg();
		render();
		if(world.health>0) Time::sleep(60000);
    
		if(switchbrd>-1) {
      switch_board(switchbrd);
			debug("\x1b[0;37mWarping to \x1b[1;37m%s\n",currentbrd->title);
      draw_board();
			redraw_status();
      if(player->getFlags()&F_SLEEPING) player->update();
      switchbrd=-1;
    } else if(switchbrd==-2) {
			break;
		} else if(switchbrd==-3) {
			//menu
			break;
		} else if(switchbrd==-4) {
			std::string s = os_save_file("Save a game","saved.sav","sav");
			if(s!="") save_game(s.c_str());
			switchbrd=-1;
		} else if(switchbrd==-5) {
			edit_zzt();
			switchbrd=-1;
			ct->color(15,1);
			ct->clear();
			dzzt_logo();
			draw_hud_ingame();
		}
  }
	ct->color(15,1);
	ct->clear();
	dzzt_logo();
	free_world();
	player=NULL;
}

#if 0

void net_menu() {
#ifdef NET
  char *buf=NULL;
  char tmp[50];
  char msg[100];
  char url[100];
  char tmp2[50];
  char title[50];
  char type[100];
  int len=0;
  int x,y,fd;
#ifdef DREAMCAST
	if(c99_net_check()==0) {
		text_window("Error","$Broadband Settings Missing\r\
\r\
This function requires an internet\r\
connection.  Please configure your\r\
Dreamcast with a static IP in Dream\r\
Passport or Broadband Passport and\r\
try again.\r\r\
!ok;Back to main menu\r",tmp,NULL);
		return;
	}		
	ct->color(15,1);
	clear_screen();
	dzzt_logo();
	fs_chdir("/ram");
#endif
#ifdef LINUX
	chdir("/tmp");
#endif
  unlink("online.tmp");
  set_status_callback(status);
  strcpy(tmp,"index.php");
  do {
    ct->color(1,0);
    for(y=0;y<BOARD_Y;y++) {
      for(x=0;x<BOARD_X;x++) {
        ct->locate(x,y);
        ct->printf("%c",177);
      }
    }
    video_refresh();
    ct->locate(0,0);
    ct->color(15,1);
    strcpy(url,"/dzztnet/");
    strcat(url,tmp);
    //ct->printf("%s\n",url);
    //video_refresh();
    http_get_file("online.tmp","www.c99.org",80,url,type,&len);
    if(!strcmp(type,"application/x-zzt-oop")) {
      if(buf!=NULL) free(buf);
      buf=malloc(len+1);
      fd=open("online.tmp",O_RDONLY);
      read(fd,buf,len);
      buf[len]='\0';
      close(fd);
      strcpy(title,"Online Content");
      x=0;
      if(buf[0]=='$') {
        do {
          title[x]=buf[x+1];
          x++;
        } while(buf[x+1]!='\r' && buf[x+1]!='\n');
        title[x]='\0';
        x+=2;
      }
      text_window(title,buf+x,tmp,NULL);
    } else if(!strcmp(type,"application/x-zzt-game")) {
      if(buf!=NULL) free(buf);
      buf=malloc(len+1);
      fd=open("online.tmp",O_RDONLY);
      read(fd,buf,len);
      buf[len]='\0';
      close(fd);
      sprintf(msg,"$Download Complete\r\r\
%s has been downloaded.\r\
Would you like to play it now?\r\r\
!yes;Yes\r\
!no;No\r\r",tmp);
      text_window("File Download",msg,tmp2,NULL);
      if(!strcmp(tmp2,"yes")) {
        play_zzt("online.tmp");
      }
      x=0;
      if(buf[0]=='$') {
        do {
          title[x]=buf[x+1];
          x++;
        } while(buf[x+1]!='\r' && buf[x+1]!='\n');
        title[x]='\0';
        x+=2;
      }
      text_window(title,buf+x,tmp,NULL);
    } else {
      text_window("Error","Invalid URL or invalid file type\r\rPress Start to Continue\r\r",tmp,NULL);
      x=0;
      if(buf[0]=='$') {
        do {
          title[x]=buf[x+1];
          x++;
        } while(buf[x+1]!='\r' && buf[x+1]!='\n');
        title[x]='\0';
        x+=2;
      }
      text_window(title,buf+x,tmp,NULL);
    }
    unlink("online.tmp");
  } while(tmp[0]!='\0');
  free(buf);
#ifdef DREAMCAST
	fs_chdir("/rd");
#endif
#ifdef LINUX
	chdir(PACKAGE_DATA_DIR);
#endif
#endif
}

#endif
