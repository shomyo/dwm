/* See LICENSE file for copyright and license details. */
#include "gaplessgrid.c"
#include <X11/XF86keysym.h>

/* appearance */
#define NUMCOLORS 8
static const char colors[NUMCOLORS][MAXCOLORS][8] = {
	/*  border   foreground  background  */
	{ "#3d3d3d", "#999999", "#303030" },  /* x01 = darkgray  */
	{ "#999999", "#d0d0d0", "#303030" },  /* x02 = lightgray */
	{ "#303030", "#e7940f", "#303030" },  /* x03 = yellow    */
	{ "#303030", "#c23b2d", "#303030" },  /* x04 = red       */
	{ "#303030", "#2aaf4d", "#303030" },  /* x05 = green     */
	{ "#303030", "#2f97de", "#303030" },  /* x06 = blue      */
	{ "#303030", "#9442b6", "#303030" },  /* x07 = magenta   */
	{ "#303030", "#0ecca7", "#303030" },  /* x08 = cyan      */
};
static const char *fonts[] = {
	"ohsnap.icons:size=14:antialias=true:autohint=true",
	"Roboto Mono:pixelsize=11:antialias=true:autohint=true",
};
static const char dmenufont[] = "ohsnap.icons:size=14";
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int gappx     = 8;        /* gap pixel between windows */
static const unsigned int snap      = 16;       /* snap pixel */
static const Bool showbar           = True;     /* False means no bar */
static const Bool topbar            = True;     /* False means bottom bar */

/* tagging */
static const char *tags[] = { "main", "web", "dev", "media", "temp" };

static const Rule rules[] = {
	/* xprop(1):
	 * WM_CLASS(STRING) = instance, class
	 * WM_NAME(STRING) = title
	 */
	/* class          instance    title       tags mask     isfloating   monitor */
	{ "chromium",     NULL,       NULL,       1 << 1,       False,       -1 },
	{ "Electrum",     NULL,       NULL,       0,            True,        -1 },
	{ "Firefox",      NULL,       NULL,       1 << 4,       False,       -1 },
	{ "Gimp",         NULL,       NULL,       0,            True,        -1 },
	{ "Steam",        NULL,       NULL,       0,            True,        -1 },
	{ "Transmission", NULL,       NULL,       1 << 3,       False,       -1 },
};

/* layout(s) */
static const float mfact      = 0.60; /* factor of master area size [0.05..0.95] */
static const int nmaster      = 1;    /* number of clients in master area */
static const Bool resizehints = False; /* True means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[t]",      tile },    /* first entry is default */
	{ "[f]",      NULL },    /* no layout function means floating behavior */
	{ "[m]",      monocle },
	{ "[#]",      gaplessgrid },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", colors[0][2], "-nf", colors[1][1], "-sb", colors[0][2], "-sf", colors[5][1], NULL };
static const char *termcmd[]       = { "st", "-g", "140x35+550+350", NULL };
static const char *urxvtcmd[]      = { "urxvt", NULL };
static const char *wwwcmd[]        = { "chromium", NULL };
static const char *mutecmd[]       = { "amixer", "set", "Master", "toggle", NULL };
static const char *volupcmd[]      = { "amixer", "set", "Master", "5%+", NULL };
static const char *voldncmd[]      = { "amixer", "set", "Master", "5%-", NULL };
static const char *mpctogglecmd[]  = { "mpc", "toggle", NULL };
static const char *mpcstopcmd[]    = { "mpc", "stop", NULL };
static const char *mpcprevcmd[]    = { "mpc", "prev", NULL };
static const char *mpcnextcmd[]    = { "mpc", "next", NULL };
static const char *mpcvolupcmd[]   = { "mpc", "volume", "+5", NULL };
static const char *mpcvoldowncmd[] = { "mpc", "volume", "-5", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_x,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_grave,  spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = urxvtcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.01} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.01} },
	{ MODKEY|ShiftMask,             XK_j,      setcfact,       {.f = +0.020} },
	{ MODKEY|ShiftMask,             XK_k,      setcfact,       {.f = -0.020} },
	{ MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_g,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_p,      quit,           {0} },
	/* custom */
	{ False,                        XF86XK_HomePage,           spawn,          {.v = wwwcmd } },
	{ False,                        XF86XK_AudioPlay,          spawn,          {.v = mpctogglecmd } },
	{ False,                        XF86XK_AudioStop,          spawn,          {.v = mpcstopcmd } },
	{ False,                        XF86XK_AudioPrev,          spawn,          {.v = mpcprevcmd } },
	{ False,                        XF86XK_AudioNext,          spawn,          {.v = mpcnextcmd } },
	{ MODKEY,                       XK_KP_Add,                 spawn,          {.v = mpcvolupcmd } },
	{ MODKEY,                       XK_KP_Subtract,            spawn,          {.v = mpcvoldowncmd  } },
	{ False,                        XF86XK_AudioMute,          spawn,          {.v = mutecmd  } },
	{ False,                        XF86XK_AudioRaiseVolume,   spawn,          {.v = volupcmd  } },
	{ False,                        XF86XK_AudioLowerVolume,   spawn,          {.v = voldncmd  } },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

