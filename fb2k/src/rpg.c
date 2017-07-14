#include "bbs.h"
#include "rpg.h"
#include <math.h>

char host1[20][40], host2[20][40], ip[20][40];

int place_monster[20];
int place_rate[20];
int place_time_start[20];
int place_time_end[20];
int place_block[20];
	
int basic_h_damage[20], counts = 0;
int plus_h_damage[20];
int basic_m_damage[20];
int plus_m_damage[20];
int n_life[20];
int n_magic[20];
int m_type[20];
int m_element[20];

//struct  mud_place m_place;
int direct[8];
	
typedef struct {
int year, month, day, hour, weekday;
int leap;	/* the lunar month is a leap month */
} Date;

Date solar, lunar, gan, zhi, gan2, zhi2, lunar2;

char str[] =
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
char datafile[80];
char userid[80] = "unknown.";

char *
n_horoscope (month, day)
char month, day;
{
	char *c_sign[28] = {
		"牛宿","女宿","虚宿","危宿","室宿","壁宿","奎宿",
			"娄宿","胃宿","昴宿","毕宿","觜宿","参宿","井宿",
			"鬼宿","柳宿","星宿","张宿","翼宿","轸宿","角宿",
			"亢宿","氐宿","房宿","心宿","尾宿","箕宿","斗宿"    
	};
	switch (month)
	{
	case 1:
		if (day < 20)
			return( c_sign[0]);
		else if(day < 29)
			return( c_sign[1]);

		else 
			return( c_sign[2]);
	case 2:
		if (day < 9)
			return( c_sign[2]);
		else if(day < 19)
			return( c_sign[3]);

		else 
			return( c_sign[4]);
	case 3:
		if (day <6)
			return( c_sign[4]);
		else if(day < 21)
			return( c_sign[5]);

		else 
			return( c_sign[6]);
	case 4:        
		if (day < 5)
			return( c_sign[6]);
		else if(day < 20)
			return( c_sign[7]);

		else 
			return( c_sign[8]);
	case 5:
		if (day < 11)
			return( c_sign[9]);
		else if(day < 21)
			return( c_sign[10]);

		else 
			return( c_sign[11]);
	case 6: 
		if (day < 6)
			return( c_sign[11]);
		else if(day < 22)
			return( c_sign[12]);

		else 
			return( c_sign[13]);
	case 7:
		if (day < 8)
			return( c_sign[13]);
		else if(day < 23)
			return( c_sign[14]);

		else 
			return( c_sign[15]); 
	case 8:
		if (day < 2)
			return( c_sign[15]);
		else if(day < 12)
			return( c_sign[16]);
		else  if (day < 23)
			return( c_sign[17]);
		else if(day < 27)
			return( c_sign[18]);


		else 
			return( c_sign[19]);
	case 9:
		if (day < 23)
			return( c_sign[19]);
		else
			return( c_sign[20]);
	case 10:
		if (day < 8)
			return( c_sign[21]);
		else if(day < 24)
			return( c_sign[22]);

		else 
			return( c_sign[23]);
	case 11:

		if (day < 3)
			return( c_sign[23]);
		else if(day < 13)
			return( c_sign[24]);

		else if(day < 23)
			return( c_sign[25]);

		else 
			return( c_sign[26]);
	case 12:
		if (day < 8)
			return( c_sign[26]);
		else if(day < 22)
			return( c_sign[27]);

		else 
			return( c_sign[0]);      
	}

	return ("");
}

int id_detail(void)
{
	char someoneID[20];
	clear();

	modify_user_mode (QUERY);
	usercomplete ("请输入您想查询的帐号: ", someoneID);

	if (!getuser (someoneID))
	{
		prints ("不正确的使用者代号\n");
		pressreturn ();
		return;
	}
	clear();

	id_detail_show(someoneID);
}


