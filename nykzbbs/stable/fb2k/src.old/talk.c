/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"

#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BBS_PAGESIZE    (t_lines - 4)

#define M_INT 8
#define P_INT 20
extern char BoardName[];
extern int iscolor;
extern int numf, friendmode, watchinboardmode;

extern time_t update_time;
extern struct user_info *user_record[MAXACTIVE];
extern char* cexp(int exp);

/*rem add by pax@sjtubbs for chess1*/ 
int a[21][21],chessx,chessy; 
char chesslog[25*40];
int chessturn,chesscolor,chesshand,chessmode=0; 
//end

struct user_info* t_search (char* sid, int pid);
int talkidletime = 0;
int ulistpage;
int friendflag = 1;

int deal_key3(char ch, int allnum, int pagenum);
void do_query3(int star, int curr);

int friend_query(int ent, struct override *fh, char *direct);
int friend_mail(int ent, struct override *fh, char *direct);
int friend_dele(int ent, struct override *fh, char *direct);
int friend_add(int ent, struct override *fh, char *direct);
int friend_edit(int ent, struct override *fh, char *direct);
int friend_help(void);
int reject_query(int ent, struct override *fh, char *direct);
int reject_dele(int ent, struct override *fh, char *direct);
int reject_add(int ent, struct override *fh, char *direct);
int reject_edit(int ent, struct override *fh, char *direct);
int reject_help(void);

#ifdef TALK_LOG
void do_log(char *msg, int who);
int talkrec = -1;
char partner[IDLEN + 1];
#endif

struct one_key friend_list[] = {
    {'r', friend_query,},
    {'m', friend_mail,},
    {'a', friend_add,},
    {'A', friend_add,},
    {'d', friend_dele,},
    {'D', friend_dele,},
    {'E', friend_edit,},
    {'h', friend_help,},
    {'H', friend_help,},
    {'\0', NULL},
};

struct one_key reject_list[] = {
    {'r', reject_query,},
    {'a', reject_add,},
    {'A', reject_add,},
    {'d', reject_dele,},
    {'D', reject_dele,},
    {'E', reject_edit,},
    {'h', reject_help,},
    {'H', reject_help,},
    {'\0', NULL},
};

struct talk_win
{
    int curcol, curln;
    int sline, eline;
};

int nowmovie;
int bind(int, const struct sockaddr *, socklen_t);

static char *refuse[] = {
    "抱歉，我现在想专心看 Board。    ", "请不要吵我，好吗？..... :)      ",
    "我现在有事，等一下再 Call 你。  ", "我马上要离开了，下次再聊吧。    ",
    "请你不要再 Page，我不想跟你聊。 ", "请先写一封自我介绍给我，好吗？  ",
    "对不起，我现在在等人。          ", "我今天很累，不想跟别人聊天。    ",
    NULL
};

extern int t_columns;

char save_page_requestor[40];
int t_cmpuids(int uid, struct user_info *up);

char *ModeType(int mode);

int ishidden(char *user)
{
    int tuid;
    struct user_info uin;

    if (!(tuid = getuser(user)))
        return 0;
    if (!search_ulist(&uin, t_cmpuids, tuid))
        return 0;
    return (uin.invisible);
}

char pagerchar(int friend, int pager)
{
    if (pager & ALL_PAGER)
        return ' ';
    if ((friend))
    {
        if (pager & FRIEND_PAGER)
            return 'O';
        else
            return '#';
    }
    return '*';
}

int canpage(int friend, int pager)
{
    if ((pager & ALL_PAGER) || HAS_PERM(PERM_SYSOP | PERM_FORCEPAGE))
        return YEA;
    if ((pager & FRIEND_PAGER))
    {
        if (friend)
            return YEA;
    }
    return NA;
}

#ifdef SHOW_IDLE_TIME
char *idle_str(struct user_info *uent)
{
    static char hh_mm_ss[32];

#ifndef BBSD

    struct stat buf;
    char tty[128];
#endif

    time_t now, diff;
    int limit, hh, mm;

    if (uent == NULL)
    {
        strcpy(hh_mm_ss, "不详");
        return hh_mm_ss;
    }
#ifndef BBSD
    strcpy(tty, uent->tty);

#ifndef SOLARIS
#ifndef AIX
#ifndef LINUX

    if ((stat(tty, &buf) != 0) || (strstr(tty, "tty") == NULL))
    {
        strcpy(hh_mm_ss, "不详");
        return hh_mm_ss;
    }
#else
    if ((stat(tty, &buf) != 0) || (strstr(tty, "dev") == NULL))
    {
        strcpy(hh_mm_ss, "不详");
        return hh_mm_ss;
    }
#endif
#else
    if ((stat(tty, &buf) != 0) || (strstr(tty, "pts") == NULL))
    {
        strcpy(hh_mm_ss, "不详");
        return hh_mm_ss;
    }
#endif
#else
    if ((stat(tty, &buf) != 0) || (strstr(tty, "pts") == NULL))
    {
        strcpy(hh_mm_ss, "不详");
        return hh_mm_ss;
    }
#endif
#endif

    now = time(0);

#ifndef BBSD

    diff = now - buf.st_atime;
#else

    if (uent->mode == TALK)
        diff = talkidletime;
    else if (uent->mode == BBSNET)
        diff = 0;
    else
        diff = now - uent->idle_time;
#endif

#ifdef DOTIMEOUT

    if (uent->ext_idle)
        limit = IDLE_TIMEOUT * 3;
    else
        limit = IDLE_TIMEOUT;

    if ((diff > limit) && (diff < 86400 * 5) && uent->pid)
        if (!is_web_user(uent))
            kill(uent->pid, SIGHUP);
#endif

    hh = diff / 3600;
    mm = (diff / 60) % 60;

    if (hh > 0)
        sprintf(hh_mm_ss, "%02d:%02d", hh, mm);
    else if (mm > 0)
        sprintf(hh_mm_ss, "%d", mm);
    else
        sprintf(hh_mm_ss, "   ");

    return hh_mm_ss;
}
#endif

int listcuent(struct user_info *uentp)
{
    if (uentp == NULL)
    {
        CreateNameList();
        return 0;
    }
    if (uentp->uid == usernum)
        return 0;
    if (!uentp->active || !uentp->pid || isreject(uentp))
        return 0;
    if (uentp->invisible && !(HAS_PERM(PERM_SYSOP | PERM_SEECLOAK)))
        return 0;
    AddToNameList(uentp->userid);
    return 0;
}

void creat_list(void)
{
    listcuent(NULL);
    apply_ulist(listcuent);
}

int t_pager(void)
{

    if (uinfo.pager & ALL_PAGER)
    {
        uinfo.pager &= ~ALL_PAGER;
        if (DEFINE(DEF_FRIENDCALL))
            uinfo.pager |= FRIEND_PAGER;
        else
            uinfo.pager &= ~FRIEND_PAGER;
    }
    else
    {
        uinfo.pager |= ALL_PAGER;
        uinfo.pager |= FRIEND_PAGER;
    }

    if (!uinfo.in_chat && uinfo.mode != TALK)
    {
        move(1, 0);
        clrtoeol();
        prints("您的呼叫器 (pager) 已经[1m%s[m了!",
               (uinfo.pager & ALL_PAGER) ? "打开" : "关闭");
        pressreturn();
    }
    update_utmp();
    return 0;
}

int show_user_plan(char *userid)
{
    char pfile[STRLEN];

    sethomefile(pfile, userid, "plans");
    if (show_one_file(pfile) == NA)
    {
        prints("[1;36m没有个人说明档[m\n");
        return NA;
    }
    return YEA;
}

int show_one_file(char *filename)
{
    int i, j, ci;
    char pbuf[256];
    FILE *pf;

    if ((pf = fopen(filename, "r")) == NULL)
    {
        return NA;
    }
    else
    {
        for (i = 1; i <= MAXQUERYLINES; i++)
        {
            if (fgets(pbuf, sizeof(pbuf), pf))
            {
                for (j = 0; j < (int) strlen(pbuf); j++)
                    if (pbuf[j] != '\033')
                        outc(pbuf[j]);
                    else
                    {
                        ci = strspn(&pbuf[j], "\033[0123456789;");
                        if (pbuf[ci + j] != 'm')
                            j += ci;
                        else
                            outc(pbuf[j]);
                    }
            }
            else
                break;
        }
        fclose(pf);
        return YEA;
    }
}

void show_dm(char *userid)
{
    int dist = 12;

    dist = WHICHDIST(userid);
    if ((dist < 12) && (dist > 0))
    {
        prints(" [\033[1;33m");
        if (dist < 10)
            prints("%d", dist);
        else
            prints("%c", 'A' + dist - 10);
        prints("\033[1;32m 区区长\033[0;1m]");
    }
    else
        prints(" [\033[1;32m本站区长\033[0;1m]");
    return;
}

