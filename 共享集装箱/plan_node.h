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

	int request_size;//�Ѿ����������ĸ���

	std::vector<city>real_path;//��ʵ·��
	std::vector<bool> mark_visited;//��������Ƿ�����

	int total_request;

	double cost = 0;


	plan_node(int total_request) :total_request(total_request) {
		request_size = 1;
		mark_visited.resize(total_request, false);
	}
};