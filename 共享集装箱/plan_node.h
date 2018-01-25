#pragma once
#include<memory>
#include<vector>
#include<string>
#include"edge.h"
#include"request.h"
#include<bitset>
#include"city.h"
class plan_node {
public:

	int request_size;//已经服务的需求的个数

	std::vector<city>real_path;//真实路径
	std::vector<bool> mark_visited;//标记需求是否服务过

	int total_request;

	double cost = 0;


	plan_node(int total_request) :total_request(total_request) {
		request_size = 1;
		mark_visited.resize(total_request, false);
	}
};