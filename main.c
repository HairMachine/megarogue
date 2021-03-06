#include <genesis.h>
#include "res/testm.h"
#include "res/potion.h"
#include "res/stairs.h"
#include "res/card.h"
#include "res/bigtiles.h"
#include "res/template_dude.h"

enum direction {
	DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST
};

enum tile {
	TIL_NULL, TIL_CORRIDOR, TIL_FLOOR, TIL_TAKEN, TIL_DOOR_NS, TIL_DOOR_EW, TIL_WALL, TIL_PLAYER, TIL_GOBLIN, TIL_STAIRS, 
	TIL_MACGUFFIN, TIL_WPN, TIL_POTION, TIL_FOOD, TIL_SCROLL, TIL_AMMO, TIL_SHOT, TIL_KEY, TIL_PIT, TIL_RAGE, TIL_TELE, 
	TIL_GODMODE, TIL_SUPER, TIL_LEVITATE, TIL_POWER
};

enum SHOTTYPE {
	SH_NONE, SH_NORMAL, SH_FIRE, SH_WATER, SH_ACID, SH_WEB, SH_SOLID, SH_CONDENSER, SH_EXPLOSIVE
};

enum GM_MODE {
	GM_WALK, GM_SHOOT
};

enum LVL_TYPE {
	LVL_NORMAL, LVL_SHOP, LVL_TREASURE
};


const u32 tile_null[8] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
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

const u32 tile_door_ns[8] = {
	0x10000000,
	0x10000000,
	0x11000001,
	0x11111111,
	0x11111111,
	0x11000001,
	0x10000000,
	0x10000000,
};

const u32 tile_door_ew[8] = {
	0x11111111,
	0x00111100,
	0x00011000,
	0x00011000,
	0x00011000,
	0x00011000,
	0x00011000,
	0x00111100,
};

enum ABILITIES {
	AB_NONE, AB_INV, AB_FADE, AB_RAGE, AB_BLINK, AB_SHAFT, AB_BLAST, AB_LEV, AB_FAM,
	AB_FIRE, AB_SHIELD, AB_HEAL, AB_APP, AB_CLAIR, AB_SENSE
};

enum FLAGS {
	FL_NONE = 0, FL_MOVES = 1, FL_PASSTHRU = 2, FL_IMMORTAL = 4, FL_OPTIONAL = 8
};

enum STATUS_ID {
	ST_NONE = 0, ST_RAGE = 1, ST_GODMODE = 2, ST_FLYING = 4, ST_POWER = 8
};

struct Status {
	enum STATUS_ID id;
	int cur_time;
};

struct Thing {
	int xpos;
	int ypos;
	int hp;
	int max_hp;
	enum FLAGS flags;
	enum tile til;
	enum SHOTTYPE st; // create bullet effets; mostly used for bullets obv
	int damage;
	int range;
	struct Status status[8];
	int dl;
};

struct vect2d {
	int x;
	int y;
};

// MOst of this needs a bit of a move around I think
//const int mapsize = 28;
const int mapsize = 10;
//const int maparraysize = 784;
const int maparraysize = 100;
const int roomsize = 8;
const int maxdoors = 8;
const int statmax = 8;
//enum tile maparray[784];
enum tile maparray[81];
struct Thing things[32];
struct Thing doors[8];
int door_count = 0;
Sprite sprite[33];
struct Thing player;
struct Thing empty;
struct Thing blocker;
struct Thing shot;
int connections[4] = {
		0, 0, 
		0, 0
};
int depth = 0;
int maxdepth = 15;
enum ABILITIES abilities[3] = {AB_NONE, AB_NONE, AB_NONE};
enum GM_MODE gm_mode = GM_WALK;
int food = 512;
int keys = 5;
int ammo[9] = {0, 4, 0, 0, 0, 0, 0, 0, 0};
int current_ammo = 1;

void level_generate();

void thing_interact(struct Thing *subj, struct Thing *obj);

int in_range(int v, int l, int u) {
	if (v >= l && v <= u)
		return 1;
	return 0;
}

void debug(int val, int x, int y) {
	char msg[15];
	sprintf(msg, "%d", val);
	VDP_drawText(msg, x, y);
}

int gsrand(int min, int max) {
	u16 val = random() & 255;
	while (in_range(val, min, max) == 0)
		val = random() & 255;
	return val;
}

void tile_draw(enum tile tilenum, int x, int y) {
	x *= 2;
	y *= 2;
	switch (tilenum) {
		case TIL_NULL:
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, TIL_NULL), x, y);
			break;
		case TIL_FLOOR:
		case TIL_TAKEN:
		case TIL_CORRIDOR:
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 0), x, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 1), x + 1, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 4), x, y + 1);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 5), x + 1, y + 1);
			break;
		case TIL_WALL:
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 2), x, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 3), x + 1, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 6), x, y + 1);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 7), x + 1, y + 1);
			break;
		case TIL_DOOR_NS:
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 9), x, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 10), x + 1, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 13), x, y + 1);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 14), x + 1, y + 1);
			break;
		case TIL_DOOR_EW:
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 9), x, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 10), x + 1, y);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 13), x, y + 1);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 14), x + 1, y + 1);
			break;
	}
}

