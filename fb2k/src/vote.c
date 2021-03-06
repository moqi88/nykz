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
#include "vote.h"

extern int page, range;
extern struct shortfile *bcache;
extern struct BCACHE *brdshm;
static char *vote_type[] = { "�Ƿ�", "��ѡ", "��ѡ", "����", "�ʴ�" };
struct votebal currvote;
char controlfile[STRLEN];
unsigned int result[33];
int vnum;
int voted_flag;
FILE *sug;
int makevote(struct votebal *ball, char *bname);
int setvoteperm(struct votebal *ball);

int cmpvuid(char *userid, void *vuv)
{
    struct ballot *uv = (struct ballot *) vuv;

    return !strcmp(userid, uv->uid);
}

int setvoteflag(char *bname, int flag)
{
    int pos;
    struct boardheader fh;

    pos = search_record(BOARDS, &fh, sizeof(fh), cmpbnames, bname);
    if (((fh.flag & VOTE_FLAG) && (flag != 0))
        || (((fh.flag & VOTE_FLAG) == 0) && (flag == 0)))
        return 0;
    if (flag == 0)
        fh.flag = fh.flag & ~VOTE_FLAG;
    else
        fh.flag = fh.flag | VOTE_FLAG;
    if (substitute_record(BOARDS, &fh, sizeof(fh), pos) == -1)
        prints("Error updating BOARDS file...\n");
    return 1;
}

void b_report(char *str)
{
    char buf[STRLEN];

    sprintf(buf, "%s %s", currboard, str);
    report(buf);
}

void makevdir(char *bname)
{
    struct stat st;
    char buf[STRLEN];

    sprintf(buf, "vote/%s", bname);
    if (stat(buf, &st) != 0)
        mkdir(buf, 0755);
}

void setvfile(char *buf, char *bname, char *filename)
{
    sprintf(buf, "vote/%s/%s", bname, filename);
}

void setcontrolfile(void)
{
    setvfile(controlfile, currboard, "control");
}

int b_notes_edit(void)
{
    char buf[STRLEN], buf2[STRLEN];
    char ans[4];
    int aborted, notetype;

    if (!chk_currBM(currBM))
        return 0;
    clear();
    move(1, 0);
    prints("�༭/ɾ�����屸��¼");
    getdata(3, 0, "�༭��ɾ�� (0)�뿪 (1)һ�㱸��¼ (2)���ܱ���¼? [1] ",
            ans, 2, DOECHO, YEA);
    if (ans[0] == '0')
        return FULLUPDATE;
    if (ans[0] != '2')
    {
        setvfile(buf, currboard, "notes");
        notetype = 1;
    }
    else
    {
        setvfile(buf, currboard, "secnotes");
        notetype = 2;
    }
    sprintf(buf2, "[%s] (E)�༭ (D)ɾ�� (A)ȡ�� [E]: ",
            (notetype == 1) ? "һ�㱸��¼" : "���ܱ���¼");
    getdata(5, 0, buf2, ans, 2, DOECHO, YEA);
    if (ans[0] == 'A' || ans[0] == 'a')
    {
        aborted = -1;
    }
    else if (ans[0] == 'D' || ans[0] == 'd')
    {
        move(6, 0);
        sprintf(buf2, "���Ҫɾ��[%s]",
                (notetype == 1) ? "һ�㱸��¼" : "���ܱ���¼");
        if (askyn(buf2, NA, NA) == YEA)
        {
            move(7, 0);
            unlink(buf);
            prints("[%s]�Ѿ�ɾ��...\n",
                   (notetype == 1) ? "һ�㱸��¼" : "���ܱ���¼");
            pressanykey();
            aborted = 1;

            log_bm_ops(currboard, "����", "ɾ��",
                       (notetype == 1) ? "һ�㱸��¼" : "���ܱ���¼");
        }
        else
            aborted = -1;
    }
    else
    {
        aborted = vedit(buf, NA, YEA);
        if (aborted != -1)
            log_bm_ops(currboard, "����", "�޸�",
                       (notetype == 1) ? "һ�㱸��¼" : "���ܱ���¼");
    }
    if (aborted == -1)
    {
        pressreturn();
    }
    else
    {
        if (notetype == 1)
            setvfile(buf, currboard, "noterec");
        else if (notetype == 2)
            setvfile(buf, currboard, "notespasswd");
        unlink(buf);
    }
    return FULLUPDATE;
}

int b_notes_passwd(void)
{
    FILE *pass;
    char passbuf[20], prepass[20];
    char buf[STRLEN];

    if (!chk_currBM(currBM))
    {
        return 0;
    }
    clear();
    move(1, 0);
    prints("�趨/����/ȡ�������ܱ���¼������...");
    setvfile(buf, currboard, "secnotes");
    if (!dashf(buf))
    {
        move(3, 0);
        prints("�����������ޡ����ܱ���¼����\n\n");
        prints("������ W ��á����ܱ���¼�������趨����...");
        pressanykey();
        return FULLUPDATE;
    }
    if (!check_notespasswd())
        return FULLUPDATE;
    getdata(3, 0, "�������µ����ܱ���¼����(Enter ȡ������): ", passbuf,
            19, NOECHO, YEA);
    if (passbuf[0] == '\0')
    {
        setvfile(buf, currboard, "notespasswd");
        unlink(buf);
        prints("�Ѿ�ȡ������¼���롣");
        pressanykey();
        log_bm_ops(currboard, "����", "", "ȡ�����ܱ���¼����");
        return FULLUPDATE;
    }
    getdata(4, 0, "ȷ���µ����ܱ���¼����: ", prepass, 19, NOECHO, YEA);
    if (strcmp(passbuf, prepass))
    {
        prints("\n���벻���, �޷��趨�����....");
        pressanykey();
        return FULLUPDATE;
    }
    setvfile(buf, currboard, "notespasswd");
    if ((pass = fopen(buf, "w")) == NULL)
    {
        move(5, 0);
        prints("����¼�����޷��趨....");
        pressanykey();
        return FULLUPDATE;
    }
    fprintf(pass, "%s\n", genpasswd(passbuf));
    fclose(pass);
    move(5, 0);
    prints("���ܱ���¼�����趨���....");
    pressanykey();
    log_bm_ops(currboard, "����", "", "�趨���ܱ���¼����");
    return FULLUPDATE;
}

int b_suckinfile(FILE * fp, char *fname)
{
    char inbuf[256];
    FILE *sfp;

    if ((sfp = fopen(fname, "r")) == NULL)
        return -1;
    while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
        fputs(inbuf, fp);
    fclose(sfp);
    return 0;
}

int catnotepad(FILE * fp, char *fname)
{
    char inbuf[256];
    FILE *sfp;
    int count;

    count = 0;
    if ((sfp = fopen(fname, "r")) == NULL)
    {
        fprintf(fp,
                "[1;34m  ��[44m__________________________________________________________________________[m \n\n");
        return -1;
    }
    while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
    {
        if (count != 0)
            fputs(inbuf, fp);
        else
            count++;
    }
    fclose(sfp);
    return 0;
}

