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
#include "edit.h"
struct textline *firstline = NULL;
struct textline *can_edit_begin = NULL;
struct textline *can_edit_end = NULL;
struct textline *top_of_win = NULL;
struct textline *currline = NULL;

struct shortfile *getbcache(char *bname);

void vedit_key(int ch);
int currpnt = 0;
extern int local_article;
extern char BoardName[];
extern int write_module;

#ifdef MARK_X_FLAG
extern int markXflag;
#endif
char searchtext[80];
int editansi = 0;
int scrollen = 2;
int moveln = 0;
int shifttmp = 0;
int ismsgline;
int tmpline;
int curr_window_line, currln;
int redraw_everything;
int insert_character = 1;
int linelen = WRAPMARGIN;

#define CLEAR_MARK()  mark_on = 0; mark_begin = mark_end = NULL;
struct textline *mark_begin, *mark_end;
int mark_on;

int enabledbchar = 1;
void process_MARK_action(int arg, char *msg);

void msgline(void)
{
    char buf[256], buf2[STRLEN * 2];
    void display_buffer(void);
    extern int talkrequest;
    int tmpshow;
    time_t now;

    if (ismsgline <= 0)
        return;
    now = time(0);
    tmpshow = showansi;
    showansi = 1;
    if (talkrequest)
    {
        talkreply();
        clear();
        showansi = 0;
        display_buffer();
        showansi = 1;
    }
    strcpy(buf, "[1;33;44m");
    if (chkmail())
        strcat(buf, "��[5;32m��[m[1;33;44m��");
    else
        strcat(buf, "��  ��");
    strcat(buf, "���� [[32m��[31mCtrl-Q[32m���[33m] ");
    sprintf(buf2,
            "[[32m%s[33m][[32m%4.4d[33m,[32m%3.3d[33m][[32m%d[33m] [[32m%2s[33m]",
            insert_character ? "����" : "��д", currln + 1, currpnt + 1,
            linelen - 1, enabledbchar ? "˫" : "��");
    strcat(buf, buf2);
    getdatestring(now, NA);
    sprintf(buf2, "[1;33m��[1;32m%.23s[33m��[m", datestring + 6);
    strcat(buf, buf2);
    move(t_lines - 1, 0);
    clrtoeol();
    prints("%s", buf);
    showansi = tmpshow;
}

void msg(int num)
{
    int x, y;
    int tmpansi;

    tmpansi = showansi;
    showansi = 1;
    getyx(&x, &y);
    msgline();
    signal(SIGALRM, msg);
    move(x, y);
    refresh();
    alarm(60);
    showansi = tmpansi;
    return;
}

void indigestion(int i)
{
    char badmsg[STRLEN];

    sprintf(badmsg, "SERIOUS INTERNAL INDIGESTION CLASS %d\n", i);
    report(badmsg);
}

struct textline *back_line(struct textline *pos, int num)
{
    moveln = 0;
    while (num-- > 0)
        if (pos && pos != can_edit_begin)
        {
            pos = pos->prev;
            moveln++;
        }
        else
            break;
    return pos;
}

struct textline *forward_line(struct textline *pos, int num)
{
    moveln = 0;
    while (num-- > 0)
        if (pos->next && pos->next != can_edit_end)
        {
            pos = pos->next;
            moveln++;
        }
        else
            break;
    return pos;
}

void countline(void)
{
    struct textline *pos;

    pos = can_edit_begin;
    moveln = 0;
    while (pos != can_edit_end)
        if (pos->next)
        {
            pos = pos->next;
            moveln++;
        }
        else
            break;
}

int getlineno(void)
{
    int cnt = 0;
    struct textline *p = currline;

    while (p != top_of_win)
    {
        if (p == NULL)
            break;
        cnt++;
        p = p->prev;
    }
    return cnt;
}

char *killsp(char *s)
{
    while (*s == ' ')
        s++;
    return s;
}

struct textline *alloc_line(void)
{
    register struct textline *p;

    p = (struct textline *) malloc(sizeof(*p));
    if (p == NULL)
    {
        indigestion(13);
        abort_bbs();
    }
    p->next = NULL;
    p->prev = NULL;
    p->data[0] = '\0';
    p->len = 0;
    p->attr = 0;
    return p;
}

void goline(int n)
{
    register struct textline *p = can_edit_begin;
    int count;

    if (n < 0)
        n = 1;
    if (n == 0)
        return;
    for (count = 1; count < n; count++)
    {
        if (p && p->next != can_edit_end)
        {
            p = p->next;
            continue;
        }
        else
            break;
    }
    if (count == n)
        count--;
    currln = count - 1;
    curr_window_line = 0;
    top_of_win = p;
    currline = p;
}

#ifdef ALLOWAUTOWRAP
void set(void)
{
    char tmp[8], theinfo[STRLEN];
    int templinelen;

    signal(SIGALRM, SIG_IGN);
    sprintf(theinfo, "�Զ�����: ÿ������ַ���(10 -- %d)[%d]: ",
            WRAPMARGIN - 1, linelen - 1);
    getdata(23, 0, theinfo, tmp, 7, DOECHO, YEA);
    msg(0);
    if (tmp[0] == '\0')
        return;
    templinelen = atoi(tmp) + 1;
    if (templinelen < 11)
        templinelen = 11;
    else if (templinelen > WRAPMARGIN)
        templinelen = WRAPMARGIN;
    linelen = templinelen;
    return;
}
#endif

void go(void)
{
    char tmp[8];
    int line;

    signal(SIGALRM, SIG_IGN);
    getdata(23, 0, "����Ҫ�����ڼ���: ", tmp, 7, DOECHO, YEA);
    msg(0);
    if (tmp[0] == '\0')
        return;
    line = atoi(tmp);
    goline(line);
    return;
}

void searchline(char *text)
{
    int tmpline, addr, count = 0, tt;
    register struct textline *p = currline;

    tmpline = currln;
    for (; p && p != can_edit_end; p = p->next)
    {
        count++;
        if (count == 1)
            tt = currpnt;
        else
            tt = 0;
        if (strstr(p->data + tt, text))
        {
            addr =
                (int) (strstr(p->data + tt, text) - p->data) +
                strlen(text);
            currpnt = addr;
            break;
        }
    }
    if (p && p != can_edit_end)
    {
        currln = currln + count - 1;
        curr_window_line = 0;
        top_of_win = p;
        currline = p;
    }
    else
    {
        goline(currln + 1);
    }
}

void search(void)
{
    char tmp[STRLEN];

    signal(SIGALRM, SIG_IGN);
    getdata(t_lines - 1, 0, "��Ѱ�ִ�: ", tmp, 65, DOECHO, YEA);
    msg(0);
    if (tmp[0] == '\0')
        return;
    else
        strcpy(searchtext, tmp);
    searchline(searchtext);
    return;
}

void append(register struct textline *p, register struct textline *line)
{
    if (p == NULL)
        return;
    p->next = line->next;
    if (line == can_edit_end)
        can_edit_end = p;
    if (line->next)
        line->next->prev = p;
    line->next = p;
    p->prev = line;
}

