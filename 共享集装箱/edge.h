#pragma once

#include<string>
class edge {
public:
	int id;
	std::string start_point;//起点
	std::string end_point;//终点
	double length;//长度
	
	edge(std::string s = "", std::string e = "",double l=0,int id=0):start_point(s),end_point(e),length(l),id(id) {
		
	}

};
