#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include "parse_metafile.h"
#include "bitfield.h"
#include "peer.h"
#include "data.h"
#include "policy.h"
#include "message.h"

#define HANDSHAKE   -2
#define KEEP_ALIVE  -1
#define CHOKE        0
#define UNCHOKE      1
#define INTERESTED   2
#define UNINTERESTED 3
#define HAVE         4
#define BITFIELD     5
#define REQUEST      6
#define PIECE        7
#define CANCEL       8
#define PORT         9

#define KEEP_ALIVE_TIME 45

extern Bitmap *bitmap;
extern char    info_hash[20];
extern char    peer_id[20];
extern int     have_piece_index[64];
extern Peer   *peer_head;

int int_to_char(int i, unsigned char c[4])
{
	c[3] = i%256;
	c[2] = (i-c[3])/256%256;
	c[1] = (i-c[3]-c[2]*256)/256/256%256;
	c[0] = (i-c[3]-c[2]*256-c[1]*256*256)/256/256/256%256;

	return 0;
}

int char_to_int(unsigned char c[4])
{
	int i;

	i = c[0]*256*256*256 + c[1]*256*256 + c[2]*256 + c[3];
	
	return i;
}

int create_handshake_msg(char *info_hash,char *peer_id,Peer *peer)
{
	int            i;
	unsigned char  keyword[20] = "BitTorrent protocol", c = 0x00;
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;

	if(len < 68)  return -1;  // 68Ϊ������Ϣ�Ĺ̶�����

	buffer[0] = 19;
	for(i = 0; i < 19; i++)  buffer[i+1]  = keyword[i];
	for(i = 0; i < 8;  i++)  buffer[i+20] = c;
	for(i = 0; i < 20; i++)  buffer[i+28] = info_hash[i];
	for(i = 0; i < 20; i++)  buffer[i+48] = peer_id[i];

	peer->msg_len += 68;
	return 0;
}

int create_keep_alive_msg(Peer *peer)
{
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;

	if(len < 4)  return -1;  // 4Ϊkeep_alive��Ϣ�Ĺ̶�����

	memset(buffer,0,4);
	peer->msg_len += 4;
	return 0;
}

int create_chock_interested_msg(int type,Peer *peer)
{
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;

	// 5Ϊchoke��unchoke��interested��uninterested��Ϣ�Ĺ̶�����
	if(len < 5)  return -1;

	memset(buffer,0,5);
	buffer[3] = 1;
	buffer[4] = type;

	peer->msg_len += 5;
	return 0;
}

int create_have_msg(int index,Peer *peer)
{
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;
	unsigned char  c[4];

	if(len < 9)  return -1;  // 9Ϊhave��Ϣ�Ĺ̶�����
	
	memset(buffer,0,9);	
	buffer[3] = 5;
	buffer[4] = 4;
	
	int_to_char(index,c);
	buffer[5] = c[0];
	buffer[6] = c[1];
	buffer[7] = c[2];
	buffer[8] = c[3];
	
	peer->msg_len += 9;
	return 0;
}

int create_bitfield_msg(char *bitfield,int bitfield_len,Peer *peer)
{
	int            i;
	unsigned char  c[4];
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;

	if( len < bitfield_len+5 )  {  // bitfield��Ϣ�ĳ���Ϊbitfield_len+5
		printf("%s:%d buffer too small\n",__FILE__,__LINE__); 
		return -1;
	}

	int_to_char(bitfield_len+1,c);
	for(i = 0; i < 4; i++)  buffer[i] = c[i];
	buffer[4] = 5;
	for(i = 0; i < bitfield_len; i++) buffer[i+5] = bitfield[i];

	peer->msg_len += bitfield_len+5;  
	return 0;
}

int create_request_msg(int index,int begin,int length,Peer *peer)
{
	int            i;
	unsigned char  c[4];
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;

	if(len < 17)  return -1;  // 17Ϊrequest��Ϣ�Ĺ̶�����

	memset(buffer,0,17);
	buffer[3] = 13;
	buffer[4] = 6;
	int_to_char(index,c);
	for(i = 0; i < 4; i++)  buffer[i+5]  = c[i];
	int_to_char(begin,c);
	for(i = 0; i < 4; i++)  buffer[i+9]  = c[i];
	int_to_char(length,c);
	for(i = 0; i < 4; i++)  buffer[i+13] = c[i];

	peer->msg_len += 17;
	return 0;
}

