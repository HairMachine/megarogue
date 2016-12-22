#include <genesis.h>

enum direction { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };

enum tile {
	TIL_NULL, TIL_FLOOR, TIL_WALL, TIL_PLAYER, TIL_GOBLIN, TIL_STAIRS, TIL_MACGUFFIN,
	TIL_WPN, TIL_POTION, TIL_SCROLL
};

const u32 tile_wall[8] = {
    0x11111111,
		0x11111111,
		0x11111111,
		0x11111111,
		0x11111111,
		0x11111111,
		0x11111111,
		0x11111111
};

const u32 tile_floor[8] = {
		0x11111111,
		0x10000000,
		0x10000000,
		0x10000000,
		0x10000000,
		0x10000000,
		0x10000000,
		0x10000000
};

const u32 tile_stairs[8] = {
		0x00000000,
		0x00044000,
		0x00000000,
		0x00444400,
		0x00000000,
		0x04444440,
		0x00000000,
		0x44444444
};

const u32 tile_hero[8] = {
		0x00343400,
		0x03344430,
		0x00560600,
		0x00566650,
		0x03344450,
		0x06333330,
		0x00300300,
		0x00000000
};

const u32 tile_wpn[8] = {
		0x00000000,
		0x00060000,
		0x00055000,
		0x00055000,
		0x00055000,
		0x00444400,
		0x00033000,
		0x00033000
};

const u32 tile_ptn[8] = {
		0x00000000,
		0x00222000,
		0x00333000,
		0x00333000,
		0x03433300,
		0x03333300,
		0x00333000,
		0x00000000
};

const u32 tile_scroll[8] = {
		0x00000000,
		0x0AAAAAA0,
		0x0A3A33A0,
		0x0A2A2AA0,
		0x0A3333A0,
		0x0AA22AA0,
		0x0AAAAAA0,
		0x00000000,
};


enum FLAGS { FL_NONE = 0, FL_MOVES = 1, FL_PASSTHRU = 2};

struct Thing {
	int xpos;
	int ypos;
	int hp;
	enum FLAGS flags;
	enum tile til;
	int damage;
};

struct vect2d {
	int x;
	int y;
};

const int mapsize = 28;
const int maparraysize = 784;
const int roomsize = 8;
enum tile maparray[784];
struct Thing things[32];
struct Thing player;
struct Thing empty;
struct Thing blocker;
int connections[9] = {
    0, 0, 0,
    0, 0, 0,
    0, 0, 0
};
int depth = 0;
int maxdepth = 15;
void level_generate();
void thing_interact(struct Thing* subj, struct Thing* obj);
struct Equipment player_e = {WP_UNARMED, AR_NONE};

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
		case TIL_FLOOR:
			VDP_setTileMapXY(APLAN, 3, x, y);
			break;
		case TIL_WALL:
			VDP_setTileMapXY(APLAN, 1, x, y);
			break;
		case TIL_GOBLIN:
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL2, 0, 1, 0, 1), x, y);
			break;
		case TIL_PLAYER:
			VDP_setTileMapXY(APLAN, 4, x, y);
			break;
		case TIL_STAIRS:
			VDP_setTileMapXY(APLAN, 2, x, y);
			break;
		case TIL_MACGUFFIN:
			VDP_setTileMapXY(APLAN, 1, x, y);
			break;
		case TIL_WPN:
			VDP_setTileMapXY(APLAN, 5, x, y);
			break;
		case TIL_POTION:
			VDP_setTileMapXY(APLAN, 6, x, y);
			break;
		case TIL_SCROLL:
			VDP_setTileMapXY(APLAN, 7, x, y);
			break;
	}
}

void redraw_tiles() {
	int x = 0, y = 0, j = 0;
	for (j = 0; j < maparraysize; ++j) {
		tile_draw(maparray[j], x, y);
		++x;
		if (x >= mapsize) {
			x = 0;
			++y;
		}
	}
}

void redraw_things() {
	int k = 0;
	for (k = 0; k < 32; ++k) {
		if (things[k].til > TIL_NULL)
			tile_draw(things[k].til, things[k].xpos, things[k].ypos);
	}
	// and the player on top
	tile_draw(player.til, player.xpos, player.ypos);
}