void show_bm(char firstchar, char *userid)
{
    FILE *stream;
    int i = 0;
    int f = 0;
   	char bm_show[256];
    char bm_file_path[28], buffer[51], *ch;
	bm_show[0] = '\0';
    sprintf(bm_file_path, "home/%c/%c/%s/.bmfile", firstchar,
            toupper(userid[1]), userid);

    if ((stream = fopen(bm_file_path, "r")) == NULL)
        prints(" [\033[1;32m本站板主\033[0;1m]");
    else
    {

        while(fgets(buffer,32,stream)&& i<4)
        {
            if(ch = strchr(buffer,10))
                *ch='\0';
            if (strncmp(buffer , "District" , 8) ) 
			{
			/*

				if (f == 0)
					prints(" [\033[1;33m");
				prints ("%s ",buffer);
				f = 1;
			}
            i++;
        }
        fclose(stream);
        if (f) prints("\033[1;32m板板主\033[0;1m]");

*/
			
				
					/*if(strcmp(buffer,"Archives")==0)
					{prints ("\033[0;1m[\033[1;33m档案馆\033[1;32m馆长\033[0;1m]");
					}
					else */if(strcmp(buffer,"SYH_Casino")==0)
					{prints ("\033[0;1m[\033[1;33m博彩中心\033[1;32m经理\033[0;1m]");
					}
					else if(strcmp(buffer,"SYH_Game")==0)
					{prints ("\033[0;1m[\033[1;33m游戏广场\033[1;32m经理\033[0;1m]");
					}
					else
					{
					if (f == 0)
					{
					strcat(bm_show,"\033[0;1m [\033[1;33m");
					f = 1;
					strcat (bm_show, buffer);
					strcat (bm_show, " ");
					}
					else
					{
					strcat (bm_show, buffer);
					strcat (bm_show, " ");
					}
				}
				
			}
            i++;
        }
        fclose(stream);
        if (f) {
//			prints(bm_show);
//			prints("\033[1;32m板板主\033[0;1m]");
            prints (" [\033[1;32m本站板主\033[0;1m]");
			}
    }
}


void show_rmail(char firstchar, char *userid)
{

    FILE *stream;
    char bm_file_path[36], buffer[51];

    sprintf(bm_file_path, "home/%c/%c/%s/real_email", firstchar,
            toupper(userid[1]), userid);

    if ((stream = fopen(bm_file_path, "r")) == NULL)
        prints(" [\033[1;32mNO_REAL_EMAIL\033[0;1m]\n");
    else
    {

        while (fgets(buffer, 32, stream))
        {
            prints("%s\n", buffer);
        }
        fclose(stream);
    }
}

struct user_info *t_search(char *sid, int pid)
{
    int i;
    extern struct UTMPFILE *utmpshm;
    struct user_info *cur, *tmp = NULL;

    resolve_utmp();
    for (i = 0; i < USHM_SIZE; i++)
    {
        cur = &(utmpshm->uinfo[i]);
        if (!cur->active || !cur->pid)
            continue;
        if (!strcasecmp(cur->userid, sid))
        {
            if (pid == 0)
                return isreject(cur) ? NULL : cur;
            tmp = cur;
            if (pid == cur->pid)
                break;
        }
    }

    return isreject(cur) ? NULL : tmp;
}

void show_honor()
{
    FILE *fp;
    char line[STRLEN], *honortype, *userid, *honor;

    fp = fopen("etc/honors", "r");
    if (fp == NULL)
        return;
    while (fgets(line, STRLEN, fp))
    {
        honortype = strtok(line, " \t\r\n");
        userid = strtok(NULL, " \t\r\n");
        honor = strtok(NULL, " \t\r\n");
        if (honortype == NULL || userid == NULL || honor == NULL || honortype[0] == '#')
            continue;

        if (!strcmp(userid, lookupuser.userid))
        {
            switch (honortype[0])
            {
            case 'H':
                prints("\033[0m荣誉:\033[0;1m");
                break;
            case 'T':
                prints("\033[0m职务:\033[0;1m");
                break;
            }
            
            prints(" [\033[1;32m%s\033[0;1m]\033[0m", honor);
            fclose(fp);
            return;
        }
    }
    fclose(fp);
}

void show_user_title()
{
    if (lookupuser.userlevel & PERM_SYSOP)
    {
        if (lookupuser.userlevel & PERM_HONOR)
            show_honor();
        else
            prints("职务:\033[0;1m[\033[1;32m本站站长\033[0;1m]");
        prints("\n");
    }
    else if (lookupuser.userlevel & PERM_HASTITLE || lookupuser.userlevel & PERM_HONOR)
    {
        if (lookupuser.userlevel & PERM_HASTITLE)
        {
            prints("职务:\033[0;1m");
            
//            if (lookupuser.userlevel & (PERM_SPECIAL8 | PERM_OBOARDS))
//                prints(" [\033[1;32m站务助理\033[0;1m]");
            if (lookupuser.userlevel & PERM_ACCOUNTS)
                prints(" [\033[1;32m账号管理员\033[0;1m]");
            if (lookupuser.userlevel & PERM_OVOTE)
                prints(" [\033[1;32m投票管理员\033[0;1m]");
            if (lookupuser.userlevel & PERM_ACHATROOM)
                prints(" [\033[1;32m南洋总管\033[0;1m]");
            if (lookupuser.userlevel & PERM_ACBOARD)
                prints(" [\033[1;32m美工总管\033[0;1m]");
            if (lookupuser.userlevel & PERM_CHATCLOAK)
                prints(" [\033[1;32m本站智囊\033[0;1m]");
//            else if (lookupuser.userlevel & PERM_SPECIAL6)
//                prints(" [\033[1;32m技术智囊\033[0;1m]");
            if (lookupuser.userlevel & PERM_SPECIAL6)
                prints (" [\033[1;32m档案馆馆长\033[0;1m]");
            if (lookupuser.userlevel & PERM_ARBITRATOR)
                prints(" [\033[1;32m本站仲裁\033[0;1m]");
            if ((lookupuser.userlevel & PERM_SPECIAL7)
                && !(lookupuser.userlevel & PERM_OBOARDS))
                show_dm(lookupuser.userid);
            if (lookupuser.userlevel & PERM_BOARDS)
                show_bm(toupper(lookupuser.userid[0]), lookupuser.userid);
    
            prints(" ");
        }

        if (lookupuser.userlevel & PERM_HONOR)
            show_honor();

        prints("\n");
    }

}