void delete_line(register struct textline *line)
{
    if (line == NULL)
        return;
    if (line == can_edit_begin && line->next == can_edit_end)
    {
        line->data[0] = '\0';
        line->len = 0;
        line->attr &= ~(M_MARK);
        mark_begin = mark_end = NULL;
        currpnt = 0;
        return;
    }

    if (mark_begin == line)
    {
        if (line->next == can_edit_end)
            mark_begin = NULL;
        else
            mark_begin = line->next;
    }
    if (mark_end == line)
    {
        if (line == can_edit_begin)
            mark_end = NULL;
        else
            mark_end = line->prev;
    }

    if (line == can_edit_begin)
        can_edit_begin = line->next;
    if (line == firstline)
    {
        line->next->prev = NULL;
        firstline = line->next;
    }
    else
    {
        line->next->prev = line->prev;
        line->prev->next = line->next;
    }

    if (line)
        free(line);
}

void split(register struct textline *line, register int pos)
{
    register struct textline *p;
    register int i, patch;

    if (pos > line->len)
        return;
    if (line->data[pos - 1] & 0x80)
    {
        for (patch = 0, i = pos - 1; i >= 0 && line->data[i] & 0x80; i--)
            patch++;
        if (patch % 2)
            pos--;
    }
#ifdef ALLOWAUTOWRAP
    if (pos && strchr("[0123456789;", line->data[pos - 1]))
    {
        for (i = pos - 1; i > 0 && line->data[i] != ''; i--)
            if (!strchr("[0123456789;", line->data[i]))
                break;
        if (line->data[i] == '')
        {
            if (pos - i < 20)
                pos = i;
            else
                pos--;
        }
    }
#endif
    p = alloc_line();
    p->len = line->len - pos;
    line->len = pos;
    strcpy(p->data, (line->data + pos));
    p->attr = line->attr;
    *(line->data + pos) = '\0';
    append(p, line);
    if ((mark_end == NULL && line == mark_begin) || line == mark_end)
        mark_end = p;
    if (line == currline && pos <= currpnt)
    {
        currline = p;
        currpnt -= pos;
        curr_window_line++;
        currln++;
    }
    redraw_everything = YEA;
}

