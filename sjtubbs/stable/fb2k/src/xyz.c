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

#include <sys/types.h>
#include <sys/wait.h>
#define EXTERN
#include "bbs.h"

int use_define = 0;
int child_pid = 0;
extern int iscolor;
extern int enabledbchar;

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif

char datestring[30];

extern struct BCACHE *brdshm;

#define TH_LOW	10
#define TH_HIGH	15

#define MAX_PLAN_SIZE 255*17

int modify_user_mode(int mode)
{
    uinfo.mode = mode;
    update_ulist(&uinfo, utmpent);
    return 0;
}

int showperminfo(unsigned int pbits, int i, int flag)
{
    char buf[STRLEN];

    sprintf(buf, "%c. %-30s %2s", 'A' + i,
            (use_define) ? user_definestr[i] : permstrings[i],
            ((pbits >> i) & 1 ? "是" : "×"));
    move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
    prints(buf);
    refresh();
    return YEA;
}

unsigned int
setperms(unsigned int pbits, char *prompt, int numbers,
         int (*showfunc) (unsigned int, int, int))
{
    int lastperm = numbers - 1;
    int i, done = NA;
    char choice[3], buf[80];

    move(4, 0);
    prints("请按下你要的代码来设定%s，按 Enter 结束.\n", prompt);
    move(6, 0);
    clrtobot();
    for (i = 0; i <= lastperm; i++)
    {
        (*showfunc) (pbits, i, NA);
    }
    while (!done)
    {
        sprintf(buf, "选择(ENTER 结束%s): ",
                ((strcmp(prompt, "权限") != 0)) ? "" : "，0 停权");
        getdata(t_lines - 1, 0, buf, choice, 2, DOECHO, YEA);
        *choice = toupper(*choice);
        if (*choice == '0')
            return (0);
        else if (*choice == '\n' || *choice == '\0')
            done = YEA;
        else if (*choice < 'A' || *choice > 'A' + lastperm)
            bell();
        else
        {
            i = *choice - 'A';
            pbits ^= (1 << i);
            if ((*showfunc) (pbits, i, YEA) == NA)
            {
                pbits ^= (1 << i);
            }
        }
    }
    return (pbits);
}

