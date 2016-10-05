#ifndef MESSAGE_H
#define MESSAGE_H
#include "peer.h"

int int_to_char(int i, unsigned char c[4]);  // �����ͱ���i���ĸ��ֽڴ�ŵ�����c��
int char_to_int(unsigned char c[4]);         // ������c�е��ĸ��ֽ�ת��Ϊһ��������

// ���º��������������͵���Ϣ
int create_handshake_msg(char *info_hash,char *peer_id,Peer *peer);
int create_keep_alive_msg(Peer *peer);
int create_chock_interested_msg(int type,Peer *peer);
int create_have_msg(int index,Peer *peer);
int create_bitfield_msg(char *bitfield,int bitfield_len,Peer *peer);
int create_request_msg(int index,int begin,int length,Peer *peer);
int create_piece_msg(int index,int begin,char *block,int b_len,Peer *peer);
int create_cancel_msg(int index,int begin,int length,Peer *peer);
int create_port_msg(int port,Peer *peer);

// ��ӡ��Ϣ�������е���Ϣ, ���ڵ���
int print_msg_buffer(unsigned char *buffer, int len);
// Ϊ����have��Ϣ��׼��,have��Ϣ��Ϊ����,��Ҫ���͸�����peer
int prepare_send_have_msg();
// �жϻ��������Ƿ�����һ����������Ϣ
int is_complete_message(unsigned char *buff,unsigned int len,int *ok_len);
// �����յ�����Ϣ,���ջ������д����һ����������Ϣ
int parse_response(Peer *peer);
// �����ܵ�����Ϣ,���ջ������г��˴��һ����������Ϣ��,���в���������Ϣ
int parse_response_uncomplete_msg(Peer *p,int ok_len);
// ������Ӧ��Ϣ
int create_response_message(Peer *peer);
// ������peer�Ͽ�ʱ,�������ͻ������е���Ϣ
void discard_send_buffer(Peer *peer);

#endif