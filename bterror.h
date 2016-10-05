#ifndef BTERROR_H
#define BTERROR_H

#define FILE_FD_ERR					-1  	// ��Ч���ļ�������
#define FILE_READ_ERR				-2  	// ���ļ�ʧ��
#define FILE_WRITE_ERR				-3  	// д�ļ�ʧ��
#define INVALID_METAFILE_ERR		-4  	// ��Ч�������ļ�
#define INVALID_SOCKET_ERR			-5  	// ��Ч���׽���
#define INVALID_TRACKER_URL_ERR		-6  	// ��Ч��Tracker URL
#define INVALID_TRACKER_REPLY_ERR	-7  	// ��Ч��Tracker��Ӧ
#define INVALID_HASH_ERR			-8  	// ��Ч��hashֵ
#define INVALID_MESSAGE_ERR			-9  	// ��Ч����Ϣ
#define INVALID_PARAMETER_ERR		-10 	// ��Ч�ĺ�������
#define FAILED_ALLOCATE_MEM_ERR		-11 	// ���붯̬�ڴ�ʧ��
#define NO_BUFFER_ERR				-12		// û���㹻�Ļ�����
#define READ_SOCKET_ERR				-13 	// ���׽���ʧ��
#define WRITE_SOCKET_ERR			-14 	// д�׽���ʧ��
#define RECEIVE_EXIT_SIGNAL_ERR		-15 	// ���յ��˳�������ź�


// ������ʾ�����ԵĴ���,������ֹ
void btexit(int errno,char *file,int line);

#endif