int x_userdefine(void)
{
    int id;
    unsigned int newlevel;

    FILE *fp;
    char homefilepath[STRLEN * 2];

    modify_user_mode(USERDEF);
    if (!(id = getuser(currentuser.userid)))
    {
        move(3, 0);
        prints("错误的使用者 ID...");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    move(1, 0);
    clrtobot();
    move(2, 0);
    use_define = 1;
    newlevel =
        setperms(lookupuser.userdefine, "参数", NUMDEFINES, showperminfo);
    move(2, 0);
    if (newlevel == lookupuser.userdefine)
        prints("参数没有修改...\n");
    else
    {
#ifdef ALLOWSWITCHCODE
        if ((!convcode && !(newlevel & DEF_USEGB))
            || (convcode && (newlevel & DEF_USEGB)))
            switch_code();
#endif

        lookupuser.userdefine = newlevel;
        currentuser.userdefine = newlevel;
        substitute_record(PASSFILE, &lookupuser, sizeof(struct userec),
                          id);

        if (lookupuser.userdefine & DEF_NOGMAIL)
        {
            sethomefile(homefilepath, lookupuser.userid, "ALLOW_GMAIL");
            unlink(homefilepath);
        }
        else
        {
            sethomefile(homefilepath, currentuser.userid, "ALLOW_GMAIL");
            fp = fopen(homefilepath, "w");
            if (fp != NULL)
                fclose(fp);
        }

        uinfo.pager |= FRIEND_PAGER;
        if (!(uinfo.pager & ALL_PAGER))
        {
            if (!DEFINE(DEF_FRIENDCALL))
                uinfo.pager &= ~FRIEND_PAGER;
        }
        uinfo.pager &= ~ALLMSG_PAGER;
        uinfo.pager &= ~FRIENDMSG_PAGER;
        if (DEFINE(DEF_DELDBLCHAR))
            enabledbchar = 1;
        else
            enabledbchar = 0;
        uinfo.from[22] = 'S';
        if (DEFINE(DEF_FRIENDMSG))
        {
            uinfo.pager |= FRIENDMSG_PAGER;
        }
        if (DEFINE(DEF_ALLMSG))
        {
            uinfo.pager |= ALLMSG_PAGER;
            uinfo.pager |= FRIENDMSG_PAGER;
        }
        update_utmp();
        prints("新的参数设定完成...\n\n");
    }
    iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
    pressreturn();
    clear();
    use_define = 0;
    return 0;
}

int x_cloak(void)
{
    modify_user_mode(GMENU);
    report("toggle cloak");
    uinfo.invisible = (uinfo.invisible) ? NA : YEA;
    update_utmp();
    if (!uinfo.in_chat)
    {
        move(1, 0);
        clrtoeol();
        prints("隐身术 (cloak) 已经%s了!",
               (uinfo.invisible) ? "启动" : "停止");
        pressreturn();
    }
    return 0;
}

void x_edits(void)
{
    int aborted;
    char ans[7], buf[STRLEN];
    int ch, num, confirm;
    extern int WishNum;
    char filepath[128];
    static char *e_file[] =
        { "plans", "signatures", "notes", "logout", "GoodWish",
        ".blockmail",
        "autoreply", NULL
    };
    static char *explain_file[] =
        { "个人说明档", "签名档", "自己的备忘录", "离站的画面",
        "底部流动信息",
        "暂停接受站外邮件", "自动回信内容", NULL
    };

    modify_user_mode(GMENU);
    clear();
    move(1, 0);
    prints("编修个人档案\n\n");
    for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++)
    {
        prints("[[1;32m%d[m] %s\n", num + 1, explain_file[num]);
    }
    prints("[[1;32m%d[m] 都不想改\n", num + 1);

    getdata(num + 5, 0, "你要编修哪一项个人档案: ", ans, 2, DOECHO, YEA);
    if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n'
        || ans[0] == '\0')
        return;

    ch = ans[0] - '0' - 1;
    if (ch != 5)
        setuserfile(genbuf, e_file[ch]);
    else
        setmfile(genbuf, currentuser.userid, e_file[5]);
    move(3, 0);
    clrtobot();
    sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
    getdata(3, 0, buf, ans, 2, DOECHO, YEA);
    if (ans[0] == 'D' || ans[0] == 'd')
    {
        confirm = askyn("你确定要删除这个档案", NA, NA);
        if (confirm != 1)
        {
            move(5, 0);
            prints("取消删除行动\n");
            pressreturn();
            clear();
            return;
        }
        unlink(genbuf);
        move(5, 0);
        prints("%s 已删除\n", explain_file[ch]);
        sprintf(buf, "delete %s", explain_file[ch]);
        report(buf);
        pressreturn();
        if (ch == 4)
            WishNum = 9999;
        clear();
        return;
    }
    modify_user_mode(EDITUFILE);
    aborted = vedit(genbuf, NA, YEA);
    clear();
    if (!aborted)
    {
        prints("%s 更新过\n", explain_file[ch]);
        sprintf(buf, "edit %s", explain_file[ch]);
        if (!strcmp(e_file[ch], "signatures"))
        {
            set_numofsig();
            prints("系统重新设定以及读入你的签名档...");
        }
        report(buf);
    }
    else
        prints("%s 取消修改\n", explain_file[ch]);

    if (!strcmp(e_file[ch], "plans"))
    {
        struct stat st;

        sethomefile(filepath, currentuser.userid, "plans");
        stat(filepath, &st);
        if (st.st_size > MAX_PLAN_SIZE)
        {
            char filebuffer[MAX_PLAN_SIZE + 1];
            FILE *fp = NULL;

            fp = fopen(filepath, "r");
            if (fp != NULL)
            {
                fread(filebuffer, MAX_PLAN_SIZE, 1, fp);
                fclose(fp);
                fp = fopen(filepath, "w");
                fwrite(filebuffer, MAX_PLAN_SIZE, 1, fp);
                fclose(fp);
            }
        }
    }

    pressreturn();
    if (ch == 4)
        WishNum = 9999;
}

