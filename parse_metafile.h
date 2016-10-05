#ifndef PARSE_METAFILE
#define PARSE_METAFILE

// ����������ļ��л�ȡ��tracker��URL
typedef struct _Announce_list {
	char    announce[128];
	struct _Announce_list  *next;
} Announce_list;

// ��������������ļ���·���ͳ���
typedef struct _Files {
	char    path[256];
	long    length;
	struct _Files *next;
} Files;

int read_metafile(char *metafile_name);         // ��ȡ�����ļ�
int find_keyword(char *keyword,long *position); // �������ļ��в���ĳ���ؼ���
int read_announce_list();                       // ��ȡ����tracker�������ĵ�ַ
int add_an_announce(char* url);                 // ��tracker�б����һ��URL

int get_piece_length();       // ��ȡÿ��piece�ĳ���,һ��Ϊ256KB
int get_pieces();             // ��ȡ����piece�Ĺ�ϣֵ

int is_multi_files();         // �ж����ص��ǵ����ļ����Ƕ���ļ�
int get_file_name();          // ��ȡ�ļ��������ڶ��ļ�����ȡ����Ŀ¼��
int get_file_length();        // ��ȡ�������ļ����ܳ���
int get_files_length_path();  // ��ȡ�ļ���·���ͳ��ȣ��Զ��ļ�������Ч

int get_info_hash();          // ��info�ؼ��ʶ�Ӧ��ֵ����info_hash
int get_peer_id();            // ����peer_id,ÿ��peer����һ��20�ֽڵ�peer_id

// �ͷ�parse_metafile.c�ж�̬������ڴ�
void release_memory_in_parse_metafile();
// ���ñ��ļ��ж���ĺ���,��ɽ��������ļ�
int  parse_metafile(char *metafile);

#endif
