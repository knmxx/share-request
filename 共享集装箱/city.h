#include<string>
#include"request.h"
#include<list>

class city {
public:
	std::string name;//���е�����
	std::list<request*>in_related_request;//����������е�����
	std::list<request*>out_related_request;//�뿪������е�����
	double real_start_time = 0;//ʵ�ʿ�ʼʱ��
	double real_end_time = 0;//ʵ�ʵ���ʱ��
	city(std::string name):name(name){  }
	bool operator==(city other) {
		return (name==other.name) && (real_start_time==other.real_start_time) && (real_end_time==other.real_end_time);
	
	}
};