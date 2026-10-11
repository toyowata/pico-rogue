/*
 * global variable initializaton
 *
 * @(#)extern.c	4.82 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

bool after;				/* True if we want after daemons */
bool again;				/* Repeating the last command */
int  noscore;				/* Was a wizard sometime */
bool seenstairs;			/* Have seen the stairs (for lsd) */
bool amulet = FALSE;			/* He found the amulet */
bool door_stop = FALSE;			/* Stop running when we pass a door */
bool fight_flush = FALSE;		/* True if toilet input */
bool firstmove = FALSE;			/* First move after setting door_stop */
bool got_ltc = FALSE;			/* We have gotten the local tty chars */
bool has_hit = FALSE;			/* Has a "hit" message pending in msg */
bool in_shell = FALSE;			/* True if executing a shell */
bool inv_describe = TRUE;		/* Say which way items are being used */
bool jump = FALSE;			/* Show running as series of jumps */
bool kamikaze = FALSE;			/* to_death really to DEATH */
bool lower_msg = FALSE;			/* Messages should start w/lower case */
bool move_on = FALSE;			/* Next move shouldn't pick up items */
bool msg_esc = FALSE;			/* Check for ESC from msg's --More-- */
bool passgo = FALSE;			/* Follow passages */
bool playing = TRUE;			/* True until he quits */
bool q_comm = FALSE;			/* Are we executing a 'Q' command? */
bool running = FALSE;			/* True if player is running */
bool save_msg = TRUE;			/* Remember last msg */
bool see_floor = TRUE;			/* Show the lamp illuminated floor */
bool stat_msg = FALSE;			/* Should status() print as a msg() */
bool terse = FALSE;			/* True if we should be short */
bool to_death = FALSE;			/* Fighting is to the death! */
bool tombstone = TRUE;			/* Print out tombstone at end */
#ifdef MASTER
int wizard = FALSE;			/* True if allows wizard commands */
#endif
bool pack_used[26] = {			/* Is the character used in the pack? */
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
};

char dir_ch;				/* Direction from last get_dir() call */
char file_name[MAXSTR];			/* Save file name */
char huh[MAXSTR];			/* The last message printed */
char *p_colors[MAXPOTIONS];		/* Colors of the potions */
char prbuf[2*MAXSTR];			/* buffer for sprintfs */
char *r_stones[MAXRINGS];		/* Stone settings of the rings */
char runch;				/* Direction player is running */
char *s_names[MAXSCROLLS];		/* Names of the scrolls */
char take;				/* Thing she is taking */
char whoami[MAXSTR];			/* Name of player */
char *ws_made[MAXSTICKS];		/* What sticks are made of */
char *ws_type[MAXSTICKS];		/* Is it a wand or a staff */
int  orig_dsusp;			/* Original dsusp char */
char fruit[MAXSTR] =			/* Favorite fruit */
		{ 's', 'l', 'i', 'm', 'e', '-', 'm', 'o', 'l', 'd', '\0' };
char home[MAXSTR] = { '\0' };		/* User's home directory */
char *inv_t_name[] = {
	"Overwrite",
	"Slow",
	"Clear"
};
char l_last_comm = '\0';		/* Last last_comm */
char l_last_dir = '\0';			/* Last last_dir */
char last_comm = '\0';			/* Last command typed */
char last_dir = '\0';			/* Last direction given */
char *tr_name[] = {			/* Names of the traps */
	"落とし穴",
	"矢の罠",
	"睡眠ガスの罠",
	"トラバサミ",
	"テレポートの罠",
	"毒矢の罠",
	"サビの罠",
    "謎の罠"
};


int n_objs;				/* # items listed in inventory() call */
int ntraps;				/* Number of traps on this level */
int hungry_state = 0;			/* How hungry is he */
int inpack = 0;				/* Number of things in pack */
int inv_type = 0;			/* Type of inventory to use */
int level = 1;				/* What level she is on */
int max_hit;				/* Max damage done to her in to_death */
int max_level;				/* Deepest player has gone */
int mpos = 0;				/* Where cursor is on top line */
int no_food = 0;			/* Number of levels without food */
int a_class[MAXARMORS] = {		/* Armor class for each armor type */
	8,	/* LEATHER */
	7,	/* RING_MAIL */
	7,	/* STUDDED_LEATHER */
	6,	/* SCALE_MAIL */
	5,	/* CHAIN_MAIL */
	4,	/* SPLINT_MAIL */
	4,	/* BANDED_MAIL */
	3,	/* PLATE_MAIL */
};