void sprite_set(int id, enum tile tilenum, int x, int y) {
	const int ts = 16;
	switch (tilenum) {
		case TIL_GOBLIN:
			SPR_initSprite(&sprite[id], &testm, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_PLAYER:
			SPR_initSprite(&sprite[id], &template_dude, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_STAIRS:
			SPR_initSprite(&sprite[id], &stairs, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_PIT:
			SPR_initSprite(&sprite[id], &stairs, x * ts, y * ts, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
			break;
		case TIL_MACGUFFIN:
			SPR_initSprite(&sprite[id], &testm, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_POTION:
			SPR_initSprite(&sprite[id], &potion, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_FOOD:
			SPR_initSprite(&sprite[id], &potion, x * ts, y * ts, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
			break;
		case TIL_KEY:
			SPR_initSprite(&sprite[id], &potion, x * ts, y * ts, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
			break;
		case TIL_AMMO:
			SPR_initSprite(&sprite[id], &card, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_SCROLL:
			SPR_initSprite(&sprite[id], &testm, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_RAGE:
			SPR_initSprite(&sprite[id], &potion, x * ts, y * ts, TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
			break;
		case TIL_TELE:
			SPR_initSprite(&sprite[id], &card, x * ts, y * ts, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
			break;
		case TIL_GODMODE:
			SPR_initSprite(&sprite[id], &card, x * ts, y * ts, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
			break;
		case TIL_SUPER:
			SPR_initSprite(&sprite[id], &potion, x * ts, y * ts, TILE_ATTR(PAL1, TRUE, TRUE, FALSE));
			break;
		case TIL_LEVITATE:
			SPR_initSprite(&sprite[id], &potion, x * ts, y * ts, TILE_ATTR(PAL2, TRUE, TRUE, FALSE));
			break;
		case TIL_POWER:
			SPR_initSprite(&sprite[id], &potion, x * ts, y * ts, TILE_ATTR(PAL3, TRUE, TRUE, FALSE));
			break;
		default:
			// actually should de-initialise the sprite rather than drawing it offscreen
			SPR_initSprite(&sprite[id], &testm, -ts, -ts, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
	}
}

void move_player_light(enum direction d) {
	int minx, miny, maxx, maxy, i;

	miny = player.ypos - player.range;
	if (miny < 0) miny = 0;
	maxy = player.ypos + player.range;
	if (maxy >= mapsize) maxy = mapsize;

	minx = player.xpos - player.range;
	if (minx < 0) minx = 0;
	maxx = player.xpos + player.range;
	if (maxx >= mapsize) maxx = mapsize;

	switch (d) {
		case DIR_NORTH:
			for (i = minx; i <= maxx; ++i) {
				tile_draw(maparray[miny * mapsize + i], i, miny);
				tile_draw(TIL_NULL, i, maxy + 1);
			}
			break;
		case DIR_EAST:
			for (i = miny; i <= maxy; ++i) {
				tile_draw(TIL_NULL, minx - 1, i);
				tile_draw(maparray[i * mapsize + maxx], maxx, i);
			}
			break;
		case DIR_SOUTH:
			for (i = minx; i <= maxx; ++i) {
				tile_draw(TIL_NULL, i, miny - 1);
				tile_draw(maparray[maxy * mapsize + i], i, maxy);
			}
			break;
		case DIR_WEST:
			for (i = miny; i <= maxy; ++i) {
				tile_draw(maparray[i * mapsize + minx], minx, i);
				tile_draw(TIL_NULL, maxx + 1, i);
			}
			break;
	}

}

void redraw_tiles() {
	int x = 0, y = 0, j = 0, absx = 0, absy = 0;
	for (j = 0; j < maparraysize; ++j) {
		absx = abs(player.xpos - x);
		absy = abs(player.ypos - y);
		if (absx <= player.range && absy <= player.range)
			tile_draw(maparray[j], x, y);
		else
			tile_draw(TIL_NULL, x, y);
		++x;
		if (x >= mapsize) {
			x = 0;
			++y;
		}
	}
}

void redraw_doors() {
	int i, absx = 0, absy = 0;
	for (i = 0; i < maxdoors; ++i) {
		absx = abs(player.xpos - doors[i].xpos);
		absy = abs(player.ypos - doors[i].ypos);
		if (doors[i].til > TIL_NULL && absx <= player.range && absy <= player.range)
			tile_draw(doors[i].til, doors[i].xpos, doors[i].ypos);
		else if (doors[i].til > TIL_NULL)
			tile_draw(TIL_NULL, 0, 0);
		else
			tile_draw(TIL_FLOOR, doors[i].xpos, doors[i].ypos);
	}
}

void redraw_things() {
	int k = 0, absx = 0, absy = 0;
	for (k = 0; k < 32; ++k) {
		absx = abs(player.xpos - things[k].xpos);
		absy = abs(player.ypos - things[k].ypos);
		if (things[k].til > TIL_NULL && absx <= player.range && absy <= player.range)
			sprite_set(k, things[k].til, things[k].xpos, things[k].ypos);
		else
			sprite_set(k, things[k].til, -1, -1); // thing is drawn in the non-display zone so it vanishes
	}
	// and the player on top
	sprite_set(32, player.til, player.xpos, player.ypos);
	SPR_update(sprite, 33);
}

void draw_health() {
	char msg[15];
	sprintf(msg, "HP: %d ", player.hp);
	VDP_drawText(msg, 30, 0);
}

void draw_food() {
	if (food >= 256)
		VDP_drawText("Full    ", 30, 1);
	else if (food >= 128)
		VDP_drawText("OK      ", 30, 1);
	else if (food >= 64)
		VDP_drawText("Hungry  ", 30, 1);
	else if (food >= 32)
		VDP_drawText("Starving", 30, 1);
	else if (food >= 16)
		VDP_drawText("Fainting", 30, 1);
}

void draw_keys() {
	char msg[15];
	sprintf(msg, "Keys: %d ", keys);
	VDP_drawText(msg, 30, 2);
}

void draw_depth() {
	char msg[15];
	sprintf(msg, "Depth: %d ", depth + 1);
	VDP_drawText(msg, 30, 3);
}

void draw_ammo() {
	char msg[15];
	switch (current_ammo) {
		case SH_NORMAL:
			VDP_drawText("WPN: Gun  ", 30, 4);
			break;
		default:
			VDP_drawText("WPN: error", 30, 4);
			break;
	}
	sprintf(msg, "Ammo: %d ", ammo[current_ammo]);
	VDP_drawText(msg, 30, 5);
}

void draw_mode() {
	switch(gm_mode) {
		case GM_WALK: VDP_drawText("     ", 30, 6); break;
		case GM_SHOOT: VDP_drawText("Shoot", 30, 6); break;
		default: VDP_drawText("error", 30, 6); break;
	}
}

void draw_status() {
	char msg[15];
	int i, c = 0;
	for (i = 0; i < statmax; ++i) {
		switch (player.status[i].id) {
			case ST_RAGE: 
				sprintf(msg, "RAGE: %d ", player.status[i].cur_time); 
				VDP_drawText(msg, 30, 7 + c);
				++c;
				break;
			case ST_GODMODE:
				sprintf(msg, "GOD:  %d ", player.status[i].cur_time);
				VDP_drawText(msg, 30, 7 + c);
				++c;
				break;
			case ST_FLYING:
				sprintf(msg, "FLY:  %d ", player.status[i].cur_time);
				VDP_drawText(msg, 30, 7 + c);
				++c;
				break;
			case ST_POWER:
				sprintf(msg, "PWR:  %d ", player.status[i].cur_time);
				VDP_drawText(msg, 30, 7 + c);
				++c;
				break;
			default: break;
		}
	}
	// Overwrite remainder
	for (i = c; i < statmax; ++i)
		VDP_drawText("       ", 30, 7 + i); 
}

void screen_game() {
	// main screen
	redraw_tiles();
	redraw_things();
	redraw_doors();
	//tile_draw(player.til, player.xpos, player.ypos);
	// sidebar
	draw_health();
	draw_food();
	draw_depth();
	draw_keys();
	draw_ammo();
	draw_mode();
	draw_status();
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
	int xp = gsrand(0, mapsize - 1);
	int yp = gsrand(0, mapsize - 1);
	while (maparray[yp * mapsize + xp] != TIL_FLOOR || (player.ypos == yp && player.xpos == xp)) {
		xp = gsrand(0, mapsize - 1);
		yp = gsrand(0, mapsize - 1);
	}
	pos.x = xp;
	pos.y = yp;
	maparray[yp * mapsize + xp] = TIL_TAKEN;
	return pos;
}


int ability_get(enum ABILITIES a) {
	int i;
	for (i = 0; i < 3; ++i) {
		if (abilities[i] == AB_NONE) {
			abilities[i] = a;
			return 1;
		}
	}
	return 0;
}

int ability_get_random() {
	int roll = gsrand(1, 14);
	return ability_get(roll);
}

// STATUS EFFECTS

void thing_status_set_at(struct Thing* t, enum STATUS_ID id, int i) {
	struct Status* s = &t->status[i];
	s->id = id;
	switch (id) {
		case ST_RAGE:
			s->cur_time = 32;
			break;
		case ST_GODMODE:
			s->cur_time = 16;
			break;
		case ST_FLYING:
			s->cur_time = 32;
			break;
		case ST_POWER:
			s->cur_time = 32;
			break;
		default:
			s->cur_time = 0;
			break;
	}
}

void thing_status_set(struct Thing* t, enum STATUS_ID id) {
	int i, j;
	int existing = 0;
	// set up a temp bit flag var with existing status effects
	for (i = 0; i < statmax; ++i) {
		existing = existing | t->status[i].id;
	}
	// apply status effect if not existing; if it is, refresh
	for (i = 0; i < statmax; ++i) {
		if (t->status[i].id == ST_NONE && !(existing & id)) {
			thing_status_set_at(t, id, i);
			return;
		}
		else if (t->status[i].id == ST_NONE) {
			for (j = 0; j < statmax; ++j) {
				if (t->status[j].id == id) {
					thing_status_set_at(t, id, j);
					return;
				}
			}
		}
	}
	
}

void thing_status_reset(struct Thing* t) {
	int i;
	for (i = 0; i < statmax; ++i) {
		thing_status_set_at(t, ST_NONE, i);
	}
}

void thing_status_countdown(struct Thing* t) {
	int i;
	for (i = 0; i < statmax; ++i) {
		if (t->status[i].id > ST_NONE) {
			t->status[i].cur_time--;
			if (t->status[i].cur_time <= 0)
				thing_status_set_at(t, ST_NONE, i);
		}
	}
}

void status_countdown() {
	// enemies etc.
	int i;
	for (i = 0; i < 32; ++i) {
		if (things[i].til > TIL_NULL)
			thing_status_countdown(&things[i]);
	}
	// player
	thing_status_countdown(&player);

	draw_status();
}

int thing_status_has(struct Thing* t, enum STATUS_ID id) {
	int i;
	for (i = 0; i < statmax; ++i) {
		if (t->status[i].id == id)
			return 1;
	}
	return 0;
}


// THINGS

struct Thing thing_make(enum tile t, int x, int y) {
	struct Thing thing;
	thing.til = t;
	thing.xpos = x;
	thing.ypos = y;
	thing.range = 5;
	thing.st = SH_NONE;
	thing_status_reset(&thing);
	thing.dl = 1;
	switch (t) {
		case TIL_PLAYER:
			thing.hp = 10;
			thing.damage = 1;
			thing.range = 32;
			thing.flags = FL_NONE;
			break;
		case TIL_GOBLIN:
			thing.hp = 1;
			thing.flags = FL_MOVES;
			thing.damage = 1;
			break;
		case TIL_WALL:
			thing.flags = FL_IMMORTAL;
			break;
		case TIL_DOOR_NS:
		case TIL_DOOR_EW:
			thing.flags = FL_NONE;
			thing.hp = 5;
			break;
		case TIL_STAIRS:
			thing.flags = FL_IMMORTAL;
			break;
		case TIL_SHOT:
			thing.flags = FL_IMMORTAL;
			break;
		case TIL_PIT:
			thing.flags = FL_IMMORTAL;
			break;
		case TIL_TELE:
			thing.flags = FL_IMMORTAL | FL_OPTIONAL;
			break;
		default:
			thing.flags = FL_PASSTHRU | FL_IMMORTAL;
			break;
	}
	thing.max_hp = thing.hp;
	return thing;
}

struct Thing thing_put(enum tile t) {
	struct vect2d ppos = position_find_valid();
	return thing_make(t, ppos.x, ppos.y);
}

void things_generate() {
	int i, roll;
	int max_m = 3 + gsrand(0, 1);
	//if (max_m > 15) max_m = 15;
	// int max_i = depth + 17;
	// if (max_i > 27) max_i = 27;
	int max_i = 2 + gsrand(0, 2);
	// first loop: monsters.
	int dl = 0, num = 0;
	while (dl < depth && num < max_m) {
		things[num] = thing_put(TIL_GOBLIN);
		dl += things[num].dl;
		++num;
	}
	// second loop: items.
	// TODO: a better system for deciding what to generate
	dl = 0; max_i += num;
	while (dl < depth && num < max_i) {
		roll = gsrand(0, 4);
		if (roll == 0)
			things[num] = thing_put(TIL_POTION);
		else if (roll == 1)
			things[num] = thing_put(TIL_FOOD);
		else if (roll == 2)
			things[num] = thing_put(TIL_AMMO);
		else if (roll == 3)
			things[num] = thing_put(TIL_PIT);
		else if (roll == 4)
			things[num] = thing_put(TIL_TELE);
		++num;
		dl += things[num].dl;
	}
	// finally the stairs on a normal level, or macguffin on last level
	if (depth < maxdepth) {
		roll = gsrand(0, 3);
		int exit_x, exit_y;
		switch (roll) {
			case 0: exit_x = 4; exit_y = 0; break;
			case 1: exit_x = mapsize - 1, exit_y = 4; break;
			case 2: exit_x = 0; exit_y = 4; break;
			case 3: exit_x = 4; exit_y = mapsize - 1; break;
		}
		maparray[exit_y * mapsize + exit_x] = TIL_FLOOR;
		things[31] = thing_make(TIL_STAIRS, exit_x, exit_y);
		// maybe a key door
		roll = gsrand(0, 4);
		enum tile d_til;
		if (roll <= 4) {
			roll = gsrand(0, 3);
			switch (roll) {
				case 0: exit_x = 5; exit_y = 0; d_til = TIL_DOOR_NS; break;
				case 1: exit_x = mapsize - 1, exit_y = 5; d_til = TIL_DOOR_EW; break;
				case 2: exit_x = 0; exit_y = 5; d_til = TIL_DOOR_EW; break;
				case 3: exit_x = 5; exit_y = mapsize - 1; d_til = TIL_DOOR_NS; break;
			}
			maparray[exit_y * mapsize + exit_x] = d_til;
			things[30] = thing_make(d_til, exit_x, exit_y);
		}
	}
	else
		things[31] = thing_put(TIL_MACGUFFIN);
}

struct Thing *thing_collide(struct Thing *t, enum direction dir) {
	int xm = 0, ym = 0;
	switch (dir) {
		case DIR_NORTH:
			ym = -1;
			break;
		case DIR_EAST:
			xm = 1;
			break;
		case DIR_SOUTH:
			ym = 1;
			break;
		case DIR_WEST:
			xm = -1;
			break;
	}
	// check wall
	int til_i = ((t->ypos + ym) * mapsize) + (t->xpos + xm);
	if (maparray[til_i] >= TIL_WALL)
		return &blocker;
	// check doors
	int i = 0;
	for (i = 0; i < maxdoors; ++i) {
		if (doors[i].til > TIL_NULL && doors[i].xpos == t->xpos + xm && doors[i].ypos == t->ypos + ym) {
			return &doors[i];
		}
	}
	// check things
	// TODO: slightly more complex
	if (thing_status_has(t, ST_FLYING))
		return &empty;

	for (i = 0; i < 32; ++i) {
		if (things[i].til > TIL_NULL && things[i].xpos == t->xpos + xm && things[i].ypos == t->ypos + ym) {
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

void thing_disable(struct Thing *t) {
	if (t->til == TIL_PLAYER)
		screen_dead();
	*t = thing_make(TIL_NULL, t->xpos, t->ypos);
}

void thing_damage(struct Thing *t, int damage) {
	if ((t->flags & FL_IMMORTAL) || thing_status_has(t, ST_GODMODE))
		return;

	t->hp -= damage;
	if (damage < 0 && t->hp > t->max_hp)
		t->hp = t->max_hp;
	if (t->til == TIL_PLAYER)
		draw_health();
	if (t->hp <= 0) 
		thing_disable(t);
}

void thing_move(struct Thing *t, enum direction d) {
	struct Thing *collided = thing_collide(t, d);
	// This looks weird but is for a reason: if a blocking thing is destroyed by an interaction we still want it to act
	// as a blocker
	if (!(collided->flags & FL_PASSTHRU) && !(collided->flags & FL_OPTIONAL)) {
		thing_interact(t, collided);
		return;
	}
	else if (!(collided->flags & FL_OPTIONAL)) 
		thing_interact(t, collided);

	// Move the object
	switch (d) {
		case DIR_NORTH:
			if (t->ypos <= 0) return;
			t->ypos--;
			break;
		case DIR_EAST:
			if (t->xpos >= mapsize - 1) return;
			t->xpos++;
			break;
		case DIR_SOUTH:
			if (t->ypos >= mapsize - 1) return;
			t->ypos++;
			break;
		case DIR_WEST:
			if (t->xpos <= 0) return;
			t->xpos--;
			break;
	}
	// if (t->til == TIL_PLAYER)
	// 	move_player_light(d);
}

void thing_move_toward(struct Thing *t, int xpos, int ypos) {
	if (!(t->flags & FL_MOVES))
		return;

	int absx, absy;
	absx = abs(xpos - t->xpos);
	absy = abs(ypos - t->ypos);
	if (absx >= t->range || absy >= t->range)
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


int ability_use(enum ABILITIES a) {
	int i, absx, absy;
	struct vect2d newpos;

	switch (a) {
		case AB_NONE:
			return 0;

		case AB_APP:
			return 1;

		case AB_BLAST:
			for (i = 0; i < 32; ++i) {
				absx = abs(player.xpos - things[i].xpos);
				absy = abs(player.ypos - things[i].ypos);
				if (absx <= player.range && absy <= player.range) {
					thing_damage(&things[i], 3);
				}
			}
			return 1;

		case AB_SHAFT:
			++depth;
			level_generate();
			return 1;

		case AB_SENSE:
			++player.range;
			screen_game();
			return 1;

		case AB_RAGE:
			++player.damage;
			return 1;

		case AB_BLINK:
			
			return 1;

		case AB_CLAIR:
			// All monsters visible briefly (status effect)
			return 1;

		case AB_FADE:
			// Player can walk through walls for a x turns
			return 1;

		case AB_FAM:
			// spawn an ally
			return 1;

		case AB_FIRE:
			// spawn a fire
			return 1;

		case AB_HEAL:
			player.hp += 5;
			draw_health();
			return 1;

		case AB_INV:
			// player gains invisibility state
			return 1;

		case AB_LEV:
			// player gains levitation state
			return 1;

		case AB_SHIELD:
			// player gains shielded state
			return 1;

		default:
			return 1;

	}
}

void thing_interact(struct Thing *subj, struct Thing *obj) {
	// do nothing if the thing isn't a thing
	if (obj->til <= TIL_FLOOR)
		return;

	// for bullets (make its own function - probably player, monsters, shot are own functions)
	if (subj->til == TIL_SHOT) {
		thing_damage(obj, subj->damage);
		thing_disable(subj);
		return;
	}

	// For all non-bullet collisions
	// Collisions are currently non-commutative, which is kind of weird I guess.
	struct vect2d newpos;

	switch (obj->til) {
		case TIL_GOBLIN:
			break;
		case TIL_PLAYER:
			if (thing_status_has(subj, ST_RAGE))
				thing_damage(obj, subj->damage * 4);
			else
				thing_damage(obj, subj->damage);
			break;
		case TIL_STAIRS:
		case TIL_PIT:
			if (subj->til == TIL_PLAYER) {
				// wrap player
				if (player.xpos == 1)	
					player.xpos = mapsize - 2;
				else if (player.ypos == 1)
					player.ypos = mapsize - 2;
				else if (player.xpos == mapsize - 2)
					player.xpos = 1;
				else if (player.ypos == mapsize - 2)
					player.ypos = 1;
				// Make a new map
				++depth;
				level_generate();
			}
			else {
				thing_disable(subj);
			}
			break;
		case TIL_DOOR_NS:
		case TIL_DOOR_EW:
			if (subj->til == TIL_PLAYER && keys > 0) {
				--keys;
				++depth;
				if (player.xpos == 1)
					player.xpos = mapsize - 2;
				else if (player.ypos == 1)
					player.ypos = mapsize - 2;
				else if (player.xpos == mapsize - 2)
					player.xpos = 1;
				else if (player.ypos == mapsize - 2)
					player.ypos = 1;
				level_generate_special(LVL_TREASURE);
				draw_keys();
			}
			break;
		case TIL_MACGUFFIN:
			if (subj->til == TIL_PLAYER)
				screen_victory();
			break;
		case TIL_POTION:
			if (subj->hp < subj->max_hp) {
				thing_damage(subj, -5);
				thing_disable(obj);
			}
			break;
		case TIL_FOOD:
			food += 16; 
			draw_food();
			thing_disable(obj);
			break;
		case TIL_AMMO:
			if (subj->til == TIL_PLAYER) {
				ammo[SH_NORMAL] += 2;
				thing_disable(obj);
				draw_ammo();
			}
			break;
		case TIL_KEY:
			if (subj->til == TIL_PLAYER) {
				++keys;
				thing_disable(obj);
				draw_keys();
			}
			break;
		case TIL_RAGE:
			thing_status_set(subj, ST_RAGE);
			thing_disable(obj);
			break;
		case TIL_TELE:
			newpos = position_find_valid();
			subj->xpos = newpos.x;
			subj->ypos = newpos.y;
			screen_game();
			break;
		case TIL_GODMODE:
			thing_status_set(subj, ST_GODMODE);
			thing_disable(obj);
			break;
		case TIL_SUPER:
			subj->hp = subj->max_hp * 2;
			draw_health();
			thing_disable(obj);
			break;
		case TIL_LEVITATE:
			thing_status_set(subj, ST_FLYING);
			thing_disable(obj);
			break;
		case TIL_POWER:
			thing_status_set(subj, ST_POWER);
			thing_disable(obj);
			break;
		default:
			break;
	}
}

void thing_interact_at(struct Thing* subj) {
	int i;
	for (i = 0; i < 32; ++i) {
		if (things[i].xpos == subj->xpos && things[i].ypos == subj->ypos) {
			thing_interact(subj, &things[i]);
		}
	}
}

void shoot_direction(struct Thing* subj, enum SHOTTYPE st, enum direction dir) {
	if (ammo[st] <= 0)
		return;

	int mshot = 0;
	shot.xpos = subj->xpos;
	shot.ypos = subj->ypos;
	shot.st = st;
	shot.damage = 1;
	if (thing_status_has(subj, ST_POWER))
		shot.damage *= 4;
	--ammo[st];
	while (shot.til == TIL_SHOT && mshot <= subj->range) {
		thing_move(&shot, dir);
		++mshot;
	}
	// reset the bullet
	shot = thing_make(TIL_SHOT, 0, 0);
	draw_ammo();
}


// LEVEL GENERATE

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
	if (y - 1 >= 0 && connections[(y - 1) * 2 + x] == 0)
		c++;
	if (x + 1 < 2 && connections[y * 2 + (x + 1)] == 0)
		c++;
	if (y + 1 < 2 && connections[(y + 1) * 2 + x] == 0)
		c++;
	if (x - 1 >= 0 && connections[y * 2 + (x - 1)] == 0)
		c++;
	return c;
}

int level_unconnected_rooms() {
	int x, y;
	int c = 0;
	for (x = 0; x < 2; ++x) {
		for (y = 0; y < 2; ++y) {
			if (connections[y * 2 + x] == 0)
				++c;
		}
	}
	return c;
}

struct vect2d level_get_first_unconnected() {
	int x, y;
	struct vect2d ntc = {-1, -1};
	for (x = 0; x < 2; ++x) {
		for (y = 0; y < 2; ++y) {
			if (connections[y * 2 + x] == 0) {
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

	int door_flag = 0;

	int c = 0;
	if (tsy < tey) {
		for (c = tsy; c < tey; ++c) {
			if (maparray[c * mapsize + tsx] == TIL_WALL) {
				if (!door_flag && door_count < maxdoors && gsrand(0, 2) == 0) {
					doors[door_count] = thing_make(TIL_DOOR_NS, tsx, c);
					door_flag = 1;
					door_count++;
				}
				maparray[c * mapsize + tsx] = TIL_CORRIDOR;
			}
		}
	}
	else if (tsy > tey) {
		for (c = tsy; c > tey; --c) {
			if (maparray[c * mapsize + tsx] == TIL_WALL) {
				if (!door_flag && door_count < maxdoors && gsrand(0, 2) == 0) {
					doors[door_count] = thing_make(TIL_DOOR_NS, tsx, c);
					door_flag = 1;
					door_count++;
				}
				maparray[c * mapsize + tsx] = TIL_CORRIDOR;
			}
		}
	}
	if (tsx < tex) {
		for (c = tsx; c < tex; ++c) {
			if (maparray[tey * mapsize + c] == TIL_WALL) {
				if (!door_flag && door_count < maxdoors && gsrand(0, 2) == 0) {
					doors[door_count] = thing_make(TIL_DOOR_EW, c, tey);
					door_flag = 1;
					door_count++;
				}
				maparray[tey * mapsize + c] = TIL_CORRIDOR;
			}
		}
	}
	else if (tsx > tex) {
		for (c = tsx; c > tex; --c) {
			if (maparray[tey * mapsize + c] == TIL_WALL) {
				if (!door_flag && door_count < maxdoors && gsrand(0, 2) == 0) {
					doors[door_count] = thing_make(TIL_DOOR_EW, c, tey);
					door_flag = 1;
					door_count++;
				}			
				maparray[tey * mapsize + c] = TIL_CORRIDOR;
			}
		}
	}
	connections[rtcy * 2 + rtcx] = 1;
}

// alternative simple level generator
void level_generate() {
	int c = gsrand(0, 3);
	int i;
	
	int room1[100] = {
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6
	};

	int room2[100] = {
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 6, 2, 2, 2, 2, 6, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 6, 6, 2, 2, 2, 6,
		6, 2, 2, 2, 6, 6, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 6, 2, 2, 2, 2, 6, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6
	};

	int room3[100] = {
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 6, 6, 6, 2, 2, 6, 6, 6, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 6, 6, 6, 2, 2, 6, 6, 6, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6
	};

	int room4[100] = {
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 6, 2, 6, 6, 2, 6, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 6, 2, 2, 2, 2, 6, 2, 6,
		6, 2, 6, 2, 2, 2, 2, 6, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 6, 2, 6, 6, 2, 6, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6
	};

	int room5[100] = {
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 2, 2, 2, 6, 2, 2, 2, 2, 6,
		6, 2, 6, 6, 6, 6, 6, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
		6, 2, 6, 6, 6, 6, 6, 2, 2, 6,
		6, 2, 2, 2, 6, 2, 2, 2, 2, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6
	};

	switch (c) {
		case 0: 
			memcpy(maparray, room1, sizeof(room1));
			break;
		case 1:
			memcpy(maparray, room2, sizeof(room2));
			break;
		case 2:
			memcpy(maparray, room3, sizeof(room3));
			break;
		case 3:
			memcpy(maparray, room4, sizeof(room4));
			break;
	}

	things_generate();

	screen_game();
}

void level_generate_special(enum LVL_TYPE lt) {
	int i;
	int room1[100] = {
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 2, 2, 2, 2, 2, 2, 2, 2, 6,
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6
	};

	for (i = 0; i < 32; ++i) {
		things[i] = thing_make(TIL_NULL, 0, 0);
	}

	switch (lt) {
		case LVL_TREASURE:
			memcpy(maparray, room1, sizeof(room1));
			things[0] = thing_make(TIL_AMMO, 2, 4);
			things[1] = thing_make(TIL_POTION, 2, 7);
			if (player.ypos == 1) {
				maparray[(player.ypos - 1) * mapsize + player.xpos] = TIL_FLOOR;
				things[31] = thing_make(TIL_STAIRS, player.xpos, player.ypos - 1);
			}
			else if (player.xpos == mapsize - 2) {
				maparray[player.ypos * mapsize + player.xpos + 1] = TIL_FLOOR;
				things[31] = thing_make(TIL_STAIRS, player.xpos + 1, player.ypos);
			}
			else if (player.ypos == mapsize - 2) {
				maparray[(player.ypos + 1) * mapsize + player.xpos] = TIL_FLOOR;
				things[31] = thing_make(TIL_STAIRS, player.xpos, player.ypos + 1);
			}
			else if (player.xpos == 1) {
				maparray[player.ypos * mapsize + player.xpos - 1] = TIL_FLOOR;
				things[31] = thing_make(TIL_STAIRS, player.xpos - 1, player.ypos);
			}
			break;
		default:
			level_generate();
			break;
	}

	screen_game();
}

// void level_generate() {
// 	// reset all room connections
// 	int i;
// 	for (i = 0; i < 4; ++i) {
// 		connections[i] = 0;
// 	}

// 	// reset doors
// 	door_count = 0;
// 	for (i = 0; i < maxdoors; ++i) {
// 		doors[i] = thing_make(TIL_NULL, 0, 0);
// 	}

// 	for (i = 0; i < maparraysize; ++i) {
// 		maparray[i] = TIL_WALL;
// 	}
// 	level_generate_room(1, 1, roomsize, roomsize);
// 	level_generate_room(roomsize + 1, 1, roomsize * 2, roomsize);
// 	//level_generate_room(roomsize * 2 + 1, 1, roomsize * 3, roomsize);
// 	level_generate_room(1, roomsize + 1, roomsize, roomsize * 2);
// 	level_generate_room(roomsize + 1, roomsize + 1, roomsize * 2, roomsize * 2);
// 	//level_generate_room(roomsize * 2 + 1, roomsize + 1, roomsize * 3, roomsize * 2);
// 	//level_generate_room(1, roomsize * 2 + 1, roomsize, roomsize * 3);
// 	//level_generate_room(roomsize + 1, roomsize * 2 + 1, roomsize * 2, roomsize * 3);
// 	//level_generate_room(roomsize * 2 + 1, roomsize * 2 + 1, roomsize * 3, roomsize * 3);

// 	int rtcx, rtcy, rtctx, rtcty, dir;

// 	rtcx = gsrand(0, 1);
// 	rtcy = gsrand(0, 1);

// 	rtctx = 2;
// 	rtcty = 2;

// 	// First step: step through, joining rooms then using that room to make a new join.
// 	// Eventually, we get into a dead end, so stop and go on to phase 2 of the generator.

// 	while (1) {

// 		// for some ungodly reason, doing this in the while condition itself does not work.
// 		if (room_open_connections(rtcx, rtcy) == 0) break;

// 		// make sure the connection is valid
// 		while (rtctx > 1 || rtcty > 1 || rtctx < 0 || rtcty < 0 || connections[rtcty * 2 + rtctx] == 1) {
// 			dir = gsrand(0, 3);
// 			switch (dir) {
// 				case 0:
// 					rtcty = rtcy - 1;
// 					rtctx = rtcx;
// 					break;
// 				case 1:
// 					rtctx = rtcx + 1;
// 					rtcty = rtcy;
// 					break;
// 				case 2:
// 					rtcty = rtcy + 1;
// 					rtctx = rtcx;
// 					break;
// 				case 3:
// 					rtctx = rtcx - 1;
// 					rtcty = rtcy;
// 					break;
// 			}
// 		}

// 		level_connect_rooms(rtcx, rtcy, rtctx, rtcty);

// 		rtcx = rtctx;
// 		rtcy = rtcty;
// 		rtctx = 2;
// 		rtcty = 2;
// 	}

// 	// Second part: select randomly an unjoined room, and join to an adjacent room, until no more unjoined rooms exist.
// 	// NOTE: there is a bug, sometimes rooms are not connected
// 	struct vect2d ntc;
// 	int wtf = 0;
// 	while (1) {
// 		ntc = level_get_first_unconnected();
// 		if (ntc.x == -1) break;

// 		rtcx = ntc.x;
// 		rtcy = ntc.y;
// 		rtctx = 2;
// 		rtcty = 2;
// 		while (rtctx > 1 || rtcty > 1 || rtctx < 0 || rtcty < 0) {
// 			dir = gsrand(0, 3);
// 			switch (dir) {
// 				case 0:
// 					rtcty = rtcy - 1;
// 					rtctx = rtcx;
// 					break;
// 				case 1:
// 					rtctx = rtcx + 1;
// 					rtcty = rtcy;
// 					break;
// 				case 2:
// 					rtcty = rtcy + 1;
// 					rtctx = rtcx;
// 					break;
// 				case 3:
// 					rtctx = rtcx - 1;
// 					rtcty = rtcy;
// 					break;
// 			}
// 		}

// 		level_connect_rooms(rtcx, rtcy, rtctx, rtcty);
// 	}

// 	// Third part: some random connections
// 	int xtra_connections = gsrand(1, 3);
// 	for (i = 0; i < xtra_connections; ++i) {
// 		rtcx = gsrand(0, 1);
// 		rtctx = gsrand(0, 1);
// 		rtcy = gsrand(0, 1);
// 		rtcty = gsrand(0, 1);
// 		level_connect_rooms(rtcx, rtcty, rtctx, rtcty);
// 	}

// 	// Fourth part: remove silly doors
// 	int cx = 0;
// 	int cy = 0;
// 	for (i = 0; i < maparraysize; ++i) {
// 		if (maparray[i] == TIL_DOOR_EW || maparray[i] == TIL_DOOR_NS) {
// 			// doors with more than 2 floors or corridors adjacent get turned into corridors
// 			if (i - mapsize >= 0 && maparray[i - mapsize] <= TIL_FLOOR)
// 				++cy;
// 			if (i + 1 < maparraysize && maparray[i + 1] <= TIL_FLOOR)
// 				++cx;
// 			if (i - 1 >= 0 && maparray[i - 1] <= TIL_FLOOR)
// 				++cx;
// 			if (i + mapsize < maparraysize && maparray[i + mapsize] <= TIL_FLOOR)
// 				++cy;
// 			if (cx > 2 || cy > 2 || (cx >= 1 && cy >= 1))
// 				maparray[i] = TIL_CORRIDOR;
// 		}
// 		cx = 0;
// 		cy = 0;
// 	}

// 	// Put stuff
// 	struct vect2d ppos = position_find_valid();
// 	player.xpos = ppos.x;
// 	player.ypos = ppos.y;

// 	things_generate();

// 	screen_game();
// }

void hunger_clock() {
	--food;
	draw_food();
	if (food <= 0)
		screen_dead();
}

void joypad_handle(u16 joy, u16 changed, u16 state) {
	int turn = 0;
	if (joy == JOY_1) {
		if (state & BUTTON_UP) {
			if (gm_mode == GM_SHOOT)
				shoot_direction(&player, current_ammo, DIR_NORTH);
			else
				thing_move(&player, DIR_NORTH);
			turn = 1;
		}
		else if (state & BUTTON_RIGHT) {
			if (gm_mode == GM_SHOOT)
				shoot_direction(&player, SH_NORMAL, DIR_EAST);
			else	
				thing_move(&player, DIR_EAST);
			turn = 1;
		}
		else if (state & BUTTON_DOWN) {
			if (gm_mode == GM_SHOOT)
				shoot_direction(&player, SH_NORMAL, DIR_SOUTH);
			else
				thing_move(&player, DIR_SOUTH);
			turn = 1;
		}
		else if (state & BUTTON_LEFT) {
			if (gm_mode == GM_SHOOT)
				shoot_direction(&player, SH_NORMAL, DIR_WEST);
			else
				thing_move(&player, DIR_WEST);
			turn = 1;
		}
		else if (state & BUTTON_A) {
			thing_interact_at(&player);
		}
		else if (state & BUTTON_B) {
			gm_mode = GM_SHOOT;
			draw_mode();
		}
		else if (state & BUTTON_C) {
			int a = current_ammo;
			while (ammo[a] <= 0 && a < 8)	{
				++a;
				if (current_ammo > 8) {
					current_ammo = 1;
				}
			}
			current_ammo = a;
			draw_ammo();
		}
		// key releases
		else if (changed & BUTTON_B) {
			gm_mode = GM_WALK;
			draw_mode();
    }
	}
	if (turn == 1) {
		// time based and status events
		hunger_clock();
		status_countdown();
		
		// monster turn
		int m = 0;
		for (m = 0; m < 32; ++m) {
			if (things[m].til > TIL_NULL) {
				thing_move_toward(&things[m], player.xpos, player.ypos);
			}
		}

		// gfx refreshes
		redraw_doors();
	}
}

int main() {
	SPR_init(256);
	JOY_init();
	JOY_setEventHandler(&joypad_handle);

	VDP_setPalette(1, testm.palette->data);

	// Temp: randomise
	int rs;
	for (rs = 0; rs < 13; ++rs) {
		random();
	}

	/*VDP_loadTileData((const u32 *) tile_null, TIL_NULL, 1, 0);
	VDP_loadTileData((const u32 *) tile_wall, TIL_WALL, 1, 0);
	VDP_loadTileData((const u32 *) tile_floor, TIL_FLOOR, 1, 0);*/
	/*VDP_loadTileData((const u32 *) tile_door_ns, TIL_DOOR_NS, 1, 0);
	VDP_loadTileData((const u32 *) tile_door_ew, TIL_DOOR_EW, 1, 0);*/

	VDP_loadTileSet(&bigtiles, TIL_NULL, 0);

	// Initialise basic stuff
	empty = thing_make(TIL_NULL, 0, 0);

	int m = 0;
	for (m = 0; m < 32; ++m) {
		things[m] = thing_make(TIL_NULL, 0, 0);
	}

	blocker = thing_make(TIL_WALL, 0, 0);
	player = thing_make(TIL_PLAYER, 1, 4);
	shot = thing_make(TIL_SHOT, 0, 0);

	// Generate a level and place everything
	level_generate();

	while (1) {
		redraw_things();
		VDP_waitVSync();
	}
	return (0);
}