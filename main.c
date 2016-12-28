#include <genesis.h>
#include "res/testm.h"
#include "res/potion.h"
#include "res/hero.h"
#include "res/stairs.h"
#include "res/card.h"

enum direction {
	DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST
};

enum tile {
	TIL_NULL, TIL_CORRIDOR, TIL_FLOOR, TIL_DOOR_NS, TIL_DOOR_EW, TIL_WALL, TIL_PLAYER, TIL_GOBLIN, TIL_STAIRS, TIL_MACGUFFIN,
	TIL_WPN, TIL_POTION, TIL_FOOD, TIL_SCROLL, TIL_ABILITY, TIL_SHOT, TIL_KEY
};

enum SHOTTYPE {
	SH_NONE, SH_NORMAL, SH_FIRE, SH_WATER, SH_ACID, SH_WEB, SH_SOLID, SH_CONDENSER, SH_EXPLOSIVE
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

struct Thing {
	int xpos;
	int ypos;
	int hp;
	enum FLAGS flags;
	enum tile til;
	enum SHOTTYPE st; // create bullet effets; mostly used for bullets obv
	int damage;
	int range;
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
Sprite sprite[33];
struct Thing player;
struct Thing empty;
struct Thing blocker;
struct Thing shot;
int connections[9] = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0
};
int depth = 0;
int maxdepth = 15;
enum ABILITIES abilities[3] = {AB_NONE, AB_NONE, AB_NONE};
int shot_mode = 0;
int food = 512;
int keys = 0;

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
	//return;
	switch (tilenum) {
		case TIL_NULL:
			VDP_setTileMapXY(APLAN, 8, x, y);
			break;
		case TIL_FLOOR:
		case TIL_CORRIDOR:
			VDP_setTileMapXY(APLAN, 3, x, y);
			break;
		case TIL_WALL:
			VDP_setTileMapXY(APLAN, 1, x, y);
			break;
		case TIL_DOOR_NS:
			VDP_setTileMapXY(APLAN, 4, x, y);
			break;
		case TIL_DOOR_EW:
			VDP_setTileMapXY(APLAN, 5, x, y);
			break;	
	}
}

