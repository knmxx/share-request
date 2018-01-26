#include<iostream>
#include<vector>
#include<list>
#include<stack>
#include<ctime>
#include<algorithm>
#include<unordered_set>
#include<chrono>
#include<fstream>
#include <iomanip>
#include<map>
#include <ilcplex/ilocplex.h>

#include"plan_node.h"
#include"edge.h"
#include"request.h"
#include"class_edge_hash.h"


using namespace std;

int total_request_one_day = 0;//当天需求总数
int total_request_two_day=0;//隔夜需求总数

int total_request_two_day_after_splice=0;//隔夜需求分解后总数
int total_request=0;//总的需求数目



const static double max_interval = 2;//最大时间间隔
const static int max_length = 200;//允许合并的最长线路
const static double speed = 50;//车辆速度


extern unordered_set<edge, edge_hash, edge_equal> get_edge();//所有的边
extern vector<request*> get_request_one_day();//所有的需求
extern void split(vector<request*>&all_two_request,vector<request*> &all_split_request_first_day, vector<request*>& all_split_request_second_day, unordered_set<edge, edge_hash, edge_equal>& all_edge);
extern vector<request*> get_request_two_day();//隔夜需求

int total_plan = 0;//方案总数                       
list<plan_node>all_plan_node_global;//存储所有可行的方案
//本文件声明
bool is_legal_path(plan_node &current_plan, const unordered_set<edge, edge_hash, edge_equal>&all_edge);
void save_result(list<plan_node>all_plan_node);
void start_with_one_request(const vector<request*>&all_request, const unordered_set<edge, edge_hash, edge_equal>&all_edge, request *r_start, int request_start_time) {
	list<plan_node>all_plan_node;//所有的方案
	list<plan_node>can_trans_plan_node;//待处理的方案
	plan_node p_start(total_request);
	p_start.mark_visited[r_start->id] = true;
	p_start.real_path.push_back(city(r_start->start_point));//当前需求的起点
	p_start.real_path.push_back(city(r_start->end_point));//当前需求的终点
	p_start.real_path.front().out_related_request.push_back(r_start);//起点城市关联当前需求
	p_start.real_path.back().in_related_request.push_back(r_start);//终点城市关联当前需求
	if (is_legal_path(p_start, all_edge)) {
		//合法
		all_plan_node.push_back(p_start);
		can_trans_plan_node.push_back(p_start);
	}
	while (!can_trans_plan_node.empty())
	{
		plan_node current_plan = can_trans_plan_node.front();
		can_trans_plan_node.pop_front();
		//cout << can_trans_plan_node.size() << endl;
		for (auto const &current_request : all_request) {
			//当前方案已经服务过当前的需求
			if (current_plan.mark_visited[current_request->id] == true)
				continue;
			string current_plan_start_city_name = current_plan.real_path.front().name;//处理过路径的起点
			string current_plan_end_city_name = current_plan.real_path.back().name;//处理过路径的终点
			 //正向判断相同
			bool has_same = false;
			int same_start_index = -1;
			for (int i = 0; i < current_plan.real_path.size() - 1; i++) {
				if (current_plan.real_path.at(i).name == current_request->start_point) {
					if (current_plan.real_path.at(i + 1).name == current_request->end_point) {
						has_same = true;
						same_start_index = i;
						break;
					}
				}
			}
			//重合
			if (has_same == true) {
				plan_node p_new(current_plan);//产生新的方案
				p_new.mark_visited[current_request->id] = true;
				p_new.real_path.at(same_start_index).out_related_request.push_back(r_start);
				p_new.real_path.at(same_start_index + 1).in_related_request.push_back(r_start);
				if (is_legal_path(p_new, all_edge)) {
					//合法
					all_plan_node.push_back(p_new);
					can_trans_plan_node.push_back(p_new);

				}
			}
			//起点相同
			if (current_plan_start_city_name == current_request->start_point) {
				//终点有交集
				bool has_same = false;
				for (auto i = 0; i < current_plan.real_path.size(); i++) {
					if (current_plan.real_path.at(i).name == current_request->end_point) {
						has_same = true;
						plan_node p_new_insertion(current_plan);//产生新的方案
						p_new_insertion.real_path.at(i).in_related_request.push_back(current_request);
						p_new_insertion.real_path.front().out_related_request.push_back(current_request);
						p_new_insertion.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new_insertion, all_edge)) {
							//合法
							all_plan_node.push_back(p_new_insertion);
							can_trans_plan_node.push_back(p_new_insertion);
						}
					}
				}
				//除起点外没有交集
				if (has_same == false) {
					auto request_path = all_edge.find(edge(current_request->start_point, current_request->end_point));
					//放在中间
					//带插入的城市
					city being_insert(current_request->end_point);
					being_insert.in_related_request.push_back(current_request);
					for (auto city_index = 1; city_index < current_plan.real_path.size(); city_index++) {
						city curreny_city = current_plan.real_path.at(city_index);
						city pre_city = current_plan.real_path.at(city_index - 1);
						auto old_path_edge_1 = all_edge.find(edge(pre_city.name, curreny_city.name));//不插入的花费
						auto new_path_edge_1 = all_edge.find(edge(current_request->end_point, curreny_city.name));
						auto new_path_edge_2 = all_edge.find(edge(pre_city.name, current_request->end_point));
						if (new_path_edge_1->length + new_path_edge_2->length > old_path_edge_1->length + request_path->length + max_length)
							continue;
						//产生新的方案
						plan_node p_new(current_plan);
						//找到插入位置
						auto insert_posttion = p_new.real_path.begin() + city_index;
						//插入
						p_new.real_path.insert(insert_posttion, being_insert);
						p_new.real_path.front().out_related_request.push_back(current_request);
						p_new.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new, all_edge)) {
							//合法
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
					//放在最后
					auto end_path = all_edge.find(edge(current_plan.real_path.back().name, current_request->end_point));
					if (end_path->length <= request_path->length + 200) {
						plan_node p_new(current_plan);//产生新的方案
													  //插入新的终点
						p_new.real_path.push_back(being_insert);
						p_new.real_path.front().out_related_request.push_back(current_request);
						p_new.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new, all_edge)) {
							//合法
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
				}
			}
			//终点相同,不在起点插入
			if (current_plan_end_city_name == current_request->end_point) {
				//起点有交集
				bool has_same = false;
				for (auto i = 0; i < current_plan.real_path.size(); i++) {
					if (current_plan.real_path.at(i).name == current_request->start_point) {
						has_same = true;
						plan_node p_new_insertion(current_plan);//产生新的方案
						p_new_insertion.real_path.back().in_related_request.push_back(current_request);
						p_new_insertion.real_path.at(i).out_related_request.push_back(current_request);
						p_new_insertion.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new_insertion, all_edge)) {
							//合法
							all_plan_node.push_back(p_new_insertion);
							can_trans_plan_node.push_back(p_new_insertion);
						}
					}
				}
				//除终点外没有任何交集
				if (has_same == false) {
					auto request_path = all_edge.find(edge(current_request->start_point, current_request->end_point));
					city being_insert(current_request->start_point);//插入的城市
					being_insert.out_related_request.push_back(current_request);
					//中间插入
					for (int city_index =1;city_index<current_plan.real_path.size();city_index++)
					{
						city pre_city = current_plan.real_path.at(city_index-1);
						city current_city = current_plan.real_path.at(city_index);
						plan_node p_new(current_plan);
						auto old_path_1 = all_edge.find(edge(pre_city.name, current_city.name));
						auto new_path_1 = all_edge.find(edge(pre_city.name, current_request->start_point));
						auto new_path_2 = all_edge.find(edge(current_request->start_point, current_city.name));
						if ((new_path_1->length + new_path_2->length) > (request_path->length + old_path_1->length + max_length))
							continue;
						auto insert_position = p_new.real_path.begin() + city_index;
						p_new.real_path.insert(insert_position, being_insert);
						p_new.real_path.back().in_related_request.push_back(current_request);
						p_new.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new, all_edge)) {
							//合法
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
					//首部插入
					auto first_path = all_edge.find(edge(current_request->start_point, current_plan.real_path.front().name));
					if (first_path->length <= request_path->length + max_length) {
						plan_node p_new(current_plan);//产生新的方案
						//插入新的终点
						p_new.real_path.insert(p_new.real_path.begin(),being_insert);
						p_new.real_path.back().in_related_request.push_back(current_request);
						p_new.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new, all_edge)) {
							//合法
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
				}
			}
			//a->b  b->d
			if (current_plan_end_city_name == current_request->start_point) {
				plan_node p_new(current_plan);//产生新的方案
				p_new.mark_visited[current_request->id] = true;
				p_new.real_path.back().out_related_request.push_back(current_request);
				p_new.real_path.push_back(city(current_request->end_point));//新的终点
				p_new.real_path.back().in_related_request.push_back(current_request);
				if (is_legal_path(p_new, all_edge)) {
					//合法
					all_plan_node.push_back(p_new);
					can_trans_plan_node.push_back(p_new);

				}
			}
			//a->b  c->d
			else {
				auto adjacent_edge = all_edge.find(edge(current_plan_end_city_name, current_request->start_point));
				if (adjacent_edge == all_edge.end()) {
					cerr << "adjacent edge not found " << endl;
					exit(-1);
				}
				else 
				{
					//相邻长度小于200
					if (adjacent_edge->length <= max_length) {
						plan_node p_new(current_plan);//产生新的方案
						p_new.mark_visited[current_request->id] = true;
						p_new.real_path.push_back(city(current_request->start_point));
						p_new.real_path.back().out_related_request.push_back(current_request);
						p_new.real_path.push_back(city(current_request->end_point));//新的终点
						p_new.real_path.back().in_related_request.push_back(current_request);
						if (is_legal_path(p_new, all_edge)) {
							//合法
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
				}
			}
		}
	}
	total_plan += all_plan_node.size();
	//cout << all_plan_node.size() << endl;
	all_plan_node_global.splice(all_plan_node_global.end(), all_plan_node);
	//save_result(all_plan_node);

}
//end time
//到达这个城市的需求的最小到达时间
double in_request_min_time(city current_city) {
	double in_min_time = std::numeric_limits<double>::max();
	for (auto const one_request : current_city.in_related_request) {
		in_min_time = min(in_min_time, one_request->end_time);
	}
	return in_min_time;
}
//end time
//到达这个城市的需求的最大到达时间
double in_request_max_time(city current_city) {
	double in_max_time = std::numeric_limits<double>::min();
	for (auto const one_request : current_city.in_related_request) {
		in_max_time = max(in_max_time, one_request->end_time);
	}
	return in_max_time;
}
//start time
//离开这个城市的需求的最小离开时间
double out_request_min_time(city current_city) {
	double out_min_time = std::numeric_limits<double>::max();
	for (auto const one_request : current_city.out_related_request) {
		out_min_time = min(out_min_time, one_request->start_time);
	}
	return out_min_time;
}
//start time
//离开这个城市的需求的最大离开时间
double out_request_max_time(city current_city) {
	double out_max_time = std::numeric_limits<double>::min();
	for (auto const one_request : current_city.out_related_request) {
		out_max_time = max(out_max_time, one_request->start_time);
	}
	return out_max_time;
}
//实际开始时间
double get_real_start_time(city current_city,double real_end_time=0) {
	
	return max(real_end_time, max(in_request_max_time(current_city), out_request_max_time(current_city)));
}
//实际到达时间
double get_real_end_time(const city pre, city current_city, const unordered_set<edge, edge_hash, edge_equal>&all_edge) {
	//s/v
	//检查pre city与当前city间是否有需求
	bool has_request_pre_and_current = false;
	for (auto const each_pre_city_request : pre.out_related_request) {
		if (each_pre_city_request->end_point == current_city.name) {
			has_request_pre_and_current = true;
			break;
		}
	}
	if (has_request_pre_and_current == false) {
		auto adjacent_edge = all_edge.find(edge(pre.name, current_city.name));
		if (adjacent_edge == all_edge.end()) {
			cerr << "get_real_end_time() not found edge" << endl;
			exit(-1);
		}
		else
			return pre.real_start_time + ceil(adjacent_edge->length / speed);
	}
	//pre->current
	//相邻两个城市在这条线路上存在需求
	else {
		for (auto const each_pre_city_request : pre.out_related_request) {
			if (each_pre_city_request->end_point == current_city.name) {
				return pre.real_start_time + (each_pre_city_request->end_time - each_pre_city_request->start_time);
			}
		}
		cerr << "get_real_end_time() not found request" << endl;
		exit(-1);
	}
}
//终点只需要rule_1
bool rule_1(city city_in_one_path) {
	return city_in_one_path.real_end_time - in_request_min_time(city_in_one_path) <= max_interval;
}
//起点只需要rule_2
bool rule_2(city city_in_one_path) {
	return out_request_max_time(city_in_one_path) - out_request_min_time(city_in_one_path) <= max_interval;
}
bool rule_3(city city_in_one_path) {
	return max(city_in_one_path.real_end_time, in_request_max_time(city_in_one_path)) - out_request_min_time(city_in_one_path) <= max_interval;
}
//一条路径有一个隔夜需求的多个中转点，认为不合法
bool rule_4(plan_node &current_plan) { 
	map<int, int>group_count;
	for (auto current_city : current_plan.real_path) {
		for (auto curreny_city_in_request : current_city.in_related_request) {
			group_count[curreny_city_in_request->group_id]++;
		}
	}
	for (auto one_count : group_count)
		if (one_count.second > 1)
			return false;
	return true;
}
//需要计算每个城市的实际时间
bool is_legal_path(plan_node &current_plan, const unordered_set<edge, edge_hash, edge_equal>&all_edge) {
	//一条路径有一个隔夜需求的多个中转点，认为不合法
	if (rule_4(current_plan)==false)
		return false;

	auto &start_city = current_plan.real_path.front();
	//起点不合法
	if (rule_2(start_city)==false) {
		return false;
	}
	//计算起点的实际开始时间和实际到达时间
	start_city.real_start_time=get_real_start_time(start_city);
	start_city.real_end_time = get_real_start_time(start_city);
	//检测中间节点的合法性
	for (size_t i = 1; i < current_plan.real_path.size() - 1; i++) {
		//计算当前节点的到达时间
		current_plan.real_path.at(i).real_end_time = get_real_end_time(current_plan.real_path.at(i - 1),//前一个城市
																		current_plan.real_path.at(i), //当前城市
																		all_edge);	
		//当前城市的实际到达时间
		int current_city_real_end_time = current_plan.real_path.at(i).real_end_time;
		//计算当前城市的开始时间
		current_plan.real_path.at(i).real_start_time = get_real_start_time(current_plan.real_path.at(i), current_city_real_end_time);
		if (rule_1(current_plan.real_path.at(i)) == false) {
			return false;
		}
		if (rule_2(current_plan.real_path.at(i)) == false) {
			return false;
		}
		if (rule_3(current_plan.real_path.at(i)) == false) {
			return false;
		}
		//当前节点合法
		
	}
	//计算终点的实际到达时间
	int current_plan_length = current_plan.real_path.size();
	current_plan.real_path.back().real_end_time = get_real_end_time(current_plan.real_path.at(current_plan_length - 2),//前一个城市
																		current_plan.real_path.back(), //当前城市
																		all_edge);
	//当前路径的终点
	auto &end_city = current_plan.real_path.back();
	//终点不合法
	if (rule_1(end_city) == false) {
		return false;
	}
	return true;
}
//输出到文件所有的方案
void save_result(list<plan_node>all_plan_node) {
	ofstream rs("rs.txt",ios::app);
	for (auto one_rs : all_plan_node) {
		rs << "---------------------" << endl;
		rs << "request " << endl;
		for (auto one_re : one_rs.real_path) {
			for (auto i : one_re.out_related_request) {
				rs << i->start_point << "-->" << i->end_point << " " << i->start_time << "-->" << i->end_time << endl;
			}
		}
		rs << "path " << endl;
		for (auto one_re : one_rs.real_path) {
			rs << one_re.name;
			if (one_re == one_rs.real_path.back())
				break;
			rs<< "--->";
		}
		rs << endl;
		rs << "real start time  " << endl;
		for (auto one_re : one_rs.real_path) {
			rs << one_re.real_start_time;
			if (one_re == one_rs.real_path.back())
				break;
			rs<< "---";
		}
		rs << endl;
		rs << "real end time  " << endl;
		for (auto one_re : one_rs.real_path) {

			rs << one_re.real_end_time;
			if (one_re == one_rs.real_path.back())
				break;
			rs<< "---";
		}
		rs << endl;

	}
}
//输出一个方案到文件
void save_one(plan_node one_rs,int &request_index) {
	ofstream rs("rs.txt", ios::app);
	rs << "---------------------" << endl;
	rs << "request:" << endl;
	for (auto one_re : one_rs.real_path) {
		for (auto i : one_re.out_related_request) {
			rs << setw(8) << std::internal<< "R" + to_string(request_index++)+":";
			rs << setw(8) << std::internal << i->start_point;
			rs << "-->"; 
			rs << setw(8) << std::internal << i->end_point;
			rs << " ";
			rs << setw(8) << std::internal << i->start_time;
			rs << "-->";
			rs << setw(8) << std::internal << i->end_time;
			rs << setw(8) << std::internal << "需求 id";
			rs << setw(8) << std::internal << i->group_id << endl;;
		}
	}
	rs << "path cost:" << one_rs.cost << endl;
	rs << "path:" << endl;
	for (auto one_re : one_rs.real_path) {
		rs << setw(8) << std::internal << one_re.name;
		if (one_re == one_rs.real_path.back())
			break;
		rs << "-->";
	}
	rs << endl;
	rs << "real start time:" << endl;
	for (auto one_re : one_rs.real_path) {
		rs << setw(8) << std::internal << one_re.real_start_time;
		if (one_re == one_rs.real_path.back())
			break;
		rs << "---";
	}
	rs << endl;
	rs << "real end time:" << endl;
	for (auto one_re : one_rs.real_path) {
		rs << setw(8) << std::internal << one_re.real_end_time;
		if (one_re == one_rs.real_path.back())
			break;
		rs << "---";
	}
	rs << endl;
}
//计算每个方案的代价
void get_cost(list<plan_node>&all_plan_node_global, const unordered_set<edge, edge_hash, edge_equal>&all_edge) {
	for (auto & one_plan : all_plan_node_global) {
		for (auto i = 1; i < one_plan.real_path.size(); i++) {
			one_plan.cost += all_edge.find(edge(one_plan.real_path.at(i - 1).name, one_plan.real_path.at(i).name))->length;
		}
		one_plan.cost += 200;
	}
}
//计算每天的cost
void save_total_cost(double total_cost,int total_request) {
	ofstream rs("rs.txt", ios::app);
	rs << "###############################################" << endl;
	rs << "total cost:" << total_cost << endl;
	rs << "total request:"<<total_request << endl;
}
//打印最优解
void save_optimization_result(const IloNumArray&vals) {
	int val_index = 0;//x的下标
	int request_index = 1;//需求的编号
	for (auto one : all_plan_node_global) {
		if (vals[val_index] == 1) {
			save_one(one, request_index);
		}
		val_index++;
	}
}
//目标函数
void insert_object_function(IloExpr &obj, IloEnv &env, IloNumVarArray &x, IloModel &model) {
	int x_index = 0;
	for (auto one_plan : all_plan_node_global) {
		obj += x[x_index++] * one_plan.cost;
	}
	model.add(IloMinimize(env, obj));
}
//不需要合并的需求
void insert_constraint_one_day(map<int, int>count_group, vector<request*> &all_request, IloEnv &env, IloNumVarArray &x, IloModel &model) {
	for (int row_index = 0; row_index < total_request; row_index++) {
		//不需要合并的需求
		if (count_group.find(all_request.at(row_index)->group_id)->second == 1) {
			IloExpr expr(env);
			int col_index = 0;
			for (auto one_plan : all_plan_node_global) {
				expr += x[col_index++] * one_plan.mark_visited[row_index];

			}
			model.add(expr == 1);
		}
	}
}
//需要合并的隔夜需求
void insert_constraint_two_day(map<int, int>count_group, vector<request*> &all_request, IloEnv &env, IloNumVarArray &x, IloModel &model) {
	for (auto one_count : count_group) {
		//隔夜需求拆分成了多个
		if (one_count.second > 1) {
			vector<int> one_row_coff(total_plan, 0);//合并后的系数
			for (int row_index = 0; row_index< total_request; row_index++) {
				if (one_count.first == all_request.at(row_index)->group_id) {
					int col_index = 0;
					for (auto plan_index : all_plan_node_global) {
						one_row_coff[col_index] = one_row_coff[col_index] || plan_index.mark_visited[row_index];
						col_index++;
					}
				}
			}
			IloExpr expr(env);
			int col_index = 0;
			while (col_index<total_plan) {
				expr += x[col_index] * one_row_coff[col_index];
				col_index++;
			}
			model.add(expr == 1);
		}
	}

}
//根据隔夜需求第一天的选择来确认第二天的需求
void get_second_real_request(vector<request*> &all_split_request_second_day, IloNumArray &vals) {
	vector<request*>second_request;
	//遍历第二天的隔夜需求
	for (int second_request_index = 0; second_request_index < all_split_request_second_day.size(); second_request_index++) {
		//遍历所有方案
		int col_index = 0;
		for (auto one_plan : all_plan_node_global) {
			//最优解中的方案
			if (vals[col_index] == 1) {
				//遍历方案中的需求
				for (auto current_plan_city : one_plan.real_path) {
					for (auto current_city_in_request : current_plan_city.in_related_request) {
						//group id 相同
						if (current_city_in_request->group_id == all_split_request_second_day.at(second_request_index)->group_id) {
							//第一天的终点与第二天的起点相同
							if (current_city_in_request->end_point == all_split_request_second_day.at(second_request_index)->start_point) {
								second_request.push_back(all_split_request_second_day.at(second_request_index));
							}
						}
					}
				}
			}
			col_index++;
		}
	}
	all_split_request_second_day = second_request;
}