int t_query(char *q_id)
{
    char uident[STRLEN];
    int tuid = 0, clr = 0;
    int num;
#ifdef ALLOWGAME
	int allmoney;
#endif
    int exp,perf;//nykz
    struct user_info uin;
    char qry_mail_dir[STRLEN];
    char planid[IDLEN + 2], buf[50], buf2[50];
    time_t now;

    int ch;

    char desc[STRLEN];

    struct user_info *uin2 = NULL;

    static int msgflag;
    extern int friendflag;

    if ((uinfo.mode != LUSERS && uinfo.mode != LAUSERS
         && uinfo.mode != WATCHINBOARD && uinfo.mode != FRIEND
         && uinfo.mode != QAUTHOR && uinfo.mode != GMENU))
    {
        modify_user_mode(QUERY);
        refresh();
        move(1, 0);
        clrtobot();
        prints("查询谁:\n<输入使用者代号, 按空白键可列出符合字串>\n");
        move(1, 8);
        usercomplete(NULL, uident);
        if (uident[0] == '\0')
            return 0;
    }
    else
    {
        if (q_id == NULL || *q_id == '\0')
            return 0;
        if (strchr(q_id, ' '))
            strtok(q_id, " ");
        strncpy(uident, q_id, sizeof(uident));
        uident[sizeof(uident) - 1] = '\0';
    }
    if (!(tuid = getuser(uident)))
    {
        move(2, 0);
        clrtoeol();
        prints("[1m不正确的使用者代号[m\n");
        pressanykey();
        return -1;
    }
    uinfo.destuid = tuid;
    update_utmp();
    move(0, 0);
    clrtobot();
    sprintf(qry_mail_dir, "mail/%c/%c/%s/%s",
            toupper(lookupuser.userid[0]), toupper(lookupuser.userid[1]),
            lookupuser.userid, DOT_DIR);
    
    exp = countexp(&lookupuser);//nykz
	perf = countperf(&lookupuser);//nykz 
	
    if (HAS_DEFINE(lookupuser.userdefine, DEF_COLOREDSEX))
        clr = (lookupuser.gender == 'F') ? 5 : 6;
    else
        clr = 2;
    if (strcasecmp(lookupuser.userid, "guest") != 0)
        sprintf(buf, "[[1;3%dm%s座 %s[m] ", clr,
                horoscope(lookupuser.birthmonth, lookupuser.birthday),
           		n_horoscope (lookupuser.birthmonth, lookupuser.birthday));//nykz
    else
        buf[0] = '\0';
    if (!HAS_DEFINE(lookupuser.userdefine, DEF_S_HOROSCOPE))
        buf[0] = '\0';
    if (strcmp(lookupuser.userid, "guest") == 0)
        buf2[0] = '\0';
    else
        sprintf(buf2, "网龄[[1;32m%ld[0;1m]天",
                (time(0) - lookupuser.firstlogin) / 86400);
    prints("[1;37m%s [m([1;33m%s[m) 共上站 [1;32m%d[m 次  %s  %s\n",
           lookupuser.userid, lookupuser.username, lookupuser.numlogins,
           buf2, buf);
    strcpy(planid, lookupuser.userid);
    num = t_search_ulist(&uin, t_cmpuids, tuid, NA, NA);
    search_ulist(&uin, t_cmpuids, getuser(lookupuser.userid));

    getdatestring(lookupuser.lastlogin, NA);
    prints("上 次 在: [[1;32m%s[m] 从 [[1;32m%s[m] 到本站一游。\n",
           datestring, lookupuser.lasthost);
    if (num)
    {
        sprintf(genbuf,
                "目前在线：[[1;32m讯息器：([36m%s[32m) 呼叫器：([36m%s[32m)[m] ",
                canmsg(&uin) ? "打开" : "关闭", canpage(hisfriend(&uin),
                                                        uin.
                                                        pager) ? "打开" :
                "关闭");
        prints("%s", genbuf);
    }
    else
    {
        if (lookupuser.lastlogout < lookupuser.lastlogin)
        {
            now =
                ((time(0) - lookupuser.lastlogin) / 120) % 47 + 1 +
                lookupuser.lastlogin;
        }
        else
        {
            now = lookupuser.lastlogout;
        }
        getdatestring(now, NA);
        prints("离站时间: [[1;32m%s[m] ", datestring);
    }

/*    prints
        ("生命力:[[1;32m%d[m] 文章:[[1;32m%d[m] 信箱:[[1;5;32m%2s[m]\n",
         compute_user_value(&lookupuser), lookupuser.numposts,
         (check_query_mail(qry_mail_dir) == 1) ? "信" : "  ");
*/
	   prints("表现值: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) 信箱: [\033[1;5;32m%2s\033[m]\n"
		   , perf,cperf(perf), (check_query_mail(qry_mail_dir) == 1) ? "信" : "  ");
//nykz
#ifdef ALLOWGAME
	   if (HAS_PERM (PERM_SPECIAL8)|| (strcmp(lookupuser.userid,currentuser.userid)==0)){
		   
		   allmoney=lookupuser.inbank;
		   prints("自由资金: [\033[1;32m%d元\033[m] 银行存款: [\033[1;32m%d元\033[m](\033[1;33m%s\033[m) 经验值: [\033[1;32m%d\033[m](\033[1;33m%s\033[m)。\n",
			   lookupuser.money,lookupuser.inbank,
			   cmoney(lookupuser.money + allmoney),exp,cexp(exp));
	   }
	   else
	   {
		   prints("自由资金: [\033[1;32m%d元\033[m] 经验值: [\033[1;32m%d\033[m](\033[1;33m%s\033[m)。\n",
			   lookupuser.money,exp,cexp(exp));
		   
	   }
	   

prints("文 章 数: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) 奖章数: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) 生命力: [\033[1;32m%d\033[m]\n",
		   lookupuser.numposts,cnumposts(lookupuser.numposts),
		   lookupuser.medals,cmedals(lookupuser.medals),
		   compute_user_value(&lookupuser));


#else
	   prints("文 章 数: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) 经验值: [\033[1;32m%d\033[m](\033[1;33m%s\033[m) 生命力: [\033[1;32m%d\033[m]\n",
		   lookupuser.numposts,cnumposts(lookupuser.numposts), 
		   exp,cexp(exp),compute_user_value(&lookupuser));
#endif

#if defined(QUERY_REALNAMES)

    if (HAS_PERM(PERM_SYSOP))
        prints("真实姓名: [%s]  ", lookupuser.realname);
#endif

    show_user_title();

    if (HAS_PERM(PERM_SYSOP))
    {
        prints("rmail:");
        show_rmail(toupper(lookupuser.userid[0]), lookupuser.userid);
    }

    t_search_ulist(&uin, t_cmpuids, tuid, YEA, NA);
    show_user_plan(planid);

    if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
        && uinfo.mode != WATCHINBOARD
        && uinfo.mode != FRIEND
        && uinfo.mode != GMENU && uinfo.mode != QUERY
        && uinfo.mode != QAUTHOR)
        pressanykey();

    else if (uinfo.mode == QUERY || uinfo.mode == QAUTHOR)
    {

        move(t_lines - 1, 0);
        prints
            ("\033[0;1;37;44m聊天[\033[1;32mt\033[37m] 寄信[\033[1;32mm\033[37m] 送讯息[\033[1;32ms\033[37m] 加,减朋友[\033[1;32mo\033[37m,\033[1;32md\033[37m] 亲戚关系查询[\033[1;32mq\033[37m] 友人馈赠查询[\033[1;32mi\033[37m]    \033[m");

        ch = igetkey();

        uin2 = t_search(lookupuser.userid, NA);
        switch (ch)
        {
        case 't':
        case 'T':
            if (!HAS_PERM(PERM_PAGE))
                return 1;

            if (uin2 == NULL)
                return 0;

            if (!HAS_PERM(PERM_SEECLOAK) && uin2->invisible == 1)
                return 1;
            if (uin2->uid != usernum)
                ttt_talk(uin2);
            else
                return 1;
            break;

        case 'm':
        case 'M':
            if (!HAS_PERM(PERM_POST))
                return 1;
            m_send(lookupuser.userid);
            break;
        case 'g':
        case 'G':
            if (!HAS_PERM(PERM_POST))
                return 1;
            sendGoodWish(lookupuser.userid);
            break;

        case 's':
        case 'S':
            if (!strcmp("guest", currentuser.userid))
                return 0;
            if (uin2 == NULL)
                return 0;
            if (!HAS_PERM(PERM_MESSAGE))
                return 0;

            if (!HAS_PERM(PERM_SEECLOAK) && uin2->invisible == 1)
                return 1;

            if (!canmsg(uin2))
            {
                sprintf(buf, "%s 已经关闭讯息呼叫器", uin2->userid);
                msgflag = YEA;
                break;
            }
            do_sendmsg(uin2, NULL, 0, uin2->pid);
            break;
        case 'o':
        case 'O':
        case 'r':
        case 'R':
#ifdef SPECIALID

            if (!strcmp(currentuser.userid, SPECIALID1)
                || !strcmp(currentuser.userid, SPECIALID2))
                return 0;
#endif

            if (!strcmp("guest", currentuser.userid))
                return 0;
            if (ch == 'o' || ch == 'O')
            {
                friendflag = YEA;
                strcpy(desc, "好友");
            }
            else
            {
                friendflag = NA;
                strcpy(desc, "坏人");
            }
            sprintf(buf, "确定要把 %s 加入%s名单吗",
                    lookupuser.userid, desc);
            move(BBS_PAGESIZE + 2, 0);
            if (askyn(buf, NA, NA) == NA)
                break;
            if (addtooverride(lookupuser.userid) == -1)
            {
                sprintf(buf, "%s 已在%s名单", lookupuser.userid, desc);
            }
            else
            {
                sprintf(buf, "%s 列入%s名单", lookupuser.userid, desc);
            }
            msgflag = YEA;
            break;

			   case 'q':
			   case 'Q':
				   clear();
				   modify_user_mode (QUERY);
#ifdef SPECIALID

	if (!strcmp (currentuser.userid, SPECIALID1)
		|| !strcmp (currentuser.userid, SPECIALID2))
		return 0;
#endif		   
	if (!strcmp ("guest", currentuser.userid))
		return 0;
	id_detail_show(lookupuser.userid);
	break;


			   case 'i':
			   case 'I':
				clear();
				modify_user_mode (QUERY);
				if (!strcmp ("guest", currentuser.userid))
				return 0;
				id_gift_show(lookupuser.userid);
				break;



        case 'd':
        case 'D':
#ifdef SPECIALID

            if (!strcmp(currentuser.userid, SPECIALID1)
                || !strcmp(currentuser.userid, SPECIALID2))
                return 0;
#endif

            if (!strcmp("guest", currentuser.userid))
                return 0;
            sprintf(buf, "确定要把 %s 从好友名单删除吗",
                    lookupuser.userid);
            move(BBS_PAGESIZE + 2, 0);
            if (askyn(buf, NA, NA) == NA)
                break;
            if (deleteoverride(lookupuser.userid, "friends") == -1)
            {
                sprintf(buf, "%s 本来就不在朋友名单中", lookupuser.userid);
            }
            else
            {
                sprintf(buf, "%s 已从朋友名单移除", lookupuser.userid);
            }
            msgflag = YEA;
            break;
        default:
            return 0;
        }

    }

    uinfo.destuid = 0;
    return 1;
}

int count_active(struct user_info *uentp)
{
    static int count;

    if (uentp == NULL)
    {
        int c = count;

        count = 0;
        return c;
    }
    if (!uentp->active || !uentp->pid)
        return 0;
    count++;
    return 1;
}

int count_useshell(struct user_info *uentp)
{
    static int count;

    if (uentp == NULL)
    {
        int c = count;

        count = 0;
        return c;
    }
    if (!uentp->active || !uentp->pid)
        return 0;
    if (uentp->mode == SYSINFO || uentp->mode == DICT
        || uentp->mode == BBSNET || uentp->mode == LOGIN)
        count++;
    return 1;
}

int count_user_logins(struct user_info *uentp)
{
    static int count;

    if (uentp == NULL)
    {
        int c = count;

        count = 0;
        return c;
    }
    if (!uentp->active || !uentp->pid)
        return 0;
    if (!strcmp(uentp->userid, save_page_requestor))
        count++;
    return 1;
}

int count_visible_active(struct user_info *uentp)
{
    static int count;

    if (uentp == NULL)
    {
        int c = count;

        count = 0;
        return c;
    }
    if (!uentp->active || !uentp->pid || isreject(uentp))
        return 0;
    if ((!uentp->invisible) || (uentp->uid == usernum)
        || (HAS_PERM(PERM_SYSOP | PERM_SEECLOAK)))
        count++;
    return 1;
}

