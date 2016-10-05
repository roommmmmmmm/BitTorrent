#ifndef DATA_H
#define DATA_H
#include "peer.h"

// ÿ��Btcache���ά��һ������Ϊ16KB�Ļ�����,�û���������һ��slice������
typedef struct _Btcache {
	unsigned char   *buff;        // ָ�򻺳�����ָ��
	int             index;        // �������ڵ�piece�������
	int             begin;        // ������piece���е���ʼλ��
	int             length;       // ���ݵĳ���

	unsigned char   in_use;       // �û������Ƿ���ʹ����
	unsigned char   read_write;   // �Ƿ��͸�peer�����ݻ��ǽ��յ�������
	                              // �������Ǵ�Ӳ�̶���,read_writeֵΪ0
	                              // �����ݽ�Ҫд��Ӳ��,read_writeֵΪ1
	unsigned char   is_full;      // �������Ƿ���
	unsigned char   is_writed;    // �������е������Ƿ��Ѿ�д�뵽Ӳ����
	int             access_count; // �Ըû������ķ��ʼ���

	struct _Btcache *next;
} Btcache;

Btcache* initialize_btcache_node();  // ΪBtcache�������ڴ�ռ䲢���г�ʼ��
int  create_btcache();               // �����ܴ�СΪ16K*1024��16MB�Ļ�����
void release_memory_in_btcache();    // �ͷ�data.c�ж�̬������ڴ�

int get_files_count();               // ��ȡ�����ļ��д����ص��ļ�����
int create_files();                  // ���������ļ��е���Ϣ���������������ݵ��ļ�

// �ж�һ��Btcache����е�����Ҫд���ĸ��ļ����ĸ�λ��,��д��
int write_btcache_node_to_harddisk(Btcache *node);
// ��Ӳ�̶���һ��slice�����ݴ�ŵ���������,��peer��Ҫʱ���͸�peer
// Ҫ����slice������index��begin��length�Ѵ浽node��ָ��Ľ����
int read_slice_from_harddisk(Btcache *node);
// ���һ��piece�������Ƿ���ȷ,����ȷ��д��Ӳ���ϵ��ļ�
int write_piece_to_harddisk(int sequence,Peer *peer);
// ��Ӳ���ϵ��ļ��ж�ȡһ��piece��ŵ�p��ָ��Ļ�������
int read_piece_from_harddisk(Btcache *p, int index);

// ��16MB�������������ص�����д�뵽Ӳ���ϵ��ļ���
int write_btcache_to_harddisk(Peer *peer);
// ��������������ʱ,�ͷ���Щ��Ӳ���϶�ȡ��piece
int release_read_btcache_node(int base_count);
// ��btcache�������������Щδ������ص�piece
void clear_btcache_before_peer_close(Peer *peer);

// ���ոմ�peer����ȡ��һ��slice��ŵ���������
int write_slice_to_btcache(int index,int begin,int length,
						   unsigned char *buff,int len,Peer *peer);
// �ӻ�������ȡһ��slice,��ȡ��slice��ŵ�peer�ķ��ͻ�������
int read_slice_for_send(int index,int begin,int length,Peer *peer);


// ������Ϊ���غ��ϴ����һ��piece�����ӵĺ��� 
// ���һ��piece��Ϊ����,��Ϊ����һ����������piece
int write_last_piece_to_btcache(Peer *peer);
int write_slice_to_last_piece(int index,int begin,int length,
							  unsigned char *buff,int len,Peer *peer);
int read_last_piece_from_harddisk(Btcache *p, int index);
int read_slice_for_send_last_piece(int index,int begin,int length,Peer *peer);
void release_last_piece();

#endif