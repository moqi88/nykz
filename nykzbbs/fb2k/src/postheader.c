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

extern int numofsig;
extern int noreply;
extern int mailtoauther;
extern char module_file[NAMELEN];
struct shortfile *getbcache(char *bname);

void check_title(char *title)
{
    if (killwordsp(title) == 0)
        return;
    if (strncmp(title, "Re: ", 4) != 0)
        return;
    title[0] = 'r';
}

int post_header(struct postheader *header, int is_module)
{
    int anonyboard = 0;
    int positiveanonyboard = 0;
    char r_prompt[20], mybuf[256], ans[5];
    char titlebuf[STRLEN];
    struct shortfile *bp;

#ifdef RNDSIGN

    int oldset = 0, rnd_sign = 0;
#endif
    if (is_module)
    {
        currentuser.signature = 0;
        return YEA;
    }
    if (currentuser.signature > numofsig || currentuser.signature < 0)
        currentuser.signature = 1;

#ifdef RNDSIGN

    if (numofsig > 0)
    {
        if (DEFINE(DEF_RANDSIGN))
        {
            oldset = currentuser.signature;
            currentuser.signature = (rand() % numofsig) + 1;
            rnd_sign = 1;
        }
        else
        {
            rnd_sign = 0;
        }
    }
#endif

    if (header->reply_mode)
    {
        strcpy(titlebuf, header->title);
        header->include_mode = 'S';
    }
    else
        titlebuf[0] = '\0';
    bp = getbcache(currboard);
    if (header->postboard)
    {
        positiveanonyboard = bp->flag & POSITIVE_ANONY_FLAG;
        anonyboard = bp->flag & ANONY_FLAG || positiveanonyboard;
    }
    header->chk_anony = ((anonyboard) ? 1 : 0)
        && ((positiveanonyboard) ? 0 : 1);
    while (1)
    {
        if (header->reply_mode)
            sprintf(r_prompt, "引言模式 [[1m%c[m]",
                    header->include_mode);
        move(t_lines - 4, 0);
        clrtobot();
        prints("[m%s [1m%s[m      %s    %s%s\n",
               (header->postboard) ? "发表文章于" : "收信人：", header->ds,
               (anonyboard) ? (header->chk_anony ==
                               1 ? "[1m要[m使用匿名" :
                               "\033[1m不[m使用匿名") : "",
               (header->
                postboard) ? ((noreply) ? "[本文[1;33m不可以[m回复" :
                              "[本文[1;33m可以[m回复") : "",
               (header->postboard
                && header->
                reply_mode) ? ((mailtoauther) ?
                               ",且[1;33m发送[m本文至作者信箱]" :
                               ",且[1;33m不发送[m本文至作者信箱]")
               : (header->postboard) ? "]" : "");
        prints("使用标题: [1m%-50s[m\n",
               (header->title[0] ==
                '\0') ? "[正在设定主题]" : header->title);
#ifdef RNDSIGN

        prints("使用第 [1m%d[m 个签名档     %s %s",
               currentuser.signature, (header->reply_mode) ? r_prompt : "",
               (rnd_sign == 1) ? "[随机签名档]" : "");
#else

        prints("使用第 [1m%d[m 个签名档     %s", currentuser.signature,
               (header->reply_mode) ? r_prompt : "");
#endif

        if (titlebuf[0] == '\0')
        {
            move(t_lines - 1, 0);
            if (header->postboard == YEA
                || strcmp(header->title, "没主题"))
                strcpy(titlebuf, header->title);
            getdata(t_lines - 1, 0, "标题: ", titlebuf, 50, DOECHO, NA);
            check_title(titlebuf);
            if (titlebuf[0] == '\0')
            {
                if (header->title[0] != '\0')
                {
                    titlebuf[0] = ' ';
                    continue;
                }
                else
                    return NA;
            }
            strcpy(header->title, titlebuf);
            continue;
        }
        move(t_lines - 1, 0);
        sprintf(mybuf,
                "[1;32m0[m~[1;32m%d V[m看签名档%s [1;32mX[m随机签名档,[1;32mT[m标题%s%s%s%s,[1;32mQ[m放弃:",
                numofsig,
                (header->
                 reply_mode) ?
                ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m 引言模式"
                : "", (anonyboard) ? "，[1;32mL[m匿名" : "",
                (header->postboard) ? ",[1;32mU[m属性" : "",
                ((bp->flag2[0] & MODULE_FLAG)
                 && ismoduleexist())? ",\033[1;32mF\033[m使用模板" : "",
                (header->postboard
                 && header->reply_mode) ? ",[1;32mM[m寄信" : "");
        getdata(t_lines - 1, 0, mybuf, ans, 3, DOECHO, YEA);
        ans[0] = toupper(ans[0]);
        if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9)
        {
            if (atoi(ans) <= numofsig)
                currentuser.signature = atoi(ans);
        }
        else if (ans[0] == 'Q' || ans[0] == 'q')
        {
            return -1;
        }
        else if (header->reply_mode &&
                 (ans[0] == 'Y' || ans[0] == 'N' || ans[0] == 'A'
                  || ans[0] == 'R' || ans[0] == 'S'))
        {
            header->include_mode = ans[0];
        }
        else if (ans[0] == 'T')
        {
            titlebuf[0] = '\0';
        }
        else if (ans[0] == 'L' && anonyboard)
        {
            header->chk_anony = (header->chk_anony == 1) ? 0 : 1;
        }
        else if (ans[0] == 'U' && header->postboard)
        {
            noreply = ~noreply;
        }
        else if (ans[0] == 'M' && header->postboard && header->reply_mode)
        {
            mailtoauther = ~mailtoauther;
        }
        else if ((ans[0] == 'F' || ans[0] == 'f')
                 && (bp->flag2[0] & MODULE_FLAG))
        {
            int num;
            char ans1[3];
            struct module_index modulename[3];

            num = showmodulename(modulename);
            getdata(0, 0, "下面是所有的模板,请输入你要选用的序号: ", ans1,
                    2, DOECHO, YEA);
            if ((ans1[0] == 'a' || ans1[0] == 'A') && num > 0)
            {
                strcpy(module_file, modulename[0].filename);
            }
            else if ((ans1[0] == 'b' || ans1[0] == 'B') && num > 1)
            {
                strcpy(module_file, modulename[1].filename);
            }
            else if ((ans1[0] == 'c' || ans1[0] == 'C') && num == 3)
            {
                strcpy(module_file, modulename[2].filename);
            }
        }
        else if (ans[0] == 'V')
        {
            setuserfile(mybuf, "signatures");
            if (askyn("预设显示前三个签名档, 要显示全部吗", NA, YEA) ==
                YEA)
                ansimore(mybuf, NA);
            else
            {
                clear();
                ansimore_partial(mybuf, NA, 0, 18);
            }
#ifdef RNDSIGN

        }
        else if (ans[0] == 'X')
        {
            if (rnd_sign == 0 && numofsig != 0)
            {
                oldset = currentuser.signature;
                currentuser.signature = (rand() % numofsig) + 1;
                rnd_sign = 1;
            }
            else if (rnd_sign == 1 && numofsig != 0)
            {
                rnd_sign = 0;
                currentuser.signature = oldset;
            }
            ans[0] = ' ';
#endif

        }
        else
        {
            if (header->title[0] == '\0')
                return NA;
            else
                return YEA;
        }
    }
}