int alcounter(struct user_info *uentp)
{
    static int vi_users, vi_friends;
    int canseecloak;

    if (uentp == NULL)
    {
        count_friends = vi_friends;
        count_users = vi_users;
        vi_users = vi_friends = 0;
        return 1;
    }
    if (!uentp->active || !uentp->pid || isreject(uentp))
        return 0;

    canseecloak = (!uentp->invisible) || (uentp->uid == usernum)
        || (HAS_PERM(PERM_SYSOP | PERM_SEECLOAK));
    if (canseecloak)
    {
        vi_users++;
        if (myfriend(uentp->uid))
            vi_friends++;
    }
    return 1;
}

int num_alcounter(void)
{

    static int last_time = 0;

    if (abs(time(0) - last_time) < 7)
        return 0;
    last_time = time(0);
    alcounter(NULL);
    apply_ulist(alcounter);
    alcounter(NULL);
    return 0;
}

int num_useshell(void)
{
    count_useshell(NULL);
    apply_ulist(count_useshell);
    return count_useshell(NULL);
}

int num_active_users(void)
{
    count_active(NULL);
    apply_ulist(count_active);
    return count_active(NULL);
}

int num_user_logins(char *uid)
{
    strcpy(save_page_requestor, uid);
    count_active(NULL);
    apply_ulist(count_user_logins);
    return count_user_logins(NULL);
}

int num_visible_users(void)
{
    count_visible_active(NULL);
    apply_ulist(count_visible_active);
    return count_visible_active(NULL);
}

int cmpfnames(char *userid, void *vuv)
{
    struct override *uv = (struct override *) vuv;

    return !strcmp(userid, uv->id);
}

int t_cmpuids(int uid, struct user_info *up)
{
    return (up->active && uid == up->uid);
}

int t_talk(void)
{
    int netty_talk;

#ifdef DOTIMEOUT

    init_alarm();
#else

    signal(SIGALRM, SIG_IGN);
#endif

    refresh();
    netty_talk = ttt_talk(NULL);
    clear();
    return (netty_talk);
}

int ttt_talk(struct user_info *userinfo)
{
    char uident[STRLEN];
    char reason[STRLEN];
    int tuid, ucount, unum, tmp;
    struct user_info uin;

    move(1, 0);
    clrtobot();

    if (uinfo.mode != LUSERS && uinfo.mode != FRIEND
        && uinfo.mode != QAUTHOR && uinfo.mode != QUERY
        && uinfo.mode != WATCHINBOARD)
    {
        move(2, 0);
        prints("<输入使用者代号>\n");
        move(1, 0);
        clrtoeol();
        prints("跟谁聊天: ");
        creat_list();
        namecomplete(NULL, uident);
        if (uident[0] == '\0')
        {
            clear();
            return 0;
        }
        if (!(tuid = searchuser(uident)) || tuid == usernum)
        {
          wrongid:
            move(2, 0);
            prints("错误代号\n");
            pressreturn();
            move(2, 0);
            clrtoeol();
            return -1;
        }
        ucount = t_search_ulist(&uin, t_cmpuids, tuid, NA, YEA);
        if (ucount > 1)
        {
          list:
            move(3, 0);
            ucount = t_search_ulist(&uin, t_cmpuids, tuid, YEA, YEA);
            clrtobot();
            tmp = ucount + 5;
            getdata(tmp, 0, "请选一个你看的比较顺眼的 [0 -- 不聊了]: ",
                    genbuf, 4, DOECHO, YEA);
            unum = atoi(genbuf);
            if (unum == 0)
            {
                clear();
                return 0;
            }
            if (unum > ucount || unum < 0)
            {
                move(tmp, 0);
                prints("笨笨！你选错了啦！\n");
                clrtobot();
                pressreturn();
                goto list;
            }
            if (!search_ulistn(&uin, t_cmpuids, tuid, unum))
                goto wrongid;
        }
        else if (!search_ulist(&uin, t_cmpuids, tuid))
            goto wrongid;
    }
    else
    {

        if (userinfo == NULL)
        {
            move(2, 0);
            prints("内部错误\n");
            pressreturn();
            move(2, 0);
            clrtoeol();
            return -1;
        }
        uin = *userinfo;
        tuid = uin.uid;
        strcpy(uident, uin.userid);
        move(1, 0);
        clrtoeol();
        prints("跟谁聊天: %s", uin.userid);
    }

    if (!strcmp(uin.userid, "guest") && !HAS_PERM(PERM_FORCEPAGE))
        return -1;

    if (!canpage(hisfriend(&uin), uin.pager))
    {
        move(2, 0);
        prints("对方呼叫器已关闭.\n");
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    }
    if (uin.mode == SYSINFO || uin.mode == BBSNET || uin.mode == DICT
        || uin.mode == ADMIN || uin.mode == LOCKSCREEN
        || uin.mode == LOGIN)
    {
        move(2, 0);
        prints("目前无法呼叫.\n");
        clrtobot();
        pressreturn();
        return -1;
    }
    if (!uin.active || (kill(uin.pid, 0) == -1))
    {
        move(2, 0);
        prints("对方已离开\n");
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    }
    else
    {
        int sock, msgsock;
        socklen_t length;
        struct sockaddr_in server;
        char c;
        char buf[512];

        move(3, 0);
        clrtobot();
        show_user_plan(uident);
        move(2, 0);
        if (askyn("确定要和他/她谈天吗", NA, NA) == NA)
        {
            clear();
            return 0;
        }
        sprintf(buf, "Talk to '%s'", uident);
        report(buf);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
            return -1;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = 0;
        if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
            return -1;
        length = sizeof(server);
        if (getsockname(sock, (struct sockaddr *) &server, &length) < 0)
            return -1;
        uinfo.sockactive = YEA;
        uinfo.sockaddr = server.sin_port;
        uinfo.destuid = tuid;
        modify_user_mode(PAGE);
        kill(uin.pid, SIGUSR1);
        clear();
        prints("呼叫 %s 中...\n输入 Ctrl-D 结束\n", uident);

        listen(sock, 1);
        add_io(sock, 20);
        while (YEA)
        {
            int ch;

            ch = igetkey();
            if (ch == I_TIMEOUT)
            {
                move(0, 0);
                prints("再次呼叫.\n");
                add_io(sock, 20);
                bell();
                if (kill(uin.pid, SIGUSR1) == -1)
                {
                    move(0, 0);
                    prints("对方已离线\n");
                    pressreturn();

                    uinfo.sockactive = NA;
                    uinfo.destuid = 0;
                    return -1;
                }
                continue;
            }
            if (ch == I_OTHERDATA)
                break;
            if (ch == '\004')
            {
                add_io(0, 0);
                close(sock);
                uinfo.sockactive = NA;
                uinfo.destuid = 0;
                clear();
                return 0;
            }
        }

        msgsock = accept(sock, (struct sockaddr *) NULL, NULL);
        add_io(0, 0);
        close(sock);
        uinfo.sockactive = NA;

        read(msgsock, &c, sizeof(c));

        clear();

        switch (c)
        {
        case 'y':
        case 'Y':
            sprintf(save_page_requestor, "%s (%s)", uin.userid,
                    uin.username);
            do_talk(msgsock);
            break;
        case 'a':
        case 'A':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[0]);
            pressreturn();
            break;
        case 'b':
        case 'B':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[1]);
            pressreturn();
            break;
        case 'c':
        case 'C':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[2]);
            pressreturn();
            break;
        case 'd':
        case 'D':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[3]);
            pressreturn();
            break;
        case 'e':
        case 'E':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[4]);
            pressreturn();
            break;
        case 'f':
        case 'F':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[5]);
            pressreturn();
            break;
        case 'g':
        case 'G':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[6]);
            pressreturn();
            break;
        case 'n':
        case 'N':
            prints("%s (%s)说：%s\n", uin.userid, uin.username, refuse[7]);
            pressreturn();
            break;
        case 'm':
        case 'M':
            read(msgsock, reason, sizeof(reason));
            prints("%s (%s)说：%s\n", uin.userid, uin.username, reason);
            pressreturn();
            break;
        default:
            sprintf(save_page_requestor, "%s (%s)", uin.userid,
                    uin.username);
#ifdef TALK_LOG

            strcpy(partner, uin.userid);
#endif

            do_talk(msgsock);
            break;
        }
        close(msgsock);
        clear();
        refresh();
        uinfo.destuid = 0;
    }
    return 0;
}

extern int talkrequest;
struct user_info ui;
char page_requestor[STRLEN];
char page_requestorid[STRLEN];

int cmpunums(int unum, struct user_info *up)
{
    if (!up->active)
        return 0;
    return (unum == up->destuid);
}

int cmpmsgnum(int unum, struct user_info *up)
{
    if (!up->active)
        return 0;
    return (unum == up->destuid && up->sockactive == 2);
}

int setpagerequest(int mode)
{
    int tuid;

    if (mode == 0)
        tuid = search_ulist(&ui, cmpunums, usernum);
    else
        tuid = search_ulist(&ui, cmpmsgnum, usernum);
    if (tuid == 0)
        return 1;
    if (!ui.sockactive)
        return 1;
    uinfo.destuid = ui.uid;
    sprintf(page_requestor, "%s (%s)", ui.userid, ui.username);
    strcpy(page_requestorid, ui.userid);
    return 0;
}