int count = 0;				/* Number of times to repeat command */
FILE *scoreboard = NULL;	/* File descriptor for score file */
int food_left;				/* Amount of food in hero's stomach */
int lastscore = -1;			/* Score before this turn */
int no_command = 0;			/* Number of turns asleep */
int no_move = 0;			/* Number of turns held in place */
int purse = 0;				/* How much gold he has */
int quiet = 0;				/* Number of quiet turns */
int vf_hit = 0;				/* Number of time flytrap has hit */

int dnum;				/* Dungeon number */
int seed;				/* Random number seed */
int e_levels[] = {
        10L,
	20L,
	40L,
	80L,
       160L,
       320L,
       640L,
      1300L,
      2600L,
      5200L,
     13000L,
     26000L,
     50000L,
    100000L,
    200000L,
    400000L,
    800000L,
   2000000L,
   4000000L,
   8000000L,
	 0L
};

coord delta;				/* Change indicated to get_dir() */
coord oldpos;				/* Position before last look() call */
coord stairs;				/* Location of staircase */

PLACE places[MAXLINES*MAXCOLS];		/* level map */

THING *cur_armor;			/* What he is wearing */
THING *cur_ring[2];			/* Which rings are being worn */
THING *cur_weapon;			/* Which weapon he is weilding */
THING *l_last_pick = NULL;		/* Last last_pick */
THING *last_pick = NULL;		/* Last object picked in get_item() */
THING *lvl_obj = NULL;			/* List of objects on this level */
THING *mlist = NULL;			/* List of monsters on the level */
THING player;				/* His stats */
					/* restart of game */

WINDOW *hw = NULL;			/* used as a scratch window */

#define INIT_STATS { 16, 0, 1, 10, 12, "1x4", 12 }

struct stats max_stats = INIT_STATS;	/* The maximum for the player */

struct room *oldrp;			/* Roomin(&oldpos) */
struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct room passages[MAXPASS] =		/* One for each passage */
{
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, {{0,0}} }
};

