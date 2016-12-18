#include <genesis.h>
#include <time.h>
#include <stdlib.h>

enum direction { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };

enum tile { TIL_NULL, TIL_FLOOR, TIL_WALL, TIL_PLAYER, TIL_GOBLIN };

const u32 tile_1[8]=
{
    0x00111100,
    0x01144110,
    0x11244211,
    0x11244211,
    0x11222211,
    0x11222211,
    0x01122110,
    0x00111100
};

struct Thing {
	int xpos;
	int ypos;
	int hp;
	enum tile til;
};

enum tile maparray[1600];
struct Thing things[32];
struct Thing player;
struct Thing empty;
struct Thing blocker;


void tile_draw(enum tile tilenum, int x, int y) {
	switch (tilenum) {
		case TIL_NULL:
		case TIL_FLOOR: VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL3, 0, 0, 0, 1), x, y); break;
		case TIL_WALL: VDP_setTileMapXY(APLAN, 1, x, y); break;
		case TIL_GOBLIN: VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL2, 0, 1, 0, 1), x, y); break;
		case TIL_PLAYER: VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 1, 0, 1), x, y); break;
	}
}

struct Thing thing_make(enum tile t, int x, int y) {
	struct Thing thing;
	thing.til = t;
	thing.xpos = x;
	thing.ypos = y;
	thing.hp = 5;
	return thing;
}

struct Thing* thing_collide(struct Thing* t, enum direction dir) {
	int xm = 0, ym = 0;
	switch (dir) {
		case DIR_NORTH: ym = -1; break;
		case DIR_EAST: xm = 1; break;
		case DIR_SOUTH: ym = 1; break;
		case DIR_WEST: xm = -1; break; 
	}
	// check wall
	int til_i = ((t->ypos + ym) * 40) + (t->xpos + xm);
	if (maparray[til_i] > TIL_FLOOR)
		return &blocker;
	// check things
	int i = 0;
	for (i = 0; i < 32; ++i) {
		if (things[i].xpos == t->xpos + xm && things[i].ypos == t->ypos + ym)
			return &things[i];
	}
	// check... player?
	if (t->til != TIL_PLAYER && player.xpos == t->xpos + xm && player.ypos == t->ypos + ym)
		return &player;
	// no collision
	return &empty;
}

void thing_destroy(struct Thing* t) {
	tile_draw(TIL_FLOOR, t->xpos, t->ypos);
	*t = thing_make(TIL_NULL, 0, 0);
}

void thing_damage(struct Thing* t, int damage) {
	t->hp -= damage;
	if (t->hp <= 0) thing_destroy(t);
}

void thing_move(struct Thing* t, enum direction d) {
	struct Thing* collided = thing_collide(t, d);
	if (collided->til >= TIL_GOBLIN) {
		thing_damage(collided, 1);
		return;
	}
	else if (collided->til >= TIL_WALL) {
		return;
	}
	switch (d) {
		case DIR_NORTH:
			if (t->ypos <= 0) return;
			tile_draw(t->til, t->xpos, t->ypos - 1);
        	tile_draw(maparray[(t->ypos * 40) + t->xpos], t->xpos, t->ypos);
        	t->ypos--;
        	break;
        case DIR_EAST:
        	if (t->xpos >= 40) return;
        	tile_draw(t->til, t->xpos + 1, t->ypos);
        	tile_draw(maparray[(t->ypos * 40) + t->xpos], t->xpos, t->ypos);
        	t->xpos++;
        	break;
        case DIR_SOUTH:
        	if (t->ypos >= 40) return;
        	tile_draw(t->til, t->xpos, t->ypos + 1);
        	tile_draw(maparray[(t->ypos * 40) + t->xpos], t->xpos, t->ypos);
        	t->ypos++;
        	break;
    	case DIR_WEST:
    		if (t->xpos <= 0) return;
    		tile_draw(t->til, t->xpos - 1, t->ypos);
        	tile_draw(maparray[(t->ypos * 40) + t->xpos], t->xpos, t->ypos);
        	t->xpos--;
        	break;
	}
}