int servicepage(int line, char *mesg)
{
    static time_t last_check;
    time_t now;
    char buf[STRLEN];
    int tuid = search_ulist(&ui, cmpunums, usernum);

    if (tuid == 0 || !ui.sockactive)
        talkrequest = NA;
    if (!talkrequest)
    {
        if (page_requestor[0])
        {
            switch (uinfo.mode)
            {
            case TALK:
                move(line, 0);
                printdash(mesg);
                break;
            default:
                sprintf(buf, "** %s 已停止呼叫.", page_requestor);
                printchatline(buf);
            }
            memset(page_requestor, 0, STRLEN);
            last_check = 0;
        }
        return NA;
    }
    else
    {
        now = time(0);
        if (now - last_check > P_INT)
        {
            last_check = now;
            if (!page_requestor[0] && setpagerequest(0))
                return NA;
            else
                switch (uinfo.mode)
                {
                case TALK:
                    move(line, 0);
                    sprintf(buf, "** %s 正在呼叫你", page_requestor);
                    printdash(buf);
                    break;
                default:
                    sprintf(buf, "** %s 正在呼叫你", page_requestor);
                    printchatline(buf);
                }
        }
    }
    return YEA;
}

int talkreply(void)
{
    int a;
    struct hostent *h;
    char buf[512];
    char reason[51];
    char hostname[STRLEN];
    struct sockaddr_in sin;
    char inbuf[STRLEN * 2];

    talkrequest = NA;
    if (setpagerequest(0))
        return 0;
#ifdef DOTIMEOUT

    init_alarm();
#else

    signal(SIGALRM, SIG_IGN);
#endif

    clear();
    move(5, 0);
    clrtobot();
    show_user_plan(page_requestorid);
    move(1, 0);
    prints("(A)【%s】(B)【%s】\n", refuse[0], refuse[1]);
    prints("(C)【%s】(D)【%s】\n", refuse[2], refuse[3]);
    prints("(E)【%s】(F)【%s】\n", refuse[4], refuse[5]);
    prints("(G)【%s】(N)【%s】\n", refuse[6], refuse[7]);
    prints("(M)【留言给 %-13s            】\n", page_requestorid);
    sprintf(inbuf, "你想跟 %s 聊聊天吗? (Y N A B C D E F G M)[Y]: ",
            page_requestor);
    strcpy(save_page_requestor, page_requestor);
#ifdef TALK_LOG

    strcpy(partner, page_requestorid);
#endif

    memset(page_requestor, 0, sizeof(page_requestor));
    memset(page_requestorid, 0, sizeof(page_requestorid));
    getdata(0, 0, inbuf, buf, 2, DOECHO, YEA);
    gethostname(hostname, STRLEN);
    if (!(h = gethostbyname(hostname)))
        return -1;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = h->h_addrtype;
    memcpy(&sin.sin_addr, h->h_addr, h->h_length);
    sin.sin_port = ui.sockaddr;
    a = socket(sin.sin_family, SOCK_STREAM, 0);
    if ((connect(a, (struct sockaddr *) &sin, sizeof(sin))))
        return -1;
    if (buf[0] != 'A' && buf[0] != 'a' && buf[0] != 'B' && buf[0] != 'b'
        && buf[0] != 'C' && buf[0] != 'c' && buf[0] != 'D' && buf[0] != 'd'
        && buf[0] != 'e' && buf[0] != 'E' && buf[0] != 'f' && buf[0] != 'F'
        && buf[0] != 'g' && buf[0] != 'G' && buf[0] != 'n' && buf[0] != 'N'
        && buf[0] != 'm' && buf[0] != 'M')
        buf[0] = 'y';
    if (buf[0] == 'M' || buf[0] == 'm')
    {
        move(1, 0);
        clrtobot();
        getdata(1, 0, "留话：", reason, 50, DOECHO, YEA);
    }
    write(a, buf, 1);
    if (buf[0] == 'M' || buf[0] == 'm')
        write(a, reason, sizeof(reason));
    if (buf[0] != 'y')
    {
        close(a);
        report("page refused");
        clear();
        refresh();
        return 0;
    }
    report("page accepted");
    clear();
    do_talk(a);
    close(a);
    clear();
    refresh();
    return 0;
}

void do_talk_nextline(struct talk_win *twin)
{

    twin->curln = twin->curln + 1;
    if (twin->curln > twin->eline)
        twin->curln = twin->sline;
    if (twin->curln != twin->eline)
    {
        move(twin->curln + 1, 0);
        clrtoeol();
    }
    move(twin->curln, 0);
    clrtoeol();
    twin->curcol = 0;
}

void do_talk_char(struct talk_win *twin, int ch)
{
    extern int dumb_term;

    if (isprint2(ch))
    {
        if (twin->curcol < 79)
        {
            move(twin->curln, (twin->curcol)++);
            prints("%c", ch);
            return;
        }
        do_talk_nextline(twin);
        twin->curcol++;
        prints("%c", ch);
        return;
    }
    switch (ch)
    {
    case Ctrl('H'):
    case '\177':
        if (dumb_term)
            ochar(Ctrl('H'));
        if (twin->curcol == 0)
        {
            return;
        }
        (twin->curcol)--;
        move(twin->curln, twin->curcol);
        if (!dumb_term)
            prints(" ");
        move(twin->curln, twin->curcol);
        return;
    case Ctrl('M'):
    case Ctrl('J'):
        if (dumb_term)
            prints("\n");
        do_talk_nextline(twin);
        return;
    case Ctrl('G'):
        bell();
        return;
    default:
        break;
    }
    return;
}

void do_talk_string(struct talk_win *twin, char *str)
{
    while (*str)
    {
        do_talk_char(twin, *str++);
    }
}

char talkobuf[80];
int talkobuflen;
int talkflushfd;

int talkflush(void)
{
    if (talkobuflen)
        write(talkflushfd, talkobuf, talkobuflen);
    talkobuflen = 0;
    return 0;
}

int moveto(int mode, struct talk_win *twin)
{
    if (mode == 1)
        twin->curln--;
    if (mode == 2)
        twin->curln++;
    if (mode == 3)
        twin->curcol++;
    if (mode == 4)
        twin->curcol--;
    if (twin->curcol < 0)
    {
        twin->curln--;
        twin->curcol = 0;
    }
    else if (twin->curcol > 79)
    {
        twin->curln++;
        twin->curcol = 0;
    }
    if (twin->curln < twin->sline)
    {
        twin->curln = twin->eline;
    }
    if (twin->curln > twin->eline)
    {
        twin->curln = twin->sline;
    }
    move(twin->curln, twin->curcol);
    return 0;
}

void endmsg(int num)
{
    int x, y;
    int tmpansi;

    tmpansi = showansi;
    showansi = 1;
    talkidletime += 60;
    if (talkidletime >= IDLE_TIMEOUT)
        kill(getpid(), SIGHUP);
    if (uinfo.in_chat == YEA)
        return;
    getyx(&x, &y);
    update_endline();
    signal(SIGALRM, endmsg);
    move(x, y);
    refresh();
    alarm(60);
    showansi = tmpansi;
    return;
}

