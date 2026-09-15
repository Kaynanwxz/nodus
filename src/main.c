#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

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
static HFONT g_display_font;
static HFONT g_heading_font;
static HFONT g_ui_font;
static HFONT g_ui_bold;
static HFONT g_mono_font;
static HFONT g_mono_small;

static const COLORREF C_BG = RGB(10,9,8);
static const COLORREF C_BG_2 = RGB(16,13,11);
static const COLORREF C_PANEL = RGB(25,21,18);
static const COLORREF C_PANEL_2 = RGB(35,29,24);
static const COLORREF C_PAPER = RGB(205,187,153);
static const COLORREF C_PAPER_DARK = RGB(139,116,87);
static const COLORREF C_INK = RGB(29,23,19);
static const COLORREF C_IVORY = RGB(237,225,203);
static const COLORREF C_MUTED = RGB(145,134,120);
static const COLORREF C_OCHRE = RGB(185,119,42);
static const COLORREF C_RUST = RGB(125,52,40);
static const COLORREF C_GREEN = RGB(102,145,108);
static const COLORREF C_LINE = RGB(57,48,41);

static void fill_rect_c(HDC dc, int x, int y, int w, int h, COLORREF color) {
    RECT r = {x,y,x+w,y+h};
    HBRUSH b = CreateSolidBrush(color);
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
    RECT r={x,y,x+w,y+h};
    HFONT old=(HFONT)SelectObject(dc,font);
    SetTextColor(dc,color);
    SetBkMode(dc,TRANSPARENT);
    DrawTextA(dc,text,-1,&r,flags);
    SelectObject(dc,old);
}

static void paper_polygon(HDC dc,int x,int y,int w,int h,COLORREF fill,COLORREF edge) {
    POINT p[14];
    HBRUSH b,ob;
    HPEN pen,op;
    p[0]=(POINT){x+10,y}; p[1]=(POINT){x+w-18,y+2}; p[2]=(POINT){x+w,y+12};
    p[3]=(POINT){x+w-6,y+h/3}; p[4]=(POINT){x+w,y+h-16}; p[5]=(POINT){x+w-13,y+h};
    p[6]=(POINT){x+w/2+18,y+h-3}; p[7]=(POINT){x+w/2-11,y+h}; p[8]=(POINT){x+15,y+h-2};
    p[9]=(POINT){x,y+h-13}; p[10]=(POINT){x+3,y+h/2+8}; p[11]=(POINT){x,y+h/3};
    p[12]=(POINT){x+5,y+13}; p[13]=(POINT){x+13,y+4};
    b=CreateSolidBrush(fill); pen=CreatePen(PS_SOLID,1,edge);
    ob=(HBRUSH)SelectObject(dc,b); op=(HPEN)SelectObject(dc,pen);
    Polygon(dc,p,14);
    SelectObject(dc,op); SelectObject(dc,ob);
    DeleteObject(pen); DeleteObject(b);
}

static void draw_eye(HDC dc,int cx,int cy,int w,int h,COLORREF iris) {
    POINT pts[8];
    HBRUSH b,ob; HPEN pen,op;
    pts[0]=(POINT){cx-w/2,cy}; pts[1]=(POINT){cx-w/4,cy-h/2}; pts[2]=(POINT){cx,cy-h/2-3};
    pts[3]=(POINT){cx+w/4,cy-h/2}; pts[4]=(POINT){cx+w/2,cy}; pts[5]=(POINT){cx+w/4,cy+h/2};
    pts[6]=(POINT){cx,cy+h/2+3}; pts[7]=(POINT){cx-w/4,cy+h/2};
    b=CreateSolidBrush(C_IVORY); pen=CreatePen(PS_SOLID,2,C_INK);
    ob=(HBRUSH)SelectObject(dc,b); op=(HPEN)SelectObject(dc,pen); Polygon(dc,pts,8);
    SelectObject(dc,op); SelectObject(dc,ob); DeleteObject(pen); DeleteObject(b);
    b=CreateSolidBrush(iris); ob=(HBRUSH)SelectObject(dc,b); Ellipse(dc,cx-18,cy-18,cx+18,cy+18); SelectObject(dc,ob); DeleteObject(b);
    b=CreateSolidBrush(C_INK); ob=(HBRUSH)SelectObject(dc,b); Ellipse(dc,cx-6,cy-6,cx+6,cy+6); SelectObject(dc,ob); DeleteObject(b);
}