int create_piece_msg(int index,int begin,char *block,int b_len,Peer *peer)
{
	int            i;
	unsigned char  c[4];
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;

	if( len < b_len+13 ) {  // piece��Ϣ�ĳ���Ϊb_len+13
		printf("IP:%s len:%d\n",peer->ip,len);
		printf("%s:%d buffer too small\n",__FILE__,__LINE__); 
		return -1;
	}
	
	int_to_char(b_len+9,c);
	for(i = 0; i < 4; i++)      buffer[i]    = c[i];
	buffer[4] = 7;
	int_to_char(index,c);
	for(i = 0; i < 4; i++)      buffer[i+5]  = c[i];
	int_to_char(begin,c);
	for(i = 0; i < 4; i++)      buffer[i+9]  = c[i];
	for(i = 0; i < b_len; i++)  buffer[i+13] = block[i];

	peer->msg_len += b_len+13;  
	return 0;
}

int create_cancel_msg(int index,int begin,int length,Peer *peer)
{
	int            i;
	unsigned char  c[4];
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;
	
	if(len < 17)  return -1;  // 17Ϊcancel��Ϣ�Ĺ̶�����
	
	memset(buffer,0,17);
	buffer[3] = 13;
	buffer[4] = 8;
	int_to_char(index,c);
	for(i = 0; i < 4; i++)  buffer[i+5]  = c[i];
	int_to_char(begin,c);
	for(i = 0; i < 4; i++)  buffer[i+9]  = c[i];
	int_to_char(length,c);
	for(i = 0; i < 4; i++)  buffer[i+13] = c[i];

	peer->msg_len += 17;	
	return 0;
}

int create_port_msg(int port,Peer *peer)
{
	unsigned char  c[4];
	unsigned char  *buffer = peer->out_msg + peer->msg_len;
	int            len = MSG_SIZE - peer->msg_len;

	if( len < 7)  return 0;  // 7Ϊport��Ϣ�Ĺ̶�����

	memset(buffer,0,7);
	buffer[3] = 3;
	buffer[4] = 9;
	int_to_char(port,c);
	buffer[5] = c[2];
	buffer[6] = c[3];

	peer->msg_len += 7;
	return 0;
}

// ��ʮ�����Ƶ���ʽ��ӡ��Ϣ������,���ڵ���
int print_msg_buffer(unsigned char *buffer, int len)
{
	int i;

	for(i = 0; i < len; i++) {
		printf("%.2x ",buffer[i]);
		if( (i+1) % 16 == 0 )  printf("\n");
	}
	printf("\n");

	return 0;
}

