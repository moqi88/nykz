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
  int size, sum, num,count;

  char filename[STRLEN];
  struct stat st;
  struct userec fboy;
  FILE *fp;
  char secu[STRLEN];

  char buf[3*STRLEN];
  char filebuf[3*STRLEN];
  FILE *target;
  //FILE *fpPASSWD; 
  int fpPASSWD; 
  struct userec luckyid;
  int luckyid_num = 0;
  int prize;//奖金
  char *tmp;//奖金
  prize = 1000;
  size = sizeof (struct userec);
 
  sprintf(filename, "%s/.PASSWDS", BBSHOME);

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
num = 0;
count =0;
  sprintf(filebuf, "%s/%s", BBSHOME, PASSFILE);
 	for(num=0;num<sum;num++){	
	 // sleep(1);
	  fp = fopen (filename, "r");
	  fseek (fp, num * size, SEEK_SET);
	  fread (&fboy, size, 1, fp);
	  fclose (fp);
	  if(strlen(fboy.userid) < 1) continue;
	  if(fboy.inbank < 1) continue;
	  if(!strcmp("SYHBank",fboy.userid)) continue;
		sprintf(buf,"%s %d %d\n",fboy.userid ,fboy.inbank , (int)((double)fboy.inbank *1.0005));
		fboy.inbank = (int)((double)fboy.inbank *1.0005);
		printf(buf);
		count += fboy.money;
		 tmp = (char *)&fboy;
		substitute_record(filebuf, tmp, sizeof(struct userec), num + 1);
	}
	sprintf(buf,"%d\n",count);
	printf(buf);

  return 1;

}
