#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define APP_W 1440
#define APP_H 900
#define TAB_COUNT 6
#define AGENT_COUNT 4

static const char *TAB_NAMES[TAB_COUNT] = {
    "OVERVIEW", "AGENTS", "TERMINAL", "TASKS", "ACTIVITY", "SETTINGS"
};

typedef struct Agent {
    const char *name;
    const char *role;
    const char *status;
    const char *description;
    const char *tasks;
    int running;
} Agent;

static Agent g_agents[AGENT_COUNT] = {
    {"HERMES", "Research & Analysis", "Active", "Finds, reads and synthesizes project knowledge.", "3 tasks", 1},
    {"CODER-01", "Build & Prototype", "Working", "Turns ideas into working artifacts and experiments.", "4 tasks", 1},
    {"RESEARCH", "Write & Communicate", "Active", "Creates clear, refined notes and project output.", "2 tasks", 1},
    {"REVIEWER", "Strategy & Planning", "Active", "Breaks down complexity and maps the next steps.", "3 tasks", 1}
};

static HWND g_hwnd;
static int g_active_tab = 0;
static int g_cpu = 31;
static int g_actions = 1428;
static HFONT g_brand_font;
static HFONT g_hero_font;
static HFONT g_hero_small;
static HFONT g_heading_font;
static HFONT g_ui_font;
static HFONT g_ui_bold;
static HFONT g_mono_font;
static HFONT g_mono_small;
static HFONT g_tiny_font;

static const COLORREF C_BG = RGB(7,7,6);
static const COLORREF C_BG_2 = RGB(12,11,9);
static const COLORREF C_PANEL = RGB(13,14,13);
static const COLORREF C_PANEL_2 = RGB(18,18,16);
static const COLORREF C_PANEL_3 = RGB(23,21,18);
static const COLORREF C_GOLD = RGB(201,142,62);
static const COLORREF C_GOLD_SOFT = RGB(153,108,58);
static const COLORREF C_IVORY = RGB(235,222,198);
static const COLORREF C_MUTED = RGB(162,151,133);
static const COLORREF C_DIM = RGB(100,92,81);
static const COLORREF C_LINE = RGB(79,61,43);
static const COLORREF C_LINE_SOFT = RGB(47,40,33);
static const COLORREF C_PAPER = RGB(208,177,136);
static const COLORREF C_PAPER_DARK = RGB(147,112,76);
static const COLORREF C_INK = RGB(37,27,20);
static const COLORREF C_GREEN = RGB(82,213,142);
static const COLORREF C_AMBER = RGB(223,155,52);