void sprite_set(int id, enum tile tilenum, int x, int y) {
	switch (tilenum) {
		case TIL_GOBLIN:
			SPR_initSprite(&sprite[id], &testm, x * 8, y * 8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_PLAYER:
			SPR_initSprite(&sprite[id], &hero, x * 8, y * 8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_STAIRS:
			SPR_initSprite(&sprite[id], &stairs, x * 8, y * 8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_MACGUFFIN:
			SPR_initSprite(&sprite[id], &testm, x * 8, y * 8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_POTION:
			SPR_initSprite(&sprite[id], &potion, x * 8, y * 8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_FOOD:
			SPR_initSprite(&sprite[id], &potion, x * 8, y * 8, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
			break;
		case TIL_KEY:
			SPR_initSprite(&sprite[id], &potion, x * 8, y * 8, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
			break;
		case TIL_ABILITY:
			SPR_initSprite(&sprite[id], &card, x * 8, y * 8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		case TIL_SCROLL:
			SPR_initSprite(&sprite[id], &testm, x * 8, y * 8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
			break;
		default:
			// actually should de-initialise the sprite rather than drawing it offscreen
			SPR_initSprite(&sprite[id], &testm, -8, -8, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
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

void draw_weapon() {
	if (shot_mode)
		VDP_drawText("WPN: Gun    ", 30, 4);
	else
		VDP_drawText("WPN: Unarmed", 30, 4);
}

void screen_game() {
	// main screen
	redraw_tiles();
	redraw_things();
	//tile_draw(player.til, player.xpos, player.ypos);
	// sidebar
	draw_health();
	draw_food();
	draw_depth();
	draw_weapon();
	draw_keys();
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


// THINGS

struct Thing thing_make(enum tile t, int x, int y) {
	struct Thing thing;
	thing.til = t;
	thing.xpos = x;
	thing.ypos = y;
	thing.range = 5;
	thing.st = SH_NONE;
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
			thing.flags = FL_IMMORTAL;
			break;
		case TIL_STAIRS:
			thing.flags = FL_IMMORTAL | FL_OPTIONAL;
			break;
		case TIL_SHOT:
			thing.flags = FL_IMMORTAL;
			break;
		default:
			thing.flags = FL_PASSTHRU | FL_IMMORTAL;
			break;
	}
	return thing;
}

struct Thing thing_put(enum tile t) {
	struct vect2d ppos = position_find_valid();
	return thing_make(t, ppos.x, ppos.y);
}

void things_generate() {
	int i, roll;
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
		roll = gsrand(0, 4);
		if (roll >= 0  && roll <= 1)
			things[i] = thing_put(TIL_POTION);
		else if (roll >= 2 && roll <= 3)
			things[i] = thing_put(TIL_FOOD);
		else if (roll == 4)
			things[i] = thing_put(TIL_KEY);
	}
	// third loop: permanent power ups.
	for (i = 27; i < 31; ++i) {
		// atm, nothing
		things[i] = thing_put(TIL_ABILITY);
	}

	// finally the stairs or macguffin on last level
	if (depth < maxdepth)
		things[31] = thing_put(TIL_STAIRS);
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

void thing_disable(struct Thing *t) {
	if (t->til == TIL_PLAYER)
		screen_dead();
	*t = thing_make(TIL_NULL, 0, 0);
}

void thing_damage(struct Thing *t, int damage) {
	if (t->flags & FL_IMMORTAL)
		return;

	t->hp -= damage;
	if (t->til == TIL_PLAYER)
		draw_health();
	if (t->hp <= 0) thing_disable(t);
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
			if (t->xpos >= mapsize) return;
			t->xpos++;
			break;
		case DIR_SOUTH:
			if (t->ypos >= mapsize) return;
			t->ypos++;
			break;
		case DIR_WEST:
			if (t->xpos <= 0) return;
			t->xpos--;
			break;
	}
	if (t->til == TIL_PLAYER)
		move_player_light(d);
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
			newpos = position_find_valid();
			player.xpos = newpos.x;
			player.ypos = newpos.y;
			screen_game();
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
		thing_damage(obj, 2);
		thing_disable(subj);
		return;
	}

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
				thing_disable(subj);
			}
			break;
		case TIL_MACGUFFIN:
			if (subj->til == TIL_PLAYER)
				screen_victory();
			break;
		case TIL_POTION:
			thing_damage(subj, -5);
			thing_disable(obj);
			break;
		case TIL_FOOD:
			food += 64; 
			draw_food();
			thing_disable(obj);
			break;
		case TIL_ABILITY:
			if (ability_get_random() == 1) {
				thing_disable(obj);
			}
			break;
		case TIL_KEY:
			if (subj->til == TIL_PLAYER) {
				++keys;
				thing_disable(obj);
				draw_keys();
			}
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
	int mshot = 0;
	shot.xpos = subj->xpos;
	shot.ypos = subj->ypos;
	shot.st = st;
	while (shot.til == TIL_SHOT && mshot <= subj->range) {
		thing_move(&shot, dir);
		++mshot;
	}
	// reset the bullet
	shot = thing_make(TIL_SHOT, 0, 0);
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

	int door_flag = 0;

	int c = 0;
	if (tsy < tey) {
		for (c = tsy; c < tey; c++) {
			if (maparray[c * mapsize + tsx] == TIL_WALL) {
				if (gsrand(0, 3) == 0)
					maparray[c * mapsize + tsx] = TIL_DOOR_NS;
				else		
					maparray[c * mapsize + tsx] = TIL_CORRIDOR;

			}
		}
	}
	else if (tsy > tey) {
		for (c = tsy; c > tey; c--) {
			if (maparray[c * mapsize + tsx] == TIL_WALL) {
				if (gsrand(0, 3) == 0)
					maparray[c * mapsize + tsx] = TIL_DOOR_NS;
				else	
					maparray[c * mapsize + tsx] = TIL_CORRIDOR;
			}
		}
	}
	if (tsx < tex) {
		for (c = tsx; c < tex; c++) {
			if (maparray[tey * mapsize + c] == TIL_WALL) {
				if (gsrand(0, 3) == 0)
					maparray[tey * mapsize + c] = TIL_DOOR_EW;
				else
					maparray[tey * mapsize + c] = TIL_CORRIDOR;
			}
		}
	}
	else if (tsx > tex) {
		for (c = tsx; c > tex; c--) {
			if (maparray[tey * mapsize + c] == TIL_WALL) {
				if (gsrand(0, 3) == 0)
					maparray[tey * mapsize + c] = TIL_DOOR_EW;
				else
					maparray[tey * mapsize + c] = TIL_CORRIDOR;
			}
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
	level_generate_room(1, 1, roomsize, roomsize);
	level_generate_room(roomsize + 1, 1, roomsize * 2, roomsize);
	level_generate_room(roomsize * 2 + 1, 1, roomsize * 3, roomsize);
	level_generate_room(1, roomsize + 1, roomsize, roomsize * 2);
	level_generate_room(roomsize + 1, roomsize + 1, roomsize * 2, roomsize * 2);
	level_generate_room(roomsize * 2 + 1, roomsize + 1, roomsize * 3, roomsize * 2);
	level_generate_room(1, roomsize * 2 + 1, roomsize, roomsize * 3);
	level_generate_room(roomsize + 1, roomsize * 2 + 1, roomsize * 2, roomsize * 3);
	level_generate_room(roomsize * 2 + 1, roomsize * 2 + 1, roomsize * 3, roomsize * 3);

	int rtcx, rtcy, rtctx, rtcty, dir;

	rtcx = gsrand(0, 2);
	rtcy = gsrand(0, 2);

	rtctx = 3;
	rtcty = 3;

	// First step: step through, joining rooms then using that room to make a new join.
	// Eventually, we get into a dead end, so stop and go on to phase 2 of the generator.

	while (1) {

		// for some ungodly reason, doing this in the while condition itself does not work.
		if (room_open_connections(rtcx, rtcy) == 0) break;

		// make sure the connection is valid
		while (rtctx > 2 || rtcty > 2 || rtctx < 0 || rtcty < 0 || connections[rtcty * 3 + rtctx] == 1) {
			dir = gsrand(0, 3);
			switch (dir) {
				case 0:
					rtcty = rtcy - 1;
					rtctx = rtcx;
					break;
				case 1:
					rtctx = rtcx + 1;
					rtcty = rtcy;
					break;
				case 2:
					rtcty = rtcy + 1;
					rtctx = rtcx;
					break;
				case 3:
					rtctx = rtcx - 1;
					rtcty = rtcy;
					break;
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

		rtcx = ntc.x;
		rtcy = ntc.y;
		rtctx = 3;
		rtcty = 3;
		while (rtctx > 2 || rtcty > 2 || rtctx < 0 || rtcty < 0) {
			dir = gsrand(0, 3);
			switch (dir) {
				case 0:
					rtcty = rtcy - 1;
					rtctx = rtcx;
					break;
				case 1:
					rtctx = rtcx + 1;
					rtcty = rtcy;
					break;
				case 2:
					rtcty = rtcy + 1;
					rtctx = rtcx;
					break;
				case 3:
					rtctx = rtcx - 1;
					rtcty = rtcy;
					break;
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
			if (shot_mode)
				shoot_direction(&player, SH_NORMAL, DIR_NORTH);
			else
				thing_move(&player, DIR_NORTH);
			turn = 1;
		}
		else if (state & BUTTON_RIGHT) {
			if (shot_mode)
				shoot_direction(&player, SH_NORMAL, DIR_EAST);
			else	
				thing_move(&player, DIR_EAST);
			turn = 1;
		}
		else if (state & BUTTON_DOWN) {
			if (shot_mode)
				shoot_direction(&player, SH_NORMAL, DIR_SOUTH);
			else
				thing_move(&player, DIR_SOUTH);
			turn = 1;
		}
		else if (state & BUTTON_LEFT) {
			if (shot_mode)
				shoot_direction(&player, SH_NORMAL, DIR_WEST);
			else
				thing_move(&player, DIR_WEST);
			turn = 1;
		}
		else if (state & BUTTON_A) {
			thing_interact_at(&player);
		}
		else if (state & BUTTON_B) {
			shot_mode = abs(1 - shot_mode);
			draw_weapon();
		}
		else if (state & BUTTON_C) {
			
		}
	}
	if (turn == 1) {
		hunger_clock();
		int m = 0;
		for (m = 0; m < 32; ++m) {
			if (things[m].til > TIL_NULL) {
				thing_move_toward(&things[m], player.xpos, player.ypos);
			}
		}
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

	VDP_loadTileData((const u32 *) tile_null, 8, 1, 0);
	VDP_loadTileData((const u32 *) tile_wall, 1, 1, 0);
	VDP_loadTileData((const u32 *) tile_floor, 3, 1, 0);
	VDP_loadTileData((const u32 *) tile_door_ns, 4, 1, 0);
	VDP_loadTileData((const u32 *) tile_door_ew, 5, 1, 0);

	// Initialise basic stuff
	empty = thing_make(TIL_NULL, 0, 0);

	int m = 0;
	for (m = 0; m < 32; ++m) {
		things[m] = thing_make(TIL_NULL, 0, 0);
	}

	blocker = thing_make(TIL_WALL, 0, 0);
	player = thing_make(TIL_PLAYER, 0, 0);
	shot = thing_make(TIL_SHOT, 0, 0);

	// Generate a level and place everything
	level_generate();

	while (1) {
		redraw_things();
		VDP_waitVSync();
	}
	return (0);
}