void draw_health() {
	char msg[15];
	sprintf(msg, "HP: %d ", player.hp);
	VDP_drawText(msg, 30, 0);
}

void draw_depth() {
	char msg[15];
	sprintf(msg, "Depth: %d ", depth + 1);
	VDP_drawText(msg, 30, 1);
}

void screen_game() {
	// main screen
	redraw_tiles();
	redraw_things();
	tile_draw(player.til, player.xpos, player.ypos);
	// sidebar
	draw_health();
	draw_depth();
}

void screen_victory() {
	VDP_clearPlan(0, 1);
	VDP_drawText("You are winned! :O", 0, 0);
}

void screen_dead() {
	VDP_clearPlan(0, 1);
	VDP_drawText("You have dead D:", 0, 0);
}

struct vect2d position_find_valid() {
	struct vect2d pos = {-1, -1};
	int xp = gsrand(0, mapsize);
	int yp = gsrand(0, mapsize);
	while (maparray[yp * mapsize + xp] != TIL_FLOOR) {
		xp = gsrand(0, mapsize);
		yp = gsrand(0, mapsize);
	}
	pos.x = xp;
	pos.y = yp;
	return pos;
}

struct Thing thing_make(enum tile t, int x, int y) {
	struct Thing thing;
	thing.til = t;
	thing.xpos = x;
	thing.ypos = y;
	switch (t) {
		case TIL_PLAYER:
			thing.hp = 20;
			thing.damage = 1;
			break;
		case TIL_GOBLIN:
			thing.hp = 5;
			thing.flags = FL_MOVES;
			thing.damage = 1;
			break;
		case TIL_WALL:
			thing.flags = FL_NONE;
			break;
		default:
			thing.flags = FL_PASSTHRU;
			break;
	}
	return thing;
}

struct Thing thing_put(enum tile t) {
	struct vect2d ppos = position_find_valid();
	return thing_make(t, ppos.x, ppos.y);
}

void things_generate() {
	int i;
	int max_m = depth + 3;
	if (max_m > 15) max_m = 15;
	int max_i = depth + 17;
	if (max_i > 27) max_i = 27;
	// first loop: monsters.
	for (i = 0; i < max_m; ++i) {
		things[i] = thing_put(TIL_GOBLIN);
	}
	// second loop: items.
	for (i = 15; i < max_i; ++i) {
		things[i] = thing_put(TIL_POTION);
	}
	// third loop: permanent power ups.
	for (i = 27; i < 31; ++i) {
		// atm, nothing
	}

	// finally the stairs or macguffin on last level
	if (depth < maxdepth)
		things[31] = thing_put(TIL_STAIRS);
	else
		things[31] = thing_put(TIL_MACGUFFIN);
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
	int til_i = ((t->ypos + ym) * mapsize) + (t->xpos + xm);
	if (maparray[til_i] > TIL_FLOOR)
		return &blocker;
	// check things
	int i = 0;
	for (i = 0; i < 32; ++i) {
		if (things[i].xpos == t->xpos + xm && things[i].ypos == t->ypos + ym) {
			return &things[i];
		}
	}
	// check player
	if (t->til != TIL_PLAYER && player.xpos == t->xpos + xm && player.ypos == t->ypos + ym) {
		return &player;
	}
	// no collision
	return &empty;
}

void thing_destroy(struct Thing* t) {
	if (t->til == TIL_PLAYER)
		screen_dead();
	tile_draw(TIL_FLOOR, t->xpos, t->ypos);
	*t = thing_make(TIL_NULL, 0, 0);
}

void thing_damage(struct Thing* t, int damage) {
	t->hp -= damage;
	if (t->til == TIL_PLAYER)
		draw_health();
	if (t->hp <= 0) thing_destroy(t);
}