// �жϻ��������Ƿ�����һ����������Ϣ
int is_complete_message(unsigned char *buff,unsigned int len,int *ok_len)
{
	unsigned int   i;
	char           btkeyword[20];

	unsigned char  keep_alive[4]   = { 0x0, 0x0, 0x0, 0x0 };
	unsigned char  chocke[5]       = { 0x0, 0x0, 0x0, 0x1, 0x0};
	unsigned char  unchocke[5]     = { 0x0, 0x0, 0x0, 0x1, 0x1};
	unsigned char  interested[5]   = { 0x0, 0x0, 0x0, 0x1, 0x2};
	unsigned char  uninterested[5] = { 0x0, 0x0, 0x0, 0x1, 0x3};
	unsigned char  have[5]         = { 0x0, 0x0, 0x0, 0x5, 0x4};
	unsigned char  request[5]      = { 0x0, 0x0, 0x0, 0xd, 0x6};
	unsigned char  cancel[5]       = { 0x0, 0x0, 0x0, 0xd, 0x8};
	unsigned char  port[5]         = { 0x0, 0x0, 0x0, 0x3, 0x9};
	
	if(buff==NULL || len<=0 || ok_len==NULL)  return -1;
	*ok_len = 0;
	
	btkeyword[0] = 19;
	memcpy(&btkeyword[1],"BitTorrent protocol",19);  // BitTorrentЭ��ؼ���

	unsigned char  c[4];
	unsigned int   length;
	
	for(i = 0; i < len; ) {
		// ���֡�chocke��have����Ϣ�ĳ����ǹ̶���
		if( i+68<=len && memcmp(&buff[i],btkeyword,20)==0 )         i += 68;
		else if( i+4 <=len && memcmp(&buff[i],keep_alive,4)==0 )    i += 4;
		else if( i+5 <=len && memcmp(&buff[i],chocke,5)==0 )        i += 5;
		else if( i+5 <=len && memcmp(&buff[i],unchocke,5)==0 )      i += 5;
		else if( i+5 <=len && memcmp(&buff[i],interested,5)==0 )    i += 5;
		else if( i+5 <=len && memcmp(&buff[i],uninterested,5)==0 )  i += 5;
		else if( i+9 <=len && memcmp(&buff[i],have,5)==0 )          i += 9;
		else if( i+17<=len && memcmp(&buff[i],request,5)==0 )       i += 17;
		else if( i+17<=len && memcmp(&buff[i],cancel,5)==0 )        i += 17;
		else if( i+7 <=len && memcmp(&buff[i],port,5)==0 )          i += 7;
		// bitfield��Ϣ�ĳ����Ǳ仯��
		else if( i+5 <=len && buff[i+4]==5 )  {
			c[0] = buff[i];   c[1] = buff[i+1];
			c[2] = buff[i+2]; c[3] = buff[i+3];
			length = char_to_int(c);	
			// ��Ϣ����ռ4�ֽ�,��Ϣ����ռlength���ֽ�
			if( i+4+length <= len )  i += 4+length;
			else { *ok_len = i; return -1; }
		}
		// piece��Ϣ�ĳ���Ҳ�Ǳ仯��
		else if( i+5 <=len && buff[i+4]==7 )  {
			c[0] = buff[i];   c[1] = buff[i+1];
			c[2] = buff[i+2]; c[3] = buff[i+3];
			length = char_to_int(c);
			// ��Ϣ����ռ4�ֽ�,��Ϣ����ռlength���ֽ�
			if( i+4+length <= len )  i += 4+length;
			else { *ok_len = i; return -1; }
		}
		else {
			// ����δ֪���͵���Ϣ
			if(i+4 <= len) {
				c[0] = buff[i];   c[1] = buff[i+1];
				c[2] = buff[i+2]; c[3] = buff[i+3];
				length = char_to_int(c);
				// ��Ϣ����ռ4�ֽ�,��Ϣ����ռlength���ֽ�
				if(i+4+length <= len)  { i += 4+length; continue; }
				else { *ok_len = i; return -1; }
			}
			// ���Ҳ����δ֪��Ϣ����,����ΪĿǰ���յ����ݻ�����һ����������Ϣ
			*ok_len = i;
			return -1;
		}
	}
	
	*ok_len = i;
	return 1;
}