int b_closepolls(void)
{
    char buf[80];
    time_t now, nextpoll;
    int i, end;

    now = time(0);
    resolve_boards();

    if (now < brdshm->pollvote)
    {
        return 1;
    }
    move(t_lines - 1, 0);
    prints("�Բ���ϵͳ�ر�ͶƱ�У����Ժ�...");
    refresh();
    nextpoll = now + 7 * 3600;
    brdshm->pollvote = nextpoll;
    strcpy(buf, currboard);
    for (i = 0; i < brdshm->number; i++)
    {
        strcpy(currboard, (&bcache[i])->filename);
        setcontrolfile();
        end = get_num_records(controlfile, sizeof(currvote));
        for (vnum = end; vnum >= 1; vnum--)
        {
            time_t closetime;

            get_record(controlfile, &currvote, sizeof(currvote), vnum);
            closetime = currvote.opendate + currvote.maxdays * 86400;
            if (now > closetime)
                mk_result(vnum);
            else if (nextpoll > closetime)
            {
                nextpoll = closetime + 300;
                brdshm->pollvote = nextpoll;
            }
        }
    }
    strcpy(currboard, buf);

    return 0;
}

int count_result(void *vptr)
{
    struct ballot *ptr = (struct ballot *) vptr;
    int i;

    if (ptr->msg[0][0] != '\0')
    {
        if (currvote.type == VOTE_ASKING)
        {
            fprintf(sug, "[1m%s [m���������£�\n", ptr->uid);
        }
        else
            fprintf(sug, "[1m%s [m�Ľ������£�\n", ptr->uid);
        for (i = 0; i < 3; i++)
            fprintf(sug, "%s\n", ptr->msg[i]);
    }
    result[32]++;
    if (currvote.type == VOTE_ASKING)
    {
        return 0;
    }
    if (currvote.type != VOTE_VALUE)
    {
        for (i = 0; i < 32; i++)
        {
            if ((ptr->voted >> i) & 1)
                (result[i])++;
        }

    }
    else
    {
        result[31] += ptr->voted;
        result[(ptr->voted * 10) / (currvote.maxtkt + 1)]++;
    }
    return 0;
}

void get_result_title(void)
{
    char buf[STRLEN];

    getdatestring(currvote.opendate, NA);
    fprintf(sug, "�� ͶƱ�����ڣ�[1m%s[m  ���[1m%s[m\n",
            datestring, vote_type[currvote.type - 1]);
    fprintf(sug, "�� ���⣺[1m%s[m\n", currvote.title);
    if (currvote.type == VOTE_VALUE)
        fprintf(sug, "�� �˴�ͶƱ��ֵ���ɳ�����[1m%d[m\n\n",
                currvote.maxtkt);
    fprintf(sug, "�� Ʊѡ��Ŀ������\n\n");
    sprintf(buf, "vote/%s/desc.%ld", currboard, currvote.opendate);
    b_suckinfile(sug, buf);
}

int HiddenBoard(char *name)
{
    struct shortfile *bptr = NULL;

    int i = 0;

    for (i = 0; i < MAXBOARD; i++)
    {
        if (!strcmp(bcache[i].filename, name))
            break;
    }
    if (i == MAXBOARD)
        return 0;
    bptr = &bcache[i];
    if ((bptr->level != 0) && (!(bptr->level & PERM_POSTMASK)))
        return 1;
    else
        return 0;
}

int getVoterIP(char *file, struct voter_info v_info[MAXUSERS])
{
    int nums = 0;
    int fd;

    if ((fd = open(file, O_RDONLY, 0)) == -1)
        return 0;
    while (read(fd, &v_info[nums], sizeof(v_info[nums])) ==
           sizeof(v_info[nums]))
    {
        nums++;
    }
    return nums;
}

int writeVoterIP(char *file, char *board,
                 struct voter_info v_info[MAXUSERS], int nums)
{
    FILE *fd;
    int i;

    if ((fd = fopen(file, "w")) == NULL)
        return 0;
    fprintf(fd, "[����] %s ���ͶƱ�������\n\n", board);
    fprintf(fd, "����ƪ���������Զ�����ϵͳ��������\n\n");
    fprintf(fd,
            "		  ID    		   IP				        ͶƱʱ��\n");
    for (i = 0; i < nums; i++)
    {
        fprintf(fd, "%12s  ", v_info[i].uid);
        fprintf(fd, "%15s  ", v_info[i].uip);
        getdatestring(v_info[i].vote_time, 0);
        fprintf(fd, "%s\n", datestring);
    }
    fclose(fd);
    return 0;
}

int compare_ip(struct voter_info a, struct voter_info b)
{
    return strcmp(a.uip, b.uip);
}

int mk_result(int num)
{
    char fname[STRLEN], nname[STRLEN];
    char sugname[STRLEN];
    char title[STRLEN];
    char title2[STRLEN];
    char ipname[STRLEN];
    struct voter_info vinfo[MAXUSERS];
    int votenums;
    char ipname2[STRLEN];
    int i;
    unsigned int total = 0;

    setcontrolfile();
    sprintf(fname, "vote/%s/flag.%ld", currboard, currvote.opendate);
    sprintf(ipname, "vote/%s/uip.%ld", currboard, currvote.opendate);
    sprintf(ipname2, "vote/%s/uip2.%ld", currboard, currvote.opendate);

    sug = NULL;
    sprintf(sugname, "vote/%s/tmp.%d", currboard, uinfo.pid);
    if ((sug = fopen(sugname, "w")) == NULL)
    {
        report("open vote tmp file error");
        prints("Error: ����ͶƱ����...\n");
        pressanykey();
    }
    (void) memset(result, 0, sizeof(result));
    if (apply_record(fname, count_result, sizeof(struct ballot)) == -1)
    {
        report("Vote apply flag error");
    }
    fprintf(sug,
            "[1;44;36m������������������������������ʹ����%s������������������������������[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "��������" : "�˴ε�����");
    fclose(sug);
    sprintf(nname, "vote/%s/results", currboard);
    if ((sug = fopen(nname, "w")) == NULL)
    {
        report("open vote newresult file error");
        prints("Error: ����ͶƱ����...\n");
    }
    get_result_title();

    fprintf(sug, "** ͶƱ���:\n\n");
    if (currvote.type == VOTE_VALUE)
    {
        total = result[32];
        for (i = 0; i < 10; i++)
        {
            fprintf(sug,
                    "[1m  %4d[m �� [1m%4d[m ֮���� [1m%4d[m Ʊ  Լռ [1m%d%%[m\n",
                    (i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1),
                    ((i + 1) * currvote.maxtkt) / 10, result[i],
                    (result[i] * 100) / ((total <= 0) ? 1 : total));
        }
        fprintf(sug, "�˴�ͶƱ���ƽ��ֵ��: [1m%d[m\n",
                result[31] / ((total <= 0) ? 1 : total));
    }
    else if (currvote.type == VOTE_ASKING)
    {
        total = result[32];
    }
    else
    {
        for (i = 0; i < currvote.totalitems; i++)
        {
            total += result[i];
        }
        for (i = 0; i < currvote.totalitems; i++)
        {
            fprintf(sug, "(%c) %-40s  %4d Ʊ  Լռ [1m%d%%[m\n", 'A' + i,
                    currvote.items[i], result[i],
                    (result[i] * 100) / ((total <= 0) ? 1 : total));
        }
    }
    fprintf(sug, "\nͶƱ������ = [1m%d[m ��\n", result[32]);
    fprintf(sug, "ͶƱ��Ʊ�� =[1m %d[m Ʊ\n\n", total);
    fprintf(sug,
            "[1;44;36m������������������������������ʹ����%s������������������������������[m\n\n\n",
            (currvote.type != VOTE_ASKING) ? "��������" : "�˴ε�����");
    b_suckinfile(sug, sugname);
    unlink(sugname);
    fclose(sug);

    sprintf(title, "[����] %s ���ͶƱ���", currboard);

    if (currvote.showip)
    {
        votenums = getVoterIP(ipname, vinfo);
        writeVoterIP(ipname2, currboard, vinfo, votenums);
        sprintf(title2, "[����] %s ���ͶƱ�������", currboard);
    }

    if (strncmp(currboard, "BMTraining", 10) == 0)
    {
        Postfile(nname, "TrainingAdmin", title, 1);

        if (currvote.showip)
            Postfile(ipname2, "TrainingAdmin", title2, 1);
    }

    else if (!HiddenBoard(currboard))
    {
        Postfile(nname, "vote", title, 1);

        if (currvote.showip)
            Postfile(ipname2, "vote", title2, 1);
    }
    if (strncmp(currboard, "vote", 4))
    {
        Postfile(nname, currboard, title, 1);

        if (currvote.showip)
            Postfile(ipname2, currboard, title2, 1);
    }
    dele_vote(num);
    return 0;
}

int get_vitems(struct votebal *bal)
{
    int num;
    char buf[STRLEN];

    move(3, 0);
    prints("�����������ѡ����, �� ENTER ����趨.\n");
    num = 0;
    for (num = 0; num < 32; num++)
    {
        sprintf(buf, "%c) ", num + 'A');
        getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num], 36,
                DOECHO, YEA);
        if (strlen(bal->items[num]) == 0)
        {
            if (num != 0)
                break;
            num = -1;
        }
    }
    bal->totalitems = num;
    return num;
}