void thing_move_toward(struct Thing* t, int xpos, int ypos) {
	int x_dir = t->xpos - xpos;
	int y_dir = t->ypos - ypos;
	int x_dist = abs(x_dir);
	int y_dist = abs(y_dir);
	if (y_dir > 0 && y_dist >= x_dist) {
		thing_move(t, DIR_NORTH);
	}
	else if (x_dir < 0 && x_dist >= y_dist) {
		thing_move(t, DIR_EAST);
	}
	else if (y_dir < 0 && y_dist >= x_dist) {
		thing_move(t, DIR_SOUTH);
	}
	else if (x_dir > 0 && x_dist >= y_dist) {
		thing_move(t, DIR_WEST);
	}
}

void level_generate_room(int minx, int miny, int maxx, int maxy) {
    int sizex = rand() % maxx - minx;
    int sizey = rand() % maxy - miny;
    int offsx = rand() % maxx - minx - sizex;
    int offsy = rand() % maxy - miny - sizey;
    int x;
    int y;
    for (x = minx + offsx; x < sizex; ++x) {
        for (y = miny + offsy; y < sizey; ++y) {
            maparray[x + (y * 40)] = TIL_FLOOR;
        }   
    }
}

void level_generate() {
    int i = 0;
    for (i = 0; i < 1600; ++i) {
        maparray[i] = TIL_WALL;
    }
    level_generate_room(0, 0, 12, 12);
}

void joypad_handle(u16 joy, u16 changed, u16 state) {
    int turn = 0;
    if (joy == JOY_1) {
        if (state & BUTTON_UP) {
        	thing_move(&player, DIR_NORTH);
        	turn = 1;
        }
        else if (state & BUTTON_RIGHT) {
        	thing_move(&player, DIR_EAST);
        	turn = 1;
        }
        else if (state & BUTTON_DOWN) {
            thing_move(&player, DIR_SOUTH);
            turn = 1;
        }
        else if (state & BUTTON_LEFT) {
        	thing_move(&player, DIR_WEST);
        	turn = 1;
        }
        else if (state & BUTTON_A) {
        	turn = 1;
        }
    }
    if (turn == 1) {
		int m = 0;
		for (m = 0; m < 32; ++m) {
			if (things[m].til > TIL_NULL) {
				thing_move_toward(&things[m], player.xpos, player.ypos);
			}
		}
    }
}

int main() {
    srand(time(NULL));
    JOY_init();
    JOY_setEventHandler(&joypad_handle);

    //we load our unique tile data at position 1 on VRAM
    VDP_loadTileData( (const u32 *)tile_1, 1, 1, 0); 

    empty = thing_make(TIL_NULL, 0, 0);

    int m = 0;
    for (m = 0; m < 32; ++m) {
    	things[m] = thing_make(TIL_NULL, 0, 0);
    }

    things[0] = thing_make(TIL_GOBLIN, 10, 10);
    things[1] = thing_make(TIL_GOBLIN, 25, 25);
    things[2] = thing_make(TIL_GOBLIN, 5, 5);

    blocker = thing_make(TIL_WALL, 0, 0);

    level_generate();

    int x = 0, y = 0, j = 0;
    for (j = 0; j < 1600; ++j) {
    	tile_draw(maparray[j], x, y);
    	++x;
    	if (x >= 40) {
    		x = 0;
    		++y;
    	}
    }

    int k = 0;
    for (k = 0; k < 32; ++k) {
    	if (things[k].til > TIL_NULL) 
    		tile_draw(things[k].til, things[k].xpos, things[k].ypos);
    }

 	player = thing_make(TIL_PLAYER, 0, 0);
    tile_draw(player.til, player.xpos, player.ypos);
    

    while(1) {
        VDP_waitVSync();
    }
    return (0);
}