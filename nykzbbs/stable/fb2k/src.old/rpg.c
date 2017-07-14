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
		"ţ��","Ů��","����","Σ��","����","����","����",
			"¦��","θ��","����","����","����","����","����",
			"����","����","����","����","����","����","����",
			"����","ص��","����","����","β��","����","����"    
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
	usercomplete ("�����������ѯ���ʺ�: ", someoneID);

	if (!getuser (someoneID))
	{
		prints ("����ȷ��ʹ���ߴ���\n");
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
	sprintf (sub, "[\033[1;3%dm%s�� %s\033[m]\n ", i,horoscope (lookupuser.birthmonth, lookupuser.birthday),n_horoscope (lookupuser.birthmonth, lookupuser.birthday));
	prints(sub);
	/*
	if ((strcmp(lookupuser.userid,currentuser.userid)==0)||(strcmp(currentuser.userid, PK_ADMIN_A)==0)||(strcmp(currentuser.userid,PK_ADMIN_B)==0)){
		sethomefile(fname, lookupuser.userid, PK_PERSON_FILE);
		
		if(dashf(fname)){
		get_record (fname, &t_person, sizeof (t_person), 1);
		prints("����%d/%d,����%d/%d,������%d/%d",t_person.LiT,t_person.LiTMax,t_person.LiN,t_person.LiNMax,t_person.LiS,t_person.LiSMax);
		}
	}*/
	//���ݹ�ϵ��ȡ
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
			{prints ("\n%s��%s ",rela.userid,rela.relation);}
			else
			{prints ("%s��%s ",rela.userid,rela.relation);}
			j++;
		}
		close(fd);
	}
		
	if(0 == j)
	{
		prints ("\nû������");

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
	
	//�����ȡ
	sethomefile(fname,lookupuser.userid,"gift.dat");
	if (fd=open(fname,O_RDONLY)!=-1)
	{
		fd=open(fname,O_RDONLY);
		stat(fname,&kkk);
		size=kkk.st_size/sizeof(struct gift);
		close(fd);
		{prints ("\n��������:");}
		while (i<size)
		{
			get_record (fname, &cgift, sizeof (struct gift), i+1);	
			i++;
			if(0 == strcmp("", cgift.name))	continue;
			if((j%3)==0)
			{prints ("\n%-12s (%s��)",cgift.name,cgift.id);}
			else
			{prints ("%-12s (%s��)",cgift.name,cgift.id);}
			j++;
		}
	}
	if(0 == j){
		prints ("û������\n");
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
		"����","ĸ��","���","�ܵ�","���","����",
			"�˾�","����","����","����","ֶ��","����",
			"�ɷ�","����","����","Ů��","үү","�⹫","����","����",
			"����","��Ů","����","����Ů",
			"��",
			"��","С��","ɩ","����","���","С��","����","С��","���","С��",
			"���","�÷�","����","��","�׼�","����","��ĸ","����","����",
			"��ϱ","Ů��","��","�ø�","��","��ĸ"
	};

	//-----------------------------------------------------------------
	clear();
	modify_user_mode (QUERY);
	//if((strcmp(currentuser.userid, ARCHIVE_ADMIN_A)==0)||(strcmp(currentuser.userid,ARCHIVE_ADMIN_B)==0||(strcmp(currentuser.userid,"you")==0))
	{
		usercomplete ("�������ʺ�: ", rela.userid);

		if (!getuser (rela.userid))
		{
			prints ("����ȷ��ʹ���ߴ���\n");
			pressreturn ();
			return 0;
		}
		//	sprintf (fname, "home/%c/%s/relative", toupper (lookupuser.userid[0]),lookupuser.userid);
		sethomefile(fname, lookupuser.userid, "relative");
		usercomplete ("������Է��ʺ�: ", rela.userid);

		if (!getuser (rela.userid))
		{
			prints ("����ȷ��ʹ���ߴ���\n");
			pressreturn ();
			return 0;
		}
		move(3,0);
		prints("1 �� 2 ĸ 3 �� 4 �� 5 �� 6 �� 7 ��  8 �� 9 �� 10 �� \n11 ֶ 12 �� 13 �� 14 �� 15 �� 16 Ů 17 үү 18 �⹫ 19 ���� 20 ����\n 21 ���� 22 ��Ů 23 ���� 24 ����Ů 25 �� 26 �� 27 С�� 28 ɩ 29 ���� 30 ��� \n31 С�� 32 ���� 33 С�� 34 ��� 35 С�� 36 ��� 37 �÷� 38 ���� 39 �� 40 �׼�\n41 ���� 42 ��ĸ 43 ���� 44 ���� 45 ��ϱ 46 Ů�� 47 �� 48 �ø� 49 �� 50 ��ĸ");
		getdata( 8, 0,"������:", msg, 3, DOECHO,  YEA);
		opt = atoi(msg);
		if (!isdigit(msg[0]) || opt <= 0 || opt > 50 || msg[0] == '\n' || msg[0] == '\0')
		{
			move(7,0);
			prints("�Բ����������!\n");
			pressanykey();
			return 0;
		}

		sprintf (rela.relation, relative_list[opt-1]);
		rela.regonizedate=time(0);	
		fd=open(fname,O_APPEND | O_WRONLY | O_CREAT ,0644 );
		write(fd,&rela,sizeof(struct relative));
		close(fd);
	}	
	sprintf (fname,"%s ��������֤��ϵ",currentuser.userid);
	securityreport(fname);
	prints (fname);
	pressreturn ();
	return 0;

	/*else{
		prints ("������˼Դ�����ݹݳ�������Ȩ������Ϣ!\n");
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
			prints ("\n��û������");
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
				sprintf (buf,"%s  %s ��",cgift.name,cgift.id);
				move(4,0);
				prints(buf);
				getdata (1, 0,	"[D]ɾ��  [J] ��һ�� [K] ��һ�� or [E] �뿪: ",ans, 7, DOECHO, YEA);
			}else{
				sprintf (buf,"%s ��������һ�����������ɾ������",cgift.id);
				move(4,0);
				prints(buf);
				getdata (1, 0,	"[J] ��һ�� [K] ��һ�� or [E] �뿪: ",ans, 7, DOECHO, YEA);
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
		usercomplete ("������׷��ʺ�: ", id1);
		if (!getuser (id1))
		{
			if(askyn ("����ȷ��ʹ���ߴ��ţ��Ƿ������", NA, NA) == NA)	return 0;
		}
		//	sprintf (fname, "home/%c/%s/relative", toupper (lookupuser.userid[0]),lookupuser.userid);
		sethomefile(fname, lookupuser.userid, "relative");

		usercomplete ("�������ҷ��ʺ�: ", id2);
		if (!getuser (id2))
		{
			if(askyn ("����ȷ��ʹ���ߴ��ţ��Ƿ������", NA, NA) == NA)	return 0;
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
	sprintf (fname,"%s ����� %s �� %s��֤��ϵ",currentuser.userid,id1,id2);
	securityreport(fname);
	prints (fname);
	pressreturn ();
	return 0;

	/*else{
		prints ("������˼Դ�����ݹݳ�������Ȩ������Ϣ!\n");
		pressreturn ();
		return;
	}*/
}