int vote_maintain(char *bname)
{
    char buf[STRLEN], ans[2];
    struct votebal *ball = &currvote;

    setcontrolfile();
    if (!HAS_PERM(PERM_OVOTE))
        if (!chk_currBM(currBM))
            return 0;
    stand_title("����ͶƱ��");
    makevdir(bname);
    for (;;)
    {
        getdata(2, 0,
                "(1)�Ƿ� (2)��ѡ (3)��ѡ (4)��ֵ (5)�ʴ� (6)ȡ�� ? [6]",
                ans, 2, DOECHO, YEA);
        ans[0] -= '0';
        if (ans[0] < 1 || ans[0] > 5)
        {
            prints("ȡ���˴�ͶƱ\n");
            return FULLUPDATE;
        }
        ball->type = (int) ans[0];
        break;
    }
    ball->opendate = time(0);
    if (makevote(ball, bname))
        return FULLUPDATE;
    setvoteflag(currboard, 1);
    clear();
    strcpy(ball->userid, currentuser.userid);
    if (append_record(controlfile, ball, sizeof(*ball)) == -1)
    {
        prints("�������صĴ����޷�����ͶƱ����ͨ��վ��");
        b_report("Append Control file Error!!");
    }
    else
    {
        char votename[STRLEN];
        int i;

        b_report("OPEN");
        prints("ͶƱ�俪���ˣ�\n");
        range++;
        sprintf(votename, "tmp/votetmp.%s.%05d", currentuser.userid,
                uinfo.pid);
        if ((sug = fopen(votename, "w")) != NULL)
        {
            sprintf(buf, "[֪ͨ] %s �ٰ�ͶƱ��%s", currboard, ball->title);
            get_result_title();
            if (ball->type != VOTE_ASKING && ball->type != VOTE_VALUE)
            {
                fprintf(sug, "\n��[1mѡ������[m��\n");
                for (i = 0; i < ball->totalitems; i++)
                {
                    fprintf(sug, "([1m%c[m) %-40s\n", 'A' + i,
                            ball->items[i]);
                }
            }
            fclose(sug);
            if (strncmp(currboard, "BMTraining", 10) == 0)
            {
                Postfile(votename, "TrainingAdmin", buf, 1);
            }
            else if (!HiddenBoard(currboard))
            {
                Postfile(votename, "vote", buf, 1);
            }

            if (strncmp(currboard, "vote", 4))
            {
                Postfile(votename, currboard, buf, 1);
            }

            unlink(votename);
        }
    }
    pressreturn();
    return FULLUPDATE;
}

int makevote(struct votebal *ball, char *bname)
{
    char buf[STRLEN];
    int aborted;

    prints("�밴�κμ���ʼ�༭�˴� [ͶƱ������]: \n");
    igetkey();
    setvfile(genbuf, bname, "desc");
    sprintf(buf, "%s.%ld", genbuf, ball->opendate);
    aborted = vedit(buf, NA, YEA);
    if (aborted)
    {
        clear();
        prints("ȡ���˴�ͶƱ�趨\n");
        pressreturn();
        return 1;
    }
    clear();
    getdata(0, 0, "�˴�ͶƱ��������[1]: ", buf, 3, DOECHO, YEA);
    if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
        ball->maxdays = 1;
    else
        ball->maxdays = atoi(buf);
    while (1)
    {
        getdata(1, 0, "ͶƱ��ı���: ", ball->title, 51, DOECHO, YEA);
        if (killwordsp(ball->title) != 0)
            break;
        bell();
    }
    switch (ball->type)
    {
    case VOTE_YN:
        ball->maxtkt = 0;
        strcpy(ball->items[0], "�޳�  ���ǵģ�");
        strcpy(ball->items[1], "���޳ɣ����ǣ�");
        strcpy(ball->items[2], "û������������");
        ball->maxtkt = 1;
        ball->totalitems = 3;
        break;
    case VOTE_SINGLE:
        get_vitems(ball);
        ball->maxtkt = 1;
        break;
    case VOTE_MULTI:
        get_vitems(ball);
        for (;;)
        {
            getdata(21, 0, "һ������༸Ʊ? [1]: ", buf, 5, DOECHO, YEA);
            ball->maxtkt = atoi(buf);
            if (ball->maxtkt <= 0)
                ball->maxtkt = 1;
            if (ball->maxtkt > ball->totalitems)
                continue;
            break;
        }
        break;
    case VOTE_VALUE:
        for (;;)
        {
            getdata(3, 0, "������ֵ��󲻵ó��� [100] : ", buf, 4, DOECHO,
                    YEA);
            ball->maxtkt = atoi(buf);
            if (ball->maxtkt <= 0)
                ball->maxtkt = 100;
            break;
        }
        break;
    case VOTE_ASKING:
        ball->maxtkt = 0;
        currvote.totalitems = 0;
        break;
    default:
        ball->maxtkt = 1;
        break;
    }
    if (askyn("����ͶƱ��Ҫ��ʾͶƱ����Ϣ��?", NA, NA))
    {
        ball->showip = 1;
    }
    else
    {
        ball->showip = 0;
    }
    return 0;
}