int process_handshake_msg(Peer *peer,unsigned char *buff,int len)
{
	if(peer==NULL || buff==NULL)  return -1;

	if(memcmp(info_hash,buff+28,20) != 0) { 
		peer->state = CLOSING;
		// �������ͻ������е�����
		discard_send_buffer(peer);
		clear_btcache_before_peer_close(peer);
		close(peer->socket);
		return -1;
	}
	
	memcpy(peer->id,buff+48,20);
	(peer->id)[20] = '\0';
	
	if(peer->state == INITIAL) {
		peer->state = HANDSHAKED;
		create_handshake_msg(info_hash,peer_id,peer);
	}
	if(peer->state == HALFSHAKED)  peer->state = HANDSHAKED;

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_keep_alive_msg(Peer *peer,unsigned char *buff,int len)
{
	if(peer==NULL || buff==NULL)  return -1;

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_choke_msg(Peer *peer,unsigned char *buff,int len)
{
	if(peer==NULL || buff==NULL)  return -1;

	if( peer->state!=CLOSING && peer->peer_choking==0 ) {
		peer->peer_choking = 1;
		peer->last_down_timestamp = 0;
		peer->down_count          = 0;
		peer->down_rate           = 0;
	}

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_unchoke_msg(Peer *peer,unsigned char *buff,int len)
{
	if(peer==NULL || buff==NULL)  return -1;

	if( peer->state!=CLOSING && peer->peer_choking==1 ) {
		peer->peer_choking = 0;
		if(peer->am_interested == 1)  create_req_slice_msg(peer);
		else {
			peer->am_interested = is_interested(&(peer->bitmap), bitmap);
			if(peer->am_interested == 1) create_req_slice_msg(peer);
			else printf("Received unchoke but Not interested to IP:%s \n",peer->ip);
		}

		peer->last_down_timestamp = 0;
		peer->down_count          = 0;
		peer->down_rate           = 0;
	}

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_interested_msg(Peer *peer,unsigned char *buff,int len)
{
	if(peer==NULL || buff==NULL)  return -1;

	if( peer->state!=CLOSING && peer->state==DATA ) {
		peer->peer_interested = is_interested(bitmap, &(peer->bitmap));
		if(peer->peer_interested == 0)  return -1;
		if(peer->am_choking == 0) create_chock_interested_msg(1,peer);
	}

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_uninterested_msg(Peer *peer,unsigned char *buff,int len)
{
	if(peer==NULL || buff==NULL)  return -1;

	if( peer->state!=CLOSING && peer->state==DATA ) {
		peer->peer_interested = 0;
		cancel_requested_list(peer);
	}

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_have_msg(Peer *peer,unsigned char *buff,int len)
{
	int           rand_num;
	unsigned char c[4];

	if(peer==NULL || buff==NULL)  return -1;

	srand(time(NULL));
	rand_num = rand() % 3;

	if( peer->state!=CLOSING && peer->state==DATA ) {
		c[0] = buff[5]; c[1] = buff[6];
		c[2] = buff[7]; c[3] = buff[8];		
		if(peer->bitmap.bitfield != NULL)
			set_bit_value(&(peer->bitmap),char_to_int(c),1);

		if(peer->am_interested == 0) {
			peer->am_interested = is_interested(&(peer->bitmap), bitmap);
			// ��ԭ���Ķ�peer������Ȥ��Ϊ����Ȥʱ,��interested��Ϣ
			if(peer->am_interested == 1) create_chock_interested_msg(2,peer);	
		} else {  // �յ�����have��һ��interested��Ϣ
			if(rand_num == 0) create_chock_interested_msg(2,peer);
		}
	}

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_cancel_msg(Peer *peer,unsigned char *buff,int len)
{
	unsigned char c[4];
	int           index, begin, length;

	if(peer==NULL || buff==NULL)  return -1;
	
	c[0] = buff[5];  c[1] = buff[6];
	c[2] = buff[7];  c[3] = buff[8];
	index = char_to_int(c);
	c[0] = buff[9];  c[1] = buff[10];
	c[2] = buff[11]; c[3] = buff[12];
	begin = char_to_int(c);
	c[0] = buff[13]; c[1] = buff[14];
	c[2] = buff[15]; c[3] = buff[16];
	length = char_to_int(c);
	
	Request_piece *p, *q;
	p = q = peer->Requested_piece_head;
	while(p != NULL) { 
		if( p->index==index && p->begin==begin && p->length==length ) {
			if(p == peer->Requested_piece_head) 
				peer->Requested_piece_head = p->next;
			else
				q->next = p->next;
			free(p);
			break;
		}
		q = p;
		p = p->next;
	}	

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_bitfield_msg(Peer *peer,unsigned char *buff,int len)
{
	unsigned char c[4];

	if(peer==NULL || buff==NULL)  return -1;
	if(peer->state==HANDSHAKED || peer->state==SENDBITFIELD) {
		c[0] = buff[0];   c[1] = buff[1];
		c[2] = buff[2];   c[3] = buff[3];			

		if( peer->bitmap.bitfield != NULL ) {
			free(peer->bitmap.bitfield);
			peer->bitmap.bitfield = NULL;
		}
		peer->bitmap.valid_length = bitmap->valid_length;
		if(bitmap->bitfield_length != char_to_int(c)-1) {
			peer->state = CLOSING;
			// �������ͻ������е�����
			discard_send_buffer(peer);
			clear_btcache_before_peer_close(peer);
			close(peer->socket);
			return -1;
		}
		peer->bitmap.bitfield_length = char_to_int(c) - 1;
		peer->bitmap.bitfield = 
			(unsigned char *)malloc(peer->bitmap.bitfield_length);
		memcpy(peer->bitmap.bitfield,&buff[5],peer->bitmap.bitfield_length);
	
		// ���ԭ״̬Ϊ������,�յ�λͼ��Ӧ����peer��λͼ
		if(peer->state == HANDSHAKED) {
			create_bitfield_msg(bitmap->bitfield,bitmap->bitfield_length,peer);
			peer->state = DATA;
		}
		// ���ԭ״̬Ϊ�ѷ���λͼ���յ�λͼ�����׼����������
		if(peer->state == SENDBITFIELD) {
			peer->state = DATA;
		}

		// �ж�peer�Ƿ�����Ǹ���Ȥ
		peer->peer_interested = is_interested(bitmap,&(peer->bitmap));
		// �ж϶�peer�Ƿ����Ȥ,��������interested��Ϣ
		peer->am_interested = is_interested(&(peer->bitmap), bitmap);
		if(peer->am_interested == 1) create_chock_interested_msg(2,peer);
	}
	
	peer->start_timestamp = time(NULL);
	return 0;
}

int process_request_msg(Peer *peer,unsigned char *buff,int len)
{
	unsigned char  c[4];
	int            index, begin, length;
	Request_piece  *request_piece, *p;
	
	if(peer==NULL || buff==NULL)  return -1;

	if(peer->am_choking==0 && peer->peer_interested==1) {
		c[0] = buff[5];  c[1] = buff[6];
		c[2] = buff[7];  c[3] = buff[8];
		index = char_to_int(c);
		c[0] = buff[9];  c[1] = buff[10];
		c[2] = buff[11]; c[3] = buff[12];
		begin = char_to_int(c);
		c[0] = buff[13]; c[1] = buff[14];
		c[2] = buff[15]; c[3] = buff[16];
		length = char_to_int(c);

		// �����slice����
		if( begin%(16*1024) != 0 ) {
			return 0;
		}
		
		// �鿴�������Ƿ��Ѵ���,���Ѵ���,�򲻽��д���
		p = peer->Requested_piece_head;
		while(p != NULL) {
			if(p->index==index && p->begin==begin && p->length==length) {
				break;
			}
			p = p->next;
		}
		if(p != NULL)  return 0;

		// ��������뵽���������
		request_piece = (Request_piece *)malloc(sizeof(Request_piece));
		if(request_piece == NULL)  { 
			printf("%s:%d error",__FILE__,__LINE__); 
			return 0; 
		}
		request_piece->index  = index;
		request_piece->begin  = begin;
		request_piece->length = length;
		request_piece->next   = NULL;
		if( peer->Requested_piece_head == NULL ) 
			peer->Requested_piece_head = request_piece;
		else {
			p = peer->Requested_piece_head;
			while(p->next != NULL)  p = p->next;
			p->next = request_piece;
		}
		//printf("*** add a request FROM IP:%s index:%-6d begin:%-6x ***\n",
		//       peer->ip,index,begin);
	}

	peer->start_timestamp = time(NULL);
	return 0;
}

int process_piece_msg(Peer *peer,unsigned char *buff,int len)
{
	unsigned char  c[4];
	int            index, begin, length;
	Request_piece  *p;

	if(peer==NULL || buff==NULL)  return -1;
	
	if(peer->peer_choking==0) {
		c[0] = buff[0];    c[1] = buff[1];
		c[2] = buff[2];    c[3] = buff[3];
		length = char_to_int(c) - 9;
		c[0] = buff[5];    c[1] = buff[6];
		c[2] = buff[7];    c[3] = buff[8];
		index = char_to_int(c);
		c[0] = buff[9];    c[1] = buff[10];
		c[2] = buff[11];   c[3] = buff[12];
		begin = char_to_int(c);

		p = peer->Request_piece_head;
		while(p != NULL) {
			if(p->index==index && p->begin==begin && p->length==length)
				break;
			p = p->next;
		}
		if(p == NULL) {printf("did not found matched request\n"); return -1;}

		if(peer->last_down_timestamp == 0)
			peer->last_down_timestamp = time(NULL);
		peer->down_count += length;
		peer->down_total += length;

		write_slice_to_btcache(index,begin,length,buff+13,length,peer);

		create_req_slice_msg(peer);
	}

	peer->start_timestamp = time(NULL);
	return 0;
}

int parse_response(Peer *peer)
{
	unsigned char  btkeyword[20];
	unsigned char  keep_alive[4] = { 0x0, 0x0, 0x0, 0x0 };
	int            index;
	unsigned char  *buff = peer->in_buff;
	int            len = peer->buff_len;

	if(buff==NULL || peer==NULL)  return -1;

	btkeyword[0] = 19;
	memcpy(&btkeyword[1],"BitTorrent protocol",19);  // BitTorrentЭ��ؼ���

	// �ֱ���12����Ϣ
	for(index = 0; index < len; ) {	

		if( (len-index >= 68) && (memcmp(&buff[index],btkeyword,20) == 0) ) {
			process_handshake_msg(peer,buff+index,68);
			index += 68;
		} 
		else if( (len-index >= 4) && (memcmp(&buff[index],keep_alive,4) == 0)){
			process_keep_alive_msg(peer,buff+index,4);
			index += 4; 
		}
		else if( (len-index >= 5) && (buff[index+4] == CHOKE) ) {
			process_choke_msg(peer,buff+index,5);
			index += 5;
		}
		else if( (len-index >= 5) && (buff[index+4] == UNCHOKE) ) {
			process_unchoke_msg(peer,buff+index,5);
			index += 5;
		}
		else if( (len-index >= 5) && (buff[index+4] == INTERESTED) ) {
			process_interested_msg(peer,buff+index,5);
			index += 5;
		}
		else if( (len-index >= 5) && (buff[index+4] == UNINTERESTED) ) {
			process_uninterested_msg(peer,buff+index,5);
			index += 5;
		}
		else if( (len-index >= 9) && (buff[index+4] == HAVE) ) {
			process_have_msg(peer,buff+index,9);
			index += 9;
		}
		else if( (len-index >= 5) && (buff[index+4] == BITFIELD) ) {
			process_bitfield_msg(peer,buff+index,peer->bitmap.bitfield_length+5);
			index += peer->bitmap.bitfield_length + 5;
		}
		else if( (len-index >= 17) && (buff[index+4] == REQUEST) ) {
			process_request_msg(peer,buff+index,17);
			index += 17;
		}
		else if( (len-index >= 13) && (buff[index+4] == PIECE) ) {
			unsigned char  c[4];
			int            length;
			
			c[0] = buff[index];    c[1] = buff[index+1];
			c[2] = buff[index+2];  c[3] = buff[index+3];
			length = char_to_int(c) - 9;
			
			process_piece_msg(peer,buff+index,length+13);
			index += length + 13; // length+13Ϊpiece��Ϣ�ĳ���
		}
		else if( (len-index >= 17) && (buff[index+4] == CANCEL) ) {
			process_cancel_msg(peer,buff+index,17);
			index += 17;
		}
		else if( (len-index >= 7) && (buff[index+4] == PORT) ) {
			index += 7;
		}
		else {
			// �����δ֪����Ϣ����,���������账��
			unsigned char c[4];
			int           length;
			if(index+4 <= len) {
				c[0] = buff[index];   c[1] = buff[index+1];
				c[2] = buff[index+2]; c[3] = buff[index+3];
				length = char_to_int(c);
				if(index+4+length <= len)  { index += 4+length; continue; }
			}
			// �����һ���������Ϣ,��ս��ջ�����
			peer->buff_len = 0;
			return -1;
		}
	} // end for

	// ���ջ������е���Ϣ������Ϻ�,��ս��ջ�����
	peer->buff_len = 0;

	return 0;
}

int parse_response_uncomplete_msg(Peer *p,int ok_len)
{
	char *tmp_buff;
	int   tmp_buff_len;

	// ����洢�ռ�,��������ջ������в���������Ϣ
	tmp_buff_len = p->buff_len - ok_len;
	if(tmp_buff_len <= 0)  return -1;
	tmp_buff = (char *)malloc(tmp_buff_len);
	if(tmp_buff == NULL) {
		printf("%s:%d error\n",__FILE__,__LINE__);
		return -1;
	}
	memcpy(tmp_buff,p->in_buff+ok_len,tmp_buff_len);
	
	// ������ջ�������ǰ����������Ϣ
	p->buff_len = ok_len;
	parse_response(p);

	// ������������Ϣ���������ջ������Ŀ�ʼ��
	memcpy(p->in_buff,tmp_buff,tmp_buff_len);
	p->buff_len = tmp_buff_len;
	if(tmp_buff != NULL)  free(tmp_buff);

	return 0;
}

// ��������һ��pieceʱ,Ӧ�������е�peer����have��Ϣ
int prepare_send_have_msg()
{
	Peer *p = peer_head;
	int  i;

	if(peer_head == NULL)  return -1;
	if(have_piece_index[0] == -1)  return -1;

	while(p != NULL) {
		for(i = 0; i < 64; i++) {
			if(have_piece_index[i] != -1) {
				create_have_msg(have_piece_index[i],p);
			}
			else {
				break;
			}
		}
		p = p->next;
	}

	for(i = 0; i < 64; i++) {
		if(have_piece_index[i] == -1) {
			break;
		}
		else {
			have_piece_index[i] = -1;
		}
	}
	
	return 0;
}

// �����������͸�peer����Ϣ,�����ǵ��յ�ĳ����Ϣ��������Ӧ
int create_response_message(Peer *peer)
{
	if(peer==NULL)  return -1;

	if(peer->state == INITIAL) {
		create_handshake_msg(info_hash,peer_id,peer);
		peer->state = HALFSHAKED;
		return 0;
	}

	if(peer->state == HANDSHAKED) {
		if(bitmap == NULL)  return -1;
		create_bitfield_msg(bitmap->bitfield,bitmap->bitfield_length,peer);
		peer->state = SENDBITFIELD;
		return 0;
	}

	// ����piece��Ϣ,�����������ļ�������
	if( peer->am_choking==0 && peer->Requested_piece_head!=NULL ) {
		Request_piece *req_p = peer->Requested_piece_head;
		int ret = read_slice_for_send(req_p->index,req_p->begin,req_p->length,peer);
		if(ret < 0 ) { printf("read_slice_for_send ERROR\n");}
		else {
			if(peer->last_up_timestamp == 0) 
				peer->last_up_timestamp = time(NULL);
			peer->up_count += req_p->length;
			peer->up_total += req_p->length;
			peer->Requested_piece_head = req_p->next;

			//printf("********* sending a slice TO:%s index:%-5d begin:%-5x *********\n",
			//peer->ip,req_p->index,req_p->begin);

			free(req_p);
			return 0;
		}
	}

	// ���3����û���յ��κ���Ϣ�ر�����
	time_t now = time(NULL);  // ��ȡ��ǰʱ��
	long interval1 = now - peer->start_timestamp;
	if( interval1 > 180 ) {
		peer->state = CLOSING;
		discard_send_buffer(peer);  // �������ͻ������е�����
		clear_btcache_before_peer_close(peer);
		close(peer->socket);
	}
	// ���45��û�з��ͺͽ�����Ϣ,����һ��keep_alive��Ϣ
	long interval2 = now - peer->recet_timestamp;
	if( interval1>45 && interval2>45 && peer->msg_len==0)
		create_keep_alive_msg(peer);

	return 0;
}

void discard_send_buffer(Peer *peer)
{
	struct linger  lin;
	int            lin_len;
	
	lin.l_onoff  = 1;
	lin.l_linger = 0;
	lin_len      = sizeof(lin);
	
	if(peer->socket > 0) {
		setsockopt(peer->socket,SOL_SOCKET,SO_LINGER,(char *)&lin,lin_len);
	}
}