void thing_move(struct Thing* t, enum direction d) {
	struct Thing* collided = thing_collide(t, d);
	// This looks weird but is for a reason: if a blocking thing is destroyed by an interaction we still want it to act
	// as a blocker
	if (!(collided->flags & FL_PASSTHRU)) {
		thing_interact(t, collided);
		return;
	}
	else thing_interact(t, collided);
	// Move the object
	switch (d) {
		case DIR_NORTH:
			if (t->ypos <= 0) return;
			tile_draw(t->til, t->xpos, t->ypos - 1);
        	tile_draw(maparray[(t->ypos * mapsize) + t->xpos], t->xpos, t->ypos);
        	t->ypos--;
        	break;
        case DIR_EAST:
        	if (t->xpos >= mapsize) return;
        	tile_draw(t->til, t->xpos + 1, t->ypos);
        	tile_draw(maparray[(t->ypos * mapsize) + t->xpos], t->xpos, t->ypos);
        	t->xpos++;
        	break;
        case DIR_SOUTH:
        	if (t->ypos >= mapsize) return;
        	tile_draw(t->til, t->xpos, t->ypos + 1);
        	tile_draw(maparray[(t->ypos * mapsize) + t->xpos], t->xpos, t->ypos);
        	t->ypos++;
        	break;
    	case DIR_WEST:
    		if (t->xpos <= 0) return;
    		tile_draw(t->til, t->xpos - 1, t->ypos);
        	tile_draw(maparray[(t->ypos * mapsize) + t->xpos], t->xpos, t->ypos);
        	t->xpos--;
        	break;
	}

	redraw_things();
}