void create_model(vector<request*> &all_request, vector<request*> &all_split_request_second_day) {
	IloEnv env;
	IloNumVarArray x(env);
	IloModel model(env);
	for (int x_index = 0; x_index < total_plan; x_index++) {
		x.add(IloNumVar(env, 0, 1, ILOINT));
	}
	map<int, int>count_group;
	//一个groupc存在多个需求，但只能满足一个
	for (auto one_re : all_request) {
		count_group[one_re->group_id]++;
	}
	insert_constraint_one_day(count_group, all_request, env, x, model);
	//合并隔夜需求 隔夜被拆分成多个，需要check
	insert_constraint_two_day(count_group, all_request, env, x, model);
	IloExpr obj(env);
	insert_object_function(obj, env,x,model);
	IloCplex cplex(model);
	cplex.solve();
	//bool ismip = cplex.isMIP();
	//cout << " Is Mip " << ismip << endl;
	//env.out() << "Solution status = " << cplex.getStatus() << endl;
	//env.out() << "Solution value  = " << cplex.getObjValue() << endl;
	save_total_cost(cplex.getObjValue(),total_request);
	IloNumArray vals(env);
	cplex.getValues(vals, x);
	save_optimization_result(vals);
	//根据拆分的选择 更新隔夜第二天的需求
	get_second_real_request(all_split_request_second_day, vals);
}
//给需求重新编号
void reset_request_id(vector<request*> &all_request) {
	int request_index = 0;
	for (auto &one : all_request){
		one->id = request_index++;
	}
}
void test() {
	unordered_set<edge, edge_hash, edge_equal> all_edge = get_edge();//图中所有的边
	vector<request*> all_request = get_request_one_day();//当日需求

	/*for (int i = 0; i < all_request.size(); i++) {
		all_request.at(i)->group_id = 1;
	}*/
	vector<request*> all_two_request = get_request_two_day();//隔夜需求
	vector<request*> all_split_request_first_day;//隔夜需求第一天
	vector<request*> all_split_request_second_day;//隔夜需求第二天
	total_request_one_day = all_request.size();//当天需求总数
	total_request_two_day = all_two_request.size();//隔夜需求总数
	//拆分隔夜需求
	split(all_two_request, all_split_request_first_day, all_split_request_second_day, all_edge);
	total_request_two_day_after_splice = all_split_request_first_day.size();//隔夜需求分解后总数
	total_request = total_request_one_day + total_request_two_day_after_splice;//总的需求数目
	//当天所有的需求包括拆分后的需求
	all_request.insert(all_request.end(), all_split_request_first_day.begin(), all_split_request_first_day.end());
	//更新需求id
	reset_request_id(all_request);
	for (int i = 0; i < all_request.size(); i++) {
		start_with_one_request(all_request, all_edge, all_request.at(i), all_request.at(i)->start_time);
	}
	cout << "total plan " << all_plan_node_global.size() << endl;
	//计算每条路径的cost
	get_cost(all_plan_node_global, all_edge);
	create_model(all_request, all_split_request_second_day);
	//处理隔夜拆分的第二天的需求
	if (!all_split_request_second_day.empty()) {
		total_request = all_split_request_second_day.size();
		all_request = all_split_request_second_day;
		total_plan = 0;
		all_plan_node_global.clear();
		reset_request_id(all_request);
		for (int i = 0; i < all_request.size(); i++) {
			start_with_one_request(all_request, all_edge, all_request.at(i), all_request.at(i)->start_time);
		}
		cout << "total plan " << all_plan_node_global.size() << endl;
		get_cost(all_plan_node_global, all_edge);
		create_model(all_request, all_split_request_second_day);
	}
}
int main() {
	auto start = std::chrono::system_clock::now();
	test();
	auto end = std::chrono::system_clock::now();
	cout<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms";
}