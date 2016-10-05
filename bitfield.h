#ifndef BITFIELD_H
#define BITFIELD_H

typedef struct _Bitmap {
	unsigned char *bitfield;       // ����λͼ
	int           bitfield_length; // λͼ��ռ�����ֽ���
	int           valid_length;    // λͼ��Ч����λ��,ÿһλ����һ��piece
} Bitmap;

int  create_bitfield();                       // ����λͼ,�����ڴ沢���г�ʼ��
int  get_bit_value(Bitmap *bitmap,int index); // ��ȡĳһλ��ֵ
int  set_bit_value(Bitmap *bitmap,int index,
				   unsigned char value);      // ����ĳһλ��ֵ
int  all_zero(Bitmap *bitmap);                // ȫ������
int  all_set(Bitmap *bitmap);                 // ȫ������Ϊ1
void release_memory_in_bitfield();            // �ͷ�bitfield.c�ж�̬������ڴ�
int  print_bitfield(Bitmap *bitmap);          // ��ӡλͼֵ,���ڵ���

int  restore_bitmap(); // ��λͼ�洢���ļ��� 
                       // ���´�����ʱ,�ȶ�ȡ���ļ���ȡ�Ѿ����صĽ���
int  is_interested(Bitmap *dst,Bitmap *src);  // ӵ��λͼsrc��peer�Ƿ��ӵ��
                                              // dstλͼ��peer����Ȥ
int  get_download_piece_num(); // ��ȡ��ǰ�����ص�����piece��

#endif
