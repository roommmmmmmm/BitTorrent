#ifndef PEER_H
#define PEER_H

#include <string.h>
#include <time.h>
#include "bitfield.h"

#define  INITIAL              -1  // �������ڳ�ʼ��״̬
#define  HALFSHAKED            0  // �������ڰ�����״̬
#define  HANDSHAKED            1  // ��������ȫ����״̬
#define  SENDBITFIELD          2  // ���������ѷ���λͼ״̬
#define  RECVBITFIELD          3  // ���������ѽ���λͼ״̬
#define  DATA                  4  // ����������peer�������ݵ�״̬
#define  CLOSING               5  // �������ڼ�����peer�Ͽ���״̬

// ���ͺͽ��ջ������Ĵ�С,16K���Դ��һ��slice,2K���Դ��������Ϣ
#define  MSG_SIZE  2*1024+16*1024

typedef struct _Request_piece {
	int     index;                // �����piece������
	int     begin;                // �����piece��ƫ��
	int     length;               // ����ĳ���,һ��Ϊ16KB
	struct _Request_piece *next;
} Request_piece;

typedef struct  _Peer {
	int            socket;                // ͨ����socket��peer����ͨ��
	char           ip[16];                // peer��ip��ַ
	unsigned short port;                  // peer�Ķ˿ں�
	char           id[21];                // peer��id

	int            state;                 // ��ǰ������״̬

	int            am_choking;            // �Ƿ�peer����
	int            am_interested;         // �Ƿ��peer����Ȥ
	int            peer_choking;          // �Ƿ�peer����
	int            peer_interested;       // �Ƿ�peer����Ȥ

	Bitmap         bitmap;                // ���peer��λͼ
	
	char           *in_buff;              // ��Ŵ�peer����ȡ����Ϣ
	int            buff_len;              // ������in_buff�ĳ���
	char           *out_msg;              // ��Ž����͸�peer����Ϣ
	int            msg_len;               // ������out_msg�ĳ���
	char           *out_msg_copy;         // out_msg�ĸ���,����ʱʹ�øû�����
	int            msg_copy_len;          // ������out_msg_copy�ĳ���
	int            msg_copy_index;        // ��һ��Ҫ���͵����ݵ�ƫ����

	Request_piece  *Request_piece_head;   // ��peer�������ݵĶ���
	Request_piece  *Requested_piece_head; // ��peer�������ݵĶ���

	unsigned int   down_total;            // �Ӹ�peer���ص����ݵ��ܺ�
	unsigned int   up_total;              // ���peer�ϴ������ݵ��ܺ�

	time_t         start_timestamp;       // ���һ�ν��յ�peer��Ϣ��ʱ��
	time_t         recet_timestamp;       // ���һ�η�����Ϣ��peer��ʱ��

	time_t         last_down_timestamp;   // ����������ݵĿ�ʼʱ��
	time_t         last_up_timestamp;     // ����ϴ����ݵĿ�ʼʱ��
	long long      down_count;            // ����ʱ���ڴ�peer���ص����ݵ��ֽ���
	long long      up_count;              // ����ʱ������peer�ϴ������ݵ��ֽ���
	float          down_rate;             // ����ʱ���ڴ�peer���������ݵ��ٶ�
	float          up_rate;               // ����ʱ������peer���ϴ����ݵ��ٶ�

	struct _Peer   *next;                 // ָ����һ��Peer�ṹ��
} Peer;

int   initialize_peer(Peer *peer);        // ��peer���г�ʼ��
Peer* add_peer_node();                    // ���һ��peer���
int   del_peer_node(Peer *peer);          // ɾ��һ��peer���
void  free_peer_node(Peer *node);         // �ͷ�һ��peer���ڴ�

int   cancel_request_list(Peer *node);    // ������ǰ�������
int   cancel_requested_list(Peer *node);  // ������ǰ���������

void  release_memory_in_peer();           // �ͷ�peer.c�еĶ�̬������ڴ�
void  print_peers_data();                 // ��ӡpeer������ĳЩ��Ա��ֵ,���ڵ���

#endif