#define ___ 1
#define XX 10
struct monster monsters[26] =
    {
/* Name		 CARRY	FLAG    str, exp, lvl, amr, hpt, dmg */
{ "アクエター",	   0,	ISMEAN,	{ XX, 20,   5,   2, ___, "0x0/0x0" } },
{ "コウモリ",	   0,	ISFLY,	{ XX,  1,   1,   3, ___, "1x2" } },
{ "ケンタウロス",	  15,	0,	{ XX, 17,   4,   4, ___, "1x2/1x5/1x5" } },
{ "ドラゴン",	 100,	ISMEAN,	{ XX,5000, 10,  -1, ___, "1x8/1x8/3x10" } },
{ "エミュー",	   0,	ISMEAN,	{ XX,  2,   1,   7, ___, "1x2" } },
{ "ハエトリ草",   0,	ISMEAN,	{ XX, 80,   8,   3, ___, "%%%x0" } },
	/* NOTE: the damage is %%% so that xstr won't merge this */
	/* string with others, since it is written on in the program */
{ "グリフィン",	  20,	ISMEAN|ISFLY|ISREGEN, { XX,2000, 13,   2, ___, "4x3/3x5" } },
{ "ホブゴブリン",   0,	ISMEAN,	{ XX,  3,   1,   5, ___, "1x8" } },
{ "アイスモンスター", 0,	0,	{ XX,  5,   1,   9, ___, "0x0" } },
{ "ジャバウォック", 70,	0,	{ XX,3000, 15,   6, ___, "2x12/2x4" } },
{ "チョウゲンボウ", 0,	ISMEAN|ISFLY,	{ XX,  1,   1,   7, ___, "1x4" } },
{ "レプラコーン",   0,	0,	{ XX, 10,   3,   8, ___, "1x1" } },
{ "メドゥーサ",	  40,	ISMEAN,	{ XX,200,   8,   2, ___, "3x4/3x4/2x5" } },
{ "ニンフ",	 100,	0,	{ XX, 37,   3,   9, ___, "0x0" } },
{ "オーク",	  15,	ISGREED,{ XX,  5,   1,   6, ___, "1x8" } },
{ "ファントム",	   0,	ISINVIS,{ XX,120,   8,   3, ___, "4x4" } },
{ "クアッガ",	   0,	ISMEAN,	{ XX, 15,   3,   3, ___, "1x5/1x5" } },
{ "ガラガラヘビ",   0,	ISMEAN,	{ XX,  9,   2,   3, ___, "1x6" } },
{ "ヘビ",	   0,	ISMEAN,	{ XX,  2,   1,   5, ___, "1x3" } },
{ "トロール",	  50,	ISREGEN|ISMEAN,{ XX, 120, 6, 4, ___, "1x8/1x8/2x6" } },
{ "黒ユニコーン",   0,	ISMEAN,	{ XX,190,   7,  -2, ___, "1x9/1x9/2x9" } },
{ "吸血鬼",	  20,	ISREGEN|ISMEAN,{ XX,350,   8,   1, ___, "1x10" } },
{ "レイス",	   0,	0,	{ XX, 55,   5,   4, ___, "1x6" } },
{ "ゼロックス",	  30,	0,	{ XX,100,   7,   7, ___, "4x4" } },
{ "イエティ",	  30,	0,	{ XX, 50,   4,   6, ___, "1x6/1x6" } },
{ "ゾンビ",	   0,	ISMEAN,	{ XX,  6,   2,   8, ___, "1x8" } }
    };
#undef ___
#undef XX

struct obj_info things[NUMTHINGS] = {
    { 0,			26 },	/* potion */
    { 0,			36 },	/* scroll */
    { 0,			16 },	/* food */
    { 0,			 7 },	/* weapon */
    { 0,			 7 },	/* armor */
    { 0,			 4 },	/* ring */
    { 0,			 4 },	/* stick */
};

