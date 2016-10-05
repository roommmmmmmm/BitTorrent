#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "data.h"
#include "tracker.h"
#include "bitfield.h"
#include "torrent.h"
#include "parse_metafile.h"
#include "signal_hander.h"
#include "policy.h"
#include "log.h"

// #define  DEBUG

int main(int argc, char *argv[])
{
	int ret;

	if(argc != 2) {
		printf("usage:%s metafile\n",argv[0]);
		exit(-1);
	}

	// �����źŴ�����
	ret = set_signal_hander();
	if(ret != 0)  { printf("%s:%d error\n",__FILE__,__LINE__); return -1; }

	// ���������ļ�
	ret = parse_metafile(argv[1]);
	if(ret != 0)  { printf("%s:%d error\n",__FILE__,__LINE__); return -1; }

	// ��ʼ��������peer
	init_unchoke_peers();

	// �������ڱ����������ݵ��ļ�
	ret = create_files();
	if(ret != 0)  { printf("%s:%d error\n",__FILE__,__LINE__); return -1; }

	// ����λͼ
	ret = create_bitfield();
	if(ret != 0)  { printf("%s:%d error\n",__FILE__,__LINE__); return -1; }

	// ����������
	ret = create_btcache();
	if(ret != 0)  { printf("%s:%d error\n",__FILE__,__LINE__); return -1; }

	// ����������Peer�շ����ݡ�������Ϣ
	download_upload_with_peers();

	// ��һЩ������,��Ҫ���ͷŶ�̬������ڴ�
	do_clear_work();

	return 0;
}