static void draw_statue(HDC dc,int x,int y,int s) {
    HBRUSH b,ob; HPEN p,op; COLORREF stone=RGB(151,142,127); POINT torso[5];
    b=CreateSolidBrush(stone); p=CreatePen(PS_SOLID,1,RGB(72,64,56)); ob=(HBRUSH)SelectObject(dc,b); op=(HPEN)SelectObject(dc,p);
    Ellipse(dc,x+s*34/100,y,x+s*72/100,y+s*48/100);
    torso[0]=(POINT){x+s*26/100,y+s*44/100}; torso[1]=(POINT){x+s*78/100,y+s*44/100};
    torso[2]=(POINT){x+s,y+s}; torso[3]=(POINT){x+s*8/100,y+s}; torso[4]=(POINT){x,y+s*78/100};
    Polygon(dc,torso,5);
    SelectObject(dc,op); SelectObject(dc,ob); DeleteObject(p); DeleteObject(b);
    line_c(dc,x+s*44/100,y+s*22/100,x+s*63/100,y+s*22/100,RGB(75,66,57),1);
    line_c(dc,x+s*51/100,y+s*28/100,x+s*55/100,y+s*38/100,RGB(75,66,57),1);
}

static void draw_star_chart(HDC dc,int cx,int cy,int r) {
    int i; HPEN p=CreatePen(PS_SOLID,1,RGB(90,74,56)); HPEN op=(HPEN)SelectObject(dc,p);
    Ellipse(dc,cx-r,cy-r,cx+r,cy+r); Ellipse(dc,cx-r*2/3,cy-r*2/3,cx+r*2/3,cy+r*2/3);
    for(i=0;i<12;i++) {
        double a=6.28318530718*i/12.0;
        int x=(int)(cx + r*cos(a)); int y=(int)(cy + r*sin(a));
        MoveToEx(dc,cx,cy,NULL); LineTo(dc,x,y);
    }
    SelectObject(dc,op); DeleteObject(p);
}

static void draw_collage_strip(HDC dc,int w,int h) {
    paper_polygon(dc,18,118,205,170,RGB(31,27,23),RGB(62,51,43));
    text_at(dc,g_mono_small,RGB(116,97,77),34,135,"FRAGMENT / OBSERVATION");
    draw_eye(dc,120,205,140,50,C_OCHRE);
    paper_polygon(dc,37,h-260,175,205,RGB(44,35,29),RGB(76,60,48));
    draw_statue(dc,56,h-242,135);
    paper_polygon(dc,w-230,115,187,210,RGB(46,38,30),RGB(79,64,49));
    draw_star_chart(dc,w-137,214,66);
    text_at(dc,g_mono_small,C_PAPER_DARK,w-211,291,"CELESTIAL INDEX / III");
    paper_polygon(dc,w-220,h-250,180,192,RGB(29,25,21),RGB(64,52,44));
    draw_eye(dc,w-132,h-160,130,46,RGB(156,96,37));
}

static int tab_width(int i){ if(i==0)return 94; if(i==4)return 106; return 96; }

static void draw_header(HDC dc,int w) {
    int x=270,i; char buf[32]; SYSTEMTIME st;
    fill_rect_c(dc,0,0,w,92,C_BG);
    text_at(dc,g_display_font,C_IVORY,26,11,"NODUS");
    text_at(dc,g_mono_small,C_MUTED,28,55,"LOCAL AGENT ORCHESTRATION");
    for(i=0;i<TAB_COUNT;i++) {
        int tw=tab_width(i);
        if(i==g_active_tab) {
            paper_polygon(dc,x-8,26,tw,36,C_PAPER,RGB(96,73,50));
            text_at(dc,g_mono_small,C_INK,x+6,39,TAB_NAMES[i]);
        } else text_at(dc,g_mono_small,C_MUTED,x+6,39,TAB_NAMES[i]);
        x+=tw+8;
    }
    GetLocalTime(&st); wsprintfA(buf,"%02d:%02d",st.wHour,st.wMinute);
    text_at(dc,g_mono_font,C_IVORY,w-154,26,buf);
    text_at(dc,g_mono_small,C_MUTED,w-154,50,"LOCAL TIME");
    text_at(dc,g_mono_font,C_MUTED,w-70,24,"_");
    text_at(dc,g_mono_small,C_MUTED,w-43,28,"[]");
    text_at(dc,g_mono_font,C_IVORY,w-19,24,"X");
    line_c(dc,0,91,w,91,C_LINE,1);
}

