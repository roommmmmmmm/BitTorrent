#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int read_metafile(char *metafile_name);         // 读取种子文件

char  *metafile_content = NULL; // 保存种子文件的内容
long filesize;

int main(int argc, char const *argv[]) {
  int   len = 0;
  long  i ;
  char announce[128];
  if(read_metafile("test.torrent") == 0){
    if (find_keyword("13:announce-list",&i) == 0) {
    // if (find_keyword("13:announce",&i) == 0) {
      printf("find nothing\n");
    }else{
      i = i + strlen("13:announce-list");
      ++i;
      while (metafile_content[i] != 'e') {
        ++i;
        while(isdigit(metafile_content[i])){
          len = len * 10 + (metafile_content[i] - '0');
          ++i;
        }
        // printf("%d\n",len );
        // printf("%d\n",i );
        // printf("%c\n",metafile_content[i]);
        if (metafile_content[i] == ':') ++i;
        else return -1;


        if( memcmp(&metafile_content[i],"http",4) == 0 ){
          strncpy(announce,&metafile_content[i],len);
          announce[len] = '\0';
          printf("%s\n",announce );
        }
        i = i + len;
        len = 0;
        ++i;
        if(i >= filesize)  return -1;
        // exit(1);
      }

      printf("find it\n");
    }
    printf("read_metafile ok\n");
  }else{
    printf("read_metafile No\n");
  }
  return 0;
}
int find_keyword(char *keyword,long *position){
	long i;

	*position = -1;
	if(keyword == NULL)  return 0;
	for(i = 0; i < filesize-strlen(keyword); i++) {
		if( memcmp(&metafile_content[i], keyword, strlen(keyword)) == 0 ) {
			*position = i;
			return 1;
		}
	}

	return 0;
}

int read_metafile(char *metafile_name)
{
	long  i;

	// 以二进制、只读的方式打开文件
	FILE *fp = fopen(metafile_name,"rb");
	if(fp == NULL) {
		printf("%s:%d can not open file\n",__FILE__,__LINE__);
		return -1;
	}

	// 获取种子文件的长度
	fseek(fp,0,SEEK_END);
	filesize = ftell(fp);
	if(filesize == -1) {
		printf("%s:%d fseek failed\n",__FILE__,__LINE__);
		return -1;
	}

	metafile_content = (char *)malloc(filesize+1);
	if(metafile_content == NULL) {
		printf("%s:%d malloc failed\n",__FILE__,__LINE__);
		return -1;
	}

	// 读取种子文件的内容到metafile_content缓冲区中
	fseek(fp,0,SEEK_SET);
	for(i = 0; i < filesize; i++)
		metafile_content[i] = fgetc(fp);
	metafile_content[i] = '\0';

	fclose(fp);

	printf("metafile size is: %ld\n",filesize);

	return 0;
}