struct obj_info arm_info[MAXARMORS] = {
    { "革の鎧",		 20,	 20, NULL, FALSE },
    { "リングメイル",		 15,	 25, NULL, FALSE },
    { "鋲付き革鎧",	 15,	 20, NULL, FALSE },
    { "スケイルメイル",		 13,	 30, NULL, FALSE },
    { "チェインメイル",		 12,	 75, NULL, FALSE },
    { "スプリントメイル",		 10,	 80, NULL, FALSE },
    { "バンデッドメイル",		 10,	 90, NULL, FALSE },
    { "プレートメイル",		  5,	150, NULL, FALSE },
};
struct obj_info pot_info[MAXPOTIONS] = {
    { "混乱の薬",		 7,   5, NULL, FALSE },
    { "幻覚の薬",		 8,   5, NULL, FALSE },
    { "毒薬",			 8,   5, NULL, FALSE },
    { "腕力増強の薬",		13, 150, NULL, FALSE },
    { "不可視視認の薬",		 3, 100, NULL, FALSE },
    { "治療の薬",		13, 130, NULL, FALSE },
    { "怪物感知の薬",		 6, 130, NULL, FALSE },
    { "魔力感知の薬",	 	 6, 105, NULL, FALSE },
    { "レベル向上の薬",		 2, 250, NULL, FALSE },
    { "超治療の薬",		 5, 200, NULL, FALSE },
    { "加速の薬",		 5, 190, NULL, FALSE },
    { "腕力回復の薬",	13, 130, NULL, FALSE },
    { "盲目の薬",		 5,   5, NULL, FALSE },
    { "浮遊の薬",		 6,  75, NULL, FALSE },
};
struct obj_info ring_info[MAXRINGS] = {
    { "守りの指輪",		 9, 400, NULL, FALSE },
    { "腕力上昇の指輪",		 9, 400, NULL, FALSE },
    { "腕力維持の指輪",	 5, 280, NULL, FALSE },
    { "探索の指輪",		10, 420, NULL, FALSE },
    { "不可視視認の指輪",	10, 310, NULL, FALSE },
    { "装飾の指輪",		 1,  10, NULL, FALSE },
    { "怪物覚醒の指輪",	10,  10, NULL, FALSE },
    { "器用さの指輪",		 8, 440, NULL, FALSE },
    { "打撃力上昇の指輪",	 8, 400, NULL, FALSE },
    { "再生の指輪",		 4, 460, NULL, FALSE },
    { "消化遅延の指輪",		 9, 240, NULL, FALSE },
    { "テレポートの指輪",	 5,  30, NULL, FALSE },
    { "忍び足の指輪",		 7, 470, NULL, FALSE },
    { "鎧維持の指輪",		 5, 380, NULL, FALSE },
};
struct obj_info scr_info[MAXSCROLLS] = {
    { "怪物混乱の巻物",		 7, 140, NULL, FALSE },
    { "魔法の地図の巻物",		 4, 150, NULL, FALSE },
    { "怪物金縛りの巻物",		 2, 180, NULL, FALSE },
    { "睡眠の巻物",			 3,   5, NULL, FALSE },
    { "鎧強化の巻物",			 7, 160, NULL, FALSE },
    { "薬識別の巻物",		10,  80, NULL, FALSE },
    { "巻物識別の巻物",		10,  80, NULL, FALSE },
    { "武器識別の巻物",		 6,  80, NULL, FALSE },
    { "鎧識別の巻物",		 	 7, 100, NULL, FALSE },
    { "装飾・杖識別の巻物",	10, 115, NULL, FALSE },
    { "怪物退散の巻物",			 3, 200, NULL, FALSE },
    { "食料感知の巻物",			 2,  60, NULL, FALSE },
    { "テレポートの巻物",		 5, 165, NULL, FALSE },
    { "武器強化の巻物",			 8, 150, NULL, FALSE },
    { "怪物生成の巻物",			 4,  75, NULL, FALSE },
    { "解呪の巻物",			 7, 105, NULL, FALSE },
    { "怪物覚醒の巻物",		 3,  20, NULL, FALSE },
    { "鎧保護の巻物",			 2, 250, NULL, FALSE },
};
struct obj_info weap_info[MAXWEAPONS + 1] = {
    { "メイス",				11,   8, NULL, FALSE },
    { "ロングソード",			11,  15, NULL, FALSE },
    { "ショートボウ",			12,  15, NULL, FALSE },
    { "矢",				12,   1, NULL, FALSE },
    { "ダガー",				 8,   3, NULL, FALSE },
    { "ツヴァイハンダー",		10,  75, NULL, FALSE },
    { "ダーツ",				12,   2, NULL, FALSE },
    { "手裏剣",				12,   5, NULL, FALSE },
    { "スピア",				12,   5, NULL, FALSE },
    { NULL, 0 },	/* DO NOT REMOVE: fake entry for dragon's breath */
};
struct obj_info ws_info[MAXSTICKS] = {
    { "明かりの杖",			12, 250, NULL, FALSE },
    { "不可視の杖",		 6,   5, NULL, FALSE },
    { "電撃の杖",		 3, 330, NULL, FALSE },
    { "火炎の杖",			 3, 330, NULL, FALSE },
    { "冷気の杖",			 3, 330, NULL, FALSE },
    { "変身の杖",		15, 310, NULL, FALSE },
    { "魔法の矢の杖",		10, 170, NULL, FALSE },
    { "怪物加速の杖",		10,   5, NULL, FALSE },
    { "怪物減速の杖",		11, 350, NULL, FALSE },
    { "生命力吸収の杖",		 9, 300, NULL, FALSE },
    { "無効果の杖",		 1,   5, NULL, FALSE },
    { "遠隔テレポートの杖",		 6, 340, NULL, FALSE },
    { "引き寄せの杖",		 6,  50, NULL, FALSE },
    { "魔力消去の杖",		 5, 280, NULL, FALSE },
};

