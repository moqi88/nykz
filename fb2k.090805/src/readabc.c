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

#include <string.h>
#include "bbs.h"

extern char currboard[STRLEN];
extern char someoneID[31];

int marked_all(int type)
{
    struct fileheader post;
    int fd;
    char tname[128], fname[128], dname[128], buf[256];
    char tempname1[STRLEN], tempname2[STRLEN];
    struct stat st1, st2;

    sprintf(dname, "boards/%s/%s", currboard, DOT_DIR);
    sprintf(fname, "boards/%s/%s2", currboard, DOT_DIR);
    switch (type)
    {
    case 0:
        sprintf(tname, "boards/%s/%s", currboard, MARKED_DIR);
        break;
    case 1:
        sprintf(tname, "boards/%s/%s", currboard, AUTHOR_DIR);
        break;
    case 2:
    case 3:
        sprintf(tname, "boards/%s/SOMEONE.%s.DIR.%d", currboard, someoneID,
                type - 2);
        break;
    case 4:
        sprintf(tname, "boards/%s/KEY.%s.DIR", currboard,
                currentuser.userid);
        break;
    }

    if (stat(dname, &st1) == -1)
        return 1;
    if (stat(tname, &st2) != -1)
    {
        if (st2.st_mtime >= st1.st_mtime)
            return 1;
    }
    unlink(tname);
    sprintf(buf, "cp %s %s", dname, fname);
    system(buf);

    if ((fd = open(fname, O_RDONLY, 0)) == -1)
        return -1;

    while (read(fd, &post, sizeof(struct fileheader)) ==
           sizeof(struct fileheader))
    {
        switch (type)
        {
        case 0:
            if (post.accessed[0] & FILE_MARKED)
                append_record(tname, &post, sizeof(post));
            break;
        case 1:
            if (strncmp(post.title, "Re: ", 4))
            {
                append_record(tname, &post, sizeof(post));
            }
            break;
        case 2:
        case 4:
            if (type == 2)
                strtolower(tempname1, post.owner);
            else
                strtolower(tempname1, post.title);
            strtolower(tempname2, someoneID);
            tempname1[sizeof(tempname1) - 1] = '\0';
            tempname2[sizeof(tempname2) - 1] = '\0';
            if (strstr(tempname1, tempname2))
                append_record(tname, &post, sizeof(post));
            break;
        case 3:
            if (!strcasecmp(post.owner, someoneID))
                append_record(tname, &post, sizeof(post));
            break;
        }
    }
    close(fd);
    unlink(fname);
    return 0;
}

int return_search_all(int type)
{
    struct fileheader post;
    int fd;
    char tname[128], fname[128], dname[128], buf[256];
    char tempname1[STRLEN], tempname2[STRLEN];
    struct stat st1, st2;

    sprintf(dname, "boards/%s/%s", currboard, RETURN_DIR);
    sprintf(fname, "boards/%s/%s2", currboard, RETURN_DIR);
    switch (type)
    {
    case 1:
        sprintf(tname, "boards/%s/SOMEONE.%s.RETURN_DIR.1", currboard,
                someoneID);
        break;
    case 2:
        sprintf(tname, "boards/%s/SOMEONE.%s.RETURN_DIR.2", currboard,
                someoneID);
        break;
    case 3:
        sprintf(tname, "boards/%s/KEY.%s.RETURN_DIR", currboard,
                currentuser.userid);
        break;
    }

    if (dashf(tname))
        unlink(tname);

    if (stat(dname, &st1) == -1)
        return 1;
    if (stat(tname, &st2) != -1)
    {
        if (st2.st_mtime >= st1.st_mtime)
            return 1;
    }
    unlink(tname);
    sprintf(buf, "cp %s %s", dname, fname);
    system(buf);

    if ((fd = open(fname, O_RDONLY, 0)) == -1)
        return -1;

    while (read(fd, &post, sizeof(struct fileheader)) ==
           sizeof(struct fileheader))
    {
        switch (type)
        {
        case 1:
        case 3:
            if (type == 1)
                strtolower(tempname1, post.owner);
            else
                strtolower(tempname1, post.title);
            strtolower(tempname2, someoneID);
            tempname1[sizeof(tempname1) - 1] = '\0';
            tempname2[sizeof(tempname2) - 1] = '\0';
            if (strstr(tempname1, tempname2))
                append_record(tname, &post, sizeof(post));
            break;
        case 2:
            if (!strcasecmp(post.owner, someoneID))
                append_record(tname, &post, sizeof(post));
            break;
        }
    }
    close(fd);
    unlink(fname);
    return 0;
}