int setvoteperm(struct votebal *ball)
{
    int flag = NA, changeit = YEA;
    char buf[6], msgbuf[STRLEN];

    if (!HAS_PERM(PERM_OVOTE))
        return 0;
    clear();
    prints("��ǰͶƱ���⣺[\033[1;33m%s\033[m]", ball->title);
    prints("\n���Ǳ�վ��ͶƱ����Ա�������Զ�ͶƱ����ͶƱ�����趨\n");
    prints("\nͶƱ�����趨�����������ƣ����������ƿ������ۺ��޶���\n");
    prints
        ("    (1) Ȩ������ [\033[32m�û�����ӵ��ĳЩȨ�޲ſ���ͶƱ\033[m]\n");
    prints
        ("    (2) ͶƱ���� [\033[35m������ͶƱ�����е��˲ſ���ͶƱ\033[m]\n");
    prints
        ("    (3) ������� [\033[36m�û���������һ���������ſ�ͶƱ\033[m]\n");
    prints("\n����ϵͳ��һ��һ��������������ͶƱ�����趨��\n\n");
    if (askyn("��ȷ��Ҫ��ʼ�Ը�ͶƱ����ͶƱ�����趨��", NA, NA) == NA)
        return 0;
    flag = (ball->level & ~(LISTMASK | VOTEMASK)) ? YEA : NA;
    prints
        ("\n\(\033[36m1\033[m) Ȩ������  [Ŀǰ״̬: \033[32m%s\033[m]\n\n",
         flag ? "������" : "������");
    if (flag == YEA)
    {
        if (askyn("��Ҫȡ��ͶƱ�ߵ�Ȩ��������", NA, NA) == YEA)
            flag = NA;
        else
            changeit = askyn("����Ҫ�޸�ͶƱ�ߵ�Ȩ��������", NA, NA);
    }
    else
        flag = askyn("��ϣ��ͶƱ�߱���߱�ĳ��Ȩ����", NA, NA);
    if (flag == NA)
    {
        ball->level = ball->level & (LISTMASK | VOTEMASK);
        prints("\n\n��ǰͶƱ\033[32m��Ȩ������\033[m��");
    }
    else if (changeit == NA)
    {
        prints("\n��ǰͶƱ\033[32m��������Ȩ������\033[m��");
    }
    else
    {
        clear();
        prints("\n�趨\033[32mͶƱ�߱���\033[m��Ȩ�ޡ�");
        ball->level =
            setperms(ball->level, "ͶƱȨ��", NUMPERMS, showperminfo);
        move(1, 0);
        if (ball->level & ~(LISTMASK | VOTEMASK))
            prints
                ("���Ѿ�\033[32m�趨��\033[mͶƱ�ı���Ȩ�ޣ�ϵͳ���������趨");
        else
            prints
                ("������\033[32mȡ����\033[mͶƱ��Ȩ���޶���ϵͳ���������趨");
    }
    prints("\n\n\n\033[33m����һ����\033[m����������ͶƱ���Ƶ��趨\n");
    pressanykey();
    clear();
    flag = (ball->level & LISTMASK) ? YEA : NA;
    prints
        ("\n(\033[36m2\033[m) ͶƱ����  [Ŀǰ״̬: \033[32m%s\033[m]\n\n",
         flag ? "�������е� ID �ſ�ͶƱ" : "������������");
    if (askyn("����ı䱾���趨��", NA, NA) == YEA)
    {
        if (flag)
        {
            ball->level &= ~LISTMASK;
            prints("\n�����趨��\033[32m��ͶƱ����ͶƱ�������޶�\033[m\n");
        }
        else
        {
            ball->level |= LISTMASK;
            prints
                ("\n�����趨��\033[32mֻ���������е� ID �ſ���ͶƱ\033[m\n");
            prints
                ("[\033[33m��ʾ\033[m]�趨��Ϻ��� Ctrl+E ���༭ͶƱ����\n");
        }
    }
    else
    {
        prints
            ("\n�����趨��\033[32m[�����趨]�������е� ID �ſ���ͶƱ\033[m\n");
    }
    flag = (ball->level & VOTEMASK) ? YEA : NA;
    prints
        ("\n(\033[36m3\033[m) �������  [Ŀǰ״̬: \033[32m%s\033[m]\n\n",
         flag ? "����������" : "������������");
    if (flag == YEA)
    {
        if (askyn("����ȡ��ͶƱ�ߵ���վ��������վʱ��ȵ�������", NA, NA)
            == YEA)
            flag = NA;
        else
            changeit = askyn("���������趨����������", NA, NA);
    }
    else
    {
        changeit = YEA;
        flag = askyn("ͶƱ���Ƿ���Ҫ�ܵ���վ����������������", NA, NA);
    }
    if (flag == NA)
    {
        ball->level &= ~VOTEMASK;
        prints
            ("\n�����趨��\033[32m���������޶�\033[m\n\nͶƱ�����趨���\n\n");
    }
    else if (changeit == NA)
    {
        prints
            ("\n�����趨��\033[32m���������޶�\033[m\n\nͶƱ�����趨���\n\n");
    }
    else
    {
        ball->level |= VOTEMASK;
        sprintf(msgbuf, "��վ�������ٴﵽ���ٴΣ�[%d]", ball->x_logins);
        getdata(11, 4, msgbuf, buf, 5, DOECHO, YEA);
        if (buf[0] != '\0')
            ball->x_logins = atoi(buf);
        sprintf(msgbuf, "���������������ж���ƪ��[%d]", ball->x_posts);
        getdata(12, 4, msgbuf, buf, 5, DOECHO, YEA);
        if (buf[0] != '\0')
            ball->x_posts = atoi(buf);
        sprintf(msgbuf, "�ڱ�վ���ۼ���վʱ�������ж���Сʱ��[%d]",
                ball->x_stay);
        getdata(13, 4, msgbuf, buf, 5, DOECHO, YEA);
        if (buf[0] != '\0')
            ball->x_stay = atoi(buf);
        sprintf(msgbuf, "���ʺ�ע��ʱ�������ж����죿[%d]", ball->x_live);
        getdata(14, 4, msgbuf, buf, 5, DOECHO, YEA);
        if (buf[0] != '\0')
            ball->x_live = atoi(buf);
        prints("\nͶƱ�����趨���\n\n");
    }
    if (askyn("��ȷ��Ҫ�޸�ͶƱ������", NA, NA) == YEA)
    {
        sprintf(msgbuf, "�޸� %s ��ͶƱ[ͶƱ����]", currboard);
        securityreport(msgbuf);
        return 1;
    }
    else
        return 0;
}

