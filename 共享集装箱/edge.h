#pragma once

#include<string>
class edge {
public:
	int id;
	std::string start_point;//���
	std::string end_point;//�յ�
	double length;//����
	
	edge(std::string s = "", std::string e = "",double l=0,int id=0):start_point(s),end_point(e),length(l),id(id) {
		
	}

};