int id_detail_show(char  *someoneID)
{
	struct relative rela;
	//struct gift cgift;
	int size;
	struct stat kkk;
//	struct person t_person;
	char fname[80];
	char sub[80];
	int i,j;
	int fd;
	
	i = (lookupuser.gender == 'F') ? 5 : 6;
	sprintf (sub, "[\033[1;3%dm%s座 %s\033[m]\n ", i,horoscope (lookupuser.birthmonth, lookupuser.birthday),n_horoscope (lookupuser.birthmonth, lookupuser.birthday));
	prints(sub);
	/*
	if ((strcmp(lookupuser.userid,currentuser.userid)==0)||(strcmp(currentuser.userid, PK_ADMIN_A)==0)||(strcmp(currentuser.userid,PK_ADMIN_B)==0)){
		sethomefile(fname, lookupuser.userid, PK_PERSON_FILE);
		
		if(dashf(fname)){
		get_record (fname, &t_person, sizeof (t_person), 1);
		prints("体力%d/%d,内力%d/%d,生命力%d/%d",t_person.LiT,t_person.LiTMax,t_person.LiN,t_person.LiNMax,t_person.LiS,t_person.LiSMax);
		}
	}*/
	//亲戚关系读取
	move(4,0);
	sethomefile(fname, lookupuser.userid, "relative");
	if(	fd=open(fname,O_RDONLY)!=-1)
	{
		fd=open(fname,O_RDONLY  );
		stat(fname,&kkk);
		size=kkk.st_size/sizeof(struct relative);	
		i=0;
		j=0;
		while (i<size)
		{
			read(fd,&rela,sizeof(struct relative));
			i++;
			if(0 == strcmp("", rela.userid)) continue;
			if((j%5)==0)
			{prints ("\n%s的%s ",rela.userid,rela.relation);}
			else
			{prints ("%s的%s ",rela.userid,rela.relation);}
			j++;
		}
		close(fd);
	}
		
	if(0 == j)
	{
		prints ("\n没有亲戚");

	}
	pressreturn ();
	return;
}



int id_gift_show(char  *someoneID)
{
	//struct relative rela;
	struct gift cgift;
	int size;
	struct stat kkk;
	//struct person t_person;
	char fname[80];
	//char sub[80];
	int i = 0,j=0;
	int fd;
	
	//礼物读取
	sethomefile(fname,lookupuser.userid,"gift.dat");
	if (fd=open(fname,O_RDONLY)!=-1)
	{
		fd=open(fname,O_RDONLY);
		stat(fname,&kkk);
		size=kkk.st_size/sizeof(struct gift);
		close(fd);
		{prints ("\n礼物如下:");}
		while (i<size)
		{
			get_record (fname, &cgift, sizeof (struct gift), i+1);	
			i++;
			if(0 == strcmp("", cgift.name))	continue;
			if((j%3)==0)
			{prints ("\n%-12s (%s赠)",cgift.name,cgift.id);}
			else
			{prints ("%-12s (%s赠)",cgift.name,cgift.id);}
			j++;
		}
	}
	if(0 == j){
		prints ("没有礼物\n");
	}
	pressreturn ();
	return;
}




