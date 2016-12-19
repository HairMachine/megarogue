#include <genesis.h>

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
int connections[9] = {
    0, 0, 0,
    0, 0, 0,
    0, 0, 0
};

int in_range(int v, int l, int u) {
    if (v >= l && v <= u) 
        return 1;
    return 0;
}

int gsrand(int min, int max) {
    u16 val = random() & 255;
    //char str[15];
    //sprintf(str, "%d %d %d", val, min, max);
    //VDP_drawText(str, 5, 0);
    while (in_range(val, min, max) == 0)    
        val = random() & 255;
    return val;
}

void tile_draw(enum tile tilenum, int x, int y) {
    //return;
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
    int sizex = 0;
    int sizey = 0;
    while (sizex - minx < 3 || sizey - miny < 3) {
        sizex = gsrand(minx, maxx);
        sizey = gsrand(miny, maxy);
    }
    int offsx = gsrand(0, maxx - sizex);
    int offsy = gsrand(0, maxy - sizey);
    //int offsx = 0;
    //int offsy = 0;
    int x;
    int y;
    //char str[15];
    //VDP_drawText(sprintf(str, "%d", sizex), 5, 10);
    for (x = minx + offsx; x < sizex + offsx; ++x) {
        for (y = miny + offsy; y < sizey + offsy; ++y) {
            maparray[x + (y * 40)] = TIL_FLOOR;
        }   
    }
}

int room_open_connections(int x, int y) {
    int c = 0;
    if (y - 1 > 0 && connections[(y - 1) * 3 + x] == 0)
        c++;
    if (x + 1 < 3 && connections[y * 3 + (x + 1)] == 0)
        c++;
    if (y + 1 < 3 && connections[(y + 1) * 3 + x] == 0)
        c++;
    if (x - 1 > 0 && connections[y * 3 + (x - 1)] == 0)
        c++;
    return c;
}

int level_unconnected_rooms() {
    int x, y;
    int c = 0;
    for (x = 0; x < 3; ++x) {
        for (y = 0; y < 3; ++y) {
            if (connections[y * 3 + x] == 0)
                ++c;
        }
    }
    return c;
}

struct vect2d {
    int x;
    int y;
};

struct vect2d level_get_first_unconnected() {
    int x, y;
    struct vect2d ntc = {-1, -1};
    for (x = 0; x < 3; ++x) {
        for (y = 0; y < 3; ++y) {
            if (connections[y * 3 + x] == 0) {
                ntc.x = x;
                ntc.y = y;
                return ntc;
            }
        }
    }
    return ntc;
}

void level_connect_rooms(int rtcx, int rtcy, int rtctx, int rtcty) {
    // Set starting tiles for connection; has to be a floor
    int tsx = 0;
    int tsy = 0;
    int ts = 0;
    while (maparray[ts] != TIL_FLOOR) {
        tsx = gsrand(rtcx * 12 + 1, rtcx * 12 + 12);
        tsy = gsrand(rtcy * 12 + 1, rtcy * 12 + 12);
        ts = tsy * 40 + tsx;
    }  
    
    int tex = 0;
    int tey = 0;
    int te = 0;
    while (maparray[te] != TIL_FLOOR) {
        tex = gsrand(rtctx * 12 + 1, rtctx * 12 + 12);
        tey = gsrand(rtcty * 12 + 1, rtcty * 12 + 12);
        te = tey * 40 + tex;
    }

    int c = 0;
    if (tsy < tey) {
        for (c = tsy; c < tey; c++) {
            maparray[c * 40 + tsx] = TIL_FLOOR;
        }
    }
    else if (tsy > tey) {
        for (c = tsy; c > tey; c--) {
            maparray[c * 40 + tsx] = TIL_FLOOR;
        }
    }
    if (tsx < tex) {
       for (c = tsx; c < tex; c++) {
            maparray[tey * 40 + c] = TIL_FLOOR;
        } 
    }
    else if (tsx > tex) {
        for (c = tsx; c > tex; c--) {
            maparray[tey * 40 + c] = TIL_FLOOR;
        }
    }

    connections[rtcy * 3 + rtcx] = 1;
}

