#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define APP_W 1360
#define APP_H 820
#define TAB_COUNT 6
#define AGENT_COUNT 4

static const char *TAB_NAMES[TAB_COUNT] = {
    "OVERVIEW", "AGENTS", "TERMINAL", "TASKS", "ACTIVITY", "SETTINGS"
};

typedef struct Agent {
    const char *name;
    const char *role;
    const char *status;
    const char *task;
    int cpu;
    int running;
} Agent;

static Agent g_agents[AGENT_COUNT] = {
    {"HERMES", "Primary coding agent", "RUNNING", "Refactoring WebSocket service", 18, 1},
    {"CODER-01", "Implementation agent", "RUNNING", "Running frontend tests", 9, 1},
    {"RESEARCH", "Research and analysis", "IDLE", "Waiting for objective", 1, 0},
    {"REVIEWER", "Review and QA", "STOPPED", "No active session", 0, 0}
};

static HWND g_hwnd;
static int g_active_tab = 0;
static int g_cpu = 31;
static int g_ram_tenths = 74;
static HFONT g_title_font;
static HFONT g_heading_font;
static HFONT g_ui_font;
static HFONT g_ui_bold;
static HFONT g_mono_font;
static HFONT g_mono_small;

static const COLORREF C_BG = RGB(12,10,9);
static const COLORREF C_PANEL = RGB(27,23,20);
static const COLORREF C_PANEL_2 = RGB(38,31,26);
static const COLORREF C_PAPER = RGB(198,181,151);
static const COLORREF C_PAPER_2 = RGB(154,133,104);
static const COLORREF C_INK = RGB(31,25,21);
static const COLORREF C_IVORY = RGB(233,221,200);
static const COLORREF C_MUTED = RGB(139,129,116);
static const COLORREF C_OCHRE = RGB(188,119,35);
static const COLORREF C_RUST = RGB(140,58,42);
static const COLORREF C_GREEN = RGB(102,145,108);
static const COLORREF C_LINE = RGB(58,49,42);

static void fill_rect_c(HDC dc, int x, int y, int w, int h, COLORREF color) {
    RECT r;
    HBRUSH b;
    r.left=x; r.top=y; r.right=x+w; r.bottom=y+h;
    b=CreateSolidBrush(color);
    FillRect(dc,&r,b);
    DeleteObject(b);
}

static void line_c(HDC dc, int x1, int y1, int x2, int y2, COLORREF color, int width) {
    HPEN p=CreatePen(PS_SOLID,width,color);
    HPEN old=(HPEN)SelectObject(dc,p);
    MoveToEx(dc,x1,y1,NULL);
    LineTo(dc,x2,y2);
    SelectObject(dc,old);
    DeleteObject(p);
}

static void text_at(HDC dc, HFONT font, COLORREF color, int x, int y, const char *text) {
    HFONT old=(HFONT)SelectObject(dc,font);
    SetTextColor(dc,color);
    SetBkMode(dc,TRANSPARENT);
    TextOutA(dc,x,y,text,(int)strlen(text));
    SelectObject(dc,old);
}

static void text_box(HDC dc, HFONT font, COLORREF color, int x, int y, int w, int h, const char *text, UINT flags) {
    RECT r;
    HFONT old;
    r.left=x; r.top=y; r.right=x+w; r.bottom=y+h;
    old=(HFONT)SelectObject(dc,font);
    SetTextColor(dc,color);
    SetBkMode(dc,TRANSPARENT);
    DrawTextA(dc,text,-1,&r,flags);
    SelectObject(dc,old);
}