int id_relation_input(void)
{
	struct relative rela;
	char fname[80];
	int opt;
	char msg[3];
	int fd;
	char *relative_list[50]={
		"父亲","母亲","哥哥","弟弟","姐姐","妹妹",
			"舅舅","阿姨","伯父","叔叔","侄子","外甥",
			"丈夫","妻子","儿子","女儿","爷爷","外公","奶奶","外婆",
			"孙子","孙女","外孙","外孙女",
			"姑",
			"大伯","小叔","嫂","弟妹","大姑","小姑","大姨","小姨","大舅","小舅",
			"姐夫","妹夫","连襟","妯娌","亲家","岳父","岳母","公公","婆婆",
			"儿媳","女婿","婶","姑父","姨","舅母"
	};

	//-----------------------------------------------------------------
	clear();
	modify_user_mode (QUERY);
	//if((strcmp(currentuser.userid, ARCHIVE_ADMIN_A)==0)||(strcmp(currentuser.userid,ARCHIVE_ADMIN_B)==0||(strcmp(currentuser.userid,"you")==0))
	{
		usercomplete ("请输入帐号: ", rela.userid);

		if (!getuser (rela.userid))
		{
			prints ("不正确的使用者代号\n");
			pressreturn ();
			return 0;
		}
		//	sprintf (fname, "home/%c/%s/relative", toupper (lookupuser.userid[0]),lookupuser.userid);
		sethomefile(fname, lookupuser.userid, "relative");
		usercomplete ("请输入对方帐号: ", rela.userid);

		if (!getuser (rela.userid))
		{
			prints ("不正确的使用者代号\n");
			pressreturn ();
			return 0;
		}
		move(3,0);
		prints("1 父 2 母 3 兄 4 弟 5 姐 6 妹 7 舅  8 姨 9 伯 10 叔 \n11 侄 12 甥 13 夫 14 妻 15 儿 16 女 17 爷爷 18 外公 19 奶奶 20 外婆\n 21 孙子 22 孙女 23 外孙 24 外孙女 25 姑 26 大伯 27 小叔 28 嫂 29 弟妹 30 大姑 \n31 小姑 32 大姨 33 小姨 34 大舅 35 小舅 36 姐夫 37 妹夫 38 连襟 39 妯娌 40 亲家\n41 岳父 42 岳母 43 公公 44 婆婆 45 儿媳 46 女婿 47 婶 48 姑父 49 姨 50 舅母");
		getdata( 8, 0,"请输入:", msg, 3, DOECHO,  YEA);
		opt = atoi(msg);
		if (!isdigit(msg[0]) || opt <= 0 || opt > 50 || msg[0] == '\n' || msg[0] == '\0')
		{
			move(7,0);
			prints("对不起，输入错误!\n");
			pressanykey();
			return 0;
		}

		sprintf (rela.relation, relative_list[opt-1]);
		rela.regonizedate=time(0);	
		fd=open(fname,O_APPEND | O_WRONLY | O_CREAT ,0644 );
		write(fd,&rela,sizeof(struct relative));
		close(fd);
	}	
	sprintf (fname,"%s 进行了认证关系",currentuser.userid);
	securityreport(fname);
	prints (fname);
	pressreturn ();
	return 0;

	/*else{
		prints ("您不是思源档案馆馆长，您无权输入信息!\n");
		pressreturn ();
		return;
	}*/
}

int id_gift_delete(void)
{

	struct gift cgift;
	int size;
	struct stat kkk;
	char fname[80];
	char buf[80];
	int i = 0;
	int fd;
    char ans[8];

	clear();
	modify_user_mode (QUERY);

	sethomefile(fname,currentuser.userid,"gift.dat");
	if ((fd = open (fname, O_RDWR , 0600)) != -1)
	{
		stat(fname,&kkk);
		size=kkk.st_size/sizeof(struct relative);	
		close (fd);	

		if(0 == size){
			prints ("\n您没有礼物");
			pressreturn ();
			return 0;
		}

		i=0;
		while (1)
		{			
			//if( (i > size) || ( i < 0)) return 0;

			get_record (fname, &cgift, sizeof (struct gift), i+1);	
			clear();
			/*if(0 == strlen(cgift.name)){
				if ((*ans == 'K' || *ans == 'k') && ( i > 0 )){
					i--;
				}else{
					i++;
				}
				continue;
			}*/
			
			if(strlen(cgift.name)){
				sprintf (buf,"%s  %s 赠",cgift.name,cgift.id);
				move(4,0);
				prints(buf);
				getdata (1, 0,	"[D]删除  [J] 下一个 [K] 上一个 or [E] 离开: ",ans, 7, DOECHO, YEA);
			}else{
				sprintf (buf,"%s 曾经送您一份礼物，但是您删除了它",cgift.id);
				move(4,0);
				prints(buf);
				getdata (1, 0,	"[J] 下一个 [K] 上一个 or [E] 离开: ",ans, 7, DOECHO, YEA);
			}

			if (*ans == 'D' || *ans == 'd')
			{ 
				if(strcmp(cgift.name,"")){
					strcpy(cgift.name , "");
					substitute_record(fname, &cgift, sizeof(struct gift),i+1);
					i++;
				}
			}
			else if ((*ans == 'J' || *ans == 'j')){
				if(size > i) i++;
			}
			else if ((*ans == 'K' || *ans == 'k')){
				if(i)	i--;
			}
			else if ((*ans == 'E' || *ans == 'e'))
			{
				return FULLUPDATE;
			}
			else{
				break;
			}
		}
	}
	return FULLUPDATE;
}
	