static void draw_card(HDC dc,int x,int y,int w,int h,COLORREF fill) {
    paper_polygon(dc,x,y,w,h,fill,RGB(68,56,47));
}

static void draw_metric(HDC dc,int x,int y,int w,const char *label,const char *value,const char *note,COLORREF fill) {
    draw_card(dc,x,y,w,106,fill);
    text_at(dc,g_mono_small,C_INK,x+14,y+14,label);
    text_at(dc,g_heading_font,C_INK,x+14,y+38,value);
    text_at(dc,g_mono_small,RGB(82,67,52),x+14,y+82,note);
}

static void status_dot(HDC dc,int x,int y,int running) {
    HBRUSH b=CreateSolidBrush(running?C_GREEN:RGB(105,98,91)); HBRUSH ob=(HBRUSH)SelectObject(dc,b);
    Ellipse(dc,x,y,x+7,y+7); SelectObject(dc,ob); DeleteObject(b);
}

static void draw_overview(HDC dc,int w,int h) {
    int content_x=245, content_w=w-490, hero_y=118, i; int metric_w=(w-100)/4; char cpu[16],ram[16];
    draw_card(dc,content_x,hero_y,content_w,225,C_PANEL);
    text_at(dc,g_mono_small,C_OCHRE,content_x+24,hero_y+20,"CONTROL NODE / 01");
    text_at(dc,g_title_font,C_IVORY,content_x+22,hero_y+43,"Mission Control");
    text_box(dc,g_ui_font,C_MUTED,content_x+25,hero_y+111,510,54,
        "A local chamber for observing, arranging and later commanding autonomous agents. Native C. Local-first. Deliberately lightweight.",DT_WORDBREAK);
    line_c(dc,content_x+25,hero_y+176,content_x+535,hero_y+176,RGB(68,56,47),1);
    text_at(dc,g_mono_small,C_MUTED,content_x+25,hero_y+190,"2 ACTIVE / 4 REGISTERED / 12 TASKS TODAY");

    draw_card(dc,w-228,hero_y+5,182,210,C_PAPER);
    text_at(dc,g_mono_small,C_INK,w-207,hero_y+21,"ANATOMY OF CONTROL");
    draw_eye(dc,w-137,hero_y+104,128,48,C_OCHRE);
    text_box(dc,g_heading_font,C_INK,w-205,hero_y+144,136,52,"The machine watches the watchers.",DT_CENTER|DT_WORDBREAK);

    wsprintfA(cpu,"%d%%",g_cpu); wsprintfA(ram,"%d.%d GB",g_ram_tenths/10,g_ram_tenths%10);
    draw_metric(dc,28,365,metric_w-16,"ACTIVE AGENTS","02","1 idle / 1 stopped",RGB(205,187,153));
    draw_metric(dc,44+metric_w,371,metric_w-16,"CPU LOAD",cpu,"simulated",RGB(158,136,105));
    draw_metric(dc,60+metric_w*2,365,metric_w-16,"MEMORY",ram,"of 16 GB",RGB(188,165,128));
    draw_metric(dc,76+metric_w*3,372,metric_w-24,"TASK QUEUE","05","2 high priority",RGB(137,113,87));

    draw_card(dc,28,500,(w-84)/2,h-548,C_PANEL);
    text_at(dc,g_mono_small,C_OCHRE,50,519,"AGENTS / LIVE GRID");
    for(i=0;i<AGENT_COUNT;i++) {
        int yy=558+i*57;
        status_dot(dc,52,yy+4,g_agents[i].running);
        text_at(dc,g_ui_bold,C_IVORY,70,yy,g_agents[i].name);
        text_at(dc,g_ui_font,C_MUTED,185,yy,g_agents[i].task);
        text_at(dc,g_mono_small,g_agents[i].running?C_GREEN:C_MUTED,(w-84)/2-82,yy,g_agents[i].status);
        if(i<AGENT_COUNT-1) line_c(dc,50,yy+35,(w-84)/2-5,yy+35,RGB(47,40,34),1);
    }

    draw_card(dc,45+(w-84)/2,508,(w-84)/2-17,h-556,C_PANEL_2);
    text_at(dc,g_mono_small,C_OCHRE,66+(w-84)/2,527,"RECENT ACTIVITY");
    text_at(dc,g_ui_bold,C_IVORY,66+(w-84)/2,568,"16:43   HERMES");
    text_at(dc,g_ui_font,C_MUTED,66+(w-84)/2,590,"Waiting for next instruction");
    text_at(dc,g_ui_bold,C_IVORY,66+(w-84)/2,627,"16:42   CODER-01");
    text_at(dc,g_ui_font,C_MUTED,66+(w-84)/2,649,"Frontend test pass completed");
    text_at(dc,g_ui_bold,C_IVORY,66+(w-84)/2,686,"16:40   HERMES");
    text_at(dc,g_ui_font,C_MUTED,66+(w-84)/2,708,"websocket.service.ts indexed");
}