struct h_list helpstr[] = {
    {'?',	"	ヘルプを表示",				TRUE},
    {'/',	"	シンボルの説明",			TRUE},
    {'h',	"	左へ移動",					TRUE},
    {'j',	"	下へ移動",					TRUE},
    {'k',	"	上へ移動",					TRUE},
    {'l',	"	右へ移動",					TRUE},
    {'y',	"	左上へ移動",				TRUE},
    {'u',	"	右上へ移動",				TRUE},
    {'b',	"	左下へ移動",				TRUE},
    {'n',	"	右下へ移動",				TRUE},
    {'H',	"	左へ走る",				FALSE},
    {'J',	"	下へ走る",				FALSE},
    {'K',	"	上へ走る",				FALSE},
    {'L',	"	右へ走る",				FALSE},
    {'Y',	"	左上へ走る",				FALSE},
    {'U',	"	右上へ走る",				FALSE},
    {'B',	"	左下へ走る",				FALSE},
    {'N',	"	右下へ走る",				FALSE},
    {CTRL('H'),	"	隣接するまで左へ走る",		FALSE},
    {CTRL('J'),	"	隣接するまで下へ走る",		FALSE},
    {CTRL('K'),	"	隣接するまで上へ走る",		FALSE},
    {CTRL('L'),	"	隣接するまで右へ走る",		FALSE},
    {CTRL('Y'),	"	隣接するまで左上へ走る",		FALSE},
    {CTRL('U'),	"	隣接するまで右上へ走る",		FALSE},
    {CTRL('B'),	"	隣接するまで左下へ走る",		FALSE},
    {CTRL('N'),	"	隣接するまで右下へ走る",		FALSE},
    {'\0',	"	<SHIFT><方向>: その方向へ走る",		TRUE},
    {'\0',	"	<CTRL><方向>: 隣接するまで走る",	TRUE},
    {'f',	"<方向>	死ぬか瀕死まで戦う",	TRUE},
    {'t',	"<方向>	アイテムを投げる",			TRUE},
    {'m',	"<方向>	拾わずに移動する",		TRUE},
    {'z',	"<方向>	その方向へ杖を振る",		TRUE},
    {'^',	"<方向>	罠の種類を調べる",			TRUE},
    {'s',	"	隠し扉・罠を探索",		TRUE},
    {'>',	"	階段を降りる",			TRUE},
    {'<',	"	階段を登る",			TRUE},
    {'.',	"	1ターン休む",			TRUE},
    {',',	"	足元の物を拾う",			TRUE},
    {'i',	"	持ち物一覧",				TRUE},
    {'I',	"	持ち物を1つ確認",			TRUE},
    {'q',	"	薬を飲む",				TRUE},
    {'r',	"	巻物を読む",				TRUE},
    {'e',	"	食料を食べる",				TRUE},
    {'w',	"	武器を装備する",			TRUE},
    {'W',	"	鎧を着る",				TRUE},
    {'T',	"	鎧を脱ぐ",				TRUE},
    {'P',	"	指輪をはめる",				TRUE},
    {'R',	"	指輪を外す",				TRUE},
    {'d',	"	アイテムを置く（捨てる）",	TRUE},
    {'c',	"	アイテムに名前をつける",	TRUE},
    {'a',	"	直前のコマンドを繰り返す",	TRUE},
    {')',	"	装備中の武器を表示",		TRUE},
    {']',	"	装備中の鎧を表示",			TRUE},
    {'=',	"	装備中の指輪を表示",		TRUE},
    {'@',	"	現在の能力値を表示",		TRUE},
    {'D',	"	識別済みのアイテム一覧",	TRUE},
    {'o',	"	オプション確認・変更",		TRUE},
    {CTRL('R'),	"	画面を再描画",				TRUE},
    {CTRL('P'),	"	直前のメッセージを再表示",	TRUE},
    {ESCAPE,	"	コマンドをキャンセル",		TRUE},
    {'S',	"	ゲームを中断保存",			TRUE},
    {'Q',	"	ゲーム終了",				TRUE},
    {'!',	"	シェルへ一時復帰",			TRUE},
    {'F',	"<方向>	どちらかが死ぬまで戦う",	TRUE},
    {'v',	"	バージョンを表示",			TRUE},
    {0,		NULL }
};