int vote_flag(char *bname, char val, int mode)
{
    char buf[STRLEN], flag;
    int fd, num, size;

    num = usernum - 1;
    switch (mode)
    {
    case 2:
        sprintf(buf, "Welcome.rec");
        break;
    case 1:
        setvfile(buf, bname, "noterec");
        break;
    default:
        return -1;
    }
    if (num >= MAXUSERS)
    {
        report("Vote Flag, Out of User Numbers");
        return -1;
    }
    if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1)
    {
        return -1;
    }
    flock(fd, LOCK_EX);
    size = (int) lseek(fd, 0, SEEK_END);
    memset(buf, 0, sizeof(buf));
    while (size <= num)
    {
        write(fd, buf, sizeof(buf));
        size += sizeof(buf);
    }
    lseek(fd, (off_t) num, SEEK_SET);
    read(fd, &flag, 1);
    if ((flag == 0 && val != 0))
    {
        lseek(fd, (off_t) num, SEEK_SET);
        write(fd, &val, 1);
    }
    flock(fd, LOCK_UN);
    close(fd);
    return flag;
}

int vote_check(int bits)
{
    int i, count;

    for (i = count = 0; i < 32; i++)
    {
        if ((bits >> i) & 1)
            count++;
    }
    return count;
}

int showvoteitems(unsigned int pbits, int i, int flag)
{
    char buf[STRLEN];
    int count;

    if (flag == YEA)
    {
        count = vote_check(pbits);
        if (count > currvote.maxtkt)
            return NA;
        move(2, 0);
        clrtoeol();
        prints("���Ѿ�Ͷ�� [1m%d[m Ʊ", count);
    }
    sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i,
            ((pbits >> i) & 1 ? "��" : "  "), currvote.items[i]);
    move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
    prints(buf);
    refresh();
    return YEA;
}

void show_voteing_title(void)
{
    time_t closedate;
    char buf[STRLEN];

    if (currvote.type != VOTE_VALUE && currvote.type != VOTE_ASKING)
        sprintf(buf, "��ͶƱ��: [1m%d[m Ʊ", currvote.maxtkt);
    else
        buf[0] = '\0';
    closedate = currvote.opendate + currvote.maxdays * 86400;
    getdatestring(closedate, NA);
    prints("ͶƱ��������: [1m%s[m  %s  %s\n",
           datestring, buf, (voted_flag) ? "([5;1m�޸�ǰ��ͶƱ[m)" : "");
    prints("ͶƱ������: [1m%-50s[m����: [1m%s[m \n", currvote.title,
           vote_type[currvote.type - 1]);
    if (currvote.showip)
        prints("[1mע�⣺����ͶƱ������ͶƱ��ID��IP��Ϣ[m\n");
}

int getsug(struct ballot *uv)
{
    int i, line;

    move(0, 0);
    clrtobot();
    if (currvote.type == VOTE_ASKING)
    {
        show_voteing_title();
        line = 3;
        prints("��������������(����):\n");
    }
    else
    {
        line = 1;
        prints("����������������(����):\n");
    }
    move(line, 0);
    for (i = 0; i < 3; i++)
    {
        prints(": %s\n", uv->msg[i]);
    }
    for (i = 0; i < 3; i++)
    {
        getdata(line + i, 0, ": ", uv->msg[i], STRLEN - 2, DOECHO, NA);
        if (uv->msg[i][0] == '\0')
            break;
    }
    return i;
}

int multivote(struct ballot *uv)
{
    unsigned int i;

    i = uv->voted;
    move(0, 0);
    show_voteing_title();
    uv->voted =
        setperms(uv->voted, "ѡƱ", currvote.totalitems, showvoteitems);
    if (uv->voted == i)
        return -1;
    return 1;
}

int valuevote(struct ballot *uv)
{
    unsigned int chs;
    char buf[10];

    chs = uv->voted;
    move(0, 0);
    show_voteing_title();
    prints("�˴������ֵ���ܳ��� [1m%d[m", currvote.maxtkt);
    if (uv->voted != 0)
        sprintf(buf, "%d", uv->voted);
    else
        memset(buf, 0, sizeof(buf));
    do
    {
        getdata(3, 0, "������һ��ֵ? [0]: ", buf, 5, DOECHO, NA);
        uv->voted = abs(atoi(buf));
    }
    while ((int) uv->voted > currvote.maxtkt && buf[0] != '\n'
           && buf[0] != '\0');
    if (buf[0] == '\n' || buf[0] == '\0' || uv->voted == chs)
        return -1;
    return 1;
}

int user_vote(int page, int num)
{
    char fname[STRLEN], bname[STRLEN], ipfname[STRLEN];
    char buf[STRLEN];
    char uipname[STRLEN];
    struct ballot uservote, tmpbal;
    struct voter_info v_info;
    int votevalue, result = NA;
    int aborted = NA, pos;

    move(t_lines - 2, 0);
    get_record(controlfile, &currvote, sizeof(struct votebal), num);
    sprintf(ipfname, "vote/%s/ip.%ld", currboard, currvote.opendate);
    sprintf(uipname, "vote/%s/uip.%ld", currboard, currvote.opendate);
    if (!haspostperm(currboard))
    {
        prints("�Բ������ڱ���û�з���Ȩ�ޣ����Բ���ͶƱ\n");
    }
    else if (currentuser.firstlogin > currvote.opendate)
    {
        prints("�Բ���, ��ͶƱ�����ʺ�����֮ǰ������������ͶƱ\n");
    }
    else if (!HAS_PERM(currvote.level & ~(LISTMASK | VOTEMASK)))
    {
        prints("�Բ�����Ŀǰ����Ȩ�ڱ�Ʊ��ͶƱ\n");
    }
    else if (currvote.level & LISTMASK)
    {
        char listfilename[STRLEN];

        setvfile(listfilename, currboard, "vote.list");
        if (!dashf(listfilename))
            prints("�Բ��𣬱�Ʊ����Ҫ�趨��ͶƱ���᷽�ɽ���ͶƱ\n");
        else if (!seek_in_file(listfilename, currentuser.userid))
            prints("�Բ���, ͶƱ�������Ҳ������Ĵ���\n");
        else
            result = YEA;
    }
    else if (currvote.level & VOTEMASK)
    {
        if (currentuser.numlogins < currvote.x_logins
            || currentuser.numposts < currvote.x_posts
            || (unsigned int) currentuser.stay < currvote.x_stay * 3600
            || (unsigned int) currentuser.firstlogin >
            (currvote.opendate - currvote.x_live * 86400))
            prints("�Բ�����Ŀǰ�в����ʸ��ڱ�Ʊ��ͶƱ\n");
        else
            result = YEA;
    }
    else
        result = YEA;
    if (result == NA)
    {
        pressanykey();
        return 0;
    }
    sprintf(fname, "vote/%s/flag.%ld", currboard, currvote.opendate);
    if ((pos = search_record(fname, &uservote, sizeof(uservote),
                             cmpvuid, currentuser.userid)) <= 0)
    {
        (void) memset(&uservote, 0, sizeof(uservote));
        voted_flag = NA;
    }
    else
    {
        voted_flag = YEA;
    }
    strcpy(uservote.uid, currentuser.userid);
    strcpy(v_info.uid, currentuser.userid);
    strcpy(v_info.uip, currentuser.lasthost);
    v_info.vote_time = time(0);
    sprintf(bname, "desc.%ld", currvote.opendate);
    setvfile(buf, currboard, bname);
    ansimore(buf, YEA);
    move(0, 0);
    clrtobot();
    switch (currvote.type)
    {
    case VOTE_SINGLE:
    case VOTE_MULTI:
    case VOTE_YN:
        votevalue = multivote(&uservote);
        if (votevalue == -1)
            aborted = YEA;
        break;
    case VOTE_VALUE:
        votevalue = valuevote(&uservote);
        if (votevalue == -1)
            aborted = YEA;
        break;
    case VOTE_ASKING:
        uservote.voted = 0;
        aborted = !getsug(&uservote);
        break;
    }
    clear();
    if (aborted == YEA)
    {
        prints("���� ��[1m%s[m��ԭ���ĵ�ͶƱ��\n", currvote.title);
    }
    else
    {
        if (currvote.type != VOTE_ASKING)
            getsug(&uservote);
        pos = search_record(fname, &tmpbal, sizeof(tmpbal),
                            cmpvuid, currentuser.userid);
        if (pos)
        {
            substitute_record(fname, &uservote, sizeof(uservote), pos);
            append_record(uipname, &v_info, sizeof(v_info));
        }
        else
        {
            if (search_ip(ipfname, currentuser.lasthost) >= MAXVOTEPERIP)
            {
                move(2, 0);
                clrtoeol();
                prints("\n�Բ�������ip ͶƱ�����ѳ�������\n");
                pressreturn();

                return 0;
            }
            else
            {
                if (append_record(fname, &uservote, sizeof(uservote)) ==
                    -1)
                {
                    move(2, 0);
                    clrtoeol();
                    prints("ͶƱʧ��! ��֪ͨվ���μ���һ��ѡ��ͶƱ\n");
                    pressreturn();
                }
                else
                {
                    if (append_record(uipname, &v_info, sizeof(v_info)) ==
                        -1)
                    {
                        move(2, 0);
                        clrtoeol();
                        prints("ͶƱʧ��! ��֪ͨվ���μ���һ��ѡ��ͶƱ\n");
                        pressreturn();
                    }
                    else
                    {
                        append_ip(ipfname, currentuser.lasthost);
                    }
                }
            }

        }

        prints("\n�Ѿ�����Ͷ��Ʊ����...\n");
    }
    pressanykey();
    return 0;
}

