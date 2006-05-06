#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "object.h"
#include "board.h"

struct object object_templates[ZZT_TYPE_COUNT];

void null_handler() {}

int id_pool=0;

#include "objdb" //autogenerated function

int str_to_color(char *color) {
  if(!strcmp(color,"black")) {
    return 0;
  } else if(!strcmp(color,"blue")) {
    return 9;
  } else if(!strcmp(color,"green")) {
    return 10;
  } else if(!strcmp(color,"cyan")) {
    return 11;
  } else if(!strcmp(color,"red")) {
    return 12;
  } else if(!strcmp(color,"purple")) {
    return 13;
  } else if(!strcmp(color,"yellow")) {
    return 14;
  } else if(!strcmp(color,"white")) {
    return 15;
  } else {
    return -1;
  }
}

char *int_to_color(int col) {
  switch(col) {
  case 0:
  case 8:
    return "black";
    break;
  case 1:
  case 9:
    return "blue";
    break;
  case 2:
  case 10:
    return "green";
    break;
  case 3:
  case 11:
    return "cyan";
    break;
  case 4:
  case 12:
    return "red";
    break;
  case 5:
  case 13:
    return "purple";
    break;
  case 6:
  case 14:
    return "yellow";
    break;
  case 7:
  case 15:
    return "white";
    break;
  }
  return NULL;
}

int str_to_obj(char *str) {
  int x;

  for(x=0;x<ZZT_TYPE_COUNT;x++) {
    if(object_templates[x].name!=NULL && !strcmp(str,object_templates[x].name)) {
      return x;
    }
  }
  return -1;
}

struct object *create_object(int type, int x, int y) {
  //create an object based on a template from object_templates[]
  struct object *current=(struct object *)malloc(sizeof(struct object));
  current->x=x;
  current->y=y;
  current->xstep=0;
  current->ystep=0;
  current->heading=5;
  current->type=type;
  current->cycle=1;
  current->arg1=0;
  current->arg2=0;
  current->arg3=0;
  current->arg4=0;
  current->arg5=0;
  current->tick=0;
  current->flags=object_templates[type].flags;
  current->shape=object_templates[type].shape;
  current->update=object_templates[type].update;
  current->message=object_templates[type].message;
  current->create=object_templates[type].create;
  current->fg=object_templates[type].fg;
  current->bg=object_templates[type].bg;
  current->color=&current->fg;
  current->name=object_templates[type].name;
  current->updated=1;
	current->prog=NULL;
	current->progpos=0;

	return current;
}

void set_name(struct object *obj, char *name) {
  if(obj->name!=NULL) {
    free(obj->name);
  }
  obj->name=(char *)malloc(strlen(name)+1);
  strcpy(obj->name,name);
}

update_handler default_update(int me) {
  //default update handler
  //struct aoe_object *myobj=get_object(me);
  //myobj->x+=myobj->move_x;
  //myobj->y+=myobj->move_y;
}

msg_handler default_msg(int me, int them, char *message) {
  //struct object *myobj=get_obj_by_id(me);
  //if(!strcmp(message,"thud")) {
    //obj->collide(me,them,them);
  //}
  //if(!strcmp(message,"shot")) {
    //myobj->shot(me,them);
  //}
}