int join(register struct textline *line)
{
    register int ovfl;

    if (!line->next || line->next == can_edit_end)
        return YEA;
    if ((line->next->data[0] == '-') && (line->next->data[1] == '-')
        && (line->next->len == 2))
        return YEA;
#ifdef ALLOWAUTOWRAP

    ovfl = line->len - num_ans_chr(line->data)
        + line->next->len - num_ans_chr(line->next->data) - linelen;
#else

    ovfl = line->len + line->next->len - linelen;
#endif

    if (ovfl < 0)
    {
        strcat(line->data, line->next->data);
        line->len += line->next->len;
        if (line->next->attr & M_MARK)
            line->attr = line->next->attr;
        if (mark_begin == line->next)
            mark_begin = line;
        if (mark_end == line->next)
            mark_end = line;
        delete_line(line->next);
        return YEA;
    }
    else
    {
        register char *s;
        register struct textline *p = line->next;

#ifdef ALLOWAUTOWRAP

        s = p->data + seekthestr(p->data, p->len - ovfl - 1);
#else

        s = p->data + p->len - ovfl - 1;
#endif

        while (s != p->data && *s == ' ')
            s--;
        while (s != p->data && *s != ' ')
            s--;
#ifdef ALLOWAUTOWRAP

        if (s == p->data || linelen - (s - p->data) > 8)
#else

        if (s == p->data)
#endif

            return YEA;
        split(p, (s - p->data) + 1);
#ifdef ALLOWAUTOWRAP

        if (line->len - num_ans_chr(line->data)
            + p->len - num_ans_chr(p->data) >= linelen)
        {
#else
        if (line->len + p->len >= linelen)
        {
#endif
            indigestion(0);
            return YEA;
        }
        join(line);
        p = line->next;
#ifdef ALLOWAUTOWRAP

        if (p->len >= 1 && p->len + 1 - num_ans_chr(p->data) < linelen)
        {
#else
        if (p->len >= 1 && p->len + 1 < linelen)
        {
#endif
            if (p->data[p->len - 1] != ' ')
            {
                strcat(p->data, " ");
                p->len++;
            }
        }
        return NA;
    }
}

void insert_char(register int ch)
{
    register char *s;
    register int i;

#ifdef	ALLOWAUTOWRAP

    register int ansinum;
    register struct textline *p = currline, *savep;
#else

    register struct textline *p = currline;
#endif

    int wordwrap = YEA;

    if (currpnt > p->len)
    {
        indigestion(1);
        return;
    }
#ifdef	ALLOWAUTOWRAP
    savep = p;
    i = currln;
    for (ansinum = num_ans_chr(p->data); p->len - ansinum > linelen;)
    {
        ansinum = seekthestr(p->data, linelen);
        split(p, ansinum);
        p = p->next;
        ansinum = num_ans_chr(p->data);
    }
    p = savep;
    currln = i;
#endif

    if (currpnt < p->len && !insert_character)
    {
        p->data[currpnt++] = ch;
    }
    else if (p->len < WRAPMARGIN)
    {
        for (i = p->len; i >= currpnt; i--)
            p->data[i + 1] = p->data[i];
        p->data[currpnt] = ch;
        p->len++;
        currpnt++;
    }
#ifdef	ALLOWAUTOWRAP
    ansinum = num_ans_chr(p->data);
    if (p->len - ansinum < linelen - 1 && p->len < WRAPMARGIN)
#else

    if (p->len < linelen - 1 && p->len < WRAPMARGIN)
#endif

        return;
#ifdef	ALLOWAUTOWRAP

    ansinum = seekthestr(p->data, p->len - 1);
    s = p->data + ansinum;
#else

    s = p->data + p->len - 1;
#endif

    while (s != p->data && *s == ' ')
        s--;
    if (!(*s & 0x80))
    {
        while (s != p->data && *s != ' ' && !(*s & 0x80))
            s--;
        if (*s != ' ')
            wordwrap = NA;
    }
    else
        wordwrap = NA;
    if (!wordwrap)
    {
#ifdef	ALLOWAUTOWRAP
        ansinum = seekthestr(p->data, p->len - 2);
        s = p->data + ansinum;
#else

        s = p->data + p->len - 2;
#endif

    }
    split(p, (s - p->data) + 1);
    p = p->next;
    if (wordwrap && p->len >= 1)
    {
        i = p->len;
        if (p->data[i - 1] != ' ')
        {
            p->data[i] = ' ';
            p->data[i + 1] = '\0';
            p->len++;
        }
    }
    while (!join(p))
    {
        p = p->next;
        if (p == NULL)
        {
            indigestion(2);
            break;
        }
    }
}

void ve_insert_str(char *str)
{
    while (*str)
        insert_char(*(str++));
}

void delete_char(void)
{
    register int i;
    register int patch = 0, currDEC = 0;

    if (currline->len == 0)
        return;
    if (currpnt >= currline->len)
    {
        indigestion(1);
        return;
    }
    if (enabledbchar && (currline->data[currpnt] & 0x80))
    {
        for (i = currpnt - 1; i >= 0 && currline->data[i] & 0x80; i--)
            patch++;
        if (patch % 2 == 0 && currpnt < currline->len - 1
            && currline->data[currpnt + 1] & 0x80)
            patch = 1;
        else if (patch % 2)
        {
            patch = 1;
            currDEC = 1;
        }
        else
            patch = 0;
    }
    if (currDEC)
        currpnt--;
    for (i = currpnt; i != currline->len; i++)
        currline->data[i] = currline->data[i + 1 + patch];
    currline->len--;
    if (patch)
        currline->len--;
}

void vedit_init(void)
{
    register struct textline *p = alloc_line();

    firstline = p;
    can_edit_begin = p;
    currline = p;
    currpnt = 0;
    process_ESC_action('M', '0');
    top_of_win = p;
    p = alloc_line();
    can_edit_begin->next = p;
    p->prev = can_edit_begin;
    can_edit_end = p;
    curr_window_line = 0;
    currln = 0;
    redraw_everything = NA;
    CLEAR_MARK();
}

void insert_to_fp(FILE * fp)
{
    int ansi = 0;
    struct textline *p;

    for (p = can_edit_begin; p != can_edit_end; p = p->next)
        if (p->data[0])
        {
            fprintf(fp, "%s\n", p->data);
            if (!ansi)
                if (strchr(p->data, '\033'))
                    ansi++;
        }
    if (ansi)
        fprintf(fp, "%s\n", ANSI_RESET);
}

void insert_from_fp(FILE * fp)
{
    int ch, backup_linelen = linelen;

    linelen = WRAPMARGIN;
    while ((ch = getc(fp)) != EOF)
    {
        if (isprint2(ch) || ch == 27)
        {
            if (currpnt < 254)
                insert_char(ch);
            else if (currpnt < 255)
                insert_char('.');
        }
        else if (ch == Ctrl('I'))
        {
            do
            {
                insert_char(' ');
            }
            while (currpnt & 0x3);
        }
        else if (ch == '\n')
            split(currline, currpnt);
    }
    linelen = backup_linelen;
}

int read_file(char *filename)
{
    FILE *fp;
    struct textline *p;
    char tmp[STRLEN];

    if (currline == NULL)
        vedit_init();
    if ((fp = fopen(filename, "r+")) == NULL)
    {
        if ((fp = fopen(filename, "w+")) != NULL)
        {
            fclose(fp);
            return 1;
        }
        indigestion(4);
        abort_bbs();
    }
    insert_from_fp(fp);
    fclose(fp);
    sprintf(tmp, ":��%s %s��[FROM:", BoardName, BBSHOST);
    for (p = can_edit_end; p != can_edit_begin; p = p->prev)
    {
        if (strstr(p->data, tmp))
        {
            can_edit_end = p;
            break;
        }
    }
    if (p != can_edit_end)
        return 1;
    return 0;
}

char save_title[STRLEN];
int in_mail;

void write_posts(void)
{
    char *ptr;
    time_t now;
    struct
    {
        char author[IDLEN + 1];
        char board[18];
        char title[66];
        time_t date;
        int number;
    } postlog;

    if (junkboard() || normal_board(currboard) != 1)
        return;
    now = time(0);
    strncpy(postlog.author, currentuser.userid, IDLEN);
    strncpy(postlog.board, currboard, 17);
    postlog.author[IDLEN] = '\0';
    postlog.board[17] = '\0';
    ptr = save_title;
    if (!strncmp(ptr, "Re: ", 4))
        ptr += 4;
    strncpy(postlog.title, ptr, 40);
    postlog.title[40] = '\0';
    postlog.date = now;
    postlog.number = 1;
    append_record(".post", &postlog, sizeof(postlog));
}

void write_header(FILE * fp, int mode, int iscross)
{
    extern char BoardName[];
    extern char fromhost[];
    extern struct postheader header;
    struct shortfile *bp;
    char uid[20];
    char uname[NAMELEN];

    time_t now;

    now = time(0);
    strncpy(uid, currentuser.userid, 20);
    uid[19] = '\0';
    if (in_mail)
#if defined(MAIL_REALNAMES)

        strncpy(uname, currentuser.realname, NAMELEN);
#else

        strncpy(uname, currentuser.username, NAMELEN);
#endif

    else
#if defined(POSTS_REALNAMES)

        strncpy(uname, currentuser.realname, NAMELEN);
#else

        strncpy(uname, currentuser.username, NAMELEN);
#endif

    uname[NAMELEN - 1] = '\0';
    save_title[STRLEN - 10] = '\0';
    bp = getbcache(currboard);
    //header.chk_anony = bp->flag & ANONY_FLAG;
    
    if (in_mail)
        fprintf(fp, "������: %s (%s)\n", uid, uname);
    else
    {
        if (mode == 0 && !(header.chk_anony))
        {
            write_posts();
        }
        fprintf(fp, "������: %s (%s), ����: %s\n",
                (header.chk_anony && !iscross) ? get_anonyid(uid) : uid,
                (header.chk_anony && !iscross) ? "����������ʹ" : uname, currboard);
    }
    fprintf(fp, "��  ��: %s\n", save_title);
    getdatestring(now, NA);
    fprintf(fp, "����վ: %s (%s)", BoardName, datestring);
    if (in_mail)
        fprintf(fp, "\n��  Դ: %s \n", fromhost);
    else
        fprintf(fp, ", %s\n", (local_article) ? "վ���ż�" : "ת��");
    fprintf(fp, "\n");
}

void addsignature(FILE * fp, int blank)
{
    FILE *sigfile;
    int i, valid_ln = 0;
    char tmpsig[MAXSIGLINES][256];
    char inbuf[256];
    char fname[STRLEN];

    setuserfile(fname, "signatures");
    if ((sigfile = fopen(fname, "r")) == NULL)
    {
        return;
    }
    if (blank)
        fputs("\n", fp);
    fputs("--\n", fp);
    for (i = 1;
         i <= (currentuser.signature - 1) * MAXSIGLINES
         && currentuser.signature != 1; i++)
    {
        if (!fgets(inbuf, sizeof(inbuf), sigfile))
        {
            fclose(sigfile);
            return;
        }
    }
    for (i = 1; i <= MAXSIGLINES; i++)
    {
        if (fgets(inbuf, sizeof(inbuf), sigfile))
        {
            if (inbuf[0] != '\n')
                valid_ln = i;
            strcpy(tmpsig[i - 1], inbuf);
        }
        else
            break;
    }
    fclose(sigfile);
    for (i = 1; i <= valid_ln; i++)
        fputs(tmpsig[i - 1], fp);
}

#define KEEP_EDITING -2
char *format_pushbuf(char *pushbuf, char *bufformatted)
{
    int i, j = 0, num = 0, lasti = 0;
    struct tm *now;
    time_t nowtime;
    char *p;
    char tempbuf[2048], buf[2048];

    for (i = 0; i <= (int) strlen(pushbuf) - 1; i++)
    {
        num++;
        if (*(pushbuf + i) == '\r')
            *(pushbuf + i) = ' ';
        if (*(pushbuf + i) == '\n')
        {
            strncpy(tempbuf + j, pushbuf + lasti, i - lasti + 1);
            j += i - lasti + 1;
            lasti = i + 1;
            num = 0;
        }
        else
        {
            if (i == (int) (strlen(pushbuf) - 1))
            {
                strncpy(tempbuf + j, pushbuf + lasti, i - lasti + 1);
                j += i - lasti + 1;
                break;
            }
            if (num <= 39)
            {
                continue;
            }
            else
            {
                strncpy(tempbuf + j, pushbuf + lasti, 40);
                lasti = i + 1;
                strncpy(tempbuf + j + 40, "\n", 1);
                j += 41;
                num = 0;
            }
        }
    }
    *(tempbuf + j) = '\0';
    j = 0;
    nowtime = time(0);
    now = localtime(&nowtime);
    snprintf(buf, 256,
             "[1;32m�� [m[1;33m%-12.12s:[m[33m%-40s[m [37m%15s[m [37m%2d/%2d[m",
             currentuser.userid, strtok(tempbuf, "\n"),
             currentuser.lasthost, now->tm_mon + 1, now->tm_mday);
    strncpy(bufformatted + j, buf, strlen(buf));
    j += strlen(buf);
    while ((p = strtok(NULL, "\n")))
    {
        snprintf(buf, 256,
                 "\n[32m�� [m[1;33m%-12.12s:[m[33m%-40s[m [37m%15s[m [37m%2d/%2d[m",
                 currentuser.userid, p, currentuser.lasthost,
                 now->tm_mon + 1, now->tm_mday);
        strncpy(bufformatted + j, buf, strlen(buf));
        j += strlen(buf);
    }
    *(bufformatted + j) = '\0';
    return bufformatted;
}

int valid_article(char *pmt, char *abort, int sure)
{
    struct textline *p = can_edit_begin;
    char ch;
    int total, lines, len, y, w;

    w = NA;

    if (uinfo.mode == POSTING || uinfo.mode == EDIT)
    {
        total = lines = len = 0;
        while (p && p != can_edit_end)
        {
            ch = p->data[0];
            if (ch == '\0')
            {
                p = p->next;
                continue;
            }
            if (strncmp(p->data, "--", 2) == 0)
            {
                break;
            }
            if (ch != ':' && ch != '>' && ch != '='
                && !strstr(p->data, "�Ĵ������ᵽ: ��"))
            {
                lines++;
                len += strlen(p->data);

            }
            total++;
            p = p->next;
        }
        y = 2;
        if (lines < total * 0.35)
        {
            move(y, 0);
            prints
                ("ע�⣺��ƪ���µ����Թ���, ������ɾ��һЩ����Ҫ������.\n");
            y += 3;
        }
#ifdef MARK_X_FLAG
        if (len < 150 && lines < 3)
        {
            move(y, 0);
            prints("ע�⣺��ƪ���¹��ڼ��\n");

            markXflag = 1;
            y += 3;
            w = YEA;
        }
        else
            markXflag = 0;
#endif

        if (w && !write_module)
        {
            strcpy(pmt,
                   "E.�ٱ༭, S.ת��, L.��վ����, A.ȡ�� ,P.ͬʱ������Blog or T.���ı���?[L]: ");
        }
    }
    if (sure)
    {
        if (local_article == YEA)
            abort[0] = 'L';
        else
            abort[0] = 'S';

        return 0;
    }
    getdata(0, 0, pmt, abort, 3, DOECHO, YEA);
    if (w && abort[0] == '\0')
        abort[0] = 'L';

    return 0;
#if 0

    switch (abort[0])
    {
    case 'A':
    case 'a':
    case 'E':
    case 'e':
        return;
    }
#endif
}
void filter_html(char *from, char *to)
{
    char *pr, *pw;

    pr = from;
    pw = to;
    for (; *pr != '\0'; ++pr)
    {
        switch (*pr)
        {
        case '&':
            *pw++ = '&';
            *pw++ = 'a';
            *pw++ = 'm';
            *pw++ = 'p';
            *pw++ = ';';
            break;
        case '<':
            *pw++ = '&';
            *pw++ = 'l';
            *pw++ = 't';
            *pw++ = ';';
            break;
        case '>':
            *pw++ = '&';
            *pw++ = 'g';
            *pw++ = 't';
            *pw++ = ';';
            break;
        default:
            *pw++ = *pr;
        }
    }
    *pw = '\0';
}

#ifdef LOGHANDLER
int
log2db(char *user_id, char *ip_address, char *filename, char *board_name,
       unsigned int reid, char *content, int blog_tag)
{
    char *query_string, *content_ok, *content_ok2, title_ok[256],
        table_name[16];

    time_t timestamp;
    int content_len, query_len;
    struct tm *tm;

    timestamp = time(NULL);
    tm = localtime(&timestamp);
    sprintf(table_name, "posts%02d", tm->tm_hour);

    content_len = my_ansi_filter(content);

    content_ok = malloc((content_len + 1) * 17 + 1024);
    if (!content_ok)
        return -1;
    content_ok2 = content_ok + (content_len + 1) * 2;
    query_string = content_ok2 + (content_len + 1) * 5;
    mysql_escape_string(content_ok, content, content_len);
    mysql_escape_string(title_ok, save_title, strlen(save_title));
    query_len =
        snprintf(query_string, (content_len + 1) * 10 + 1024,
                 LOGHANDLER_SQL
                 "insert into %s (user_id,ip_addr,time,filename,board,title,content,reid) values ('%s','%s',FROM_UNIXTIME(%ld),'%s','%s','%s','%s','%u')\n",
                 table_name, user_id, ip_address, timestamp, filename,
                 board_name, title_ok, content_ok, reid);
    loghander_write(query_string, query_len);
    if (blog_tag == 1)
    {
        filter_html(content_ok, content_ok2);
        query_len =
            snprintf(query_string, (content_len + 1) * 10 + 1024,
                     LOGHANDLER_SQL
                     "INSERT INTO blog.yssy_articles( user_id,blog_id,status,date,properties, slug ) VALUES ( %s,%s,'1',FROM_UNIXTIME(%ld),'a:1:{s:16:\"comments_enabled\";s:1:\"1\";}','%s')\n",
                     blog_user_id, blog_id, timestamp, title_ok);
        query_len +=
            snprintf(&query_string[query_len],
                     (content_len + 1) * 10 + 1024 - query_len,
                     LOGHANDLER_SQL
                     "INSERT INTO blog.yssy_article_categories_link (article_id, category_id) VALUES (LAST_INSERT_ID(), %s )\n",
                     blog_catId);
        query_len +=
            snprintf(&query_string[query_len],
                     (content_len + 1) * 10 + 1024 - query_len,
                     LOGHANDLER_SQL
                     "INSERT INTO blog.yssy_articles_text (article_id, topic, text, normalized_text, normalized_topic) VALUES( LAST_INSERT_ID(), '%s','<pre>%s</pre>','%s', '%s')\n",
                     title_ok, content_ok2, content_ok2, title_ok);
        loghander_write(query_string, query_len);
    }
    free(content_ok);
    return 0;
}
#endif
int
write_file(char *filename, int write_header_to_file, int addfrom, int sure,
           unsigned int reid)
{
    struct textline *p;
    FILE *fp = NULL;
    char abort[6], p_buf[100];
    int aborted = 0;

    signal(SIGALRM, SIG_IGN);
    clear();

    if (uinfo.mode == POSTING)
    {
        if (local_article == YEA && write_module == 0)
        {
            if (HAS_PERM(PERM_BOARDS | PERM_EXT_IDLE))
                strcpy(p_buf,
                       "L.��վ����, S.ת��, A.ȡ��, T.���ı��� ,P.ͬʱ������blog or E.�ٱ༭? [L]: ");
            else
                strcpy(p_buf,
                       "L.��վ����, S.ת��, A.ȡ��, T.���ı��� or E.�ٱ༭? [L]: ");
        }
        else if (local_article == YEA && write_module == 1)
            strcpy(p_buf, "(L)���浵��, (A)�����༭, (E)�����༭? [L]: ");
        else
            strcpy(p_buf,
                   "S.ת��, L.��վ����, A.ȡ��, T.���ı��� or E.�ٱ༭? [S]: ");
    }
    else if (uinfo.mode == SMAIL)
        strcpy(p_buf, "(S)�ĳ�, (A)ȡ��, or (E)�ٱ༭? [S]: ");
    else
        strcpy(p_buf, "(S)���浵��, (A)�����༭, (E)�����༭? [S]: ");
    valid_article(p_buf, abort, sure);

    if (abort[0] == 'a' || abort[0] == 'A')
    {
        struct stat stbuf;

        clear();

        prints("ȡ��...\n");
        refresh();

        if (stat(filename, &stbuf) || stbuf.st_size == 0)
            unlink(filename);
        aborted = -1;
    }
    else if (abort[0] == 'e' || abort[0] == 'E')
    {
        msg(0);
        return KEEP_EDITING;
    }
    else if ((abort[0] == 't' || abort[0] == 'T')
             && uinfo.mode == POSTING && write_module == 0)
    {
        char buf[STRLEN];

        move(1, 0);
        prints("�ɱ���: %s", save_title);
        sprintf(buf, "%s", save_title);
        getdata(2, 0, "�±���: ", buf, 50, DOECHO, NA);
        if (strcmp(save_title, buf) && strlen(buf) != 0)
        {
            check_title(buf);
            if(strlen(buf) != 0)
	            strncpy(save_title, buf, STRLEN);
        }
    }

    else if ((abort[0] == 's' || abort[0] == 'S')
             && (uinfo.mode == POSTING) && write_module == 0)
    {
        local_article = NA;
    }
    else if ((abort[0] == 'l' || abort[0] == 'L')
             && (uinfo.mode == POSTING))
    {
        local_article = YEA;
    }

    if (aborted != -1)
    {
        if ((fp = fopen(filename, "w+")) == NULL)
        {
            indigestion(5);
            abort_bbs();
        }
        if (write_header_to_file)
            write_header(fp, 0, 0);
    }
    p = can_edit_end;
    if (p != NULL)
        for (; p->next != NULL; p = p->next);
    while (1)
    {
        struct textline *v = p->prev;

        if (p != can_edit_begin
            && (p->data[0] == '\0' || !strcmp(p->data, "\n")))
        {
            free(p);
            v->next = NULL;
            p = v;
        }
        else
        {
            can_edit_end = p;
            p = firstline;
            break;
        }
    }
    firstline = NULL;
    while (p != NULL)
    {
        struct textline *v = p->next;

        if (aborted != -1)
        {
            if (uinfo.mode != EDIT || p != can_edit_end->prev ||
                (ADD_EDITMARK
                 && strncmp(p->data, "[1;36m�� �޸�����:��", 17)))
            {
                if (p->next != NULL || p->data[0] != '\0')
                {
                    fwrite(p->data, p->len, 1, fp);
                    fputc('\n', fp);

                }
            }

        }
        free(p);
        p = v;
    }

    if ((uinfo.mode == EDIT) && (aborted != -1))
    {

        fprintf(fp,
                "\n[1;36m�� �޸�����:��%s �� %16.16s �޸ı��ġ�[FROM: %-.46s][m\n",
                currentuser.userid, datestring + 6, currentuser.lasthost);

    }

    if ((uinfo.mode == POSTING || uinfo.mode == SMAIL)
        && addfrom != 0 && aborted != -1 && write_header_to_file)
    {
        int color, noidboard;
        char fname[STRLEN];
        extern char fromhost[];
        extern struct postheader header;
        struct shortfile *bp;

        bp = getbcache(currboard);
        noidboard = (header.chk_anony);
        color = (rand() % 7) + 31;
        setuserfile(fname, "signatures");
        if (!dashf(fname) || currentuser.signature == 0 || noidboard)
        {
            fputc('-', fp);
            fputc('-', fp);
            fputc('\n', fp);
        }
        fprintf(fp, "\n[m[1;%2dm�� ��Դ:��%s %s��[FROM: %-.46s][m\n",
                color, BoardName, BBSHOST,
                (noidboard) ? "������ʹ�ļ�" : fromhost);
    }
#ifdef LOGHANDLER
    if (aborted != -1)
    {
        int file_size;
        char *buf;

        fflush(fp);
        file_size = (int) ftell(fp);
        file_size = file_size < 65534 ? file_size : 65534;
        buf = malloc(file_size + 1);
        if (buf)
        {
            fseek(fp, 0, SEEK_SET);
            fread(buf, file_size, 1, fp);
            buf[file_size] = '\0';
            if (uinfo.mode == POSTING)
            {
                if ((abort[0] == 'p' || abort[0] == 'P')
                    && HAS_PERM(PERM_BOARDS))
                {
                    log2db(currentuser.userid, currentuser.lasthost,
                           (strrchr(filename, '/') + 1), currboard, reid,
                           buf, 1);
                }
                else
                {
                    log2db(currentuser.userid, currentuser.lasthost,
                           (strrchr(filename, '/') + 1), currboard, reid,
                           buf, 0);
                }
            }
            free(buf);
        }
        fclose(fp);
    }
#endif
    currline = NULL;
    can_edit_end = NULL;
    firstline = NULL;
    can_edit_begin = NULL;
    can_edit_end = NULL;
    return aborted;
}

void keep_fail_post(void)
{
    char filename[STRLEN];
    struct textline *p = firstline;
    FILE *fp;

    sprintf(filename, "home/%c/%c/%s/%s.deadve",
            toupper(currentuser.userid[0]), toupper(currentuser.userid[1]),
            currentuser.userid, currentuser.userid);
    if ((fp = fopen(filename, "w")) == NULL)
    {
        indigestion(5);
        return;
    }
    while (p != NULL)
    {
        struct textline *v = p->next;

        if (p->next != NULL || p->data[0] != '\0')
            fprintf(fp, "%s\n", p->data);
        free(p);
        p = v;
    }
    return;
}

void strnput(char *str)
{
    int count = 0;

    while ((*str != '\0') && (++count < STRLEN))
    {
        if (*str == KEY_ESC)
        {
            outc('*');
            str++;
            continue;
        }
        outc(*str++);
    }
}

void cstrnput(char *str)
{
    int count = 0, tmp = 0;

    tmp = num_ans_chr(str);
    prints("%s", ANSI_REVERSE);
    while ((*str != '\0') && (++count < STRLEN))
    {
        if (*str == KEY_ESC)
        {
            outc('*');
            tmp--;
            str++;
            continue;
        }
        outc(*str++);
    }
    while (++count < STRLEN + tmp)
        outc(' ');
    clrtoeol();
    prints("%s", ANSI_RESET);
}

void display_buffer(void)
{
    register struct textline *p;
    register int i;
    int shift;
    int temp_showansi;

    temp_showansi = showansi;

    for (p = top_of_win, i = 0; i < t_lines - 1; i++)
    {
        move(i, 0);
        if (p != can_edit_end)
        {
            shift = (currpnt + 2 > STRLEN) ?
                (currpnt / (STRLEN - scrollen)) * (STRLEN - scrollen) : 0;
            if (editansi)
            {
                showansi = 1;
                prints("%s", p->data);
            }
            else if ((p->attr & M_MARK))
            {
                showansi = 1;
                clear_whole_line(i);
                cstrnput(p->data + shift);
            }
            else
            {
                if (p->len >= shift)
                {
                    showansi = 0;
                    strnput(p->data + shift);
                }
                else
                    clrtoeol();
            }
            p = p->next;
        }
        else
            prints("%s~", editansi ? ANSI_RESET : "");
        clrtoeol();
    }
    showansi = temp_showansi;
    msgline();
    return;
}

int vedit_process_ESC(int arg)
{
    int ch2, action;

#define WHICH_ACTION_COLOR    \
"(M)���鴦�� (I/E)��ȡ/д������� (C)ʹ�ò�ɫ (F/B/R)ǰ��/����/��ԭɫ��"
#define WHICH_ACTION_MONO    \
"(M)���鴦�� (I/E)��ȡ/д������� (C)ʹ�õ�ɫ (F/B/R)ǰ��/����/��ԭɫ��"

#define CHOOSE_MARK    "(0)ȡ����� (1)�趨������ (2)���Ʊ������ (3)ɾ������"
#define FROM_WHICH_PAGE "��ȡ�������ڼ�ҳ? (0-9,a-f) [Ԥ��Ϊ 0]"
#define SAVE_ALL_TO     "����ƪ����д��������ڼ�ҳ? (0-9,a-f) [Ԥ��Ϊ 0]"
#define SAVE_PART_TO    "����������д��������ڼ�ҳ? (0-9,a-f) [Ԥ��Ϊ 0]"
#define FROM_WHICH_SIG  "ȡ��ǩ�����ڼ�ҳ? (0-7) [Ԥ��Ϊ 0]"
#define CHOOSE_FG     "ǰ����ɫ? 0)�� 1)�� 2)�� 3)�� 4)���� 5)�ۺ� 6)ǳ�� 7)�� "
#define CHOOSE_BG     "������ɫ? 0)�� 1)�� 2)�� 3)�� 4)���� 5)�ۺ� 6)ǳ�� 7)�� "
#define CHOOSE_ERROR    "ѡ�����"

    switch (arg)
    {
    case 'M':
    case 'm':
        ch2 = ask(CHOOSE_MARK);
        action = 'M';
        break;
    case 'I':
    case 'i':
        ch2 = ask(FROM_WHICH_PAGE);
        action = 'I';
        break;
    case 'E':
    case 'e':
        mark_on = mark_block();
        ch2 = ask(mark_on ? SAVE_PART_TO : SAVE_ALL_TO);
        action = 'E';
        break;
    case 'S':
    case 's':
        ch2 = '0';
        action = 'S';
        break;
    case 'F':
    case 'f':
        ch2 = ask(CHOOSE_FG);
        action = 'F';
        break;
    case 'X':
    case 'x':
        ch2 = '0';
        action = 'X';
        break;
    case 'B':
    case 'b':
        ch2 = ask(CHOOSE_BG);
        action = 'B';
        break;
    case 'R':
    case 'r':
        ch2 = '0';
        action = 'R';
        break;
    case 'D':
    case 'd':
        ch2 = '3';
        action = 'M';
        break;
    case 'N':
    case 'n':
        ch2 = '0';
        action = 'N';
        break;
    case 'G':
    case 'g':
        ch2 = '1';
        action = 'G';
        break;
    case 'L':
    case 'l':
        ch2 = '0';
        action = 'L';
        break;
    case 'C':
    case 'c':
        ch2 = '0';
        action = 'C';
        break;
    case 'Q':
    case 'q':
        ch2 = '0';
        action = 'M';
        break;
    default:
        return arg;
    }
    if (strchr("IES", action) && (ch2 == '\n' || ch2 == '\r'))
        ch2 = '0';
    if ((ch2 >= '0' && ch2 <= '9') || (ch2 >= 'a' && ch2 <= 'a' + 5))
        return process_ESC_action(action, ch2);
    else
    {
        return ask(CHOOSE_ERROR);
    }
}

int mark_block(void)
{
    struct textline *p;
    int pass_mark = 0;

    for (p = can_edit_begin; p && p != can_edit_end; p = p->next)
        p->attr &= ~(M_MARK);

    if (mark_begin == NULL && mark_end == NULL)
        return 0;
    if (mark_begin == mark_end)
    {
        mark_begin->attr |= M_MARK;
        return 2;
    }
    if (mark_begin == NULL)
    {
        mark_end->attr |= M_MARK;
        return 1;
    }
    if (mark_end == NULL)
    {
        mark_begin->attr |= M_MARK;
        return 1;
    }
    for (p = can_edit_begin; p != can_edit_end; p = p->next)
    {
        if (p == mark_begin || p == mark_end)
        {
            pass_mark++;
            p->attr |= M_MARK;
            continue;
        }
        if (pass_mark == 1)
            p->attr |= M_MARK;
        else
            p->attr &= ~(M_MARK);
    }
    return 2;
}

void process_MARK_action(int arg, char *msg)
{
    struct textline *p;

    switch (arg)
    {
    case '0':
        for (p = can_edit_begin; p && p != can_edit_end; p = p->next)
            p->attr &= ~(M_MARK);
        CLEAR_MARK();
        break;
    case '1':
        if (mark_begin)
            mark_end = currline;
        else
            mark_begin = currline;
        mark_on = mark_block();

        break;
    case '2':
        mark_on = mark_block();
        if (mark_on && !(currline->attr & M_MARK))
        {
            for (p = mark_begin; p != can_edit_end; p = p->next)
            {
                if (p->attr & M_MARK)
                {
                    ve_insert_str(p->data);
                    split(currline, currpnt);
                }
                else
                    break;
            }

            break;
        }
        else if (mark_on)
            strcpy(msg, "�����������ſɸ���");
        else
            strcpy(msg, "����δ�� Ctrl+U ��������");
        bell();
        break;
    case '3':
        mark_on = mark_block();
        if (mark_on == 0)
            break;
        for (p = can_edit_begin; p != can_edit_end;)
        {
            if (p->attr & M_MARK)
            {
                delete_line(p);
                p = can_edit_begin;
            }
            else
                p = p->next;
        }
        CLEAR_MARK()goline(1);
        break;
    default:
        strcpy(msg, CHOOSE_ERROR);
    }
}

int process_ESC_action(int action, int arg)
{
    int newch = 0;
    char msg[80], buf[80];
    char filename[80];
    FILE *fp;

    msg[0] = '\0';
    switch (action)
    {
    case 'L':
        if (ismsgline >= 1)
        {
            ismsgline = 0;
            move(t_lines - 1, 0);
            clrtoeol();
            refresh();
        }
        else
            ismsgline = 1;
        break;
    case 'M':
        process_MARK_action(arg, msg);
        break;
    case 'I':
        sprintf(filename, "home/%c/%c/%s/clip_%c",
                toupper(currentuser.userid[0]),
                toupper(currentuser.userid[1]), currentuser.userid, arg);
        if ((fp = fopen(filename, "r")) != NULL)
        {
            insert_from_fp(fp);
            fclose(fp);
            sprintf(msg, "��ȡ���������� %c ҳ", arg);
        }
        else
            sprintf(msg, "�޷�ȡ���������� %c ҳ", arg);
        break;
#ifdef ALLOWAUTOWRAP

    case 'X':
        set();
        break;
#endif

    case 'G':
        go();
        redraw_everything = YEA;
        break;
    case 'E':
        sprintf(filename, "home/%c/%c/%s/clip_%c",
                toupper(currentuser.userid[0]),
                toupper(currentuser.userid[1]), currentuser.userid, arg);
        if ((fp = fopen(filename, "w")) != NULL)
        {
            if (mark_on)
            {
                struct textline *p;

                for (p = mark_begin; p != can_edit_end; p = p->next)
                {
                    if (p->attr & M_MARK)
                        fprintf(fp, "%s\n", p->data);
                    else
                        break;
                }
            }
            else
                insert_to_fp(fp);
            fclose(fp);
            sprintf(msg, "�������������� %c ҳ", arg);
        }
        else
            sprintf(msg, "�޷������������� %c ҳ", arg);
        break;
    case 'N':
        searchline(searchtext);
        redraw_everything = YEA;
        break;
    case 'S':
        search();
        redraw_everything = YEA;
        break;
    case 'F':
        sprintf(buf, "%c[3%cm", 27, arg);
        ve_insert_str(buf);
        break;
    case 'B':
        sprintf(buf, "%c[4%cm", 27, arg);
        ve_insert_str(buf);
        break;
    case 'R':
        ve_insert_str(ANSI_RESET);
        break;
    case 'C':
        editansi = showansi = 1;
        redraw_everything = YEA;
        clear();
        display_buffer();
        redoscr();
        strcpy(msg, "����ʾ��ɫ�༭�ɹ��������лص�ɫģʽ");
        break;
    }
    if (strchr("FBRCM", action))
        redraw_everything = YEA;
    if (msg[0] != '\0')
    {
        if (action == 'C')
        {
            move(t_lines - 2, 0);
            clrtoeol();
            prints("[1m%s%s%s[m", msg, ", �밴��������ر༭����...",
                   ANSI_RESET);
            igetkey();
            newch = '\0';
            editansi = showansi = 0;
            clear();
            display_buffer();
        }
        else
            newch = ask(strcat(msg, "��������༭��"));
    }
    else
        newch = '\0';
    return newch;
}

void vedit_key(int ch)
{
    int i, patch;

#define NO_ANSI_MODIFY  if(no_touch) { warn++; break; }

    static int lastindent = -1;
    int no_touch, warn, shift;

    if (ch == Ctrl('P') || ch == KEY_UP || ch == Ctrl('N')
        || ch == KEY_DOWN)
    {
        if (lastindent == -1)
            lastindent = currpnt;
    }
    else
        lastindent = -1;
    no_touch = (editansi && strchr(currline->data, '\033')) ? 1 : 0;
    warn = 0;
    if (ch < 0x100 && isprint2(ch))
    {
        if (no_touch)
            warn++;
        else
            insert_char(ch);
    }
    else
    {
        switch (ch)
        {
        case Ctrl('I'):
            NO_ANSI_MODIFY;
            if (currpnt >= linelen - 4)
            {
                split(currline, currpnt);
                break;
            }
            do
            {
                insert_char(' ');
            }
            while (currpnt & 0x3);
            break;
        case '\r':
        case '\n':
            NO_ANSI_MODIFY;
            split(currline, currpnt);
            break;
        case Ctrl('R'):
            enabledbchar = !enabledbchar;
            break;
        case Ctrl('G'):
            clear();
            redraw_everything = YEA;
            break;
        case Ctrl('Q'):
            show_help("help/edithelp");
            redraw_everything = YEA;
            break;
        case KEY_LEFT:
            if (currpnt > 0)
            {
                currpnt--;
            }
            else if (currline != can_edit_begin && currline->prev)
            {
                curr_window_line--;
                currln--;
                currline = currline->prev;
                currpnt = currline->len;
            }
            break;
        case Ctrl('C'):
            process_ESC_action('M', '2');
            break;
        case Ctrl('U'):
            process_ESC_action('M', '1');
            clear();
            break;
        case Ctrl('V'):
        case KEY_RIGHT:
            if (currline->len != currpnt)
            {
                currpnt++;
            }
            else if (currline->next && currline->next != can_edit_end)
            {
                currpnt = 0;
                curr_window_line++;
                currln++;
                currline = currline->next;
            }
            break;
        case Ctrl('P'):
        case KEY_UP:
            if (currline != can_edit_begin)
            {
                currln--;
                curr_window_line--;
                currline = currline->prev;
                currpnt =
                    (currline->len >
                     lastindent) ? lastindent : currline->len;
            }
            break;
        case Ctrl('N'):
        case KEY_DOWN:
            if (currline->next != can_edit_end)
            {
                currline = currline->next;
                curr_window_line++;
                currln++;
                currpnt =
                    (currline->len >
                     lastindent) ? lastindent : currline->len;
            }
            break;
        case Ctrl('B'):
        case KEY_PGUP:
            top_of_win = back_line(top_of_win, t_lines - 2);
            currline = back_line(currline, t_lines - 2);
            currln -= moveln;
            curr_window_line = getlineno();
            if (currpnt > currline->len)
                currpnt = currline->len;
            redraw_everything = YEA;
            break;
        case Ctrl('F'):
        case KEY_PGDN:
            top_of_win = forward_line(top_of_win, 22);
            currline = forward_line(currline, 22);
            currln += moveln;
            curr_window_line = getlineno();
            if (currpnt > currline->len)
                currpnt = currline->len;
            redraw_everything = YEA;
            break;
        case Ctrl('A'):
        case KEY_HOME:
            currpnt = 0;
            break;
        case Ctrl('E'):
        case KEY_END:
            currpnt = currline->len;
            break;
        case Ctrl('S'):
            top_of_win = can_edit_begin;
            currline = top_of_win;
            currpnt = 0;
            curr_window_line = 0;
            currln = 0;
            redraw_everything = YEA;
            break;
        case Ctrl('T'):
            top_of_win = back_line(can_edit_end->prev, 22);
            countline();
            currln = moveln;
            currline = can_edit_end->prev;
            curr_window_line = getlineno();
            currpnt = 0;
            redraw_everything = YEA;
            break;
        case Ctrl('O'):
        case KEY_INS:
            insert_character = !insert_character;
            break;
        case Ctrl('H'):
        case '\177':
            NO_ANSI_MODIFY;
            if (currpnt == 0)
            {
                struct textline *p;

                if (currline == can_edit_begin)
                    break;
                if (currline->len + currline->prev->len >= 254)
                    break;
                currln--;
                currline = currline->prev;
                currpnt = currline->len;
                curr_window_line--;
                if (curr_window_line < 0)
                    top_of_win = top_of_win->next;
                if (*killsp(currline->next->data) == '\0')
                {
                    delete_line(currline->next);
                    redraw_everything = YEA;
                    break;
                }
                p = currline;
                while (!join(p))
                {
                    p = p->next;
                    if (p == NULL)
                    {
                        indigestion(2);
                        abort_bbs();
                    }
                }
                redraw_everything = YEA;
                break;
            }
            currpnt--;
            delete_char();
            break;
        case Ctrl('D'):
        case KEY_DEL:
            NO_ANSI_MODIFY;
            if (currline->len == currpnt)
            {
                if (currline->len + currline->next->len >= 254)
                    break;
                vedit_key(Ctrl('K'));
                break;
            }
            delete_char();
            break;
        case Ctrl('Y'):
            no_touch = 0;
            currpnt = 0;
            currline->len = 0;
            vedit_key(Ctrl('K'));
            break;
        case Ctrl('K'):
            NO_ANSI_MODIFY;
            if (currline->next == can_edit_end)
            {
                if (currline->len > currpnt)
                {
                    currline->len = currpnt;
                }
                else if (currpnt != 0)
                    currline->len = currpnt = 0;
                ;
                currline->data[currpnt] = '\0';
                break;
            }
            if (currline->len == 0)
            {
                struct textline *p = currline->next;

                if (currline == top_of_win)
                    top_of_win = p;
                delete_line(currline);
                currline = p;
                redraw_everything = YEA;
                break;
            }
            if (currline->len == currpnt)
            {
                struct textline *p = currline;

                while (!join(p))
                {
                    p = p->next;
                    if (p == NULL)
                    {
                        indigestion(2);
                        abort_bbs();
                    }
                }
                redraw_everything = YEA;
                break;
            }
            for (patch = 0, i = currpnt - 1;
                 i >= 0 && currline->data[i] & 0x80; i--)
                patch++;
            if (patch % 2)
                currpnt--;
            currline->len = currpnt;
            currline->data[currpnt] = '\0';
            break;
        default:
            break;
        }
    }
    if (curr_window_line < 0)
    {
        curr_window_line = 0;
        if (top_of_win != can_edit_begin)
        {
            top_of_win = top_of_win->prev;
            rscroll();
        }
    }
    if (curr_window_line >= t_lines - 1)
    {
        for (i = curr_window_line - t_lines + 1; i >= 0; i--)
        {
            curr_window_line--;
            if (top_of_win->next != can_edit_end)
            {
                top_of_win = top_of_win->next;
                scroll();
            }
        }
    }
    if (editansi)
        redraw_everything = YEA;
    shift = (currpnt + 2 > STRLEN) ?
        (currpnt / (STRLEN - scrollen)) * (STRLEN - scrollen) : 0;
    msgline();
    if (shifttmp != shift || redraw_everything == YEA)
    {
        redraw_everything = YEA;
        shifttmp = shift;
    }
    else
        redraw_everything = NA;

    move(curr_window_line, 0);
    if (currline->attr & M_MARK)
    {
        showansi = 1;
        cstrnput(currline->data + shift);
        showansi = 0;
    }
    else
    {
        strnput(currline->data + shift);
        clrtoeol();
    }
}

int
raw_vedit(char *filename, int write_header_to_file, int modifyheader,
          unsigned int reid)
{
    int newch, ch = 0, foo, shift, sure = 0;
    int addfrom = 0;

    addfrom = read_file(filename);
    if (!modifyheader)
    {
        top_of_win = firstline;
        while (top_of_win->next != can_edit_end && ch < 4)
        {
            top_of_win = top_of_win->next;
            ch++;
        }
        ch = 0;
        can_edit_begin = top_of_win;
    }
    currline = top_of_win;
    curr_window_line = 0;
    currln = 0;
    currpnt = 0;
    clear();
    display_buffer();
    msgline();
    while (ch != EOF)
    {
        newch = '\0';
        switch (ch)
        {
        case Ctrl('X'):
            sure = 1;
        case Ctrl('W'):

            foo =
                write_file(filename, write_header_to_file, YEA, sure,
                           reid);
            if (foo != KEEP_EDITING)
                return foo;
            redraw_everything = YEA;
            break;
        case KEY_ESC:
            if (KEY_ESC_arg == KEY_ESC)
                insert_char(KEY_ESC);
            else
            {
                newch = vedit_process_ESC(KEY_ESC_arg);
                clear();
            }
            redraw_everything = YEA;
            break;
        default:
            vedit_key(ch);
        }
        if (redraw_everything)
            display_buffer();
        redraw_everything = NA;
        shift = (currpnt + 2 > STRLEN) ?
            (currpnt / (STRLEN - scrollen)) * (STRLEN - scrollen) : 0;
        move(curr_window_line, currpnt - shift);
        ch = (newch != '\0') ? newch : igetkey();
    }
    return 1;
}

int vedit(char *filename, int write_header_to_file, int modifyheader)
{
    int ans, t = showansi;

    showansi = 0;
#ifdef ALLOWAUTOWRAP

    if (DEFINE(DEF_AUTOWRAP) && uinfo.mode == POSTING)
        linelen = 79;
    else
#endif

        linelen = WRAPMARGIN;
    init_alarm();
    ismsgline = (DEFINE(DEF_EDITMSG)) ? 1 : 0;
    msg(0);
    ans = raw_vedit(filename, write_header_to_file, modifyheader, 0);
    showansi = t;
    signal(SIGALRM, SIG_IGN);
    return ans;
}
int vedit_reid(char *filename, int write_header_to_file, int modifyheader,
               unsigned int reid)
{
    int ans, t = showansi;

    showansi = 0;
#ifdef ALLOWAUTOWRAP

    if (DEFINE(DEF_AUTOWRAP) && uinfo.mode == POSTING)
        linelen = 79;
    else
#endif

        linelen = WRAPMARGIN;
    init_alarm();
    ismsgline = (DEFINE(DEF_EDITMSG)) ? 1 : 0;
    msg(0);
    ans = raw_vedit(filename, write_header_to_file, modifyheader, reid);
    showansi = t;
    signal(SIGALRM, SIG_IGN);
    return ans;
}