int gettheuserid(int x, char *title, int *id)
{
    move(x, 0);
    usercomplete(title, genbuf);
    if (*genbuf == '\0')
    {
        clear();
        return 0;
    }
    if (!(*id = getuser(genbuf)))
    {
        move(x + 3, 0);
        prints("错误的使用者代号");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    return 1;
}
int getthedirname(int x, char *title, char *dname)
{
    move(x, 0);
    make_blist();
    namecomplete(title, dname);
    if (*dname == '\0')
    {
        return 0;
    }
    return 1;
}
int
gettheboardname(int x, char *title, int *pos, struct boardheader *fh,
                char *bname)
{

    move(x, 0);
    make_blist();

    namecomplete(title, bname);
    if (*bname == '\0')
    {
        return 0;
    }
    *pos = search_record(BOARDS, fh, sizeof(struct boardheader), cmpbnames,
                         bname);
    if (!(*pos))
    {
        move(x + 3, 0);
        prints("不正确的讨论区名称");
        pressreturn();
        clear();
        return 0;
    }
    return 1;
}

void x_lockscreen(void)
{
    char buf[PASSLEN + 1];
    time_t now;

    modify_user_mode(LOCKSCREEN);
    move(9, 0);
    clrtobot();
    update_endline();
    buf[0] = '\0';
    now = time(0);
    getdatestring(now, NA);
    move(9, 0);
    prints("[1;37m");
    prints
        ("\n       _       _____   ___     _   _   ___     ___       __");
    prints
        ("\n      ( )     (  _  ) (  _`\\  ( ) ( ) (  _`\\  (  _`\\    |  |");
    prints
        ("\n      | |     | ( ) | | ( (_) | |/'/' | (_(_) | | ) |   |  |");
    prints
        ("\n      | |  _  | | | | | |  _  | , <   |  _)_  | | | )   |  |");
    prints
        ("\n      | |_( ) | (_) | | (_( ) | |\\`\\  | (_( ) | |_) |   |==|");
    prints
        ("\n      (____/' (_____) (____/' (_) (_) (____/' (____/'   |__|\n");

    prints
        ("\n[1;36m萤幕已在[33m %s[36m 时被[32m %-12s [36m暂时锁住了...[m",
         datestring, currentuser.userid);
    while (*buf == '\0' || !checkpasswd(currentuser.passwd, buf))
    {
        move(18, 0);
        clrtobot();
        update_endline();
        getdata(19, 0, "请输入你的密码以解锁: ", buf, PASSLEN, NOECHO,
                YEA);
    }
}

int heavyload(void)
{
#ifndef BBSD
    double cpu_load[3];

    get_load(cpu_load);
    if (cpu_load[0] > 15)
        return 1;
    else
        return 0;
#else
#ifdef chkload

    register int load;
    register time_t uptime;

    if (time(0) > uptime)
    {
        load = chkload(load ? TH_LOW : TH_HIGH);
        uptime = time(0) + load + 45;
    }
    return load;
#else

    return 0;
#endif
#endif
}

void exec_cmd(int umode, int pager, char *cmdfile, char *param1)
{
    char buf[160];
    char *my_argv[18], *ptr;
    int save_pager, i;

    signal(SIGALRM, SIG_IGN);
    modify_user_mode(umode);
    clear();
    move(2, 0);
    if ((!HAS_PERM(PERM_EXT_IDLE)) && (num_useshell() > MAX_USESHELL))
    {
        prints("太多人使用外部程式了，你等一下再用吧...");
        pressanykey();
        return;
    }
    if (!HAS_PERM(PERM_SYSOP) && heavyload())
    {
        clear();
        prints("抱歉，目前系统负荷过重，此功能暂时不能执行...");
        pressanykey();
        return;
    }
    if (!dashf(cmdfile))
    {
        prints("文件 [%s] 不存在！\n", cmdfile);
        pressreturn();
        return;
    }
    save_pager = uinfo.pager;
    if (pager == NA)
    {
        uinfo.pager = 0;
    }
    sprintf(buf, "%s %d", cmdfile, getpid());
    report(buf);
    my_argv[0] = cmdfile;
    strcpy(buf, param1);
    if (buf[0] != '\0')
        ptr = strtok(buf, " \t");
    else
        ptr = NULL;
    for (i = 1; i < 18; i++)
    {
        if (ptr)
        {
            my_argv[i] = ptr;
            ptr = strtok(NULL, " \t");
        }
        else
            my_argv[i] = NULL;
    }
#ifdef MY_DEBUG
    for (i = 0; i < 18; i++)
    {
        if (my_argv[i] != NULL)
            prints("my_argv[%d] = %s\n", i, my_argv[i]);
        else
            prints("my_argv[%d] = (none)\n", i);
    }
    pressanykey();
#else

    child_pid = fork();
    if (child_pid == -1)
    {
        prints("资源紧缺，fork() 失败，请稍后再使用");
        child_pid = 0;
        pressreturn();
        return;
    }
    if (child_pid == 0)
    {
        execv(cmdfile, my_argv);
        exit(0);
    }
    else
        waitpid(child_pid, NULL, 0);
#endif

    child_pid = 0;
    uinfo.pager = save_pager;
    clear();
}

void x_showuser(void)
{
    char buf[STRLEN];

    modify_user_mode(SYSINFO);
    clear();
    stand_title("本站使用者资料查询");
    ansimore("etc/showuser.msg", NA);
    getdata(20, 0, "Parameter: ", buf, 30, DOECHO, YEA);
    if ((buf[0] == '\0') || dashf("tmp/showuser.result"))
        return;
    securityreport("查询使用者资料");
    exec_cmd(SYSINFO, YEA, "bin/showuser", buf);
    sprintf(buf, "tmp/showuser.result");
    if (dashf(buf))
    {
        mail_file(buf, currentuser.userid, "使用者资料查询结果");
        unlink(buf);
    }
}

int ent_sortdir(void)
{
    char buf[STRLEN];

    clear();
    stand_title("板面文章排序");
    get_a_boardname(buf, "输入讨论区名 (按空白键自动搜寻): ");
    exec_cmd(READBRD, YEA, "bin/sortdir", buf);
    return 0;
}

int ent_bnet(void)
{
    char buf[80];

    sprintf(buf, "etc/bbsnet.ini %s", currentuser.userid);
    exec_cmd(BBSNET, NA, "bin/bbsnet", buf);
    return 0;
}

int ent_bnet2(void)
{
    char buf[80];

    sprintf(buf, "etc/bbsnet2.ini %s", currentuser.userid);
    exec_cmd(BBSNET, NA, "bin/bbsnet", buf);
    return 0;
}
int ent_bnet3(void)
{
    char buf[80];

    sprintf(buf, "etc/bbsnet3.ini %s", currentuser.userid);
    exec_cmd(BBSNET, NA, "bin/bbsnet", buf);
    return 0;
}
void fill_date(void)
{
    time_t now, next;
    char *buf, *buf2, *index, index_buf[5], *t = NULL;
    char h[3], m[3], s[3];
    int foo, foo2, i;
    struct tm *mytm;
    FILE *fp;

    now = time(0);
    resolve_boards();

    if (now < brdshm->fresh_date && strlen(brdshm->date) != 0)
        return;

    mytm = localtime(&now);
    strftime(h, 3, "%H", mytm);
    strftime(m, 3, "%M", mytm);
    strftime(s, 3, "%S", mytm);

    next =
        (time_t) time(0) - ((atoi(h) * 3600) + (atoi(m) * 60) + atoi(s)) +
        86400;

    sprintf(genbuf, "纪念日更新, 下一次更新时间 %s", Cdate(&next));
    report(genbuf);

    buf = (char *) malloc(80);
    buf2 = (char *) malloc(30);
    index = (char *) malloc(5);

    fp = fopen(DEF_FILE, "r");

    if (fp == NULL)
    {
        free(buf);
        free(buf2);
        free(index);
        return;
    }
    now = time(0);
    mytm = localtime(&now);
    strftime(index_buf, 5, "%m%d", mytm);

    strcpy(brdshm->date, DEF_VALUE);

    while (fgets(buf, 80, fp) != NULL)
    {
        if (buf[0] == ';' || buf[0] == '#' || buf[0] == ' ')
            continue;

        buf[35] = '\0';
        strncpy(index, buf, 4);
        index[4] = '\0';
        strcpy(buf2, buf + 5);
        t = strchr(buf2, '\n');
        if (t)
            *t = '\0';

        if (index[0] == '\0' || buf2[0] == '\0')
            continue;

        if (strcmp(index, "0000") == 0 || strcmp(index_buf, index) == 0)
        {
            foo = strlen(buf2);
            foo2 = (30 - foo) / 2;
            strcpy(brdshm->date, "");
            for (i = 0; i < foo2; i++)
                strcat(brdshm->date, " ");
            strcat(brdshm->date, buf2);
            for (i = 0; i < 30 - (foo + foo2); i++)
                strcat(brdshm->date, " ");
        }
    }

    fclose(fp);
    brdshm->fresh_date = next;

    free(buf);
    free(buf2);
    free(index);

    return;
}

int is_birth(struct userec user)
{
    struct tm *tm;
    time_t now;

    now = time(0);
    tm = localtime(&now);

    if (strcasecmp(user.userid, "guest") == 0)
        return NA;

    if (user.birthmonth == (tm->tm_mon + 1)
        && user.birthday == tm->tm_mday)
        return YEA;
    else
        return NA;
}

int sendgoodwish(char *uid)
{
    return sendGoodWish(NULL);
}

int sendGoodWish(char *userid)
{
    FILE *fp;
    int tuid, i, count, rejects = NA;
    time_t now;
    char buf[5][STRLEN], tmpbuf[STRLEN];
    char uid[IDLEN + 1], *timestr;

    modify_user_mode(GOODWISH);
    clear();
    move(1, 0);
    prints("[0;1;32m留言本[m\n您可以在这里给您的朋友送去您的祝福，");
    prints("\n也可以为您给他/她捎上一句悄悄话。");
    move(5, 0);
    if (userid == NULL)
    {
        usercomplete("请输入他的 ID: ", uid);
        if (uid[0] == '\0')
        {
            clear();
            return 0;
        }
    }
    else
        strcpy(uid, userid);
    if (!(tuid = getuser(uid)))
    {
        move(7, 0);
        prints("[1m您输入的使用者代号( ID )不存在！[m\n");
        pressanykey();
        clear();
        return -1;
    }
    if (!strcmp(uid, "guest"))
        rejects = YEA;
    if (!rejects)
    {
        struct override fh;

        sethomefile(tmpbuf, uid, "rejects");
        rejects = search_record(tmpbuf, &fh, sizeof(fh),
                                cmpfnames, currentuser.userid);
    }
    if (rejects)
    {
        move(7, 0);
        prints("对不起， %s 不愿意接受您的祝福/留言\n", uid);
        pressanykey();
        clear();
        return -1;
    }
    move(5, 0);
    prints("[m【给 [1m%s[m 留言】\n", uid);
    move(6, 0);
    prints
        ("您的留言[直接按 ENTER 结束留言，最多 5 句，每句最长 50 字符]:");
    for (count = 0; count < 5; count++)
    {
        getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
        if (tmpbuf[0] == '\0')
            break;
        ;
        if (killwordsp(tmpbuf) == 0)
        {
            count--;
            continue;
        }
        strcpy(buf[count], tmpbuf);
    }
    if (count == 0)
        return 0;
    sprintf(genbuf, "你确定要发送这条留言给 [1m%s[m 吗", uid);
    move(9 + count, 0);
    if (askyn(genbuf, YEA, NA) == NA)
    {
        clear();
        return 0;
    }
    sethomefile(genbuf, uid, "GoodWish");
    if ((fp = fopen(genbuf, "a")) == NULL)
    {
        prints("无法开启该用户的底部流动信息文件，请通知站长...\n");
        pressanykey();
        return NA;
    }
    now = time(0);
    timestr = ctime(&now) + 11;
    *(timestr + 5) = '\0';
    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s(%s)[%d/%d]：%s\n",
                currentuser.userid, timestr, i + 1, count, buf[i]);
    }
    fclose(fp);
    sethomefile(genbuf, uid, "HaveNewWish");
    if ((fp = fopen(genbuf, "w+")) != NULL)
    {
        fputs("Have New Wish", fp);
        fclose(fp);
    }
    move(11 + count, 0);
    prints("已经帮您送出您的留言了。");
    sprintf(genbuf, "SendGoodWish to %s", uid);
    report(genbuf);
    pressanykey();
    clear();
    return 0;
}

int getdatestring(time_t now, int chang)
{
    struct tm *tm;
    char weeknum[7][3] = { "天", "一", "二", "三", "四", "五", "六" };

    tm = localtime(&now);
    sprintf(datestring, "%4d年%02d月%02d日%02d:%02d:%02d 星期%2s",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, weeknum[tm->tm_wday]);
    return (tm->tm_sec % 10);
}

void anywhere_lockscreen(void)
{
    char buf[PASSLEN + 1];
    time_t now;

    modify_user_mode(LOCKSCREEN);
    buf[0] = '\0';
    now = time(0);
    getdatestring(now, NA);
    if (!strcmp("guest", currentuser.userid))
        return;
    while (*buf == '\0' || !checkpasswd(currentuser.passwd, buf))
    {
        move(t_lines - 1, 0);
        clrtobot();
        update_endline();
        move(t_lines - 1, 87);
        prints
            ("[33m[[36m锁屏中[33m][32m 请输入密码[33m:[m               ");
        getdata(t_lines - 1, 66, "", buf, PASSLEN, NOECHO, YEA);
    }
}
