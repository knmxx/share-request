#pragma once
#include <string>
#include <iostream>
#include <iomanip>
class request {
public:
	int id;//每个需求唯一的编号
	std::string start_point;//起点
	std::string end_point;//终点
	double start_time = 0;//客户需求起始时间
	double end_time = 0;//客户需求结束时间
	int group_id;//一个隔夜需求拆分后的 group id 相同
	bool const operator==(request other) const {//判断需求是否相等
		return (start_point == other.start_point)&&(end_point == other.end_point)&&this->time_equal(other);
	}
	bool const time_equal(request other) const {//判断时间窗是否相等
		return start_time == other.start_time&&end_time == other.end_time;
	}
	
};
					