int do_talk(int fd)
{
    struct talk_win mywin, itswin;
    char mid_line[256];
    int page_pending = NA;
    int i, i2;
    char ans[3];
    int previous_mode;

#ifdef TALK_LOG

    char mywords[80], itswords[80], talkbuf[80];
    int mlen = 0, ilen = 0;
    time_t now;

    mywords[0] = itswords[0] = '\0';
#endif

    signal(SIGALRM, SIG_IGN);
    endmsg(0);
    refresh();
    previous_mode = uinfo.mode;
    modify_user_mode(TALK);
    sprintf(mid_line, " %s (%s) 和 %s 正在畅谈中",
            currentuser.userid, currentuser.username, save_page_requestor);

    memset(&mywin, 0, sizeof(mywin));
    memset(&itswin, 0, sizeof(itswin));
    i = (t_lines - 1) / 2;
    mywin.eline = i - 1;
    itswin.curln = itswin.sline = i + 1;
    itswin.eline = t_lines - 2;
    move(i, 0);
    printdash(mid_line);
    move(0, 0);

    talkobuflen = 0;
    talkflushfd = fd;
    add_io(fd, 0);
    add_flush(talkflush);

    while (YEA)
    {
        int ch;

        if (talkrequest)
            page_pending = YEA;
        if (page_pending)
            page_pending = servicepage((t_lines - 1) / 2, mid_line);
        ch = igetkey();
        talkidletime = 0;
        if (ch == '')
        {
            igetkey();
            igetkey();
            continue;
        }
        if (ch == I_OTHERDATA)
        {
            char data[80];
            int datac;
            register int i;

            datac = read(fd, data, 80);
            if (datac <= 0)
                break;
            for (i = 0; i < datac; i++)
            {
                if (data[i] >= 1 && data[i] <= 4)
                {
                    moveto(data[i] - '\0', &itswin);
                    continue;
                }
#ifdef TALK_LOG

                else if (isprint2(data[i]))
                {
                    if (ilen >= 80)
                    {
                        itswords[79] = '\0';
                        (void) do_log(itswords, 2);
                        ilen = 0;
                    }
                    else
                    {
                        itswords[ilen] = data[i];
                        ilen++;
                    }
                }
                else if ((data[i] == Ctrl('H') || data[i] == '\177')
                         && !ilen)
                {
                    itswords[ilen--] = '\0';
                }
                else if (data[i] == Ctrl('M') || data[i] == '\r' ||
                         data[i] == '\n')
                {
                    itswords[ilen] = '\0';
                    (void) do_log(itswords, 2);
                    ilen = 0;
                }
#endif
                do_talk_char(&itswin, data[i]);
            }
        }
        else
        {
            if (ch == Ctrl('D') || ch == Ctrl('C'))
                break;
            if (isprint2(ch) || ch == Ctrl('H') || ch == '\177'
                || ch == Ctrl('G'))
            {
                talkobuf[talkobuflen++] = ch;
                if (talkobuflen == 80)
                    talkflush();
#ifdef TALK_LOG

                if (mlen < 80)
                {
                    if ((ch == Ctrl('H') || ch == '\177') && mlen != 0)
                    {
                        mywords[mlen--] = '\0';
                    }
                    else
                    {
                        mywords[mlen] = ch;
                        mlen++;
                    }
                }
                else if (mlen >= 80)
                {
                    mywords[79] = '\0';
                    (void) do_log(mywords, 1);
                    mlen = 0;
                }
#endif
                do_talk_char(&mywin, ch);
            }
            else if (ch == '\n' || ch == Ctrl('M') || ch == '\r')
            {
#ifdef TALK_LOG
                if (mywords[0] != '\0')
                {
                    mywords[mlen++] = '\0';
                    (void) do_log(mywords, 1);
                    mlen = 0;
                }
#endif
                talkobuf[talkobuflen++] = '\r';
                talkflush();
                do_talk_char(&mywin, '\r');
            }
            else if (ch >= KEY_UP && ch <= KEY_LEFT)
            {
                moveto(ch - KEY_UP + 1, &mywin);
                talkobuf[talkobuflen++] = ch - KEY_UP + 1;
                if (talkobuflen == 80)
                    talkflush();
            }
            else if (ch == Ctrl('E'))
            {
                for (i2 = 0; i2 <= 10; i2++)
                {
                    talkobuf[talkobuflen++] = '\r';
                    talkflush();
                    do_talk_char(&mywin, '\r');
                }
            }
            else if (ch == Ctrl('P') && HAS_PERM(PERM_BASIC))
            {
                t_pager();
                update_utmp();
                update_endline();
            }
        }
    }
    add_io(0, 0);
    talkflush();
    signal(SIGALRM, SIG_IGN);
    add_flush(NULL);
    modify_user_mode(previous_mode);

#ifdef TALK_LOG

    mywords[mlen] = '\0';
    itswords[ilen] = '\0';

    if (mywords[0] != '\0')
        do_log(mywords, 1);
    if (itswords[0] != '\0')
        do_log(itswords, 2);

    now = time(0);
    sprintf(talkbuf, "\n[1;34m通话结束, 时间: %s [m\n", Cdate(&now));
    write(talkrec, talkbuf, strlen(talkbuf));

    close(talkrec);

    sethomefile(genbuf, currentuser.userid, "talklog");

    getdata(23, 0, "是否寄回聊天纪录 [Y/n]: ", ans, 2, DOECHO, YEA);

    switch (ans[0])
    {
    case 'n':
    case 'N':
        break;
    default:
        sethomefile(talkbuf, currentuser.userid, "talklog");
        sprintf(mywords, "跟 %s 的聊天记录 [%s]", partner,
                Cdate(&now) + 4);
        {
            char temp[STRLEN];

            strcpy(temp, save_title);
            mail_file(talkbuf, currentuser.userid, mywords);
            strcpy(save_title, temp);
        }
    }
    sethomefile(talkbuf, currentuser.userid, "talklog");
    unlink(talkbuf);
#endif

    return 0;
}

int shortulist(void)
{
    int i;
    int pageusers = 60;
    extern struct user_info *user_record[];
    extern int range;

    fill_userlist();
    if (ulistpage > ((range - 1) / pageusers))
        ulistpage = 0;
    if (ulistpage < 0)
        ulistpage = (range - 1) / pageusers;
    move(1, 0);
    clrtoeol();
    prints
        ("每隔 [1;32m%d[m 秒更新一次，[1;32mCtrl-C[m 或 [1;32mCtrl-D[m 离开，[1;32mF[m 更换模式 [1;32m↑↓[m 上、下一页 第[1;32m %1d[m 页",
         M_INT, ulistpage + 1);
    clrtoeol();
    move(3, 0);
    clrtobot();
    for (i = ulistpage * pageusers;
         i < (ulistpage + 1) * pageusers && i < range; i++)
    {
        char ubuf[STRLEN];
        int ovv;

        if (i < numf || friendmode)
            ovv = YEA;
        else
            ovv = NA;
        sprintf(ubuf, "%s%-12.12s %s%-10.10s[m",
                (ovv) ? "[1;32m√" : "  ", user_record[i]->userid,
                (user_record[i]->invisible == YEA) ? "[1;34m" : "",
                ModeType(user_record[i]->mode));

        prints("%s", ubuf);
        if ((i + 1) % 3 == 0)
            outc('\n');
        else
            prints(" |");
    }
    return range;
}

int do_list(char *modestr)
{
    char buf[STRLEN];
    int count;
    extern int RMSG;

    if (RMSG != YEA)
    {
        move(0, 0);
        clrtoeol();
        if (chkmail())
            showtitle(modestr, "[您有信件]");
        else
            showtitle(modestr, BoardName);
    }
    move(2, 0);
    clrtoeol();
    sprintf(buf, "  %-12s %-10s", "使用者代号", "目前动态");
    prints("[1;33;44m%s |%s |%s[m", buf, buf, buf);
    count = shortulist();
    if (uinfo.mode == MONITOR)
    {
        time_t thetime = time(0);

        move(t_lines - 1, 0);
        getdatestring(thetime, NA);
        sprintf(genbuf,
                "[1;44;33m  目前有 [32m%3d[33m %6s上线, 时间: [32m%22.22s [33m, 目前状态：[36m%10s   [m",
                count, friendmode ? "好朋友" : "使用者", datestring,
                friendmode ? "你的好朋友" : "所有使用者");
        prints(genbuf);
    }
    refresh();
    return 0;
}

int t_list(void)
{
    if (watchinboardmode)
        modify_user_mode(WATCHINBOARD);
    else
        modify_user_mode(LUSERS);
    report("t_list");
    do_list("使用者状态");
    pressreturn();
    refresh();
    clear();
    return 0;
}

void sig_catcher(int num)
{
    ulistpage++;
    if (uinfo.mode != MONITOR)
    {
#ifdef DOTIMEOUT
        init_alarm();
#else

        signal(SIGALRM, SIG_IGN);
#endif

        return;
    }
    if (signal(SIGALRM, sig_catcher) == SIG_ERR)
        exit(1);
    do_list("探视民情");
    alarm(M_INT);
}

int t_monitor(void)
{
    int i;
    char modestr[] = "探视民情";

    alarm(0);
    signal(SIGALRM, sig_catcher);

    report("monitor");
    modify_user_mode(MONITOR);
    ulistpage = 0;
    do_list(modestr);
    alarm(M_INT);
    while (YEA)
    {
        i = egetch();
        if (i == 'f' || i == 'F')
        {
            if (friendmode == YEA)
                friendmode = NA;
            else
                friendmode = YEA;
            do_list(modestr);
        }
        if (i == KEY_DOWN)
        {
            ulistpage++;
            do_list(modestr);
        }
        if (i == KEY_UP)
        {
            ulistpage--;
            do_list(modestr);
        }
        if (i == Ctrl('D') || i == Ctrl('C') || i == KEY_LEFT)
            break;

    }
    move(2, 0);
    clrtoeol();
    clear();
    return 0;
}