static void draw_agents(HDC dc,int w,int h) {
    int i,cardw=(w-120)/2;
    text_at(dc,g_mono_small,C_OCHRE,34,122,"REGISTRY / AGENT NODES");
    text_at(dc,g_title_font,C_IVORY,34,143,"Agents");
    for(i=0;i<AGENT_COUNT;i++) {
        int col=i%2,row=i/2,x=34+col*(cardw+20),y=226+row*220;
        draw_card(dc,x,y,cardw,190,(i%2)?C_PANEL_2:C_PANEL);
        text_at(dc,g_heading_font,C_IVORY,x+22,y+21,g_agents[i].name);
        text_at(dc,g_ui_font,C_MUTED,x+22,y+58,g_agents[i].role);
        text_at(dc,g_mono_small,g_agents[i].running?C_GREEN:C_MUTED,x+22,y+92,g_agents[i].status);
        text_box(dc,g_ui_font,C_MUTED,x+22,y+119,cardw-44,42,g_agents[i].task,DT_WORDBREAK);
    }
    (void)h;
}

static void draw_terminal(HDC dc,int w,int h) {
    draw_card(dc,34,126,w-68,h-182,RGB(13,12,10));
    text_at(dc,g_mono_small,C_OCHRE,56,146,"LOCAL SHELL / PREVIEW");
    text_at(dc,g_heading_font,C_IVORY,56,176,"Hermes terminal");
    line_c(dc,56,218,w-56,218,C_LINE,1);
    text_at(dc,g_mono_font,C_OCHRE,58,246,"PS C:\\Agents\\Hermes>"); text_at(dc,g_mono_font,C_IVORY,260,246,"hermes --resume");
    text_at(dc,g_mono_font,C_GREEN,58,278,"[OK]"); text_at(dc,g_mono_font,C_MUTED,104,278,"workspace loaded");
    text_at(dc,g_mono_font,C_MUTED,58,310,"[16:43:11] websocket.service.ts indexed");
    text_at(dc,g_mono_font,C_MUTED,58,342,"[16:43:14] waiting for next instruction");
    text_at(dc,g_mono_font,C_OCHRE,58,392,"PS C:\\Agents\\Hermes>"); text_at(dc,g_mono_font,C_IVORY,260,392,"_");
    text_at(dc,g_mono_small,C_MUTED,58,h-88,"Simulation mode in 0.3.0. Real process bridge comes next.");
}

static void draw_tasks(HDC dc,int w,int h) {
    const char *cols[4]={"BACKLOG","RUNNING","REVIEW","DONE"}; int i; int cw=(w-100)/4;
    text_at(dc,g_mono_small,C_OCHRE,34,122,"OPERATIONS / TASK BOARD");
    text_at(dc,g_title_font,C_IVORY,34,143,"Tasks");
    for(i=0;i<4;i++) {
        int x=28+i*(cw+14),y=220;
        draw_card(dc,x,y,cw,h-275,(i%2)?C_PANEL_2:C_PANEL);
        text_at(dc,g_ui_bold,C_IVORY,x+18,y+18,cols[i]);
        line_c(dc,x+18,y+49,x+cw-18,y+49,C_LINE,1);
        draw_card(dc,x+16,y+75,cw-32,102,RGB(39,32,27));
        text_at(dc,g_ui_bold,C_IVORY,x+29,y+90,(i==0)?"Inspect auth flow":(i==1)?"WebSocket refactor":(i==2)?"Patient navigation":"Project indexing");
        text_box(dc,g_ui_font,C_MUTED,x+29,y+116,cw-58,45,"Assigned node and current task summary.",DT_WORDBREAK);
    }
}