void level_generate() {
    int i = 0;
    for (i = 0; i < 1600; ++i) {
        maparray[i] = TIL_WALL;
    }
    level_generate_room(1 , 1 , 12, 12);
    level_generate_room(13, 1 , 24, 12);
    level_generate_room(25, 1 , 36, 12);
    level_generate_room(1 , 13, 12, 24);
    level_generate_room(13, 13, 24, 24);
    level_generate_room(25, 13, 36, 24);
    level_generate_room(1 , 25, 12, 36);
    level_generate_room(13, 25, 24, 36);
    level_generate_room(25, 25, 36, 36);

    int adjacent[4] = {0, 0, 0, 0};

    int rtcx, rtcy, rtctx, rtcty, dir;

    int lol = 0;
    int lil = 1;

    rtcx = gsrand(0, 2);
    rtcy = gsrand(0, 2);
    /*while (room_open_connections(rtcx, rtcy) == 0) {
        rtcx = gsrand(0, 2);
        rtcy = gsrand(0, 2);
    }*/
    
    rtctx = 3;
    rtcty = 3;

    // First step: step through, joining rooms then using that room to make a new join.
    // Eventually, we get into a dead end, so stop and go on to phase 2 of the generator.
    int failsafe = 0;

    while(1) {

        // for some ungodly reason, doing this in the while condition itself does not work.
        if (room_open_connections(rtcx, rtcy) == 0 || failsafe > 10) break;
        
        // make sure the connection is valid
        while (rtctx > 2 || rtcty > 2 || rtctx < 0 || rtcty < 0 || connections[rtcty * 3 + rtctx] == 1) {
            dir = gsrand(0, 3);
            switch (dir) {
                case 0: rtcty = rtcy - 1; rtctx = rtcx; break;
                case 1: rtctx = rtcx + 1; rtcty = rtcy; break;
                case 2: rtcty = rtcy + 1; rtctx = rtcx; break;
                case 3: rtctx = rtcx - 1; rtcty = rtcy; break;
            }
        }

        level_connect_rooms(rtcx, rtcy, rtctx, rtcty);

        rtcx = rtctx;
        rtcy = rtcty;
        rtctx = 3;
        rtcty = 3;

        failsafe++;

    }

    //Second part: select randomly an unjoined room, and join to an adjacent room, until no more unjoined rooms exist.
    struct vect2d ntc;
    int wtf = 0;
    while (1) {
        ntc = level_get_first_unconnected();
        if (ntc.x == -1) break;

        char str[15];
        sprintf(str, "%d, %d", ntc.x, ntc.y);
        VDP_drawText(str, 5, wtf);
        VDP_drawText("    ", 5, wtf+2);

        rtcx = ntc.x;
        rtcy = ntc.y;
        rtctx = 3;
        rtcty = 3;
        while (rtctx > 2 || rtcty > 2 || rtctx < 0 || rtcty < 0) {
            dir = gsrand(0, 3);
            switch (dir) {
                case 0: rtcty = rtcy - 1; rtctx = rtcx; break;
                case 1: rtctx = rtcx + 1; rtcty = rtcy; break;
                case 2: rtcty = rtcy + 1; rtctx = rtcx; break;
                case 3: rtctx = rtcx - 1; rtcty = rtcy; break;
            }
        }

        level_connect_rooms(rtcx, rtcy, rtctx, rtcty);

        sprintf(str, "%d, %d, %d, %d", rtcx, rtcy, rtctx, rtcty);
        VDP_drawText(str, 5, wtf+1);

        VDP_drawText("Blah", 5, wtf+2);

        wtf += 3;
    }

    VDP_drawText("DONE!", 5, wtf+3);

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
    JOY_init();
    JOY_setEventHandler(&joypad_handle);
    
    // Temp: randomise
    int rs;
    for (rs = 0; rs < 123; ++rs) {
        random();
    }

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