int id_relation_delete(void)
{
	struct relative rela;
	char id1[STRLEN];
	char id2[STRLEN];
	struct stat kkk;
	char fname[80];
	int fd;
	int size,i;


	clear();
	modify_user_mode (QUERY);
	//if((strcmp(currentuser.userid, ARCHIVE_ADMIN_A)==0)||(strcmp(currentuser.userid,ARCHIVE_ADMIN_B)==0||(strcmp(currentuser.userid,"you")==0))
	{
		usercomplete ("请输入甲方帐号: ", id1);
		if (!getuser (id1))
		{
			if(askyn ("不正确的使用者代号，是否继续？", NA, NA) == NA)	return 0;
		}
		//	sprintf (fname, "home/%c/%s/relative", toupper (lookupuser.userid[0]),lookupuser.userid);
		sethomefile(fname, lookupuser.userid, "relative");

		usercomplete ("请输入乙方帐号: ", id2);
		if (!getuser (id2))
		{
			if(askyn ("不正确的使用者代号，是否继续？", NA, NA) == NA)	return 0;
		}
		move(3,0);


		if ((fd = open (fname, O_RDWR , 0600)) != -1)
		{
			flock (fd, LOCK_EX);
			stat(fname,&kkk);
			size=kkk.st_size/sizeof(struct relative);		
			i=0;
			while (i<size)
			{
				read(fd,&rela,sizeof(struct relative));
				i++;
					if(0 == strcmp(rela.userid,id2)){			
						strncpy (rela.userid, "\0", sizeof (rela.userid));
						lseek (fd, (off_t) -sizeof(rela), SEEK_CUR);
						write (fd, &rela, sizeof(rela));
					}
			}
			flock (fd, LOCK_UN);
			close (fd);
		}




		/*if(	fd=open(fname,O_RDWR)!=-1)
		{
			flock (fd, LOCK_EX);
			stat(fname,&kkk);
			size=kkk.st_size/sizeof(struct relative);	
			i=0;
			while (i<size)
			{
				read(fd,&rela,sizeof(struct relative));
				i++;
				if(0 == strcmp(rela.userid,id2)){			
				    strncpy (rela.userid, "\0", sizeof (rela.userid));
					substitute_record (fname,&rela, sizeof(struct relative), i);
				}
			}
			flock (fd, LOCK_UN);
			close(fd);
		}*/


		sethomefile(fname, lookupuser.userid, "relative");
		if ((fd = open (fname, O_RDWR , 0600)) != -1)
		{
			flock (fd, LOCK_EX);
			stat(fname,&kkk);
			size=kkk.st_size/sizeof(struct relative);		
			i=0;
			while (i<size)
			{
				read(fd,&rela,sizeof(struct relative));
				i++;
					if(0 == strcmp(rela.userid,id1)){			
						strncpy (rela.userid, "\0", sizeof (rela.userid));
						lseek (fd, (off_t) -sizeof(rela), SEEK_CUR);
						write (fd, &rela, sizeof(rela));
					}
			}
			flock (fd, LOCK_UN);
			close (fd);
		}

		/*if(	fd=open(fname,O_RDWR)!=-1)
		{
			fd=open(fname,O_RDWR);
			stat(fname,&kkk);
			size=kkk.st_size/sizeof(struct relative);	
			i=0;
			while (i<size)
			{
				read(fd,&rela,sizeof(struct relative));
				i++;
				if(0 == strcmp(rela.userid,id1)){			
				    strncpy (rela.userid, "\0", sizeof (rela.userid));
					substitute_record (fname,&rela, sizeof(struct relative), i);
				}
			}
			close(fd);
		}*/

	}	
	sprintf (fname,"%s 解除了 %s 和 %s认证关系",currentuser.userid,id1,id2);
	securityreport(fname);
	prints (fname);
	pressreturn ();
	return 0;

	/*else{
		prints ("您不是思源档案馆馆长，您无权输入信息!\n");
		pressreturn ();
		return;
	}*/
}


