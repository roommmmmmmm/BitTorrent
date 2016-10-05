#ifndef POLICY_H
#define POLICY_H
#include "peer.h"

// ���ļ�ʵ����bittorrentЭ���һЩ�ؼ��㷨,��Ҫ�У�
// ��ˮ��ҵ(һ�����ɶ�ĳ��peer�Ķ��slice����,һ��Ϊ5��)
// Ƭ��ѡ���㷨(��Բ�ͬ�����ؽ׶�,�в�ͬ��ѡ�����)
// �����㷨(�����ٶ�ѡ�������peer)�Լ�ѡ���Ż�������peer
// �ж��Ƿ��������(����λͼ�����ж�,������ϼ���ֹ����)

// ÿ��10�����һ�θ���peer���ϴ������ٶ�
#define COMPUTE_RATE_TIME  10
// ���½ṹ��洢�����ٶ�����4��peer��ָ��
#define UNCHOKE_COUNT  4
// ÿ�������slice��
#define REQ_SLICE_NUM  5

typedef struct _Unchoke_peers {
	Peer*  unchkpeer[UNCHOKE_COUNT];
	int    count;
	Peer*  optunchkpeer;
} Unchoke_peers;


void init_unchoke_peers();     // ��ʼ��ȫ�ֱ���unchoke_peers


int select_unchoke_peer();     // ѡ��unchoke peer
int select_optunchoke_peer();  // ��peer������ѡ��һ���Ż�������peer


int compute_rate();            // �������һ��ʱ��(10��)ÿ��peer���ϴ������ٶ�
int compute_total_rate();      // �����ܵ��ϴ������ٶ�


int is_seed(Peer *node);       // �ж�ĳ��peer�Ƿ�Ϊ����

// ������������,Ϊ�����Ч��һ������5��slice
int create_req_slice_msg(Peer *node);  

#endif