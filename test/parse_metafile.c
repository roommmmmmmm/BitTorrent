#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "parse_metafile.h"
//#include "sha1.h"

char  *metafile_content = NULL; // 保存种子文件的内容
long  filesize;                 // 种子文件的长度

int       piece_length  = 0;    // 每个piece的长度,通常为256KB即262144字节
char      *pieces       = NULL; // 保存每个pieces的哈希值,每个哈希值为20字节
int       pieces_length = 0;    // pieces缓冲区的长度

int       multi_file    = 0;    // 指明是单文件还是多文件
char      *file_name    = NULL; // 对于单文件,存放文件名;对于多文件,存放目录名
long long file_length   = 0;    // 存放待下载文件的总长度
Files     *files_head   = NULL; // 只对多文件种子有效,存放各个文件的路径和长度

unsigned char info_hash[20];    // 保存info_hash的值,连接tracker和peer时使用
unsigned char peer_id[20];      // 保存peer_id的值,连接peer时使用

Announce_list *announce_list_head = NULL; // 用于保存所有tracker服务器的URL

int parse_metafile(char *metafile)
{
	int ret;

	// 读取种子文件
	ret = read_metafile(metafile);
	if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }

	// 从种子文件中获取tracker服务器的地址
	ret = read_announce_list();
	if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }

	// 判断是否为多文件
	ret = is_multi_files();
	if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }

	// 获取每个piece的长度,一般为256KB
	ret = get_piece_length();
	if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }

	// // 读取各个piece的哈希值
	// ret = get_pieces();
	// if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }

	// // 获取要下载的文件名，对于多文件的种子，获取的是目录名
	// ret = get_file_name();
	// if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }
	//
	// // 对于多文件的种子，获取各个待下载的文件路径和文件长度
	// ret = get_files_length_path();
	// if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }
	//
	// // 获取待下载的文件的总长度
	// ret = get_file_length();
	// if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }
	//
	// // 获得info_hash，生成peer_id
	// ret = get_info_hash();
	// if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }
	// ret = get_peer_id();
	// if(ret < 0) { printf("%s:%d wrong",__FILE__,__LINE__); return -1; }

	return 0;
}
int get_piece_length()
{
	long i;

	if( find_keyword("12:piece length",&i) == 1 ) {
		i = i + strlen("12:piece length");  // skip "12:piece length"
		i++;  // skip 'i'
		while(metafile_content[i] != 'e') {
			piece_length = piece_length * 10 + (metafile_content[i] - '0');
			i++;
		}
	} else {
		return -1;
	}

#ifdef DEBUG
	printf("piece length:%d\n",piece_length);
#endif

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

#ifdef DEBUG
	printf("metafile size is: %ld\n",filesize);
#endif

	return 0;
}

int find_keyword(char *keyword,long *position)
{
	long i;

	*position = -1;
	if(keyword == NULL)  return 0;

	for(i = 0; i < filesize-strlen(keyword); i++) {
		if( memcmp(&metafile_content[i], keyword, strlen(keyword)) == 0 ) {
			*position = i;
			// printf("%d\n", i);
			return 1;
		}
	}

	return 0;
}

int read_announce_list()
{
	Announce_list  *node = NULL;
	Announce_list  *p    = NULL;
	int            len   = 0;
	long           i;

	if( find_keyword("13:announce-list",&i) == 0 ) {
		if( find_keyword("8:announce",&i) == 1 ) {
			i = i + strlen("8:announce");
			while( isdigit(metafile_content[i]) ) {
				len = len * 10 + (metafile_content[i] - '0');
				i++;
			}
			i++;  // 跳过 ':'

			node = (Announce_list *)malloc(sizeof(Announce_list));
			strncpy(node->announce,&metafile_content[i],len);
			node->announce[len] = '\0';
			node->next = NULL;
			announce_list_head = node;
		}
	}
	else {  // 如果有13:announce-list关键词就不用处理8:announce关键词
		i = i + strlen("13:announce-list");
		i++;         // skip 'l'
		while(metafile_content[i] != 'e') {
			i++;     // skip 'l'
			while( isdigit(metafile_content[i]) ) {
				// 提取tracker服务器地址的长度信息
				len = len * 10 + (metafile_content[i] - '0');
				i++;
			}
			// 按照规定，长度过后一定是一个 : 号，不是，则直接返回报错信息
			if( metafile_content[i] == ':' )  i++;
			else  return -1;

			// 只处理以http开头的tracker地址,不处理以udp开头的地址
			if( memcmp(&metafile_content[i],"http",4) == 0 ) {
				node = (Announce_list *)malloc(sizeof(Announce_list));
				strncpy(node->announce,&metafile_content[i],len);
				node->announce[len] = '\0';
				node->next = NULL;

				if(announce_list_head == NULL)
					announce_list_head = node;
				else {
					p = announce_list_head;
					while( p->next != NULL) p = p->next; // 使p指向最后个结点
					p->next = node; // node成为tracker列表的最后一个结点
				}
			}

			i = i + len;
			len = 0;
			i++;    // skip 'e'
			if(i >= filesize)  return -1;
		}
	}

#ifdef DEBUG
	p = announce_list_head;
	while(p != NULL) {
		printf("%s\n",p->announce);
		p = p->next;
	}
#endif

	return 0;
}

int is_multi_files(){
	long i;
	if( find_keyword("5:files",&i) == 1 ) {
		multi_file = 1;
#ifdef DEBUG
	printf("is_multi_files : Yes\n");
#endif
		return 1;
	}
#ifdef DEBUG
	printf("is_multi_files : No!\n");
#endif
	return 0;
}