int seek_in_file(char *filename, char *seekstr)
{
    FILE *fp;
    char buf[STRLEN];
    char *namep;

    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    while (fgets(buf, STRLEN, fp) != NULL)
    {
        namep = (char *) strtok(buf, ": \n\r\t");
        if (namep != NULL && strcasecmp(namep, seekstr) == 0)
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int listfilecontent(char *fname, int y)
{
    FILE *fp;
    int x = 0, cnt = 0, max = 0, len;
    char u_buf[20], line[STRLEN], *nick;

    move(y, x);
    CreateNameList();
    strcpy(genbuf, fname);
    if ((fp = fopen(genbuf, "r")) == NULL)
    {
        prints("(none)\n");
        return 0;
    }
    while (fgets(genbuf, 1024, fp) != NULL)
    {
        strtok(genbuf, " \n\r\t");
        strncpy(u_buf, genbuf, 20);
        u_buf[19] = '\0';
        if (!AddToNameList(u_buf))
            continue;
        nick = (char *) strtok(NULL, "\n\r\t");
        if (nick != NULL)
        {
            while (*nick == ' ')
                nick++;
            if (*nick == '\0')
                nick = NULL;
        }
        if (nick == NULL)
        {
            strcpy(line, u_buf);
        }
        else
        {
            sprintf(line, "%-12s%s", u_buf, nick);
        }
        if ((len = strlen(line)) > max)
            max = len;
        if (x + len > 78)
            line[78 - x] = '\0';
        prints("%s", line);
        cnt++;
        if ((++y) >= t_lines - 1)
        {
            y = 3;
            x += max + 2;
            max = 0;
            if (x > 70)
                break;
        }
        move(y, x);
    }
    fclose(fp);
    if (cnt == 0)
        prints("(none)\n");
    return cnt;
}

int listfilecontent2(char *fname, int y, int startline)
{
    FILE *fp;
    int x = 0, cnt = 0, max = 0, len;
    char u_buf[20], line[STRLEN], *nick;

    int currentline = 0;

    move(y, x);
    CreateNameList();
    strcpy(genbuf, fname);
    if ((fp = fopen(genbuf, "r")) == NULL)
    {
        prints("(none)\n");
        return 0;
    }
    while (fgets(genbuf, 1024, fp) != NULL)
    {
        strtok(genbuf, " \n\r\t");
        strncpy(u_buf, genbuf, 20);
        u_buf[19] = '\0';
        if (!AddToNameList(u_buf))
            continue;
        nick = (char *) strtok(NULL, "\n\r\t");
        if (nick != NULL)
        {
            while (*nick == ' ')
                nick++;
            if (*nick == '\0')
                nick = NULL;
        }
        if (nick == NULL)
        {
            strcpy(line, u_buf);
        }
        else
        {
            sprintf(line, "%-12s%s", u_buf, nick);
        }
        if ((len = strlen(line)) > max)
            max = len;
        if (x + len > 78)
            line[78 - x] = '\0';

        if (currentline >= startline)
        {
            prints("%s", line);
            cnt++;
            if ((++y) >= t_lines - 1)
            {
                y = 3;
                x += max + 2;
                max = 0;
                if (x > 70)
                    break;
            }
            move(y, x);
        }
        ++currentline;

    }
    fclose(fp);
    if (cnt == 0)
        prints("(none)\n");
    return cnt;
}

int add_to_file(char *filename, char *str)
{
    FILE *fp;
    int rc;

    if ((fp = fopen(filename, "a")) == NULL)
        return -1;
    flock(fileno(fp), LOCK_EX);
    rc = fprintf(fp, "%s\n", str);
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    return (rc == EOF ? -1 : 1);
}

int addtooverride(char *uident)
{
    struct override tmp;
    int n;
    char buf[STRLEN];
    char desc[5];

    memset(&tmp, 0, sizeof(tmp));
    if (friendflag)
    {
        setuserfile(buf, "friends");
        n = MAXFRIENDS;
        strcpy(desc, "好友");
    }
    else
    {
        setuserfile(buf, "rejects");
        n = MAXREJECTS;
        strcpy(desc, "坏人");
    }
    if (get_num_records(buf, sizeof(struct override)) >= n)
    {
        move(t_lines - 2, 0);
        clrtoeol();
        prints("抱歉，本站目前仅可以设定 %d 个%s, 请按任何件继续...", n,
               desc);
        igetkey();
        move(t_lines - 2, 0);
        clrtoeol();
        return -1;
    }
    else
    {
        if (friendflag)
        {
            if (myfriend(searchuser(uident)))
            {
                sprintf(buf, "%s 已在好友名单", uident);
                show_message(buf);
                return -1;
            }
        }
        else if (search_record(buf, &tmp, sizeof(tmp), cmpfnames, uident)
                 > 0)
        {
            sprintf(buf, "%s 已在坏人名单", uident);
            show_message(buf);
            return -1;
        }
    }
    if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
        && uinfo.mode != FRIEND && uinfo.mode != WATCHINBOARD
        && uinfo.mode != QUERY && uinfo.mode != QAUTHOR)
        n = 2;
    else
        n = t_lines - 2;

    strcpy(tmp.id, uident);
    move(n, 0);
    clrtoeol();
    refresh();
    sprintf(genbuf, "请输入给%s【%s】的说明: ", desc, tmp.id);
    getdata(n, 0, genbuf, tmp.exp, 40, DOECHO, YEA);

    n = append_record(buf, &tmp, sizeof(struct override));
    if (n != -1)
        (friendflag) ? getfriendstr() : getrejectstr();
    else
        report("append override error");
    return n;
}

int del_from_file(char *filename, char *str)
{
    FILE *fp, *nfp;
    int deleted = NA;
    char tmpbuf[1024], fnnew[STRLEN];

    if ((fp = fopen(filename, "r")) == NULL)
        return -1;
    sprintf(fnnew, "%s.%d", filename, getuid());
    if ((nfp = fopen(fnnew, "w")) == NULL)
        return -1;
    while (fgets(tmpbuf, 1024, fp) != NULL)
    {
        if (strncmp(tmpbuf, str, strlen(str)) == 0
            && (tmpbuf[strlen(str)] == '\0' || tmpbuf[strlen(str)] == ' '
                || tmpbuf[strlen(str)] == '\n'))
            deleted = YEA;
        else if (*tmpbuf > ' ')
            fputs(tmpbuf, nfp);
    }
    fclose(fp);
    fclose(nfp);
    if (!deleted)
        return -1;
    return (rename(fnnew, filename) + 1);
}

int deleteoverride(char *uident, char *filename)
{
    int deleted;
    struct override fh;
    char buf[STRLEN];

    setuserfile(buf, filename);
    deleted = search_record(buf, &fh, sizeof(fh), cmpfnames, uident);
    if (deleted > 0)
    {
        if (delete_record(buf, sizeof(fh), deleted) != -1)
        {
            (friendflag) ? getfriendstr() : getrejectstr();
        }
        else
        {
            deleted = -1;
            report("delete override error");
        }
    }
    return (deleted > 0) ? 1 : -1;
}

void override_title(void)
{
    char desc[5];

    if (chkmail())
        strcpy(genbuf, "[您有信件]");
    else
        strcpy(genbuf, BoardName);
    if (friendflag)
    {
        showtitle("[编辑好友名单]", genbuf);
        strcpy(desc, "好友");
    }
    else
    {
        showtitle("[编辑坏人名单]", genbuf);
        strcpy(desc, "坏人");
    }
    prints
        (" [[1;32m←[m,[1;32me[m] 离开 [[1;32mh[m] 求助 [[1;32m→[m,[1;32mRtn[m] %s说明档 [[1;32m↑[m,[1;32m↓[m] 选择 [[1;32ma[m] 增加%s [[1;32md[m] 删除%s\n",
         desc, desc, desc);
    prints
        ("[1;44m 编号  %s代号      %s说明                                                   [m\n",
         desc, desc);
}

char *override_doentry(int ent, void *vfh)
{
    struct override *fh = (struct override *) vfh;
    static char buf[STRLEN];

    sprintf(buf, " %4d  %-12.12s  %s", ent, fh->id, fh->exp);
    return buf;
}

int override_edit(int ent, struct override *fh, char *direc)
{
    struct override nh;
    char buf[STRLEN / 2];
    int pos;

    pos = search_record(direc, &nh, sizeof(nh), cmpfnames, fh->id);
    move(t_lines - 2, 0);
    clrtoeol();
    if (pos > 0)
    {
        sprintf(buf, "请输入 %s 的新%s说明: ", fh->id,
                (friendflag) ? "好友" : "坏人");
        getdata(t_lines - 2, 0, buf, nh.exp, 40, DOECHO, NA);
    }
    if (substitute_record(direc, &nh, sizeof(nh), pos) <= 0)
        report("Override files subs err");
    move(t_lines - 2, 0);
    clrtoeol();
    return NEWDIRECT;
}

int override_add(int ent, struct override *fh, char *direct)
{
    char uident[13];

    clear();
    move(1, 0);
    usercomplete("请输入要增加的代号: ", uident);
    if (uident[0] != '\0')
    {
        if (getuser(uident) <= 0)
        {
            move(2, 0);
            prints("错误的使用者代号...");
            pressanykey();
            return FULLUPDATE;
        }
        else
            addtooverride(uident);
        prints("\n把 %s 加入%s名单中...", uident,
               (friendflag) ? "好友" : "坏人");
        pressanykey();
    }
    return FULLUPDATE;
}

int override_dele(int ent, struct override *fh, char *direct)
{
    char buf[STRLEN];
    char desc[5];
    char fname[10];
    int deleted = NA;

    if (friendflag)
    {
        strcpy(desc, "好友");
        strcpy(fname, "friends");
    }
    else
    {
        strcpy(desc, "坏人");
        strcpy(fname, "rejects");
    }
    saveline(t_lines - 2, 0);
    move(t_lines - 2, 0);
    sprintf(buf, "是否把【%s】从%s名单中去除", fh->id, desc);
    if (askyn(buf, NA, NA) == YEA)
    {
        move(t_lines - 2, 0);
        clrtoeol();
        if (deleteoverride(fh->id, fname) == 1)
        {
            prints("已从%s名单中移除【%s】,按任何键继续...", desc, fh->id);
            deleted = YEA;
        }
        else
            prints("找不到【%s】,按任何键继续...", fh->id);
    }
    else
    {
        move(t_lines - 2, 0);
        clrtoeol();
        prints("取消删除%s...", desc);
    }
    igetkey();
    move(t_lines - 2, 0);
    clrtoeol();
    saveline(t_lines - 2, 1);
    return (deleted) ? PARTUPDATE : DONOTHING;
}

int friend_edit(int ent, struct override *fh, char *direct)
{
    friendflag = YEA;
    return override_edit(ent, fh, direct);
}

int friend_add(int ent, struct override *fh, char *direct)
{
    friendflag = YEA;
    return override_add(ent, fh, direct);
}

int friend_dele(int ent, struct override *fh, char *direct)
{
    friendflag = YEA;
    return override_dele(ent, fh, direct);
}

int friend_mail(int ent, struct override *fh, char *direct)
{
    if (!HAS_PERM(PERM_POST))
        return DONOTHING;
    m_send(fh->id);
    return FULLUPDATE;
}

int friend_query(int ent, struct override *fh, char *direct)
{
    int ch;

    if (t_query(fh->id) == -1)
        return FULLUPDATE;
    move(t_lines - 1, 0);
    clrtoeol();
    prints
        ("[0;1;44;31m[读取好友说明档][33m 寄信给好友 m │ 结束 Q,← │上一位 ↑│下一位 <Space>,↓      [m");
    ch = egetch();
    switch (ch)
    {
    case 'N':
    case 'Q':
    case 'n':
    case 'q':
    case KEY_LEFT:
        break;
    case 'm':
    case 'M':
        m_send(fh->id);
        break;
    case ' ':
    case 'j':
    case KEY_RIGHT:
    case KEY_DOWN:
    case KEY_PGDN:
        return READ_NEXT;
    case KEY_UP:
    case KEY_PGUP:
        return READ_PREV;
    default:
        break;
    }
    return FULLUPDATE;
}

int friend_help(void)
{
    show_help("help/friendshelp");
    return FULLUPDATE;
}

int reject_edit(int ent, struct override *fh, char *direct)
{
    friendflag = NA;
    return override_edit(ent, fh, direct);
}

int reject_add(int ent, struct override *fh, char *direct)
{
    friendflag = NA;
    return override_add(ent, fh, direct);
}

int reject_dele(int ent, struct override *fh, char *direct)
{
    friendflag = NA;
    return override_dele(ent, fh, direct);
}

int reject_query(int ent, struct override *fh, char *direct)
{
    int ch;

    if (t_query(fh->id) == -1)
        return FULLUPDATE;
    move(t_lines - 1, 0);
    clrtoeol();
    prints
        ("[0;1;44;31m[读取坏人说明档][33m 结束 Q,← │上一位 ↑│下一位 <Space>,↓                      [m");
    ch = egetch();
    switch (ch)
    {
    case 'N':
    case 'Q':
    case 'n':
    case 'q':
    case KEY_LEFT:
        break;
    case ' ':
    case 'j':
    case KEY_RIGHT:
    case KEY_DOWN:
    case KEY_PGDN:
        return READ_NEXT;
    case KEY_UP:
    case KEY_PGUP:
        return READ_PREV;
    default:
        break;
    }
    return FULLUPDATE;
}

int reject_help(void)
{
    show_help("help/rejectshelp");
    return FULLUPDATE;
}

void t_friend(void)
{
    char buf[STRLEN];

    friendflag = YEA;
    setuserfile(buf, "friends");
    i_read(GMENU, buf, override_title, override_doentry, friend_list,
           sizeof(struct override));
    clear();
    return;
}

void t_reject(void)
{
    char buf[STRLEN];

    friendflag = NA;
    setuserfile(buf, "rejects");
    i_read(GMENU, buf, override_title, override_doentry, reject_list,
           sizeof(struct override));
    clear();
    return;
}

int cmpfuid(const void *va, const void *vb)
{
    unsigned int *a = (unsigned int *) va, *b = (unsigned int *) vb;

    return *a - *b;
}

int getfriendstr(void)
{
    int i;
    struct override *tmp;

    memset(uinfo.friends, 0, sizeof(uinfo.friends));
    setuserfile(genbuf, "friends");
    uinfo.fnum = get_num_records(genbuf, sizeof(struct override));
    if (uinfo.fnum <= 0)
        return 0;
    uinfo.fnum = (uinfo.fnum >= MAXFRIENDS) ? MAXFRIENDS : uinfo.fnum;
    tmp = (struct override *) calloc(sizeof(struct override), uinfo.fnum);
    get_records(genbuf, tmp, sizeof(struct override), 1, uinfo.fnum);
    for (i = 0; i < uinfo.fnum; i++)
    {
        uinfo.friends[i] = searchuser(tmp[i].id);
        if (uinfo.friends[i] == 0)
            deleteoverride(tmp[i].id, "friends");

    }
    free(tmp);
    qsort(&uinfo.friends, uinfo.fnum, sizeof(uinfo.friends[0]), cmpfuid);
    update_ulist(&uinfo, utmpent);
    return 1;
}

int getrejectstr(void)
{
    int nr, i;
    struct override *tmp;

    memset(uinfo.reject, 0, sizeof(uinfo.reject));
    setuserfile(genbuf, "rejects");
    nr = get_num_records(genbuf, sizeof(struct override));
    if (nr <= 0)
        return 0;
    nr = (nr >= MAXREJECTS) ? MAXREJECTS : nr;
    tmp = (struct override *) calloc(sizeof(struct override), nr);
    get_records(genbuf, tmp, sizeof(struct override), 1, nr);
    for (i = 0; i < nr; i++)
    {
        uinfo.reject[i] = searchuser(tmp[i].id);
        if (uinfo.reject[i] == 0)
            deleteoverride(tmp[i].id, "rejects");
    }
    free(tmp);
    update_ulist(&uinfo, utmpent);
    return 1;
}

#ifdef TALK_LOG

void do_log(char *msg, int who)
{

    time_t now;
    char buf[100];

    now = time(0);
    if (msg[strlen(msg)] == '\n')
        msg[strlen(msg)] = '\0';

    if (strlen(msg) < 1 || msg[0] == '\r' || msg[0] == '\n')
        return;

    sethomefile(buf, currentuser.userid, "talklog");

    if (!dashf(buf) || talkrec == -1)
    {
        talkrec = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0644);
        sprintf(buf, "[1;32m与 %s 的情话绵绵, 日期: %s [m\n",
                save_page_requestor, Cdate(&now));
        write(talkrec, buf, strlen(buf));
        sprintf(buf, "\t颜色分别代表: [1;33m%s[m [1;36m%s[m \n\n",
                currentuser.userid, partner);
        write(talkrec, buf, strlen(buf));
    }
    if (who == 1)
    {
        sprintf(buf, "[1;33m-=> %s [m\n", msg);
        write(talkrec, buf, strlen(buf));
    }
    else if (who == 2)
    {
        sprintf(buf, "[1;36m--> %s [m\n", msg);
        write(talkrec, buf, strlen(buf));
    }
}
#endif