static void torn_paper(HDC dc, int x, int y, int w, int h, COLORREF fill, COLORREF edge) {
    POINT pts[12];
    HBRUSH b,oldb;
    HPEN p,oldp;
    pts[0].x=x+7; pts[0].y=y;
    pts[1].x=x+w-16; pts[1].y=y+3;
    pts[2].x=x+w; pts[2].y=y+11;
    pts[3].x=x+w-5; pts[3].y=y+h/3;
    pts[4].x=x+w; pts[4].y=y+h-13;
    pts[5].x=x+w-10; pts[5].y=y+h;
    pts[6].x=x+w/2; pts[6].y=y+h-3;
    pts[7].x=x+11; pts[7].y=y+h;
    pts[8].x=x; pts[8].y=y+h-14;
    pts[9].x=x+4; pts[9].y=y+h/2;
    pts[10].x=x; pts[10].y=y+13;
    pts[11].x=x+9; pts[11].y=y+4;
    b=CreateSolidBrush(fill);
    p=CreatePen(PS_SOLID,1,edge);
    oldb=(HBRUSH)SelectObject(dc,b);
    oldp=(HPEN)SelectObject(dc,p);
    Polygon(dc,pts,12);
    SelectObject(dc,oldp);
    SelectObject(dc,oldb);
    DeleteObject(p);
    DeleteObject(b);
}

static void draw_eye(HDC dc, int cx, int cy, int w, int h) {
    POINT pts[8];
    HBRUSH b,oldb;
    HPEN p,oldp;
    pts[0].x=cx-w/2; pts[0].y=cy;
    pts[1].x=cx-w/4; pts[1].y=cy-h/2;
    pts[2].x=cx; pts[2].y=cy-h/2-2;
    pts[3].x=cx+w/4; pts[3].y=cy-h/2;
    pts[4].x=cx+w/2; pts[4].y=cy;
    pts[5].x=cx+w/4; pts[5].y=cy+h/2;
    pts[6].x=cx; pts[6].y=cy+h/2+2;
    pts[7].x=cx-w/4; pts[7].y=cy+h/2;
    b=CreateSolidBrush(C_IVORY);
    p=CreatePen(PS_SOLID,2,C_INK);
    oldb=(HBRUSH)SelectObject(dc,b);
    oldp=(HPEN)SelectObject(dc,p);
    Polygon(dc,pts,8);
    SelectObject(dc,oldp);
    SelectObject(dc,oldb);
    DeleteObject(p);
    DeleteObject(b);
    b=CreateSolidBrush(C_OCHRE);
    oldb=(HBRUSH)SelectObject(dc,b);
    Ellipse(dc,cx-19,cy-19,cx+19,cy+19);
    SelectObject(dc,oldb);
    DeleteObject(b);
    b=CreateSolidBrush(C_INK);
    oldb=(HBRUSH)SelectObject(dc,b);
    Ellipse(dc,cx-7,cy-7,cx+7,cy+7);
    SelectObject(dc,oldb);
    DeleteObject(b);
}

static void draw_bust(HDC dc, int x, int y) {
    HBRUSH b,oldb;
    HPEN p,oldp;
    POINT torso[4];
    COLORREF stone=RGB(146,137,122);
    b=CreateSolidBrush(stone);
    p=CreatePen(PS_SOLID,1,RGB(71,63,55));
    oldb=(HBRUSH)SelectObject(dc,b);
    oldp=(HPEN)SelectObject(dc,p);
    Ellipse(dc,x+37,y,x+89,y+72);
    torso[0].x=x+25; torso[0].y=y+68;
    torso[1].x=x+101; torso[1].y=y+68;
    torso[2].x=x+122; torso[2].y=y+139;
    torso[3].x=x+3; torso[3].y=y+139;
    Polygon(dc,torso,4);
    SelectObject(dc,oldp);
    SelectObject(dc,oldb);
    DeleteObject(p);
    DeleteObject(b);
    line_c(dc,x+51,y+32,x+74,y+32,RGB(72,63,55),1);
    line_c(dc,x+61,y+39,x+66,y+54,RGB(72,63,55),1);
    line_c(dc,x+51,y+58,x+73,y+58,RGB(72,63,55),1);
}