int check_anonyboard(char *currboard)
{
    struct shortfile *bp;
    int anonyboard = 0;

    bp = getbcache(currboard);
    anonyboard = bp->flag & ANONY_FLAG;
    if (anonyboard == 8)
    {
        return YEA;

    }
    else
    {
        return NA;
    }
}

int readmodulename(struct module_index *module)
{
    FILE *fp;
    int number = 0;
    char buf[STRLEN];
    int i = 0;

    setmodulefile(buf, currboard, "modulename");
    fp = fopen(buf, "r");
    while (fread(&module[number], sizeof(struct module_index), 1, fp) != 0)
    {
        i = strlen(module[number].filename);
        module[number].filename[i - 2] = '\0';
        number++;
    }
    fclose(fp);
    return number;
}
int showmodulename(struct module_index *module)
{
    char buf[NAMELEN + 6];

    int num;
    int i = 0;
    char a[] = "a";

    while (i < 6)
    {
        move(i, 0);
        clrtoeol();
        i++;
    }
    i = 0;
    num = readmodulename(module);
    while (i < num)
    {
        move(3 + i, 0);
        sprintf(buf, "(%s) %s", a, module[i].filename);
        prints(buf);
        i++;
        a[0] = a[0] + 1;
    }
    return num;
}
int ismoduleexist(void)
{
    struct stat test;
    char buf[STRLEN];

    setmodulefile(buf, currboard, "modulename");
    lstat(buf, &test);
    if ((test.st_size / sizeof(struct module_index)) < 1)
        return NA;
    else
        return YEA;
}