int voteexp(void)
{
    clrtoeol();
    prints("[1;44m��� ����ͶƱ���� ������ %-40s��� ���� ����[m\n",
           "ͶƱ����");
    return 0;
}

int printvote(void *vent)
{
    struct votebal *ent = (struct votebal *) vent;
    static int i;
    struct ballot uservote;
    char buf[STRLEN + 10];
    char flagname[STRLEN];
    int num_voted;

    if (ent == NULL)
    {
        move(2, 0);
        voteexp();
        i = 0;
        return 0;
    }
    i++;
    if (i > page + 20 || i > range)
        return QUIT;
    else if (i <= page)
        return 0;
    sprintf(buf, "flag.%ld", ent->opendate);
    setvfile(flagname, currboard, buf);
    if (search_record(flagname, &uservote, sizeof(uservote), cmpvuid,
                      currentuser.userid) <= 0)
    {
        voted_flag = NA;
    }
    else
        voted_flag = YEA;
    num_voted = get_num_records(flagname, sizeof(struct ballot));
    getdatestring(ent->opendate, NA);
    sprintf(buf, " %s%3d %-12.12s %6.6s%s%-40.40s%-4.4s %3d  %4d[m\n",
            (voted_flag == NA) ? "[1m" : "", i, ent->userid,
            datestring + 6, ent->level ? "\033[33ms\033[37m" : " ",
            ent->title, vote_type[ent->type - 1], ent->maxdays, num_voted);
    prints("%s", buf);
    return 0;
}

