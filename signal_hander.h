#ifndef SIGNAL_HANDER_H
#define SIGNAL_HANDER_H

// ��һЩ������,���ͷŶ�̬������ڴ�
void do_clear_work();

// ����һЩ�ź�
void process_signal(int signo);

// �����źŴ�����
int set_signal_hander();

#endif