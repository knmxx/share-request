#pragma once
#include<iostream>
#include<vector>
#include<list>
#include<sstream>
#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<map>
#include<set>
#include"edge.h"
#include"request.h"
#include"class_edge_hash.h"
using namespace std;

const  static int  start_index = 1;
//当日需求
vector<request*> get_request_one_day() {
	fstream rq("request_one_day.txt");
	vector<request*>all_requset;
	stringstream one_line_stream;
	string one_line;
	while (getline(rq, one_line)) {
		stringstream  one_line_stream(one_line);
		request one;
		string end_time;
		string start_time;
		one_line_stream >> one.id>>one.start_point >> one.end_point>>start_time>>end_time;
		//需求开始时间
		auto s_f = start_time.find(':');
		start_time.replace(s_f, 1, ".");
		one.start_time = stod(start_time);
		//需求结束时间
		auto e_f = end_time.find(':');
		end_time.replace(e_f,1,".");
		one.end_time = stod(end_time);
		request *p_request =new request(one);
		all_requset.push_back(std::move(p_request));//requset中未实现移动构造函数
	}
	for (int i = all_requset.size()-1; i >=0 ; i--) {
		all_requset.at(i)->group_id = all_requset.at(i)->id;
		all_requset.at(i)->id -= start_index;
		
	}
	return all_requset;
}
//隔夜需求
vector<request*> get_request_two_day() {
	fstream rq("request_two_day.txt");
	vector<request*>all_requset;
	stringstream one_line_stream;
	string one_line;
	while (getline(rq, one_line)) {
		stringstream  one_line_stream(one_line);
		request one;
		string end_time;
		string start_time;
		one_line_stream >> one.id >> one.start_point >> one.end_point >> start_time >> end_time;
		//需求开始时间
		auto s_f = start_time.find(':');
		start_time.replace(s_f, 1, ".");
		one.start_time = stod(start_time);
		//需求结束时间
		auto e_f = end_time.find(':');
		end_time.replace(e_f, 1, ".");
		end_time = end_time.substr(4);
		one.end_time = stod(end_time);
		request *p_request = new request(one);
		all_requset.push_back(std::move(p_request));//requset中未实现移动构造函数
	}
	for (int i = all_requset.size() - 1; i >= 0; i--) {
		all_requset.at(i)->group_id = all_requset.at(i)->id;
		all_requset.at(i)->id -= start_index;
	}
	return all_requset;
}
unordered_set<edge,edge_hash, edge_equal> get_edge() {
	set<string>total_city;//城市的个数
	fstream eg("map.txt");
	unordered_set<edge,edge_hash,edge_equal>all_edge;
	stringstream one_line_stream;
	string one_line;
	while (getline(eg, one_line)) {
		stringstream  one_line_stream(one_line);
		edge one;
		one_line_stream >> one.id >> one.start_point >> one.end_point>>one.length;
		total_city.insert(one.start_point);
		all_edge.insert(std::move(one));//edge中未实现移动构造函数
	}
	for (auto one_edge : all_edge) {
		one_edge.id -= 1;//标号从0开始
	}
	//添加自环边
	int current_edge_number = all_edge.size();
	for (auto one_city : total_city) {
		all_edge.insert(std::move(edge(one_city, one_city,0, current_edge_number++)));//edge中未实现移动构造函数
	}
	return all_edge;
	
}
//int main() {
//	get_request_two_day();
//}