static void draw_background(HDC dc, int w, int h) {
    int i;
    fill_rect_c(dc,0,0,w,h,C_BG);
    torn_paper(dc,18,108,210,126,RGB(34,29,25),RGB(61,50,42));
    for(i=0;i<9;i++) line_c(dc,30,125+i*11,207-i*4,125+i*11,RGB(85,72,61),1);
    torn_paper(dc,w-285,94,244,168,RGB(48,40,33),RGB(82,66,52));
    draw_eye(dc,w-165,176,166,64);
    torn_paper(dc,w-214,h-220,174,160,RGB(31,27,23),RGB(63,52,44));
    draw_bust(dc,w-197,h-203);
    line_c(dc,0,98,w,98,C_LINE,1);
    line_c(dc,0,h-32,w,h-32,RGB(42,36,31),1);
}

static int tab_width(int index) {
    if(index==0) return 94;
    if(index==4) return 106;
    return 98;
}

static void draw_header(HDC dc, int w) {
    int x=246;
    int i;
    char timebuf[32];
    SYSTEMTIME st;
    text_at(dc,g_heading_font,C_IVORY,26,13,"NODUS");
    text_at(dc,g_mono_small,C_MUTED,29,49,"LOCAL AGENT ORCHESTRATION");
    for(i=0;i<TAB_COUNT;i++) {
        int tw=tab_width(i);
        if(i==g_active_tab) {
            torn_paper(dc,x-7,29,tw,34,C_OCHRE,RGB(118,75,29));
            text_at(dc,g_mono_small,C_INK,x+4,40,TAB_NAMES[i]);
        } else {
            text_at(dc,g_mono_small,C_MUTED,x+4,40,TAB_NAMES[i]);
        }
        x+=tw+9;
    }
    GetLocalTime(&st);
    wsprintfA(timebuf,"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
    text_at(dc,g_mono_font,C_MUTED,w-112,29,timebuf);
}

static void status_dot(HDC dc, int x, int y, int running) {
    HBRUSH b=CreateSolidBrush(running?C_GREEN:RGB(103,98,90));
    HBRUSH old=(HBRUSH)SelectObject(dc,b);
    Ellipse(dc,x,y,x+7,y+7);
    SelectObject(dc,old);
    DeleteObject(b);
}

static void metric(HDC dc, int x, int y, int w, const char *label, const char *value, const char *note, COLORREF paper) {
    torn_paper(dc,x,y,w,104,paper,RGB(78,63,49));
    text_at(dc,g_mono_small,C_INK,x+14,y+13,label);
    text_at(dc,g_heading_font,C_INK,x+13,y+35,value);
    text_at(dc,g_mono_small,RGB(89,72,55),x+14,y+81,note);
}

static void draw_overview(HDC dc, int w, int h) {
    int y=117;
    int mw=(w-318)/4;
    char cpu[24],ram[24];
    int i;
    torn_paper(dc,250,y,w-560,224,C_PANEL,RGB(72,59,48));
    text_at(dc,g_mono_small,C_OCHRE,275,y+20,"CONTROL NODE / 01");
    text_at(dc,g_title_font,C_IVORY,273,y+47,"Mission Control");
    text_box(dc,g_ui_font,C_MUTED,277,y+112,520,58,"A local room for watching, organizing and later commanding your AI agents. Native C, local-first and intentionally lightweight.",DT_WORDBREAK);
    line_c(dc,277,y+180,790,y+180,RGB(69,57,46),1);
    text_at(dc,g_mono_small,C_MUTED,277,y+194,"2 ACTIVE / 4 REGISTERED / 12 TASKS TODAY");

    torn_paper(dc,w-276,y+17,214,192,C_PAPER,RGB(109,87,65));
    text_at(dc,g_mono_small,C_INK,w-253,y+34,"ANATOMY OF CONTROL");
    draw_eye(dc,w-170,y+106,146,56);
    text_box(dc,g_heading_font,C_INK,w-250,y+143,175,40,"The machine watches the watchers.",DT_CENTER|DT_WORDBREAK);

    wsprintfA(cpu,"%d%%",g_cpu);
    wsprintfA(ram,"%d.%d GB",g_ram_tenths/10,g_ram_tenths%10);
    metric(dc,28,y+245,mw,"ACTIVE AGENTS","02","1 idle / 1 stopped",RGB(194,179,151));
    metric(dc,42+mw,y+250,mw,"CPU LOAD",cpu,"simulated in V0.2",RGB(155,135,107));
    metric(dc,56+mw*2,y+244,mw,"MEMORY",ram,"of 16 GB",RGB(181,159,124));
    metric(dc,70+mw*3,y+252,mw,"TASK QUEUE","05","2 high priority",RGB(137,115,89));

    torn_paper(dc,28,y+375,(w-86)/2,h-y-430,C_PANEL,RGB(68,56,47));
    text_at(dc,g_mono_small,C_OCHRE,50,y+393,"AGENTS / LIVE GRID");
    for(i=0;i<AGENT_COUNT;i++) {
        int ay=y+430+i*55;
        status_dot(dc,52,ay+4,g_agents[i].running);
        text_at(dc,g_ui_bold,C_IVORY,69,ay,g_agents[i].name);
        text_at(dc,g_ui_font,C_MUTED,175,ay,g_agents[i].task);
        text_at(dc,g_mono_small,g_agents[i].running?C_GREEN:C_MUTED,50+(w-86)/2-118,ay,g_agents[i].status);
        if(i<AGENT_COUNT-1) line_c(dc,50,ay+33,28+(w-86)/2-24,ay+33,RGB(48,41,35),1);
    }

    torn_paper(dc,43+(w-86)/2,y+382,(w-86)/2-15,h-y-437,C_PANEL_2,RGB(74,60,49));
    text_at(dc,g_mono_small,C_OCHRE,64+(w-86)/2,y+400,"RECENT ACTIVITY");
    text_at(dc,g_ui_bold,C_IVORY,64+(w-86)/2,y+438,"16:43  HERMES");
    text_at(dc,g_ui_font,C_MUTED,64+(w-86)/2,y+460,"Waiting for next instruction");
    text_at(dc,g_ui_bold,C_IVORY,64+(w-86)/2,y+494,"16:42  CODER-01");
    text_at(dc,g_ui_font,C_MUTED,64+(w-86)/2,y+516,"Frontend test pass completed");
    text_at(dc,g_ui_bold,C_IVORY,64+(w-86)/2,y+550,"16:40  HERMES");
    text_at(dc,g_ui_font,C_MUTED,64+(w-86)/2,y+572,"websocket.service.ts indexed");
}

static void draw_agents(HDC dc, int w, int h) {
    int i;
    int cardw=(w-112)/2;
    (void)h;
    text_at(dc,g_mono_small,C_OCHRE,31,119,"REGISTRY / LOCAL NODES");
    text_at(dc,g_title_font,C_IVORY,29,144,"Agents");
    text_at(dc,g_ui_font,C_MUTED,32,201,"Registered agents and their current state.");
    for(i=0;i<AGENT_COUNT;i++) {
        int col=i%2;
        int row=i/2;
        int x=28+col*(cardw+20);
        int y=248+row*190;
        COLORREF paper=(i%2)?C_PAPER_2:C_PAPER;
        torn_paper(dc,x,y,cardw,164,paper,RGB(92,73,55));
        text_at(dc,g_mono_small,C_INK,x+18,y+17,g_agents[i].status);
        text_at(dc,g_heading_font,C_INK,x+17,y+44,g_agents[i].name);
        text_at(dc,g_ui_font,RGB(80,65,50),x+18,y+82,g_agents[i].role);
        text_box(dc,g_ui_font,RGB(72,58,45),x+18,y+108,cardw-36,42,g_agents[i].task,DT_WORDBREAK);
        status_dot(dc,x+cardw-30,y+20,g_agents[i].running);
    }
}

static void draw_terminal(HDC dc, int w, int h) {
    text_at(dc,g_mono_small,C_OCHRE,31,119,"LOCAL SHELL / PREVIEW");
    text_at(dc,g_title_font,C_IVORY,29,144,"Terminal");
    text_at(dc,g_ui_font,C_MUTED,32,201,"Simulation only. Real process execution arrives with CreateProcess + pipes.");
    torn_paper(dc,28,244,w-56,h-307,RGB(24,21,18),RGB(70,58,47));
    text_at(dc,g_mono_font,C_MUTED,52,273,"NODUS NODE / HERMES");
    line_c(dc,50,298,w-75,298,RGB(53,45,38),1);
    text_at(dc,g_mono_font,C_OCHRE,52,323,"PS C:\\Agents\\Hermes>");
    text_at(dc,g_mono_font,C_IVORY,250,323,"hermes --resume");
    text_at(dc,g_mono_font,C_GREEN,52,354,"[ok] workspace loaded");
    text_at(dc,g_mono_font,C_MUTED,52,384,"[16:43:08] reading project context...");
    text_at(dc,g_mono_font,C_MUTED,52,414,"[16:43:11] websocket.service.ts indexed");
    text_at(dc,g_mono_font,C_MUTED,52,444,"[16:43:14] waiting for next instruction");
    text_at(dc,g_mono_font,C_OCHRE,52,h-104,"PS C:\\Agents\\Hermes>");
    line_c(dc,250,h-91,w-78,h-91,C_OCHRE,1);
}

static void draw_tasks(HDC dc, int w, int h) {
    const char *cols[4]={"BACKLOG","RUNNING","REVIEW","DONE"};
    const char *top[4]={"Inspect auth flow","WebSocket refactor","Patient navigation","Project indexing"};
    const char *bottom[4]={"Map deployment steps","Frontend tests","Review queue","Agent dashboard V1"};
    int i;
    int cw=(w-100)/4;
    text_at(dc,g_mono_small,C_OCHRE,31,119,"OPERATIONS / TASK BOARD");
    text_at(dc,g_title_font,C_IVORY,29,144,"Tasks");
    for(i=0;i<4;i++) {
        int x=28+i*(cw+14);
        torn_paper(dc,x,225,cw,h-288,C_PANEL,RGB(68,56,47));
        text_at(dc,g_mono_small,C_OCHRE,x+15,245,cols[i]);
        torn_paper(dc,x+13,286,cw-26,112,C_PAPER,RGB(100,82,62));
        text_box(dc,g_ui_bold,C_INK,x+26,302,cw-52,35,top[i],DT_WORDBREAK);
        text_at(dc,g_mono_small,RGB(91,72,54),x+26,362,i==1?"HERMES / HIGH":"REVIEWER / MEDIUM");
        torn_paper(dc,x+17,418,cw-34,112,C_PAPER_2,RGB(92,74,57));
        text_box(dc,g_ui_bold,C_INK,x+30,434,cw-60,35,bottom[i],DT_WORDBREAK);
        text_at(dc,g_mono_small,RGB(75,60,46),x+30,495,i==3?"SYSTEM / DONE":"CODER-01 / MEDIUM");
    }
}

static void draw_activity(HDC dc, int w, int h) {
    const char *times[6]={"16:43:14","16:42:57","16:40:31","16:37:18","16:33:02","16:29:40"};
    const char *agents[6]={"HERMES","CODER-01","HERMES","RESEARCH","SYSTEM","REVIEWER"};
    const char *events[6]={"Waiting for next instruction","Frontend test pass completed","websocket.service.ts indexed","Research task moved to idle","Nodus initialized","Review queue checked"};
    int i;
    text_at(dc,g_mono_small,C_OCHRE,31,119,"AUDIT STREAM / ARCHIVE");
    text_at(dc,g_title_font,C_IVORY,29,144,"Activity");
    torn_paper(dc,28,233,w-56,h-296,C_PANEL,RGB(70,58,47));
    for(i=0;i<6;i++) {
        int y=273+i*67;
        text_at(dc,g_mono_small,RGB(101,89,75),54,y,times[i]);
        line_c(dc,130,y+6,165,y+6,C_OCHRE,1);
        status_dot(dc,176,y+2,i<3);
        text_at(dc,g_ui_bold,C_IVORY,195,y-5,agents[i]);
        text_at(dc,g_ui_font,C_MUTED,195,y+16,events[i]);
        line_c(dc,195,y+43,w-64,y+43,RGB(48,41,35),1);
    }
}

static void draw_settings(HDC dc, int w, int h) {
    int cw=(w-92)/3;
    int x1=28,x2=46+cw,x3=64+cw*2;
    int y=252;
    (void)h;
    text_at(dc,g_mono_small,C_OCHRE,31,119,"CONTROL PLANE / LOCAL ONLY");
    text_at(dc,g_title_font,C_IVORY,29,144,"Settings");
    text_at(dc,g_ui_font,C_MUTED,32,201,"Nodus stays local-first and lightweight.");
    torn_paper(dc,x1,y,cw,278,C_PAPER,RGB(99,81,61));
    text_at(dc,g_mono_small,C_INK,x1+17,y+18,"INTERFACE");
    text_at(dc,g_heading_font,C_INK,x1+16,y+48,"Collage only");
    text_box(dc,g_ui_font,RGB(77,63,49),x1+17,y+91,cw-34,92,"Dark paper, old engravings, anatomy, classical cutouts and warm aged tones. No cyberpunk layer.",DT_WORDBREAK);
    text_at(dc,g_mono_small,RGB(82,67,51),x1+18,y+216,"TOP NAVIGATION / ENABLED");
    torn_paper(dc,x2,y+8,cw,278,C_PAPER_2,RGB(92,74,57));
    text_at(dc,g_mono_small,C_INK,x2+17,y+26,"RUNTIME");
    text_at(dc,g_heading_font,C_INK,x2+16,y+56,"Native C");
    text_box(dc,g_ui_font,RGB(67,55,43),x2+17,y+99,cw-34,92,"No HTML, JavaScript, Electron or embedded browser. Win32 drawing and native Windows APIs.",DT_WORDBREAK);
    text_at(dc,g_mono_small,RGB(76,60,46),x2+18,y+224,"LIGHTWEIGHT / TARGET");
    torn_paper(dc,x3,y-5,cw,286,C_PANEL_2,RGB(78,64,51));
    text_at(dc,g_mono_small,C_OCHRE,x3+17,y+13,"NEXT STEP");
    text_at(dc,g_heading_font,C_IVORY,x3+16,y+43,"Real agents");
    text_box(dc,g_ui_font,C_MUTED,x3+17,y+87,cw-34,112,"Connect each node to a Windows process with CreateProcess, pipes and live stdout/stderr streaming.",DT_WORDBREAK);
    text_at(dc,g_mono_small,C_RUST,x3+18,y+220,"STATUS / NOT CONNECTED");
}

static void draw_footer(HDC dc, int w, int h) {
    text_at(dc,g_mono_small,RGB(76,69,61),24,h-23,"NODUS / C NATIVE BUILD");
    text_at(dc,g_mono_small,RGB(76,69,61),w-176,h-23,"LOCAL NODE / 0.2.1");
}

static int hit_tab(int mx, int my) {
    int x=246;
    int i;
    if(my<26 || my>70) return -1;
    for(i=0;i<TAB_COUNT;i++) {
        int tw=tab_width(i);
        if(mx>=x-7 && mx<=x+tw-7) return i;
        x+=tw+9;
    }
    return -1;
}

static void paint_app(HWND hwnd) {
    PAINTSTRUCT ps;
    RECT rc;
    HDC dc,mem;
    HBITMAP bmp,oldbmp;
    int w,h;
    dc=BeginPaint(hwnd,&ps);
    GetClientRect(hwnd,&rc);
    w=rc.right; h=rc.bottom;
    mem=CreateCompatibleDC(dc);
    bmp=CreateCompatibleBitmap(dc,w,h);
    oldbmp=(HBITMAP)SelectObject(mem,bmp);
    draw_background(mem,w,h);
    draw_header(mem,w);
    switch(g_active_tab) {
        case 0: draw_overview(mem,w,h); break;
        case 1: draw_agents(mem,w,h); break;
        case 2: draw_terminal(mem,w,h); break;
        case 3: draw_tasks(mem,w,h); break;
        case 4: draw_activity(mem,w,h); break;
        case 5: draw_settings(mem,w,h); break;
    }
    draw_footer(mem,w,h);
    BitBlt(dc,0,0,w,h,mem,0,0,SRCCOPY);
    SelectObject(mem,oldbmp);
    DeleteObject(bmp);
    DeleteDC(mem);
    EndPaint(hwnd,&ps);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
        case WM_CREATE:
            SetTimer(hwnd,1,1000,NULL);
            return 0;
        case WM_TIMER:
            g_cpu+=3; if(g_cpu>42) g_cpu=24;
            g_ram_tenths+=1; if(g_ram_tenths>81) g_ram_tenths=69;
            InvalidateRect(hwnd,NULL,FALSE);
            return 0;
        case WM_LBUTTONDOWN: {
            int mx=(int)(short)LOWORD(lp);
            int my=(int)(short)HIWORD(lp);
            int tab=hit_tab(mx,my);
            if(tab>=0) { g_active_tab=tab; InvalidateRect(hwnd,NULL,FALSE); }
            return 0;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO *mmi=(MINMAXINFO*)lp;
            mmi->ptMinTrackSize.x=1000;
            mmi->ptMinTrackSize.y=660;
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            paint_app(hwnd);
            return 0;
        case WM_DESTROY:
            KillTimer(hwnd,1);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd,msg,wp,lp);
}

static void create_fonts(void) {
    g_title_font=CreateFontA(-46,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"Georgia");
    g_heading_font=CreateFontA(-25,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"Georgia");
    g_ui_font=CreateFontA(-15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"Segoe UI");
    g_ui_bold=CreateFontA(-15,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"Segoe UI");
    g_mono_font=CreateFontA(-15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
    g_mono_small=CreateFontA(-12,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
}

static void destroy_fonts(void) {
    DeleteObject(g_title_font);
    DeleteObject(g_heading_font);
    DeleteObject(g_ui_font);
    DeleteObject(g_ui_bold);
    DeleteObject(g_mono_font);
    DeleteObject(g_mono_small);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) {
    WNDCLASSEXA wc;
    MSG msg;
    int sw,sh,x,y;
    (void)prev; (void)cmd;
    create_fonts();
    ZeroMemory(&wc,sizeof(wc));
    wc.cbSize=sizeof(wc);
    wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc=WndProc;
    wc.hInstance=inst;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    wc.hbrBackground=NULL;
    wc.lpszClassName="NodusNativeWindow";
    if(!RegisterClassExA(&wc)) {
        MessageBoxA(NULL,"Could not register window class.","Nodus",MB_ICONERROR);
        destroy_fonts();
        return 1;
    }
    sw=GetSystemMetrics(SM_CXSCREEN);
    sh=GetSystemMetrics(SM_CYSCREEN);
    x=(sw-APP_W)/2;
    y=(sh-APP_H)/2;
    g_hwnd=CreateWindowExA(0,wc.lpszClassName,"NODUS",WS_OVERLAPPEDWINDOW,x,y,APP_W,APP_H,NULL,NULL,inst,NULL);
    if(!g_hwnd) {
        MessageBoxA(NULL,"Could not create main window.","Nodus",MB_ICONERROR);
        destroy_fonts();
        return 1;
    }
    ShowWindow(g_hwnd,show);
    UpdateWindow(g_hwnd);
    while(GetMessageA(&msg,NULL,0,0)>0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    destroy_fonts();
    return (int)msg.wParam;
}
