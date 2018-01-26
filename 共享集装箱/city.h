#include<string>
#include"request.h"
#include<list>

class city {
public:
	std::string name;//城市的名字
	std::list<request*>in_related_request;//到达这个城市的需求
	std::list<request*>out_related_request;//离开这个城市的需求
	double real_start_time = 0;//实际开始时间
	double real_end_time = 0;//实际到达时间
	city(std::string name):name(name){  }
	bool operator==(city other) {
		return (name==other.name) && (real_start_time==other.real_start_time) && (real_end_time==other.real_end_time);
	
	}
};