static void fill_rect_c(HDC dc, int x, int y, int w, int h, COLORREF color) {
    RECT r = {x, y, x + w, y + h};
    HBRUSH b = CreateSolidBrush(color);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

static void line_c(HDC dc, int x1, int y1, int x2, int y2, COLORREF color, int width) {
    HPEN p = CreatePen(PS_SOLID, width, color);
    HPEN old = (HPEN)SelectObject(dc, p);
    MoveToEx(dc, x1, y1, NULL);
    LineTo(dc, x2, y2);
    SelectObject(dc, old);
    DeleteObject(p);
}

static void text_at(HDC dc, HFONT font, COLORREF color, int x, int y, const char *text) {
    HFONT old = (HFONT)SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    TextOutA(dc, x, y, text, (int)strlen(text));
    SelectObject(dc, old);
}

static void text_box(HDC dc, HFONT font, COLORREF color, int x, int y, int w, int h, const char *text, UINT flags) {
    RECT r = {x, y, x + w, y + h};
    HFONT old = (HFONT)SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    DrawTextA(dc, text, -1, &r, flags);
    SelectObject(dc, old);
}

static void panel(HDC dc, int x, int y, int w, int h, COLORREF fill, COLORREF edge) {
    HBRUSH b = CreateSolidBrush(fill);
    HPEN p = CreatePen(PS_SOLID, 1, edge);
    HBRUSH ob = (HBRUSH)SelectObject(dc, b);
    HPEN op = (HPEN)SelectObject(dc, p);
    Rectangle(dc, x, y, x + w, y + h);
    SelectObject(dc, op);
    SelectObject(dc, ob);
    DeleteObject(p);
    DeleteObject(b);
}

static void round_panel(HDC dc, int x, int y, int w, int h, int radius, COLORREF fill, COLORREF edge) {
    HBRUSH b = CreateSolidBrush(fill);
    HPEN p = CreatePen(PS_SOLID, 1, edge);
    HBRUSH ob = (HBRUSH)SelectObject(dc, b);
    HPEN op = (HPEN)SelectObject(dc, p);
    RoundRect(dc, x, y, x + w, y + h, radius, radius);
    SelectObject(dc, op);
    SelectObject(dc, ob);
    DeleteObject(p);
    DeleteObject(b);
}

static void paper_polygon(HDC dc, int x, int y, int w, int h, COLORREF fill, COLORREF edge) {
    POINT p[14];
    HBRUSH b, ob;
    HPEN pen, op;
    p[0].x=x+9; p[0].y=y+1; p[1].x=x+w-16; p[1].y=y; p[2].x=x+w; p[2].y=y+10;
    p[3].x=x+w-5; p[3].y=y+h/3; p[4].x=x+w; p[4].y=y+h-12; p[5].x=x+w-11; p[5].y=y+h;
    p[6].x=x+w/2+15; p[6].y=y+h-2; p[7].x=x+w/2-9; p[7].y=y+h; p[8].x=x+13; p[8].y=y+h-1;
    p[9].x=x; p[9].y=y+h-10; p[10].x=x+2; p[10].y=y+h/2+7; p[11].x=x; p[11].y=y+h/3;
    p[12].x=x+4; p[12].y=y+11; p[13].x=x+11; p[13].y=y+4;
    b = CreateSolidBrush(fill);
    pen = CreatePen(PS_SOLID, 1, edge);
    ob = (HBRUSH)SelectObject(dc, b);
    op = (HPEN)SelectObject(dc, pen);
    Polygon(dc, p, 14);
    SelectObject(dc, op);
    SelectObject(dc, ob);
    DeleteObject(pen);
    DeleteObject(b);
}

static void circle_fill(HDC dc, int cx, int cy, int r, COLORREF color) {
    HBRUSH b = CreateSolidBrush(color);
    HBRUSH ob = (HBRUSH)SelectObject(dc, b);
    HPEN p = CreatePen(PS_SOLID, 1, color);
    HPEN op = (HPEN)SelectObject(dc, p);
    Ellipse(dc, cx-r, cy-r, cx+r, cy+r);
    SelectObject(dc, op);
    SelectObject(dc, ob);
    DeleteObject(p);
    DeleteObject(b);
}

static void status_dot(HDC dc, int x, int y, COLORREF color) {
    circle_fill(dc, x, y, 5, color);
}

static void draw_eye(HDC dc, int cx, int cy, int w, int h, COLORREF iris) {
    POINT pts[8];
    HBRUSH b, ob;
    HPEN p, op;
    pts[0].x=cx-w/2; pts[0].y=cy;
    pts[1].x=cx-w/4; pts[1].y=cy-h/2;
    pts[2].x=cx; pts[2].y=cy-h/2-4;
    pts[3].x=cx+w/4; pts[3].y=cy-h/2;
    pts[4].x=cx+w/2; pts[4].y=cy;
    pts[5].x=cx+w/4; pts[5].y=cy+h/2;
    pts[6].x=cx; pts[6].y=cy+h/2+4;
    pts[7].x=cx-w/4; pts[7].y=cy+h/2;
    b = CreateSolidBrush(RGB(186,169,145));
    p = CreatePen(PS_SOLID, 1, RGB(82,64,46));
    ob = (HBRUSH)SelectObject(dc, b);
    op = (HPEN)SelectObject(dc, p);
    Polygon(dc, pts, 8);
    SelectObject(dc, op);
    SelectObject(dc, ob);
    DeleteObject(p);
    DeleteObject(b);
    circle_fill(dc, cx, cy, h/3, iris);
    circle_fill(dc, cx, cy, h/8, C_BG);
}

static void draw_celestial(HDC dc, int cx, int cy, int r) {
    int i;
    HPEN p = CreatePen(PS_SOLID, 1, RGB(105,77,49));
    HPEN op = (HPEN)SelectObject(dc, p);
    HBRUSH b = CreateSolidBrush(RGB(115,78,44));
    HBRUSH ob = (HBRUSH)SelectObject(dc, b);
    Ellipse(dc, cx-r, cy-r, cx+r, cy+r);
    SelectObject(dc, ob);
    DeleteObject(b);
    for(i=0;i<24;i++) {
        double a = 6.28318530718 * i / 24.0;
        int x1=(int)(cx+(r+3)*cos(a));
        int y1=(int)(cy+(r+3)*sin(a));
        int x2=(int)(cx+(r+74)*cos(a));
        int y2=(int)(cy+(r+74)*sin(a));
        MoveToEx(dc,x1,y1,NULL); LineTo(dc,x2,y2);
    }
    Ellipse(dc,cx-r/2,cy-r/2,cx+r/2,cy+r/2);
    SelectObject(dc, op);
    DeleteObject(p);
}

static void draw_bust(HDC dc, int x, int y, int s, int facing_right) {
    COLORREF stone = RGB(150,139,122);
    COLORREF shadow = RGB(68,59,50);
    HBRUSH b = CreateSolidBrush(stone);
    HPEN p = CreatePen(PS_SOLID, 1, shadow);
    HBRUSH ob = (HBRUSH)SelectObject(dc, b);
    HPEN op = (HPEN)SelectObject(dc, p);
    POINT torso[6];
    int fx = facing_right ? 1 : -1;
    Ellipse(dc, x+s*28/100, y, x+s*70/100, y+s*46/100);
    torso[0].x=x+s*25/100; torso[0].y=y+s*40/100;
    torso[1].x=x+s*73/100; torso[1].y=y+s*42/100;
    torso[2].x=x+s*88/100; torso[2].y=y+s*66/100;
    torso[3].x=x+s; torso[3].y=y+s;
    torso[4].x=x+s*5/100; torso[4].y=y+s;
    torso[5].x=x+s*10/100; torso[5].y=y+s*66/100;
    Polygon(dc, torso, 6);
    SelectObject(dc, op);
    SelectObject(dc, ob);
    DeleteObject(p);
    DeleteObject(b);
    line_c(dc, x+s*42/100, y+s*20/100, x+s*(42+fx*13)/100, y+s*20/100, shadow, 1);
    line_c(dc, x+s*49/100, y+s*27/100, x+s*(49+fx*5)/100, y+s*35/100, shadow, 1);
    line_c(dc, x+s*37/100, y+s*9/100, x+s*61/100, y+s*6/100, RGB(92,79,65), 2);
}

static void draw_wing(HDC dc, int x, int y, int w, int h) {
    int i;
    HPEN p = CreatePen(PS_SOLID, 1, RGB(137,122,101));
    HPEN op = (HPEN)SelectObject(dc, p);
    for(i=0;i<9;i++) {
        MoveToEx(dc, x, y+h/2, NULL);
        LineTo(dc, x+w-i*7, y+i*7);
        LineTo(dc, x+w-12-i*5, y+h-i*5);
    }
    SelectObject(dc, op);
    DeleteObject(p);
}

static void draw_butterfly(HDC dc, int cx, int cy, int s) {
    HPEN p = CreatePen(PS_SOLID, 1, RGB(120,83,49));
    HPEN op = (HPEN)SelectObject(dc, p);
    HBRUSH b = CreateSolidBrush(RGB(96,61,40));
    HBRUSH ob = (HBRUSH)SelectObject(dc, b);
    Ellipse(dc,cx-s,cy-s/2,cx,cy+s/2);
    Ellipse(dc,cx,cy-s/2,cx+s,cy+s/2);
    SelectObject(dc,ob);
    DeleteObject(b);
    line_c(dc,cx,cy-s/2,cx,cy+s/2,RGB(40,31,25),2);
    SelectObject(dc,op);
    DeleteObject(p);
}

static void draw_texture(HDC dc) {
    int i;
    for(i=0;i<190;i++) {
        int x=(i*83+37)%APP_W;
        int y=(i*47+19)%APP_H;
        COLORREF c=(i%4==0)?RGB(26,22,18):RGB(18,17,14);
        fill_rect_c(dc,x,y,1+(i%3),1,c);
    }
    for(i=0;i<16;i++) {
        int x=35+i*91;
        line_c(dc,x,92,x-115,335,RGB(24,21,18),1);
    }
}

static void draw_edge_collage(HDC dc) {
    draw_bust(dc,-30,106,235,1);
    draw_eye(dc,95,560,170,62,RGB(121,76,41));
    draw_butterfly(dc,123,458,47);
    draw_bust(dc,12,676,150,0);
    text_at(dc,g_mono_small,C_MUTED,29,306,"INTELLIGENCE");
    text_at(dc,g_mono_small,C_MUTED,29,324,"IN SERVICE");
    text_at(dc,g_mono_small,C_MUTED,29,342,"OF A MORE");
    text_at(dc,g_mono_small,C_MUTED,29,360,"HUMAN MIND.");
    line_c(dc,28,384,47,384,C_PAPER_DARK,2);

    draw_eye(dc,1322,143,176,70,RGB(111,75,43));
    draw_bust(dc,1274,458,190,0);
    draw_celestial(dc,1286,103,78);
    draw_bust(dc,1285,704,155,1);
    text_at(dc,g_mono_small,C_MUTED,1311,710,"THINK");
    text_at(dc,g_mono_small,C_MUTED,1311,728,"BUILD");
    text_at(dc,g_mono_small,C_MUTED,1311,746,"DELEGATE");
    text_at(dc,g_mono_small,C_MUTED,1311,764,"REFINE");
    text_at(dc,g_mono_small,C_MUTED,1311,782,"REPEAT");
}

static int tab_width(int i) {
    static const int widths[TAB_COUNT] = {88,74,88,68,86,86};
    return widths[i];
}

static int tab_start_x(void) { return 290; }

static void draw_header(HDC dc) {
    int x=tab_start_x();
    int i;
    char timebuf[32], datebuf[32];
    SYSTEMTIME st;
    fill_rect_c(dc,0,0,APP_W,92,RGB(8,8,7));
    line_c(dc,0,91,APP_W,91,C_LINE,1);
    text_at(dc,g_brand_font,C_IVORY,48,18,"NODUS");
    text_at(dc,g_tiny_font,RGB(177,153,119),50,63,"IDEAS. AGENTS. ACTIONS. TOGETHER.");

    for(i=0;i<TAB_COUNT;i++) {
        int tw=tab_width(i);
        if(i==g_active_tab) {
            paper_polygon(dc,x-9,30,tw+18,36,C_GOLD,RGB(107,72,38));
            text_at(dc,g_mono_small,C_INK,x+6,42,TAB_NAMES[i]);
        } else {
            text_at(dc,g_mono_small,C_MUTED,x+5,42,TAB_NAMES[i]);
        }
        x += tw + 17;
    }

    round_panel(dc,918,29,264,43,26,RGB(11,11,10),C_LINE);
    circle_fill(dc,942,50,7,RGB(13,13,12));
    {
        HPEN p=CreatePen(PS_SOLID,2,C_IVORY), op=(HPEN)SelectObject(dc,p);
        HBRUSH hb=(HBRUSH)GetStockObject(NULL_BRUSH), oldb=(HBRUSH)SelectObject(dc,hb);
        Ellipse(dc,936,44,948,56);
        MoveToEx(dc,947,55,NULL); LineTo(dc,952,60);
        SelectObject(dc,oldb); SelectObject(dc,op); DeleteObject(p);
    }
    text_at(dc,g_ui_font,C_DIM,962,42,"Search agents, tasks, or knowledge...");

    circle_fill(dc,1210,50,19,RGB(13,13,12));
    {
        HPEN p=CreatePen(PS_SOLID,2,C_GOLD), op=(HPEN)SelectObject(dc,p);
        MoveToEx(dc,1198,52,NULL); LineTo(dc,1204,52); LineTo(dc,1208,42); LineTo(dc,1213,59); LineTo(dc,1217,47); LineTo(dc,1223,47);
        SelectObject(dc,op); DeleteObject(p);
    }

    GetLocalTime(&st);
    wsprintfA(timebuf,"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
    wsprintfA(datebuf,"%02d/%02d/%04d",st.wDay,st.wMonth,st.wYear);
    text_at(dc,g_mono_font,C_IVORY,1243,29,timebuf);
    text_at(dc,g_tiny_font,C_MUTED,1245,54,datebuf);

    circle_fill(dc,1366,49,25,RGB(54,47,39));
    circle_fill(dc,1366,49,21,RGB(172,155,133));
    circle_fill(dc,1366,46,8,RGB(58,51,44));
    line_c(dc,1357,61,1366,52,RGB(58,51,44),2);
    line_c(dc,1366,52,1376,61,RGB(58,51,44),2);

    text_at(dc,g_tiny_font,C_DIM,1348,4,"-");
    text_at(dc,g_tiny_font,C_DIM,1376,4,"[]");
    text_at(dc,g_tiny_font,C_MUTED,1412,4,"x");
}

static void draw_hero(HDC dc) {
    draw_celestial(dc,865,173,74);
    draw_wing(dc,979,133,104,127);
    draw_bust(dc,980,139,132,0);
    line_c(dc,934,170,1005,122,RGB(145,126,101),3);
    circle_fill(dc,933,170,7,RGB(166,145,113));

    text_at(dc,g_hero_font,C_IVORY,282,116,"A MORE");
    text_at(dc,g_hero_font,C_IVORY,282,164,"THOUGHTFUL");
    text_at(dc,g_hero_font,C_IVORY,282,212,"TOMORROW,");
    text_at(dc,g_hero_small,RGB(218,188,143),286,269,"C O M P O S E D   T O D A Y .");

    text_at(dc,g_tiny_font,C_MUTED,669,196,"HUMAN");
    text_at(dc,g_tiny_font,C_MUTED,669,213,"DIRECTION");
    text_at(dc,g_tiny_font,C_MUTED,669,237,"MACHINE");
    text_at(dc,g_tiny_font,C_MUTED,669,254,"EFFORT");
    text_at(dc,g_tiny_font,C_MUTED,669,278,"GREATER");
    text_at(dc,g_tiny_font,C_MUTED,669,295,"POSSIBILITIES");
    line_c(dc,670,317,690,317,C_PAPER_DARK,2);

    paper_polygon(dc,1211,211,145,100,RGB(210,187,153),RGB(121,91,62));
    text_at(dc,g_mono_small,C_INK,1239,231,"SAME");
    text_at(dc,g_mono_small,C_INK,1239,249,"QUESTIONS.");
    text_at(dc,g_mono_small,C_INK,1239,268,"DEEPER");
    text_at(dc,g_mono_small,C_INK,1239,286,"ANSWERS.");
}

static void draw_metric_card(HDC dc, int x, int y, int w, const char *label, const char *value, const char *note, int icon) {
    panel(dc,x,y,w,96,C_PANEL,C_LINE);
    circle_fill(dc,x+38,y+47,20,RGB(18,18,16));
    {
        HPEN p=CreatePen(PS_SOLID,2,C_GOLD), op=(HPEN)SelectObject(dc,p);
        if(icon==0) {
            Ellipse(dc,x+30,y+39,x+38,y+47); Ellipse(dc,x+40,y+37,x+47,y+44);
            Arc(dc,x+26,y+45,x+43,y+59,0,0,0,0); Arc(dc,x+36,y+43,x+51,y+57,0,0,0,0);
        } else if(icon==1) {
            Rectangle(dc,x+31,y+39,x+45,y+54); MoveToEx(dc,x+34,y+46,NULL); LineTo(dc,x+38,y+50); LineTo(dc,x+43,y+43);
        } else if(icon==2) {
            MoveToEx(dc,x+26,y+50,NULL); LineTo(dc,x+32,y+50); LineTo(dc,x+36,y+38); LineTo(dc,x+41,y+60); LineTo(dc,x+46,y+45); LineTo(dc,x+51,y+45);
        } else {
            Ellipse(dc,x+30,y+39,x+46,y+55); Ellipse(dc,x+35,y+44,x+41,y+50);
        }
        SelectObject(dc,op); DeleteObject(p);
    }
    text_at(dc,g_ui_font,C_IVORY,x+77,y+18,label);
    text_at(dc,g_heading_font,C_IVORY,x+77,y+40,value);
    if(note && note[0]) {
        if(note[0]=='+') status_dot(dc,x+78,y+79,C_GREEN);
        text_at(dc,g_tiny_font,C_MUTED,x+(note[0]=='+'?89:77),y+73,note);
    }
}

static void draw_agent_card(HDC dc, int x, int y, int w, int index) {
    Agent *a=&g_agents[index];
    COLORREF status=(index==1)?C_AMBER:C_GREEN;
    panel(dc,x,y,w,202,RGB(14,15,14),C_LINE_SOFT);
    circle_fill(dc,x+42,y+42,28,RGB(31,29,25));
    draw_bust(dc,x+16,y+13,52,(index%2)==0);
    status_dot(dc,x+70,y+69,status);
    text_at(dc,g_tiny_font,C_MUTED,x+w-34,y+15,"...");
    text_at(dc,g_heading_font,C_IVORY,x+14,y+79,a->name);
    text_at(dc,g_ui_font,C_MUTED,x+14,y+105,a->role);
    text_box(dc,g_ui_font,RGB(187,178,164),x+14,y+130,w-28,42,a->description,DT_WORDBREAK);
    round_panel(dc,x+14,y+177,72,22,9,RGB(17,34,26),RGB(42,94,61));
    status_dot(dc,x+25,y+188,status);
    text_at(dc,g_tiny_font,status,x+35,y+181,a->status);
    round_panel(dc,x+w-70,y+177,56,22,9,RGB(20,20,18),C_LINE_SOFT);
    text_at(dc,g_tiny_font,C_MUTED,x+w-60,y+181,a->tasks);
}

static void draw_agents_block(HDC dc) {
    int i;
    int x=206,y=451,w=736,h=237;
    panel(dc,x,y,w,h,RGB(11,12,11),C_LINE);
    text_at(dc,g_heading_font,C_IVORY,x+20,y+15,"A G E N T S");
    line_c(dc,x+124,y+30,x+148,y+30,C_GOLD_SOFT,1);
    text_at(dc,g_tiny_font,C_GOLD,x+w-79,y+20,"View all ->");
    for(i=0;i<4;i++) draw_agent_card(dc,x+14+i*178,y+45,168,i);
}

static void draw_tasks_block(HDC dc) {
    int x=206,y=698,w=736,h=169;
    const char *names[4]={"Compile competitive landscape","Build interactive map prototype","Write launch announcement","Plan Q4 strategic initiatives"};
    const char *agent[4]={"HERMES","CODER-01","RESEARCH","REVIEWER"};
    const char *state[4]={"Running","Running","Queued","Queued"};
    const char *time[4]={"14m left","32m left","-","-"};
    int i;
    panel(dc,x,y,w,h,RGB(11,12,11),C_LINE);
    text_at(dc,g_heading_font,C_IVORY,x+20,y+12,"T A S K S");
    line_c(dc,x+103,y+27,x+128,y+27,C_GOLD_SOFT,1);
    text_at(dc,g_tiny_font,C_GOLD,x+w-78,y+18,"View all ->");
    for(i=0;i<4;i++) {
        int yy=y+45+i*29;
        HPEN p=CreatePen(PS_SOLID,1,C_MUTED),op=(HPEN)SelectObject(dc,p);
        HBRUSH hb=(HBRUSH)GetStockObject(NULL_BRUSH),ob=(HBRUSH)SelectObject(dc,hb);
        Rectangle(dc,x+23,yy+2,x+35,yy+14);
        SelectObject(dc,ob); SelectObject(dc,op); DeleteObject(p);
        text_at(dc,g_ui_font,C_IVORY,x+54,yy,names[i]);
        circle_fill(dc,x+292,yy+8,9,RGB(96,84,68));
        text_at(dc,g_ui_font,C_MUTED,x+310,yy,agent[i]);
        round_panel(dc,x+424,yy-2,70,20,9,RGB(25,24,21),C_LINE_SOFT);
        text_at(dc,g_tiny_font,(i<2)?C_AMBER:C_MUTED,x+440,yy+1,state[i]);
        text_at(dc,g_ui_font,C_MUTED,x+535,yy,time[i]);
        text_at(dc,g_tiny_font,C_MUTED,x+w-44,yy,"...");
        if(i<3) line_c(dc,x+18,yy+24,x+w-18,yy+24,C_LINE_SOFT,1);
    }
}

static void draw_activity_block(HDC dc) {
    int x=951,y=451,w=328,h=416;
    const char *title[7]={"Hermes completed research","Research generated document","Coder-01 deployed prototype","Reviewer added a new task","Task completed","You updated agent settings","Coder-01 started a task"};
    const char *sub[7]={"Trends in generative cities","Market analysis brief v2","nodus-map v0.3","Evaluate partnership opportunities","Summarize Q3 learnings","Hermes - knowledge sources","Build data visualization"};
    const char *ago[7]={"2m ago","12m ago","28m ago","1h ago","2h ago","3h ago","3h ago"};
    int i;
    panel(dc,x,y,w,h,RGB(11,12,11),C_LINE);
    text_at(dc,g_heading_font,C_IVORY,x+18,y+15,"A C T I V I T Y");
    line_c(dc,x+145,y+30,x+168,y+30,C_GOLD_SOFT,1);
    text_at(dc,g_tiny_font,C_GOLD,x+w-72,y+20,"View all ->");
    for(i=0;i<7;i++) {
        int yy=y+57+i*48;
        COLORREF c=(i==0||i==4)?C_GREEN:((i==6)?C_AMBER:C_IVORY);
        circle_fill(dc,x+24,yy+6,6,c);
        text_at(dc,g_ui_font,C_IVORY,x+47,yy-2,title[i]);
        text_at(dc,g_ui_font,C_MUTED,x+47,yy+16,sub[i]);
        text_at(dc,g_tiny_font,C_MUTED,x+w-56,yy,ago[i]);
    }
}

static void draw_overview(HDC dc) {
    char actions[32];
    draw_hero(dc);
    draw_metric_card(dc,216,340,235,"Agents Online","4 / 6","+ 3 active · 1 idle",0);
    draw_metric_card(dc,462,340,235,"Tasks Running","12","+ 3 completed today",1);
    wsprintfA(actions,"%d",g_actions);
    draw_metric_card(dc,708,340,235,"Total Actions",actions,"+18% vs. yesterday",2);
    draw_metric_card(dc,954,340,235,"System Health","98%","+ All systems nominal",3);
    draw_agents_block(dc);
    draw_tasks_block(dc);
    draw_activity_block(dc);

    paper_polygon(dc,54,645,115,86,RGB(199,173,139),RGB(107,81,57));
    text_at(dc,g_tiny_font,C_INK,73,663,"CURIOSITY");
    text_at(dc,g_tiny_font,C_INK,73,680,"CREATES");
    text_at(dc,g_tiny_font,C_INK,73,697,"BETTER");
    text_at(dc,g_tiny_font,C_INK,73,714,"WORLDS.");
}

static void page_title(HDC dc,const char *eyebrow,const char *title) {
    text_at(dc,g_mono_small,C_GOLD,210,128,eyebrow);
    text_at(dc,g_hero_small,C_IVORY,210,153,title);
    line_c(dc,210,205,1235,205,C_LINE,1);
}

static void draw_agents_page(HDC dc) {
    int i;
    page_title(dc,"REGISTRY / AGENT NODES","Agents");
    for(i=0;i<4;i++) {
        int x=220+(i%2)*505;
        int y=242+(i/2)*275;
        panel(dc,x,y,470,238,C_PANEL,C_LINE);
        circle_fill(dc,x+61,y+61,38,RGB(38,34,29));
        draw_bust(dc,x+27,y+23,70,(i%2)==0);
        text_at(dc,g_heading_font,C_IVORY,x+120,y+29,g_agents[i].name);
        text_at(dc,g_ui_font,C_MUTED,x+120,y+64,g_agents[i].role);
        status_dot(dc,x+122,y+101,(i==1)?C_AMBER:C_GREEN);
        text_at(dc,g_ui_font,(i==1)?C_AMBER:C_GREEN,x+138,y+93,g_agents[i].status);
        text_box(dc,g_ui_font,C_IVORY,x+28,y+139,410,52,g_agents[i].description,DT_WORDBREAK);
    }
}

static void draw_terminal_page(HDC dc) {
    page_title(dc,"LOCAL SHELL / PREVIEW","Terminal");
    panel(dc,220,236,1000,560,RGB(8,9,8),C_LINE);
    text_at(dc,g_heading_font,C_IVORY,247,260,"Hermes terminal");
    line_c(dc,247,304,1194,304,C_LINE_SOFT,1);
    text_at(dc,g_mono_font,C_GOLD,250,338,"PS C:\\Agents\\Hermes>");
    text_at(dc,g_mono_font,C_IVORY,450,338,"hermes --resume");
    text_at(dc,g_mono_font,C_GREEN,250,374,"[OK]");
    text_at(dc,g_mono_font,C_MUTED,297,374,"workspace loaded");
    text_at(dc,g_mono_font,C_MUTED,250,408,"[17:48:09] Nodus interface ready");
    text_at(dc,g_mono_font,C_MUTED,250,440,"[17:48:11] waiting for next instruction");
    text_at(dc,g_mono_font,C_GOLD,250,490,"PS C:\\Agents\\Hermes>");
    text_at(dc,g_mono_font,C_IVORY,450,490,"_");
    text_at(dc,g_tiny_font,C_MUTED,250,755,"Simulation shell. Process bridge comes after the interface shell is approved.");
}

static void draw_tasks_page(HDC dc) {
    const char *cols[4]={"BACKLOG","RUNNING","REVIEW","DONE"};
    int i;
    page_title(dc,"OPERATIONS / TASK BOARD","Tasks");
    for(i=0;i<4;i++) {
        int x=210+i*255;
        panel(dc,x,242,235,530,(i%2)?C_PANEL_2:C_PANEL,C_LINE);
        text_at(dc,g_ui_bold,C_IVORY,x+18,262,cols[i]);
        line_c(dc,x+18,291,x+217,291,C_LINE_SOFT,1);
        panel(dc,x+14,316,207,122,RGB(25,23,20),C_LINE_SOFT);
        text_at(dc,g_ui_bold,C_IVORY,x+28,333,(i==0)?"Inspect auth flow":(i==1)?"Nodus redesign":(i==2)?"Review navigation":"Index project");
        text_box(dc,g_ui_font,C_MUTED,x+28,361,175,56,"Assigned node and a concise summary of the current task.",DT_WORDBREAK);
    }
}

static void draw_activity_page(HDC dc) {
    int i;
    const char *events[6]={"Hermes completed research","Coder-01 finished a build","Research generated a document","Reviewer created a new task","System metrics refreshed","Nodus session started"};
    page_title(dc,"AUDIT STREAM","Activity");
    panel(dc,220,242,1000,530,C_PANEL,C_LINE);
    for(i=0;i<6;i++) {
        int y=278+i*72;
        status_dot(dc,250,y+8,(i<2)?C_GREEN:C_GOLD);
        text_at(dc,g_ui_bold,C_IVORY,280,y,events[i]);
        text_at(dc,g_ui_font,C_MUTED,280,y+24,"Recorded in the local project activity stream.");
        text_at(dc,g_tiny_font,C_DIM,1100,y+5,(i==0)?"now":"recent");
        if(i<5) line_c(dc,250,y+53,1188,y+53,C_LINE_SOFT,1);
    }
}

static void draw_settings_page(HDC dc) {
    page_title(dc,"CONTROL PLANE","Settings");
    panel(dc,220,242,480,290,C_PANEL,C_LINE);
    text_at(dc,g_heading_font,C_IVORY,247,268,"Interface");
    text_at(dc,g_ui_font,C_MUTED,247,321,"Theme"); text_at(dc,g_ui_font,C_IVORY,410,321,"Nodus Classical Collage");
    text_at(dc,g_ui_font,C_MUTED,247,358,"Density"); text_at(dc,g_ui_font,C_IVORY,410,358,"Balanced");
    text_at(dc,g_ui_font,C_MUTED,247,395,"Motion"); text_at(dc,g_ui_font,C_IVORY,410,395,"Minimal");
    panel(dc,720,242,500,290,C_PANEL_2,C_LINE);
    text_at(dc,g_heading_font,C_IVORY,747,268,"Agent defaults");
    text_at(dc,g_ui_font,C_MUTED,747,321,"Workspace"); text_at(dc,g_ui_font,C_IVORY,910,321,"C:\\Agents");
    text_at(dc,g_ui_font,C_MUTED,747,358,"Shell"); text_at(dc,g_ui_font,C_IVORY,910,358,"PowerShell");
    text_at(dc,g_ui_font,C_MUTED,747,395,"Runtime"); text_at(dc,g_ui_font,C_IVORY,910,395,"Local first");
    text_at(dc,g_tiny_font,C_MUTED,220,790,"Nodus 0.4.0 design branch · Native C / Win32");
}

static void create_fonts(void) {
    g_brand_font=CreateFontA(-39,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_hero_font=CreateFontA(-49,0,0,0,FW_NORMAL,TRUE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_hero_small=CreateFontA(-26,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_heading_font=CreateFontA(-23,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_ui_font=CreateFontA(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_ui_bold=CreateFontA(-14,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_mono_font=CreateFontA(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
    g_mono_small=CreateFontA(-11,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
    g_tiny_font=CreateFontA(-10,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
}

static void destroy_fonts(void) {
    DeleteObject(g_brand_font); DeleteObject(g_hero_font); DeleteObject(g_hero_small); DeleteObject(g_heading_font);
    DeleteObject(g_ui_font); DeleteObject(g_ui_bold); DeleteObject(g_mono_font); DeleteObject(g_mono_small); DeleteObject(g_tiny_font);
}

static int hit_tab(int mx, int my) {
    int x=tab_start_x();
    int i;
    if(my<24 || my>78) return -1;
    for(i=0;i<TAB_COUNT;i++) {
        int tw=tab_width(i);
        if(mx>=x-9 && mx<=x+tw+9) return i;
        x += tw + 17;
    }
    return -1;
}

static void actual_to_design(HWND hwnd, int ax, int ay, int *dx, int *dy) {
    RECT r;
    GetClientRect(hwnd,&r);
    *dx = (r.right>0)?MulDiv(ax,APP_W,r.right):ax;
    *dy = (r.bottom>0)?MulDiv(ay,APP_H,r.bottom):ay;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
        case WM_NCCALCSIZE:
            if(wp) return 0;
            break;
        case WM_GETMINMAXINFO: {
            MINMAXINFO *mmi=(MINMAXINFO*)lp;
            mmi->ptMinTrackSize.x=1024;
            mmi->ptMinTrackSize.y=640;
            return 0;
        }
        case WM_NCHITTEST: {
            POINT pt;
            RECT cr;
            int dx,dy;
            const int g=7;
            pt.x=(short)LOWORD(lp); pt.y=(short)HIWORD(lp);
            ScreenToClient(hwnd,&pt);
            GetClientRect(hwnd,&cr);
            if(pt.x<g && pt.y<g) return HTTOPLEFT;
            if(pt.x>=cr.right-g && pt.y<g) return HTTOPRIGHT;
            if(pt.x<g && pt.y>=cr.bottom-g) return HTBOTTOMLEFT;
            if(pt.x>=cr.right-g && pt.y>=cr.bottom-g) return HTBOTTOMRIGHT;
            if(pt.x<g) return HTLEFT;
            if(pt.x>=cr.right-g) return HTRIGHT;
            if(pt.y<g) return HTTOP;
            if(pt.y>=cr.bottom-g) return HTBOTTOM;
            actual_to_design(hwnd,pt.x,pt.y,&dx,&dy);
            if(dy<92 && hit_tab(dx,dy)<0 && !(dx>=910 && dx<=1398 && dy>=20 && dy<=79)) return HTCAPTION;
            return HTCLIENT;
        }
        case WM_LBUTTONDOWN: {
            int ax=(short)LOWORD(lp), ay=(short)HIWORD(lp), mx,my,tab;
            actual_to_design(hwnd,ax,ay,&mx,&my);
            if(my<22 && mx>=1400) { PostMessageA(hwnd,WM_CLOSE,0,0); return 0; }
            if(my<22 && mx>=1366) { ShowWindow(hwnd,IsZoomed(hwnd)?SW_RESTORE:SW_MAXIMIZE); return 0; }
            if(my<22 && mx>=1336) { ShowWindow(hwnd,SW_MINIMIZE); return 0; }
            tab=hit_tab(mx,my);
            if(tab>=0) { g_active_tab=tab; InvalidateRect(hwnd,NULL,FALSE); return 0; }
            break;
        }
        case WM_TIMER:
            g_cpu=26+(GetTickCount()/1000)%13;
            g_actions=1428+(GetTickCount()/5000)%37;
            InvalidateRect(hwnd,NULL,FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            RECT cr;
            HDC dc=BeginPaint(hwnd,&ps);
            HDC mem;
            HBITMAP bmp,oldbmp;
            GetClientRect(hwnd,&cr);
            mem=CreateCompatibleDC(dc);
            bmp=CreateCompatibleBitmap(dc,APP_W,APP_H);
            oldbmp=(HBITMAP)SelectObject(mem,bmp);
            fill_rect_c(mem,0,0,APP_W,APP_H,C_BG);
            draw_texture(mem);
            draw_edge_collage(mem);
            draw_header(mem);
            if(g_active_tab==0) draw_overview(mem);
            else if(g_active_tab==1) draw_agents_page(mem);
            else if(g_active_tab==2) draw_terminal_page(mem);
            else if(g_active_tab==3) draw_tasks_page(mem);
            else if(g_active_tab==4) draw_activity_page(mem);
            else draw_settings_page(mem);
            SetStretchBltMode(dc,HALFTONE);
            StretchBlt(dc,0,0,cr.right,cr.bottom,mem,0,0,APP_W,APP_H,SRCCOPY);
            SelectObject(mem,oldbmp);
            DeleteObject(bmp);
            DeleteDC(mem);
            EndPaint(hwnd,&ps);
            return 0;
        }
        case WM_DESTROY:
            KillTimer(hwnd,1);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd,msg,wp,lp);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) {
    WNDCLASSEXA wc;
    MSG msg;
    int sw,sh,x,y;
    (void)prev; (void)cmd;
    ZeroMemory(&wc,sizeof(wc));
    wc.cbSize=sizeof(wc);
    wc.lpfnWndProc=WndProc;
    wc.hInstance=inst;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName="NodusNativeWindow";
    create_fonts();
    if(!RegisterClassExA(&wc)) {
        MessageBoxA(NULL,"Could not register window class.","Nodus",MB_ICONERROR);
        destroy_fonts();
        return 1;
    }
    sw=GetSystemMetrics(SM_CXSCREEN);
    sh=GetSystemMetrics(SM_CYSCREEN);
    x=(sw-APP_W)/2;
    y=(sh-APP_H)/2;
    g_hwnd=CreateWindowExA(0,wc.lpszClassName,"NODUS",WS_POPUP|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU,x,y,APP_W,APP_H,NULL,NULL,inst,NULL);
    if(!g_hwnd) {
        MessageBoxA(NULL,"Could not create main window.","Nodus",MB_ICONERROR);
        destroy_fonts();
        return 1;
    }
    ShowWindow(g_hwnd,show);
    UpdateWindow(g_hwnd);
    SetTimer(g_hwnd,1,1000,NULL);
    while(GetMessageA(&msg,NULL,0,0)>0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    destroy_fonts();
    return (int)msg.wParam;
}