int deal_key3(char ch, int allnum, int pagenum)
{
    char buf[STRLEN], desc[5];
    static int msgflag;
    extern int friendflag;

    if (msgflag == YEA)
    {
        show_message(NULL);
        msgflag = NA;
    }
    switch (ch)
    {
    case 't':
    case 'T':
        if (!HAS_PERM(PERM_PAGE))
            return 1;
        if (user_record[allnum]->uid != usernum)
            ttt_talk(user_record[allnum]);
        else
            return 1;
        break;

    case 'm':
    case 'M':
        if (!HAS_PERM(PERM_POST))
            return 1;
        m_send(user_record[allnum]->userid);
        break;
    case 'g':
    case 'G':
        if (!HAS_PERM(PERM_POST))
            return 1;
        sendGoodWish(user_record[allnum]->userid);
        break;

    case 's':
    case 'S':
        if (!strcmp("guest", currentuser.userid))
            return 0;
        if (!HAS_PERM(PERM_MESSAGE))
            return 0;
        if (!canmsg(user_record[allnum]))
        {
            sprintf(buf, "%s 已经关闭讯息呼叫器",
                    user_record[allnum]->userid);
            msgflag = YEA;
            break;
        }
        do_sendmsg(user_record[allnum], NULL, 0, user_record[allnum]->pid);
        break;
    case 'o':
    case 'O':
    case 'r':
    case 'R':
#ifdef SPECIALID

        if (!strcmp(currentuser.userid, SPECIALID1)
            || !strcmp(currentuser.userid, SPECIALID2))
            return 0;
#endif

        if (!strcmp("guest", currentuser.userid))
            return 0;
        if (ch == 'o' || ch == 'O')
        {
            friendflag = YEA;
            strcpy(desc, "好友");
        }
        else
        {
            friendflag = NA;
            strcpy(desc, "坏人");
        }
        sprintf(buf, "确定要把 %s 加入%s名单吗",
                user_record[allnum]->userid, desc);
        move(BBS_PAGESIZE + 2, 0);
        if (askyn(buf, NA, NA) == NA)
            break;
        if (addtooverride(user_record[allnum]->userid) == -1)
        {
            sprintf(buf, "%s 已在%s名单", user_record[allnum]->userid,
                    desc);
        }
        else
        {
            sprintf(buf, "%s 列入%s名单", user_record[allnum]->userid,
                    desc);
        }
        msgflag = YEA;
        break;
    case 'd':
    case 'D':
#ifdef SPECIALID

        if (!strcmp(currentuser.userid, SPECIALID1)
            || !strcmp(currentuser.userid, SPECIALID2))
            return 0;
#endif

        if (!strcmp("guest", currentuser.userid))
            return 0;
        sprintf(buf, "确定要把 %s 从好友名单删除吗",
                user_record[allnum]->userid);
        move(BBS_PAGESIZE + 2, 0);
        if (askyn(buf, NA, NA) == NA)
            break;
        if (deleteoverride(user_record[allnum]->userid, "friends") == -1)
        {
            sprintf(buf, "%s 本来就不在朋友名单中",
                    user_record[allnum]->userid);
        }
        else
        {
            sprintf(buf, "%s 已从朋友名单移除",
                    user_record[allnum]->userid);
        }
        msgflag = YEA;
        break;
    default:
        return 0;
    }

    return 1;
}

void do_query3(int star, int curr)
{
    if (user_record[curr]->userid != NULL)
    {
        clear();
        t_query(user_record[curr]->userid);
        move(t_lines - 1, 0);
        prints
            ("\033[0;1;37;44m聊天[\033[1;32mt\033[37m] 寄信[\033[1;32mm\033[37m] 送讯息[\033[1;32ms\033[37m] 加,减朋友[\033[1;32mo\033[37m,\033[1;32md\033[37m] \033[m");
    }
}

int IsGoodBm(char *id)
{
    return seek_in_file("etc/goodbm", id);
}