void thing_move_toward(struct Thing* t, int xpos, int ypos) {
	if (!(t->flags & FL_MOVES))
		return;

	int x_dir = t->xpos - xpos;
	int y_dir = t->ypos - ypos;
	int x_dist = abs(x_dir);
	int y_dist = abs(y_dir);
	// If the way is blocked, try and use the other direction
	if (y_dist >= x_dist) {
		if (y_dir > 0 && thing_collide(t, DIR_NORTH)->til == TIL_WALL) {
			x_dist = y_dist + 1;
		}
		else if (y_dir < 0 && thing_collide(t, DIR_SOUTH)->til == TIL_WALL) {
			x_dist = y_dist + 1;
		}
	}
	if (x_dist >= y_dist) {
		if (x_dir > 0 && thing_collide(t, DIR_WEST)->til == TIL_WALL) {
			y_dist = x_dist + 1;
		}
		else if (x_dir < 0 && thing_collide(t, DIR_EAST)->til == TIL_WALL) {
			y_dist = x_dist + 1;
		}
	}
	// Move
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

void thing_interact(struct Thing* subj, struct Thing* obj) {
	switch (obj->til) {
		case TIL_GOBLIN:
		case TIL_PLAYER:
			thing_damage(obj, subj->damage);
			break;
		case TIL_STAIRS:
			if (subj->til == TIL_PLAYER) {
				++depth;
				level_generate();
			}
			else {
				thing_destroy(subj);
			}
			break;
		case TIL_MACGUFFIN:
			if (subj->til == TIL_PLAYER)
				screen_victory();
			break;
		case TIL_POTION:
			thing_damage(subj, -5);
			thing_destroy(obj);
			tile_draw(subj->til, subj->xpos, subj->ypos);
			break;
		default:
			break;
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
    int x;
    int y;
    for (x = minx + offsx; x < sizex + offsx; ++x) {
        for (y = miny + offsy; y < sizey + offsy; ++y) {
            maparray[x + (y * mapsize)] = TIL_FLOOR;
        }
    }
}

int room_open_connections(int x, int y) {
    int c = 0;
    if (y - 1 >= 0 && connections[(y - 1) * 3 + x] == 0)
        c++;
    if (x + 1 < 3 && connections[y * 3 + (x + 1)] == 0)
        c++;
    if (y + 1 < 3 && connections[(y + 1) * 3 + x] == 0)
        c++;
    if (x - 1 >= 0 && connections[y * 3 + (x - 1)] == 0)
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
        tsx = gsrand(rtcx * roomsize + 1, rtcx * roomsize + roomsize);
        tsy = gsrand(rtcy * roomsize + 1, rtcy * roomsize + roomsize);
        ts = tsy * mapsize + tsx;
    }

    int tex = 0;
    int tey = 0;
    int te = 0;
    while (maparray[te] != TIL_FLOOR) {
        tex = gsrand(rtctx * roomsize + 1, rtctx * roomsize + roomsize);
        tey = gsrand(rtcty * roomsize + 1, rtcty * roomsize + roomsize);
        te = tey * mapsize + tex;
    }

    int c = 0;
    if (tsy < tey) {
        for (c = tsy; c < tey; c++) {
            maparray[c * mapsize + tsx] = TIL_FLOOR;
        }
    }
    else if (tsy > tey) {
        for (c = tsy; c > tey; c--) {
            maparray[c * mapsize + tsx] = TIL_FLOOR;
        }
    }
    if (tsx < tex) {
       for (c = tsx; c < tex; c++) {
            maparray[tey * mapsize + c] = TIL_FLOOR;
        }
    }
    else if (tsx > tex) {
        for (c = tsx; c > tex; c--) {
            maparray[tey * mapsize + c] = TIL_FLOOR;
        }
    }
    connections[rtcy * 3 + rtcx] = 1;
}

void level_generate() {
	// reset all room connections
	int i;
	for (i = 0; i < 9; ++i) {
		connections[i] = 0;
	}

    for (i = 0; i < maparraysize; ++i) {
        maparray[i] = TIL_WALL;
    }
    level_generate_room(1 , 1 , roomsize, roomsize);
    level_generate_room(roomsize+1, 1 , roomsize*2, roomsize);
    level_generate_room(roomsize*2+1, 1 , roomsize*3, roomsize);
    level_generate_room(1 , roomsize+1, roomsize, roomsize*2);
    level_generate_room(roomsize+1, roomsize+1, roomsize*2, roomsize*2);
    level_generate_room(roomsize*2+1, roomsize+1, roomsize*3, roomsize*2);
    level_generate_room(1 , roomsize*2+1, roomsize, roomsize*3);
    level_generate_room(roomsize+1, roomsize*2+1, roomsize*2, roomsize*3);
    level_generate_room(roomsize*2+1, roomsize*2+1, roomsize*3, roomsize*3);

    int rtcx, rtcy, rtctx, rtcty, dir;

    rtcx = gsrand(0, 2);
    rtcy = gsrand(0, 2);

    rtctx = 3;
    rtcty = 3;

    // First step: step through, joining rooms then using that room to make a new join.
    // Eventually, we get into a dead end, so stop and go on to phase 2 of the generator.

	while(1) {

        // for some ungodly reason, doing this in the while condition itself does not work.
        if (room_open_connections(rtcx, rtcy) == 0) break;

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
    }

	// Second part: select randomly an unjoined room, and join to an adjacent room, until no more unjoined rooms exist.
	// NOTE: there is a bug, sometimes rooms are not connected
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
    }

	// Third part: some random connections
	int xtra_connections = gsrand(3, 7);
	for (i = 0; i < xtra_connections; ++i) {
		rtcx = gsrand(0, 2);
		rtctx = gsrand(0, 2);
		rtcy = gsrand(0, 2);
		rtcty = gsrand(0, 2);
		level_connect_rooms(rtcx, rtcty, rtctx, rtcty);
	}

	// Put stuff
	struct vect2d ppos = position_find_valid();
	player.xpos = ppos.x;
	player.ypos = ppos.y;

	things_generate();

	screen_game();
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
    for (rs = 0; rs < 13; ++rs) {
        random();
    }

    //we load our unique tile data at position 1 on VRAM
    VDP_loadTileData((const u32 *)tile_wall, 1, 1, 0);
		VDP_loadTileData((const u32 *)tile_stairs, 2, 1, 0);
		VDP_loadTileData((const u32 *)tile_floor, 3, 1, 0);
		VDP_loadTileData((const u32 *)tile_hero, 4, 1, 0);
		VDP_loadTileData((const u32 *)tile_wpn, 5, 1, 0);
		VDP_loadTileData((const u32 *)tile_ptn, 6, 1, 0);
		VDP_loadTileData((const u32 *)tile_scroll, 7, 1, 0);

		// Initialise basic stuff
    empty = thing_make(TIL_NULL, 0, 0);

    int m = 0;
    for (m = 0; m < 32; ++m) {
    	things[m] = thing_make(TIL_NULL, 0, 0);
    }

    blocker = thing_make(TIL_WALL, 0, 0);

	player = thing_make(TIL_PLAYER, 0, 0);

		// Generate a level and place everything
		level_generate();

    while(1) {
        VDP_waitVSync();
    }
    return (0);
}