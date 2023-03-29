#include"Demo.h"

void Demo::sendRequest(const char* ip, const char* port,const char* request)
{
	Client it;
	it.init();
	it.connectTo(ip, port);
	//�����ַ�ʽ���������ڴ�����ʱ���Զ�����==����
	unique_lock<mutex>unilock(m);
	//����ӡ����������ʱ��ֹͣ��������
	while (Q.size() > MAX_CACHE_LENGTH)
		condInput.wait(unilock);//�ȴ����ѣ��Զ�����m.unlock()
	//��������ѣ��Զ�����m.lock()
	it.sendMessage(request);
	it.close();

	CacheData temp;
	temp.id = ID++;
	temp.data = it.getRecv();
	Q.push(temp);//������ѹ���ӡ����
	condOutput.notify_one();//���Ѵ�ӡ����������ӡ

	//���˳���ʱ���Զ�����
	//ͨ�������Ӧ�����Ƚ����ٻ��������̵߳�
}

void Demo::printData()
{
	//����
	unique_lock<mutex>unilock(m);
	while (Q.empty())
		condOutput.wait(unilock);//�ȴ���ӡ�����г��ִ���ӡ����
	CacheData temp = Q.front();
	Q.pop();
	printf("Data %d is printing:  ", temp.id);
	printf("%s\n", temp.data.c_str());
	//Ϊ�����ֶ��̣߳������ӡ��Ҫ1����
	unilock.unlock();//�ֶ�����
	condInput.notify_one();//���ѷ���������߳�
	Sleep(1000);
}

void Demo::printerAction()
{
	while (flag)
		printData();
}

void Demo::printStart()
{
	//�����߳�
	t = thread(&Demo::printerAction, this);
}

void Demo::powerOn()
{
	flag = true;
}

void Demo::powerOff()
{
	flag = false;
}

