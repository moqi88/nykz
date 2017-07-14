#include "www.h"

static int IsGoodBm(char *id)
{
    return file_has_word("etc/goodbm", id);
}

static void show_duty(struct userec *lookupuser)
{

    if ((lookupuser->userlevel & PERM_SYSOP))
        printf("Ö°Îñ:[<font class=c32>±¾Õ¾Õ¾³¤<font class=c37>]\n");

    else if (lookupuser->userlevel & (PERM_SPECIAL8 | PERM_OBOARDS))
    {
        printf("Ö°Îñ:[<font class=c32>Õ¾ÎñÖúÀí<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACCOUNTS)
            printf(" [<font class=c32>ÕËºÅ¹ÜÀíÔ±<font class=c37>]");
        if (lookupuser->userlevel & PERM_OVOTE)
            printf(" [<font class=c32>Í¶Æ±¹ÜÀíÔ±<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACHATROOM)
            printf(" [<font class=c32>ÄÏÑó×Ü¹Ü<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACBOARD)
            printf(" [<font class=c32>ÃÀ¹¤×Ü¹Ü<font class=c37>]");
        if (lookupuser->userlevel & PERM_CHATCLOAK)
            printf(" [<font class=c32>±¾Õ¾ÖÇÄÒ<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>¼¼ÊõÖÇÄÒ<font class=c37>]");
        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }

    else if ((lookupuser->userlevel & PERM_ACCOUNTS))
    {
        printf("Ö°Îñ:[<font class=c32>ÕËºÅ¹ÜÀíÔ±<font class=c37>]");
        if (lookupuser->userlevel & PERM_OVOTE)
            printf(" [<font class=c32>Í¶Æ±¹ÜÀíÔ±<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACHATROOM)
            printf(" [<font class=c32>ÄÏÑó×Ü¹Ü<font class=c37>]");
        if (lookupuser->userlevel & PERM_ACBOARD)
            printf(" [<font class=c32>ÃÀ¹¤×Ü¹Ü<font class=c37>]");
        if (lookupuser->userlevel & PERM_CHATCLOAK)
            printf(" [<font class=c32>±¾Õ¾ÖÇÄÒ<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>¼¼ÊõÖÇÄÒ<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))

            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_OVOTE)
    {
        printf("Ö°Îñ:[<font class=c32>Í¶Æ±¹ÜÀíÔ±<font class=c37>]");
        if (lookupuser->userlevel & PERM_CHATCLOAK)
            printf(" [<font class=c32>±¾Õ¾ÖÇÄÒ<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>¼¼ÊõÖÇÄÒ<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_ACHATROOM)
    {
        printf("Ö°Îñ:[<font class=c32>ÄÏÑó×Ü¹Ü<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>¼¼ÊõÖÇÄÒ<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_ACBOARD)
    {
        printf("Ö°Îñ:[<font class=c32>ÃÀ¹¤×Ü¹Ü<font class=c37>]");
        if (lookupuser->userlevel & PERM_SPECIAL6)
            printf(" [<font class=c32>¼¼ÊõÖÇÄÒ<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_CHATCLOAK)
    {
        printf("Ö°Îñ:[<font class=c32>±¾Õ¾ÖÇÄÒ<font class=c37>]");
        if ((lookupuser->userlevel & PERM_SPECIAL7)
            && !(lookupuser->userlevel & PERM_OBOARDS))
            show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }
    else if (lookupuser->userlevel & PERM_SPECIAL6)
    {
        printf("Ö°Îñ:[<font class=c32>¼¼ÊõÖÇÄÒ<font class=c37>]");
        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
        }
        if (lookupuser->userlevel & PERM_BOARDS)
            show_bm(toupper(lookupuser->userid[0]), lookupuser->userid);
        printf("\n");
    }

    else if ((lookupuser->userlevel & PERM_SPECIAL7)
             && !(lookupuser->userlevel & PERM_OBOARDS))
    {
        printf("Ö°Îñ:");
        show_dm(lookupuser->userid);

        if (lookupuser->userlevel & PERM_XEMPT)
        {
            if (IsGoodBm(lookupuser->userid))
                printf(" [<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
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
            printf("Ö°Îñ:[<font class=c32>ÓÅĞã°åÖ÷<font class=c37>]");
            tmp = 1;
        }

        if (lookupuser->userlevel & PERM_BOARDS)
        {
            if (tmp == 0)
            {
                printf("Ö°Îñ:");
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
        printf("Ö°Îñ:");
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
            hprintf(" [1;31m¡ï[0;36m%s[1;31m¡ï[m", name);
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
            ("ÇëÊäÈëÒª²éÑ¯µÄÓÃ»§Ãû: <input name=userid maxlength=12 size=12> ");
        printf("<input type=submit value=È·¶¨²éÑ¯>\n");
        printf("</form><hr>\n");
        http_quit();
    }
    x = getuser(userid);
    if (x == 0)
    {
        printf("ÓÃ»§ [%s] ²»´æÔÚ.", nohtml4(userid));
        http_quit();
    }
    printf("<table class=show_border width=610>\n");
    printf("<tr><td><pre class=tight>\n");
    sprintf(buf,
            "%s ([33m%s[37m) ¹²ÉÏÕ¾ [32m%d[m ´Î  ÍøÁä[[32m%ld[m]Ìì  ",
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
    hprintf("ÉÏ ´Î ÔÚ: [[1;32m%s[m] ´Ó [[1;32m%s[m] µ½±¾Õ¾Ò»ÓÎ¡£\n",
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
                    ("Ä¿Ç°ÔÚÏß£º[[1;32mÑ¶Ï¢Æ÷£º([36m%s[32m) ºô½ĞÆ÷£º([36m%s[32m)[m] ",
                     canmsg(u) ? "´ò¿ª" : "¹Ø±Õ",
                     canpage(isfriend(x->userid),
                             u->pager) ? "´ò¿ª" : "¹Ø±Õ");
                hprintf("ÉúÃüÁ¦:[[32m%d[37m] ", count_life_value(x));
                hprintf("ÎÄÕÂ:[[32m%d[37m] ", x->numposts);
                hprintf("ĞÅÏä:[[32m%s[37m]\n", tmp2 ? "¡Ñ" : "  ");
                show_duty(x);
                hprintf("Ä¿Ç° %s ×´Ì¬ÈçÏÂ£º\n", x->userid);
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
        hprintf("ÀëÕ¾Ê±¼ä: [[1;32m%s[m] ", datestring);
        hprintf("ÉúÃüÁ¦:[[32m%d[37m] ", count_life_value(x));
        hprintf("ÎÄÕÂ:[[32m%d[37m] ", x->numposts);
        hprintf("ĞÅÏä:[[32m%s[37m]\n", tmp2 ? "¡Ñ" : "  ");
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
        hprintf("[36mÃ»ÓĞ¸öÈËËµÃ÷µµ[37m\n");
    }
    printf("</pre>");
    printf("</table>\n");
    printf("<hr><a href=bbspstmail?userid=%s&title=Ã»Ö÷Ìâ>[Ğ´ĞÅÎÊºò]</a> ",
           x->userid);
    printf("<a href=bbssendmsg?destid=%s>[·¢ËÍÑ¶Ï¢]</a> ", x->userid);
    printf("<a href=bbsfadd?userid=%s>[¼ÓÈëºÃÓÑ]</a> ", x->userid);
    printf("<a href=bbsfdel?userid=%s>[É¾³ıºÃÓÑ]</a> ", x->userid);
    if (loginok)
        printf("<a href=\"/wiki/%s\">[Ë®Ô´°Ù¿ÆÒ³Ãæ]</a>", x->userid);

    printf("</center>\n");
    printf("</html>");
    return 0;
}
