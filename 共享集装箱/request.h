#pragma once
#include <string>
#include <iostream>
#include <iomanip>
class request {
public:
	int id;//ÿ������Ψһ�ı��
	std::string start_point;//���
	std::string end_point;//�յ�
	double start_time = 0;//�ͻ�������ʼʱ��
	double end_time = 0;//�ͻ��������ʱ��
	int group_id;//һ����ҹ�����ֺ�� group id ��ͬ
	bool const operator==(request other) const {//�ж������Ƿ����
		return (start_point == other.start_point)&&(end_point == other.end_point)&&this->time_equal(other);
	}
	bool const time_equal(request other) const {//�ж�ʱ�䴰�Ƿ����
		return start_time == other.start_time&&end_time == other.end_time;
	}
	
};
					
