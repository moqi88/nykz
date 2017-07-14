/****************************************************************/
/* 	this file is created by pax @ syh bbs	        			*/
/* 	每日悲情ID													*/
/****************************************************************/

#include "../include/bbs.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>



int safewrite(int fd, char *buf, int size)
{
	int     cc, sz = size, origsz = size;
	char   *bp = buf;
	
	do {
		cc = write(fd, bp, sz);
		if ((cc < 0) && (errno != EINTR)) {
//			report("safewrite err!");
			return -1;
		}
		if (cc > 0) {
			bp += cc;
			sz -= cc;
		}
	} while (sz > 0);
	return origsz;
}


int get_record(char *filename, char *rptr, int size, int id)
{
	int     fd;
	
	if ((fd = open(filename, O_RDONLY, 0)) == -1) return -1;
	if (lseek(fd, (off_t) (size * (id - 1)), SEEK_SET) == -1) {
		close(fd);
		return -1;
	}
	if (read(fd, rptr, size) != size) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}


int substitute_record(char *filename, char *rptr, int size, int id)
{
	int     fd;
	
	if(id < 1) {
//		report("substitue_record(...) id <= 0");
		return -1;
	}	
	if(id > 70000){ // if id too big, maybe error!
//		report("substitue_record(...) id > 70000");
		return -1;
	}
	if((fd=open(filename,O_WRONLY|O_CREAT,0644))==-1) return -1;
	flock(fd, LOCK_EX);
	if(lseek(fd, (off_t) (size * (id - 1)), SEEK_SET) == -1){ 
//		report("subrec seek err");
		;
	} else {
		if (safewrite(fd, rptr, size) != size)
			;
//			report("subrec write err");
	} 

	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}




int
main()
{
  int size, sum, num;
  int bank_num=0;

  char filename[STRLEN];
  struct stat st;
  struct userec fboy;
  FILE *fp;
  char secu[STRLEN]= {0};

  char buf[3*STRLEN];
  char filebuf[3*STRLEN];
  FILE *target;
  //FILE *fpPASSWD; 
  int fpPASSWD; 
  struct userec luckyid;
  struct userec bankid;
  int luckyid_num = 0;
  int prize;//奖金
  char *tmp;//奖金
  prize = 1000;
  size = sizeof (struct userec);
 
  sprintf(filename, "%s/.PASSWDS", BBSHOME);

  strcpy(secu, "SYHBank");	
  
  if (stat (filename, &st) == -1)
  {

    return;
  }
  sum = st.st_size / size;
	
  if(sum == 0)
  {
	  return;
  }

  fboy.userlevel = 0;
 // while(!(fboy.userlevel & 0400000000))
/*modify by pax@1003*/
	//上站次数 >=50
	//文章数   >=500
	//生命力   >=30

	while((!(fboy.userlevel & 0400000000)) || ((fboy.inbank + fboy.money)<10000))
	{
	  sleep(1);
	  srand (time (NULL));
	  num = rand () % sum;

	  fp = fopen (filename, "r");
	  fseek (fp, num * size, SEEK_SET);
	  fread (&fboy, size, 1, fp);
	  fclose (fp);
	}
	
	//罚款5%
		prize = (fboy.inbank + fboy.money)/10000*500 + (fboy.inbank + fboy.money)/50000*1000 + (fboy.inbank + fboy.money)/100000*2000 + (fboy.inbank + fboy.money)/500000*10000 ; 

  sprintf(buf,"%s/0Announce/bbslist/unluckyid", BBSHOME);
  if ((target = fopen(buf, "w")) == NULL)
  {
	  printf("Can't open unluckyid\n");
	  return;
  }

  fprintf(target, "today's unlucky id is %s\n the fine is %d", fboy.userid,prize);
  fclose(target);
  sprintf(filebuf, "%s/%s", BBSHOME, PASSFILE);

  if ((fpPASSWD = open(filebuf, O_RDONLY, 0)) == -1)
  {
	  printf("error\n");
	  return;
  }
  

  while (read(fpPASSWD, &luckyid, sizeof(luckyid)) == sizeof(luckyid))
  {		
	  if(!strncmp(luckyid.userid, fboy.userid, 12))
		  break;
	  else
		  luckyid_num ++;
  }

  close(fpPASSWD);
  fpPASSWD = NULL;
   printf("%s\n", luckyid.userid);


   printf("secu %s\n", secu);
   fpPASSWD = open(filebuf, O_RDONLY, 0);

  while (read(fpPASSWD, &bankid, sizeof(bankid)) == sizeof(bankid))
  {		
	  if(!strncmp(bankid.userid, secu, 12))
	  {
	  	printf("find it\n");
		  break;
		}
	  else
		  bank_num ++;
  }

  close(fpPASSWD);
   printf("secu %s\n", secu);
   printf("%s\n", bankid.userid);


//  luckyid.money += 1000;
  if(prize < luckyid.money ){
	  luckyid.money -= prize;
  
  }else{
	  luckyid.inbank -= (prize - luckyid.money);
	  luckyid.money = 0;
	}

	printf("%d\n", bankid.money);
	  bankid.money += prize;
	  printf("%d\n", bankid.money);

  tmp = (char *)&luckyid;
  substitute_record(filebuf, tmp, sizeof(struct userec), luckyid_num + 1);
  tmp = (char *)&bankid;
  substitute_record(filebuf, tmp, sizeof(struct userec), bank_num + 1);

  return 1;

}
