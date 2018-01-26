#pragma once
#include<iostream>
#include<vector>
#include<list>
#include<sstream>
#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<set>
#include<map>
#include<set>
#include"edge.h"
#include"request.h"
#include"class_edge_hash.h"
using namespace std;

const static int day_start_time = 8;//开始工作的时间
const static int day_end_time = 18;//结束工作的时间
const static double max_interval = 2;//最大时间间隔





//计算隔夜需求的平均速度
int get_one_two_day_request_speed( unordered_set<edge, edge_hash, edge_equal>& all_edge, const request *re_one) {

	auto request_edge = all_edge.find(edge(re_one->start_point, re_one->end_point));
	return request_edge->length /ceil(((day_end_time - re_one->start_time) + (re_one->end_time - day_start_time)));

}

//得到所有的城市
set<string>get_all_city(unordered_set<edge, edge_hash, edge_equal> &all_edge) {
	
	set<string> all_city;
	for (auto one_edge : all_edge) {
		all_city.insert(one_edge.start_point);
	}
	return all_city;
}

bool is_satisfy_1(const edge &first_day_edge, const edge &second_day_edge,const request *re_one,int current_request_speed) {
	return first_day_edge.length + second_day_edge.length <= current_request_speed
		*((day_end_time - re_one->start_time) + (re_one->end_time - day_start_time) + max_interval);
}
bool is_satisfy_2(const edge &first_day_edge, const request *re_one, int current_request_speed) {
	return (first_day_edge.length>=(day_end_time-re_one->start_time- max_interval)*current_request_speed) && (first_day_edge.length<= (day_end_time - re_one->start_time + max_interval)*current_request_speed);
}
//分割一个隔夜需求
void split(vector<request*> &all_split_request_first_day, vector<request*> &all_split_request_second_day, unordered_set<edge, edge_hash, edge_equal> &all_edge,set<string>&all_city,const request *re_one) {
	int current_request_speed = get_one_two_day_request_speed(all_edge,re_one);
	auto request_edge = all_edge.find(edge(re_one->start_point, re_one->end_point));//当前需求的边
	for (auto one_cicy : all_city) {
		auto first_day_edge= all_edge.find(edge(re_one->start_point, one_cicy));
		auto second_day_edge= all_edge.find(edge(one_cicy, re_one->end_point));
		if (is_satisfy_1(*first_day_edge, *second_day_edge, re_one, current_request_speed) && is_satisfy_2(*first_day_edge, re_one, current_request_speed)) {
			//产生新的需求
			request *new_re_first = new request(*re_one);
			new_re_first->end_point = one_cicy;
			new_re_first->end_time = day_end_time;
			all_split_request_first_day.push_back(new_re_first);


			request *new_re_second = new request(*re_one);
			new_re_second->start_point = one_cicy;
			new_re_second->start_time = day_start_time;
			all_split_request_second_day.push_back(new_re_second);

		}
	}
	//没有找到中转点
	if (!all_split_request_first_day.empty())
	{
		cout << "当前参数没有找到中转点，需再次计算" << endl;
		exit(-1);
	}
}
//分割所有的隔夜需求
void split(vector<request*>&all_two_request,vector<request*> &all_split_request_first_day, vector<request*>& all_split_request_second_day, unordered_set<edge, edge_hash, edge_equal>& all_edge) {
	auto all_city = get_all_city(all_edge);
	for (int i = 0; i < all_two_request.size(); i++) {
		split(all_split_request_first_day, all_split_request_second_day,all_edge, all_city, all_two_request.at(i));
	}
}


