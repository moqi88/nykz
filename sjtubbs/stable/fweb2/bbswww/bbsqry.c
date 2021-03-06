#include "www.h"

static int IsGoodBm(char *id)
{
    return file_has_word("etc/goodbm", id);
}

static void show_duty(struct userec *lookupuser)
{

    if ((lookupuser->userlevel & PERM_SYSOP))
        printf("职务:[<font class=c32>本站站长<font class=c37>]\n");

    else if (lookupuser->userlevel & (PERM_SPECIAL8 | PERM_OBOARDS))
    {
        printf("职务:[<font class=c32>站务助理<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACCOUNTS)
            printf(" [<font class=c32>账号管理员<font class=c37>]");
        if (lookupuser->userlevel & PERM_OVOTE)
            printf(" [<font class=c32>投票管理员<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACHATROOM)
            printf(" [<font class=c32>南洋总管<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACBOARD)
            printf(" [<font class=c32>美工总管<font class=c37>]");
        if (lookupuser->userlevel & PERM_CHATCLOAK)
            printf(" [<font class=c32>本站智囊<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>技术智囊<font class=c37>]");
        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }

    else if ((lookupuser->userlevel & PERM_ACCOUNTS))
    {
        printf("职务:[<font class=c32>账号管理员<font class=c37>]");
        if (lookupuser->userlevel & PERM_OVOTE)
            printf(" [<font class=c32>投票管理员<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACHATROOM)
            printf(" [<font class=c32>南洋总管<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACBOARD)
            printf(" [<font class=c32>美工总管<font class=c37>]");
        if (lookupuser->userlevel & PERM_CHATCLOAK)
            printf(" [<font class=c32>本站智囊<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>技术智囊<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))

            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_OVOTE)
    {
        printf("职务:[<font class=c32>投票管理员<font class=c37>]");
        if (lookupuser->userlevel & PERM_CHATCLOAK)
            printf(" [<font class=c32>本站智囊<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>技术智囊<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_ACHATROOM)
    {
        printf("职务:[<font class=c32>南洋总管<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>技术智囊<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_ACBOARD)
    {
        printf("职务:[<font class=c32>美工总管<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>技术智囊<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_CHATCLOAK)
    {
        printf("职务:[<font class=c32>本站智囊<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_SPECIAL6)
    {
        printf("职务:[<font class=c32>技术智囊<font class=c37>]");
        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }

    else if ((lookupuser->userlevel & PERM_SPECIAL7)
             && !(lookupuser->userlevel & PERM_OBOARDS))
    {
        printf("职务:");
        show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>优秀板主<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_XEMPT)
    {
        int tmp = 0;

        if (IsGoodBm(lookupuser->userid))
        {
            printf("职务:[<font class=c32>优秀板主<font class=c37>]");
            tmp = 1;
        }

        if (lookupuser->userlevel & PERM_BOARDS)
        {
            if (tmp == 0)
            {
                printf("职务:");
            }
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
            tmp = 1;
        }
        if (tmp)
        {
            printf("\n");
        }
    }

    else if (lookupuser->userlevel & PERM_BOARDS)
    {
        printf("职务:");
        show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    return;
}
static void show_special(char *id2)
{
    FILE *fp;
    char id1[80], name[80];

    fp = fopen("etc/sysops", "r");
    if (fp == 0)
        return;
    while (1)
    {
        id1[0] = 0;
        name[0] = 0;
        if (fscanf(FCGI_ToFILE(fp), "%s %s", id1, name) <= 0)
            break;
        if (!strcmp(id1, id2))
            hprintf(" [1;31m★[0;36m%s[1;31m★[m", name);
    }
    fclose(fp);
}

int bbsqry_main()
{
    FILE *fp;
    char userid[14], filename[80], buf[512];
    struct userec *x;
    struct user_info *u;
    int i, tmp1, tmp2, num, clr;

    cache_age = 30 * 60;
    init_all();
    strsncpy(userid, getparm("userid"), 13);
    printf("<center>");
    printf("<hr>\n");
    if (userid[0] == 0)
    {
        printf("<form action=bbsqry>\n");
        printf
            ("请输入要查询的用户名: <input name=userid maxlength=12 size=12> ");
        printf("<input type=submit value=确定查询>\n");
        printf("</form><hr>\n");
        http_quit();
    }
    x = getuser(userid);
    if (x == 0)
    {
        printf("用户 [%s] 不存在.", nohtml4(userid));
        http_quit();
    }
    printf("<table class=show_border width=610>\n");
    printf("<tr><td><pre class=tight>\n");
    sprintf(buf,
            "%s ([33m%s[37m) 共上站 [32m%d[m 次  网龄[[32m%ld[m]天  ",
            x->userid, x->username, x->numlogins,
            (time(0) - x->firstlogin) / 86400);
    hprintf("%s", buf);
    show_special(x->userid);
    if (x->userdefine & DEF_COLOREDSEX)
    {
        clr = (x->gender == 'F') ? 35 : 36;
    }
    else
    {
        clr = 32;
    }
    if (x->userdefine & DEF_S_HOROSCOPE)
        hprintf("[[1;%dm%s[m]", clr,
                horoscope(x->birthmonth, x->birthday));
    printf("\n");
    getdatestring(x->lastlogin, NA);
    hprintf("上 次 在: [[1;32m%s[m] 从 [[1;32m%s[m] 到本站一游。\n",
            datestring, x->lasthost);
    count_mails(x->userid, &tmp1, &tmp2);

    num = 0;
    for (i = 0; i < MAXACTIVE; i++)
    {
        u = &(shm_utmp->uinfo[i]);
        if (!strcmp(u->userid, x->userid))
        {
            if (u->active == 0 || u->pid == 0 || (u->invisible
                                                  &&
                                                  !HAS_PERM
                                                  (PERM_SEECLOAK)))
                continue;
            num++;
            if (num == 1)
            {
                loadfriend(u->userid);
                hprintf
                    ("目前在线：[[1;32m讯息器：([36m%s[32m) 呼叫器：([36m%s[32m)[m] ",
                     canmsg(u) ? "打开" : "关闭",
                     canpage(isfriend(x->userid),
                             u->pager) ? "打开" : "关闭");
                hprintf("生命力:[[32m%d[37m] ", count_life_value(x));
                hprintf("文章:[[32m%d[37m] ", x->numposts);
                hprintf("信箱:[[32m%s[37m]\n", tmp2 ? "⊙" : "  ");
                show_duty(x);
                hprintf("目前 %s 状态如下：\n", x->userid);
            }
            if (u->invisible)
                hprintf("[36mC[37m");
            hprintf("[32m%s[m ", ModeType(u->mode));
            if (num % 5 == 0)
                printf("\n");
        }
    }
    if (num == 0)
    {
        sprintf(filename, "home/%c/%c/%s", toupper(x->userid[0]),
                toupper(x->userid[1]), x->userid);
        getdatestring(x->lastlogout >=
                      x->lastlogin ? x->
                      lastlogout : ((time(0) - x->lastlogin) / 120) % 47 +
                      1 + x->lastlogin, NA);
        hprintf("离站时间: [[1;32m%s[m] ", datestring);
        hprintf("生命力:[[32m%d[37m] ", count_life_value(x));
        hprintf("文章:[[32m%d[37m] ", x->numposts);
        hprintf("信箱:[[32m%s[37m]\n", tmp2 ? "⊙" : "  ");
        show_duty(x);
    }

    printf("\n\n");

    sprintf(filename, "home/%c/%c/%s/plans", toupper(x->userid[0]),
            toupper(x->userid[1]), x->userid);
    fp = fopen(filename, "r");
    if (fp)
    {
        while (1)
        {
            if (fgets(buf, 256, fp) == 0)
                break;
            hhprintf("%s", buf);
        }
        fclose(fp);
    }
    else
    {
        hprintf("[36m没有个人说明档[37m\n");
    }
    printf("</pre>");
    printf("</table>\n");
    printf("<hr><a href=bbspstmail?userid=%s&title=没主题>[写信问候]</a> ",
           x->userid);
    printf("<a href=bbssendmsg?destid=%s>[发送讯息]</a> ", x->userid);
    printf("<a href=bbsfadd?userid=%s>[加入好友]</a> ", x->userid);
    printf("<a href=bbsfdel?userid=%s>[删除好友]</a> ", x->userid);
    printf("<a href=\"/wiki/%s\">[水源百科页面]</a>", x->userid);

    printf("</center>\n");
    printf("</html>");
    return 0;
}