static void draw_activity(HDC dc,int w,int h) {
    int i; const char *a[5]={"Hermes / waiting for instruction","Coder-01 / tests completed","Hermes / source indexed","Research / task completed","System / Nodus started"};
    text_at(dc,g_mono_small,C_OCHRE,34,122,"AUDIT STREAM"); text_at(dc,g_title_font,C_IVORY,34,143,"Activity");
    draw_card(dc,34,220,w-68,h-275,C_PANEL);
    for(i=0;i<5;i++) { int y=252+i*84; text_at(dc,g_mono_small,C_OCHRE,58,y,"TRACE"); text_at(dc,g_ui_bold,C_IVORY,118,y,a[i]); line_c(dc,58,y+44,w-60,y+44,C_LINE,1); }
}

static void draw_settings(HDC dc,int w,int h) {
    text_at(dc,g_mono_small,C_OCHRE,34,122,"CONTROL PLANE"); text_at(dc,g_title_font,C_IVORY,34,143,"Settings");
    draw_card(dc,34,220,(w-96)/2,245,C_PANEL); text_at(dc,g_heading_font,C_IVORY,56,244,"Interface");
    text_at(dc,g_ui_font,C_MUTED,56,292,"Theme        Nodus Collage Dark"); text_at(dc,g_ui_font,C_MUTED,56,328,"Density      Comfortable");
    draw_card(dc,56+(w-96)/2,220,(w-96)/2-22,245,C_PANEL_2); text_at(dc,g_heading_font,C_IVORY,78+(w-96)/2,244,"Agent defaults");
    text_at(dc,g_ui_font,C_MUTED,78+(w-96)/2,292,"Workspace    C:\\Agents"); text_at(dc,g_ui_font,C_MUTED,78+(w-96)/2,328,"Shell        PowerShell");
    text_at(dc,g_mono_small,C_MUTED,34,h-72,"Nodus 0.3.0 / Native C / Win32");
}