int dele_vote(int num)
{
    char buf[STRLEN];

    sprintf(buf, "vote/%s/ip.%ld", currboard, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/flag.%ld", currboard, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/desc.%ld", currboard, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/uip.%ld", currboard, currvote.opendate);
    unlink(buf);
    sprintf(buf, "vote/%s/uip2.%ld", currboard, currvote.opendate);
    unlink(buf);

    if (delete_record(controlfile, sizeof(currvote), num) == -1)
    {
        prints("����������֪ͨվ��....");
        pressanykey();
    }
    range--;
    if (get_num_records(controlfile, sizeof(currvote)) == 0)
    {
        setvoteflag(currboard, 0);
    }
    return 0;
}

int vote_results(char *bname)
{
    char buf[STRLEN];

    setvfile(buf, bname, "results");
    if (ansimore(buf, YEA) == -1)
    {
        move(3, 0);
        prints("Ŀǰû���κ�ͶƱ�Ľ����\n");
        clrtobot();
        pressreturn();
    }
    else
        clear();
    return FULLUPDATE;
}

int b_vote_maintain(void)
{
    return vote_maintain(currboard);
}

int vote_title(void)
{

    docmdtitle("[ͶƱ���б�]",
               "[[1;32m��[m,[1;32me[m] �뿪 [[1;32mh[m] ���� [[1;32m��[m,[1;32mr <cr>[m] ����ͶƱ [[1;32m��[m,[1;32m��[m] ��,��ѡ�� [1m������[m��ʾ��δͶƱ");
    update_endline();
    return 0;
}

#define maxrecp 300
int ListFileEdit(char *fname, char *msg, int response)
{
    char listfile[STRLEN], uident[13], ans[3];
    int cnt, i, n, fmode = NA;

    char mail_buffer[1024] = { 0 };
    char mail_title[STRLEN] = { 0 };

    if (fname == NULL)
        return 0;
    strcpy(listfile, fname);
    clear();
    cnt = listfilecontent(listfile, 3);
    while (1)
    {
        if (msg != NULL)
        {
            move(0, 0);
            prints("\033[0;1;47;31m%s\033[m", msg);
        }
        if (cnt > maxrecp - 10)
        {
            move(2, 0);
            prints("Ŀǰ���Ʊ�������� ^[[1m%d^[[m ��", maxrecp);
        }
        getdata(1, 0,
                "(A)���� (D)ɾ�� (I)������� (C)���Ŀǰ���� (E)ȷ�� [E]: ",
                ans, 2, DOECHO, YEA);
        if (ans[0] != 'a' && ans[0] != 'A' && ans[0] != 'd'
            && ans[0] != 'D' && ans[0] != 'i' && ans[0] != 'I'
            && ans[0] != 'c' && ans[0] != 'C')
        {
            clear();
            return cnt;
        }
        if (ans[0] == 'a' || ans[0] == 'd' || ans[0] == 'A'
            || ans[0] == 'D')
        {
            move(2, 0);
            if (ans[0] == 'a' || ans[0] == 'A')
                usercomplete("����������ʹ���ߴ���(ֻ�� ENTER ��������): ",
                             uident);
            else
                namecomplete("����������ʹ���ߴ���(ֻ�� ENTER ��������): ",
                             uident);
            move(2, 0);
            clrtoeol();
            if (uident[0] == '\0')
                continue;
        }
        switch (ans[0])
        {
        case 'A':
        case 'a':
            if (!getuser(uident))
            {
                move(2, 0);
                prints("���ʹ���ߴ����Ǵ����.");
                break;
            }
            if (!(lookupuser.userlevel & PERM_READMAIL))
            {
                move(2, 0);
                prints("[%s] Ŀǰ��ͣȨ\n", uident);
                break;
            }
            if (SeekInNameList(uident))
            {
                move(2, 0);
                prints("[%s] �Ѿ���������\n", uident);
                break;
            }
            add_to_file(listfile, uident);
            cnt++;

            if (response)
            {
                memset(mail_buffer, 0x00, 1024);
                memset(mail_title, 0x00, STRLEN);
                sprintf(mail_title, "%s ���� %s ��ɶ�д����",
                        uident, currboard);
                sprintf(mail_buffer,
                        "\n  %s ����: \n\n"
                        "\t �������� %s ��ġ��ɶ�д������\n\n"
                        "\t �����ڿ�ʼ��������� %s ������ͷ��ĵ�Ȩ��\n\n"
                        "\t\t\t\t\t\t\t\t\t\t\t\tBy %s",
                        uident, currboard, currboard, currentuser.userid);
                autoreport(mail_title, mail_buffer, uident, NULL);
            }

            break;
        case 'D':
        case 'd':
            if (seek_in_file(listfile, uident))
            {
                del_from_file(listfile, uident);
                DelFromNameList(uident);
                cnt--;

                if ((response) && (getuser(uident)))

                {
                    memset(mail_buffer, 0x00, 1024);
                    memset(mail_title, 0x00, STRLEN);
                    sprintf(mail_title,
                            "%s �������� %s ��ɶ�д����",
                            uident, currboard);
                    sprintf(mail_buffer,
                            "\n  %s ����: \n\n"
                            "\t ������������ %s ��ġ��ɶ�д������\n\n"
                            "\t �����ڿ�ʼ������ʧȥ�� %s ������ͷ��ĵ�Ȩ��\n\n"
                            "\t\t\t\t\t\t\t\t\t\t\t\tBy %s",
                            uident, currboard, currboard,
                            currentuser.userid);
                    autoreport(mail_title, mail_buffer, uident, NULL);
                }

            }
            break;
        case 'I':
        case 'i':
            n = 0;
            clear();
            for (i = cnt; i < maxrecp && n < uinfo.fnum; i++)
            {
                int key;

                move(2, 0);
                getuserid(uident, uinfo.friends[n]);
                prints("%s\n", uident);
                move(3, 0);
                n++;
                prints("(A)ȫ������ (Y)���� (N)������ (Q)����? [Y]:");
                if (!fmode)
                    key = igetkey();
                else
                    key = 'Y';
                if (key == 'q' || key == 'Q')
                    break;
                if (key == 'A' || key == 'a')
                {
                    fmode = YEA;
                    key = 'Y';
                }
                if (key == '\0' || key == '\n' || key == 'y' || key == 'Y')
                {
                    if (!getuser(uident))
                    {
                        move(4, 0);
                        prints("���ʹ���ߴ����Ǵ����.\n");
                        i--;
                        continue;
                    }
                    else if (!(lookupuser.userlevel & PERM_READMAIL))
                    {
                        move(4, 0);
                        prints("[%s] Ŀǰ��ͣȨ\n", uident);
                        i--;
                        continue;
                    }
                    else if (SeekInNameList(uident))
                    {
                        move(4, 0);
                        prints("[%s] �Ѿ���������\n", uident);
                        i--;
                        continue;
                    }
                    add_to_file(listfile, uident);
                    cnt++;

                }
            }
            fmode = NA;
            clear();
            break;
        case 'C':
        case 'c':
            unlink(listfile);
            cnt = 0;
        }
        move(5, 0);
        clrtobot();
        move(3, 0);
        clrtobot();
        cnt = listfilecontent(listfile, 3);
    }
}

int vote_key(int ch, int allnum, int pagenum)
{
    int deal = 0;
    char buf[STRLEN];

    switch (ch)
    {
    case 'v':
    case 'V':
    case '\n':
    case '\r':
    case 'r':
    case KEY_RIGHT:
        user_vote(0, allnum + 1);
        deal = 1;
        break;
    case 'R':
        vote_results(currboard);
        deal = 1;
        break;
    case 'H':
    case 'h':
        show_help("help/votehelp");
        deal = 1;
        break;
    case 'A':
    case 'a':
        if (!chk_currBM(currBM))
            return YEA;
        vote_maintain(currboard);
        deal = 1;
        break;
    case 'O':
    case 'o':
        if (!chk_currBM(currBM))
            return YEA;
        clear();
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        prints("[5;1;31m����!![m\nͶƱ����⣺[1m%s[m\n",
               currvote.title);
        if (askyn("��ȷ��Ҫ����������ͶƱ��", NA, NA) != YEA)
        {
            move(2, 0);
            prints("ȡ���������ͶƱ�ж�\n");
            pressreturn();
            clear();
            break;
        }
        mk_result(allnum + 1);
        sprintf(buf, "�������ͶƱ %s", currvote.title);
        securityreport(buf);
        break;
    case 'T':
        if (!chk_currBM(currBM))
            return YEA;
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        getdata(t_lines - 1, 0, "��ͶƱ���⣺", currvote.title, 51, DOECHO,
                YEA);
        if (currvote.title[0] != '\0')
        {
            substitute_record(controlfile, &currvote,
                              sizeof(struct votebal), allnum + 1);
        }
        break;
    case 's':
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        if (currvote.level == 0)
            break;
        deal = 1;
        move(t_lines - 8, 0);
        clrtoeol();
        prints("\033[47;30m��\033[31m%s\033[30m��ͶƱ����˵����\033[m\n",
               currvote.title);
        if (currvote.level & ~(LISTMASK | VOTEMASK))
        {
            int num, len;

            strcpy(genbuf, "bTCPRD#@XWBA#VS-DOM-F0s23456789t");
            len = strlen(genbuf);
            for (num = 0; num < len; num++)
                if (!(currvote.level & (1 << num)))
                    genbuf[num] = '-';
            prints("Ȩ�����ޣ�[\033[32m%s\033[m]\n", genbuf);
        }
        else
            prints("Ȩ�����ޣ�[\033[32m��Ʊ��ͶƱ����Ȩ������\033[m]\n");
        if (currvote.level & LISTMASK)
            prints
                ("�������ޣ�[\033[35mֻ��ͶƱ�����е� ID �ſ�ͶƱ  \033[m]\n");
        else
            prints
                ("�������ޣ�[\033[35m��Ʊ��ͶƱ����ͶƱ��������    \033[m]\n");
        if (currvote.level & VOTEMASK)
        {
            prints("          �� �١��ڱ�վ����վ�������� [%-4d] ��\n",
                   currvote.x_logins);
            prints("�������ޣ��� �ڡ��ڱ�վ������������   [%-4d] ƪ\n",
                   currvote.x_posts);
            prints("          �� �ۡ�ʵ���ۼ���վʱ������ [%-4d] Сʱ\n",
                   currvote.x_stay);
            prints("          �� �ܡ��� ID ��ע��ʱ������ [%-4d] ��\n",
                   currvote.x_live);
        }
        else
            prints
                ("�������ޣ�[\033[36m��Ʊ��ͶƱ���ܸ�����������\033[m    ]\n");
        pressanykey();
        break;
    case 'S':
        if (!HAS_PERM(PERM_OVOTE))
            return YEA;
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        if (setvoteperm(&currvote) != 0)
        {
            substitute_record(controlfile, &currvote,
                              sizeof(struct votebal), allnum + 1);
        }
        break;
    case Ctrl('E'):
        if (!HAS_PERM(PERM_OVOTE))
            return YEA;
        deal = 1;
        setvfile(genbuf, currboard, "vote.list");
        ListFileEdit(genbuf, "�༭��ͶƱ������", 0);
        break;
    case 'E':
        if (!chk_currBM(currBM))
            return YEA;
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        setvfile(genbuf, currboard, "desc");
        sprintf(buf, "%s.%ld", genbuf, currvote.opendate);
        vedit(buf, NA, YEA);
        break;
    case 'M':
    case 'm':
        if (!chk_currBM(currBM))
            return YEA;
        clear();
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        prints("[5;1;31m����!![m\n");
        prints("ͶƱ����⣺[1m%s[m\n", currvote.title);
        if (askyn("��ȷ��Ҫ�޸����ͶƱ���趨��", NA, NA) != YEA)
        {
            move(2, 0);
            prints("ȡ���޸�ͶƱ�ж�\n");
            pressreturn();
            clear();
            break;
        }
        makevote(&currvote, currboard);
        substitute_record(controlfile, &currvote,
                          sizeof(struct votebal), allnum + 1);
        sprintf(buf, "�޸�ͶƱ�趨 %s", currvote.title);
        securityreport(buf);
        break;
    case 'D':
    case 'd':
        if (!HAS_PERM(PERM_OVOTE))
        {
            if (!chk_currBM(currBM))
                return YEA;
        }
        deal = 1;
        get_record(controlfile, &currvote, sizeof(struct votebal),
                   allnum + 1);
        clear();
        prints("[5;1;31m����!![m\n");
        prints("ͶƱ����⣺[1m%s[m\n", currvote.title);
        if (askyn("��ȷ��Ҫǿ�ƹر����ͶƱ��", NA, NA) != YEA)
        {
            move(2, 0);
            prints("ȡ��ǿ�ƹر��ж�\n");
            pressreturn();
            clear();
            break;
        }
        sprintf(buf, "ǿ�ƹر�ͶƱ %s", currvote.title);
        securityreport(buf);
        dele_vote(allnum + 1);
        break;
    default:
        return 0;
    }
    if (deal)
    {
        Show_Votes();
        vote_title();
    }
    return 1;
}

int Show_Votes(void)
{

    move(3, 0);
    clrtobot();
    printvote(NULL);
    setcontrolfile();
    if (apply_record(controlfile, printvote, sizeof(struct votebal)) == -1)
    {
        prints("����û��ͶƱ�俪��....");
        pressreturn();
        return 0;
    }
    clrtobot();
    return 0;
}

int b_vote(void)
{
    int num_of_vote;
    int voting;

    if (!HAS_PERM(PERM_VOTE)
        || ((currentuser.stay < 1800) && (currentuser.numlogins < 10)))
        return DONOTHING;
    setcontrolfile();
    num_of_vote = get_num_records(controlfile, sizeof(struct votebal));
    if (num_of_vote == 0)
    {
        move(2, 0);
        clrtobot();
        prints("\n��Ǹ, Ŀǰ��û���κ�ͶƱ���С�\n");
        pressreturn();
        setvoteflag(currboard, 0);
        return FULLUPDATE;
    }
    setvoteflag(currboard, 1);
    modify_user_mode(VOTING);
    setlistrange(num_of_vote);
    clear();
    voting = choose(NA, 0, vote_title, vote_key, Show_Votes, user_vote);
    clear();
    return FULLUPDATE;
}

int b_results(void)
{
    return vote_results(currboard);
}

int m_vote(void)
{
    char buf[STRLEN];

    strcpy(buf, currboard);
    strcpy(currboard, DEFAULTBOARD);
    modify_user_mode(ADMIN);
    vote_maintain(DEFAULTBOARD);
    strcpy(currboard, buf);
    return 0;
}

int x_vote(void)
{
    char buf[STRLEN];

    modify_user_mode(XMENU);
    strcpy(buf, currboard);
    strcpy(currboard, DEFAULTBOARD);
    b_vote();
    strcpy(currboard, buf);
    return 0;
}

int x_results(void)
{
    modify_user_mode(XMENU);
    return vote_results(DEFAULTBOARD);
}

int check_global_vote_status(char *bname)
{

    FILE *fp_control = NULL;
    FILE *fp_flag = NULL;
    int num_of_vote = 0;

    int callforvoted = 0;
    struct votebal vote_rec;
    struct ballot vote_ballot;
    char path[STRLEN];

    if (!HAS_PERM(PERM_LOGINOK))
        return 0;
    if (currentuser.stay < 1800)
        return 0;
    if ((time(0) - lookupuser.firstlogin) / 86400 < 10)
        return 0;
    memset(&vote_rec, 0, sizeof(vote_rec));
    memset(&vote_ballot, 0, sizeof(vote_ballot));
    strcpy(currboard, DEFAULTBOARD);
    setcontrolfile();
    num_of_vote = get_num_records(controlfile, sizeof(struct votebal));
    if (num_of_vote <= 0)
    {
        return 0;
    }
    fp_control = fopen(controlfile, "r");

    callforvoted = 0;
    while ((fread(&vote_rec, sizeof(struct votebal), 1, fp_control)) > 0)
    {
        sprintf(path, "vote/%s/flag.%ld", currboard, vote_rec.opendate);
        fp_flag = fopen(path, "r");
        if (fp_flag == NULL)
            return 1;
        while ((fread(&vote_ballot, sizeof(struct ballot), 1, fp_flag)) >
               0)
        {
            callforvoted = 1;
            if (!strcmp(vote_ballot.uid, currentuser.userid))
            {
                callforvoted = 0;
                break;
            }
        }
        fclose(fp_flag);
        fp_flag = NULL;
        if (callforvoted == 1)
            return 1;

    }
    fclose(fp_control);
    fp_control = NULL;
    return 0;
}

void show_global_vote(void)
{
    int voted = 0;
    char user_path[STRLEN];

    FILE *fp;

    clear();
    move(23, 0);
    voted = check_global_vote_status(DEFAULTBOARD);
    if (voted == 0)
        return;
    setuserfile(user_path, "novote_sjtubbs");
    if (dashf(user_path))
        return;

    if (askyn("���ھ���ϵͳͶƱ�У�Ҫȥ������?", YEA, YEA) == NA)
    {
        clear();
        move(23, 0);
        if (askyn("ϣ���´ε�½ʱϵͳ������ʾ��?", NA, NA) == YEA)
        {
            fp = fopen(user_path, "w");
            fclose(fp);
            fp = NULL;
        }
        return;
    }
    strcpy(currboard, DEFAULTBOARD);
    b_vote();
    return;
}