static void create_fonts(void) {
    g_title_font=CreateFontA(-58,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_display_font=CreateFontA(-34,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_heading_font=CreateFontA(-26,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Georgia");
    g_ui_font=CreateFontA(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Segoe UI");
    g_ui_bold=CreateFontA(-16,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,"Segoe UI");
    g_mono_font=CreateFontA(-15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
    g_mono_small=CreateFontA(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
}

static void destroy_fonts(void) {
    DeleteObject(g_title_font); DeleteObject(g_display_font); DeleteObject(g_heading_font); DeleteObject(g_ui_font); DeleteObject(g_ui_bold); DeleteObject(g_mono_font); DeleteObject(g_mono_small);
}

static int hit_tab(int mx,int my) {
    int x=270,i; if(my<20||my>76)return -1;
    for(i=0;i<TAB_COUNT;i++){int tw=tab_width(i); if(mx>=x-8&&mx<=x+tw)return i; x+=tw+8;} return -1;
}

static LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg) {
        case WM_NCHITTEST: {
            LRESULT r=DefWindowProcA(hwnd,msg,wp,lp); POINT pt; RECT cr; const int g=7;
            if(r!=HTCLIENT)return r;
            pt.x=(short)LOWORD(lp); pt.y=(short)HIWORD(lp); ScreenToClient(hwnd,&pt); GetClientRect(hwnd,&cr);
            if(pt.x<g&&pt.y<g)return HTTOPLEFT; if(pt.x>=cr.right-g&&pt.y<g)return HTTOPRIGHT;
            if(pt.x<g&&pt.y>=cr.bottom-g)return HTBOTTOMLEFT; if(pt.x>=cr.right-g&&pt.y>=cr.bottom-g)return HTBOTTOMRIGHT;
            if(pt.x<g)return HTLEFT; if(pt.x>=cr.right-g)return HTRIGHT; if(pt.y<g)return HTTOP; if(pt.y>=cr.bottom-g)return HTBOTTOM;
            if(pt.y<92 && hit_tab(pt.x,pt.y)<0 && pt.x<cr.right-180) return HTCAPTION;
            return HTCLIENT;
        }
        case WM_LBUTTONDOWN: {
            int mx=(short)LOWORD(lp),my=(short)HIWORD(lp),tab; RECT cr; GetClientRect(hwnd,&cr);
            if(my<72 && mx>=cr.right-34){PostMessageA(hwnd,WM_CLOSE,0,0);return 0;}
            if(my<72 && mx>=cr.right-70){ShowWindow(hwnd,IsZoomed(hwnd)?SW_RESTORE:SW_MAXIMIZE);return 0;}
            if(my<72 && mx>=cr.right-100){ShowWindow(hwnd,SW_MINIMIZE);return 0;}
            tab=hit_tab(mx,my); if(tab>=0){g_active_tab=tab;InvalidateRect(hwnd,NULL,FALSE);return 0;}
            break;
        }
        case WM_TIMER: g_cpu=26+(GetTickCount()/1000)%13; g_ram_tenths=69+(GetTickCount()/2000)%10; InvalidateRect(hwnd,NULL,FALSE); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps; RECT cr; HDC dc=BeginPaint(hwnd,&ps); HDC mem; HBITMAP bmp,oldbmp; GetClientRect(hwnd,&cr);
            mem=CreateCompatibleDC(dc); bmp=CreateCompatibleBitmap(dc,cr.right,cr.bottom); oldbmp=(HBITMAP)SelectObject(mem,bmp);
            fill_rect_c(mem,0,0,cr.right,cr.bottom,C_BG); draw_collage_strip(mem,cr.right,cr.bottom); draw_header(mem,cr.right);
            if(g_active_tab==0)draw_overview(mem,cr.right,cr.bottom); else if(g_active_tab==1)draw_agents(mem,cr.right,cr.bottom); else if(g_active_tab==2)draw_terminal(mem,cr.right,cr.bottom); else if(g_active_tab==3)draw_tasks(mem,cr.right,cr.bottom); else if(g_active_tab==4)draw_activity(mem,cr.right,cr.bottom); else draw_settings(mem,cr.right,cr.bottom);
            BitBlt(dc,0,0,cr.right,cr.bottom,mem,0,0,SRCCOPY); SelectObject(mem,oldbmp); DeleteObject(bmp); DeleteDC(mem); EndPaint(hwnd,&ps); return 0;
        }
        case WM_DESTROY: KillTimer(hwnd,1); PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(hwnd,msg,wp,lp);
}

int WINAPI WinMain(HINSTANCE inst,HINSTANCE prev,LPSTR cmd,int show) {
    WNDCLASSEXA wc; MSG msg; int sw,sh,x,y; (void)prev;(void)cmd;
    ZeroMemory(&wc,sizeof(wc)); wc.cbSize=sizeof(wc); wc.lpfnWndProc=WndProc; wc.hInstance=inst; wc.hCursor=LoadCursor(NULL,IDC_ARROW); wc.hIcon=LoadIcon(NULL,IDI_APPLICATION); wc.lpszClassName="NodusNativeWindow";
    create_fonts();
    if(!RegisterClassExA(&wc)){MessageBoxA(NULL,"Could not register window class.","Nodus",MB_ICONERROR);destroy_fonts();return 1;}
    sw=GetSystemMetrics(SM_CXSCREEN); sh=GetSystemMetrics(SM_CYSCREEN); x=(sw-APP_W)/2; y=(sh-APP_H)/2;
    g_hwnd=CreateWindowExA(0,wc.lpszClassName,"NODUS",WS_POPUP|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU,x,y,APP_W,APP_H,NULL,NULL,inst,NULL);
    if(!g_hwnd){MessageBoxA(NULL,"Could not create main window.","Nodus",MB_ICONERROR);destroy_fonts();return 1;}
    ShowWindow(g_hwnd,show); UpdateWindow(g_hwnd); SetTimer(g_hwnd,1,1000,NULL);
    while(GetMessageA(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);} destroy_fonts(); return (int)msg.wParam;
}
