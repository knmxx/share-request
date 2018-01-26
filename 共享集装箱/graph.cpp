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

int total_request_one_day = 0;//������������
int total_request_two_day=0;//��ҹ��������

int total_request_two_day_after_splice=0;//��ҹ����ֽ������
int total_request=0;//�ܵ�������Ŀ



const static double max_interval = 2;//���ʱ����
const static int max_length = 200;//����ϲ������·
const static double speed = 50;//�����ٶ�


extern unordered_set<edge, edge_hash, edge_equal> get_edge();//���еı�
extern vector<request*> get_request_one_day();//���е�����
extern void split(vector<request*>&all_two_request,vector<request*> &all_split_request_first_day, vector<request*>& all_split_request_second_day, unordered_set<edge, edge_hash, edge_equal>& all_edge);
extern vector<request*> get_request_two_day();//��ҹ����

int total_plan = 0;//��������                       
list<plan_node>all_plan_node_global;//�洢���п��еķ���
//���ļ�����
bool is_legal_path(plan_node &current_plan, const unordered_set<edge, edge_hash, edge_equal>&all_edge);
void save_result(list<plan_node>all_plan_node);
void start_with_one_request(const vector<request*>&all_request, const unordered_set<edge, edge_hash, edge_equal>&all_edge, request *r_start, int request_start_time) {
	list<plan_node>all_plan_node;//���еķ���
	list<plan_node>can_trans_plan_node;//������ķ���
	plan_node p_start(total_request);
	p_start.mark_visited[r_start->id] = true;
	p_start.real_path.push_back(city(r_start->start_point));//��ǰ��������
	p_start.real_path.push_back(city(r_start->end_point));//��ǰ������յ�
	p_start.real_path.front().out_related_request.push_back(r_start);//�����й�����ǰ����
	p_start.real_path.back().in_related_request.push_back(r_start);//�յ���й�����ǰ����
	if (is_legal_path(p_start, all_edge)) {
		//�Ϸ�
		all_plan_node.push_back(p_start);
		can_trans_plan_node.push_back(p_start);
	}
	while (!can_trans_plan_node.empty())
	{
		plan_node current_plan = can_trans_plan_node.front();
		can_trans_plan_node.pop_front();
		//cout << can_trans_plan_node.size() << endl;
		for (auto const &current_request : all_request) {
			//��ǰ�����Ѿ��������ǰ������
			if (current_plan.mark_visited[current_request->id] == true)
				continue;
			string current_plan_start_city_name = current_plan.real_path.front().name;//�����·�������
			string current_plan_end_city_name = current_plan.real_path.back().name;//�����·�����յ�
			 //�����ж���ͬ
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
			//�غ�
			if (has_same == true) {
				plan_node p_new(current_plan);//�����µķ���
				p_new.mark_visited[current_request->id] = true;
				p_new.real_path.at(same_start_index).out_related_request.push_back(r_start);
				p_new.real_path.at(same_start_index + 1).in_related_request.push_back(r_start);
				if (is_legal_path(p_new, all_edge)) {
					//�Ϸ�
					all_plan_node.push_back(p_new);
					can_trans_plan_node.push_back(p_new);

				}
			}
			//�����ͬ
			if (current_plan_start_city_name == current_request->start_point) {
				//�յ��н���
				bool has_same = false;
				for (auto i = 0; i < current_plan.real_path.size(); i++) {
					if (current_plan.real_path.at(i).name == current_request->end_point) {
						has_same = true;
						plan_node p_new_insertion(current_plan);//�����µķ���
						p_new_insertion.real_path.at(i).in_related_request.push_back(current_request);
						p_new_insertion.real_path.front().out_related_request.push_back(current_request);
						p_new_insertion.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new_insertion, all_edge)) {
							//�Ϸ�
							all_plan_node.push_back(p_new_insertion);
							can_trans_plan_node.push_back(p_new_insertion);
						}
					}
				}
				//�������û�н���
				if (has_same == false) {
					auto request_path = all_edge.find(edge(current_request->start_point, current_request->end_point));
					//�����м�
					//������ĳ���
					city being_insert(current_request->end_point);
					being_insert.in_related_request.push_back(current_request);
					for (auto city_index = 1; city_index < current_plan.real_path.size(); city_index++) {
						city curreny_city = current_plan.real_path.at(city_index);
						city pre_city = current_plan.real_path.at(city_index - 1);
						auto old_path_edge_1 = all_edge.find(edge(pre_city.name, curreny_city.name));//������Ļ���
						auto new_path_edge_1 = all_edge.find(edge(current_request->end_point, curreny_city.name));
						auto new_path_edge_2 = all_edge.find(edge(pre_city.name, current_request->end_point));
						if (new_path_edge_1->length + new_path_edge_2->length > old_path_edge_1->length + request_path->length + max_length)
							continue;
						//�����µķ���
						plan_node p_new(current_plan);
						//�ҵ�����λ��
						auto insert_posttion = p_new.real_path.begin() + city_index;
						//����
						p_new.real_path.insert(insert_posttion, being_insert);
						p_new.real_path.front().out_related_request.push_back(current_request);
						p_new.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new, all_edge)) {
							//�Ϸ�
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
					//�������
					auto end_path = all_edge.find(edge(current_plan.real_path.back().name, current_request->end_point));
					if (end_path->length <= request_path->length + 200) {
						plan_node p_new(current_plan);//�����µķ���
													  //�����µ��յ�
						p_new.real_path.push_back(being_insert);
						p_new.real_path.front().out_related_request.push_back(current_request);
						p_new.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new, all_edge)) {
							//�Ϸ�
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
				}
			}
			//�յ���ͬ,����������
			if (current_plan_end_city_name == current_request->end_point) {
				//����н���
				bool has_same = false;
				for (auto i = 0; i < current_plan.real_path.size(); i++) {
					if (current_plan.real_path.at(i).name == current_request->start_point) {
						has_same = true;
						plan_node p_new_insertion(current_plan);//�����µķ���
						p_new_insertion.real_path.back().in_related_request.push_back(current_request);
						p_new_insertion.real_path.at(i).out_related_request.push_back(current_request);
						p_new_insertion.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new_insertion, all_edge)) {
							//�Ϸ�
							all_plan_node.push_back(p_new_insertion);
							can_trans_plan_node.push_back(p_new_insertion);
						}
					}
				}
				//���յ���û���κν���
				if (has_same == false) {
					auto request_path = all_edge.find(edge(current_request->start_point, current_request->end_point));
					city being_insert(current_request->start_point);//����ĳ���
					being_insert.out_related_request.push_back(current_request);
					//�м����
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
							//�Ϸ�
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
					//�ײ�����
					auto first_path = all_edge.find(edge(current_request->start_point, current_plan.real_path.front().name));
					if (first_path->length <= request_path->length + max_length) {
						plan_node p_new(current_plan);//�����µķ���
						//�����µ��յ�
						p_new.real_path.insert(p_new.real_path.begin(),being_insert);
						p_new.real_path.back().in_related_request.push_back(current_request);
						p_new.mark_visited[current_request->id] = true;
						if (is_legal_path(p_new, all_edge)) {
							//�Ϸ�
							all_plan_node.push_back(p_new);
							can_trans_plan_node.push_back(p_new);
						}
					}
				}
			}
			//a->b  b->d
			if (current_plan_end_city_name == current_request->start_point) {
				plan_node p_new(current_plan);//�����µķ���
				p_new.mark_visited[current_request->id] = true;
				p_new.real_path.back().out_related_request.push_back(current_request);
				p_new.real_path.push_back(city(current_request->end_point));//�µ��յ�
				p_new.real_path.back().in_related_request.push_back(current_request);
				if (is_legal_path(p_new, all_edge)) {
					//�Ϸ�
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
					//���ڳ���С��200
					if (adjacent_edge->length <= max_length) {
						plan_node p_new(current_plan);//�����µķ���
						p_new.mark_visited[current_request->id] = true;
						p_new.real_path.push_back(city(current_request->start_point));
						p_new.real_path.back().out_related_request.push_back(current_request);
						p_new.real_path.push_back(city(current_request->end_point));//�µ��յ�
						p_new.real_path.back().in_related_request.push_back(current_request);
						if (is_legal_path(p_new, all_edge)) {
							//�Ϸ�
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
//����������е��������С����ʱ��
double in_request_min_time(city current_city) {
	double in_min_time = std::numeric_limits<double>::max();
	for (auto const one_request : current_city.in_related_request) {
		in_min_time = min(in_min_time, one_request->end_time);
	}
	return in_min_time;
}
//end time
//����������е��������󵽴�ʱ��
double in_request_max_time(city current_city) {
	double in_max_time = std::numeric_limits<double>::min();
	for (auto const one_request : current_city.in_related_request) {
		in_max_time = max(in_max_time, one_request->end_time);
	}
	return in_max_time;
}
//start time
//�뿪������е��������С�뿪ʱ��
double out_request_min_time(city current_city) {
	double out_min_time = std::numeric_limits<double>::max();
	for (auto const one_request : current_city.out_related_request) {
		out_min_time = min(out_min_time, one_request->start_time);
	}
	return out_min_time;
}
//start time
//�뿪������е����������뿪ʱ��
double out_request_max_time(city current_city) {
	double out_max_time = std::numeric_limits<double>::min();
	for (auto const one_request : current_city.out_related_request) {
		out_max_time = max(out_max_time, one_request->start_time);
	}
	return out_max_time;
}
//ʵ�ʿ�ʼʱ��
double get_real_start_time(city current_city,double real_end_time=0) {
	
	return max(real_end_time, max(in_request_max_time(current_city), out_request_max_time(current_city)));
}
//ʵ�ʵ���ʱ��
double get_real_end_time(const city pre, city current_city, const unordered_set<edge, edge_hash, edge_equal>&all_edge) {
	//s/v
	//���pre city�뵱ǰcity���Ƿ�������
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
	//��������������������·�ϴ�������
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
//�յ�ֻ��Ҫrule_1
bool rule_1(city city_in_one_path) {
	return city_in_one_path.real_end_time - in_request_min_time(city_in_one_path) <= max_interval;
}
//���ֻ��Ҫrule_2
bool rule_2(city city_in_one_path) {
	return out_request_max_time(city_in_one_path) - out_request_min_time(city_in_one_path) <= max_interval;
}
bool rule_3(city city_in_one_path) {
	return max(city_in_one_path.real_end_time, in_request_max_time(city_in_one_path)) - out_request_min_time(city_in_one_path) <= max_interval;
}
//һ��·����һ����ҹ����Ķ����ת�㣬��Ϊ���Ϸ�
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
//��Ҫ����ÿ�����е�ʵ��ʱ��
bool is_legal_path(plan_node &current_plan, const unordered_set<edge, edge_hash, edge_equal>&all_edge) {
	//һ��·����һ����ҹ����Ķ����ת�㣬��Ϊ���Ϸ�
	if (rule_4(current_plan)==false)
		return false;

	auto &start_city = current_plan.real_path.front();
	//��㲻�Ϸ�
	if (rule_2(start_city)==false) {
		return false;
	}
	//��������ʵ�ʿ�ʼʱ���ʵ�ʵ���ʱ��
	start_city.real_start_time=get_real_start_time(start_city);
	start_city.real_end_time = get_real_start_time(start_city);
	//����м�ڵ�ĺϷ���
	for (size_t i = 1; i < current_plan.real_path.size() - 1; i++) {
		//���㵱ǰ�ڵ�ĵ���ʱ��
		current_plan.real_path.at(i).real_end_time = get_real_end_time(current_plan.real_path.at(i - 1),//ǰһ������
																		current_plan.real_path.at(i), //��ǰ����
																		all_edge);	
		//��ǰ���е�ʵ�ʵ���ʱ��
		int current_city_real_end_time = current_plan.real_path.at(i).real_end_time;
		//���㵱ǰ���еĿ�ʼʱ��
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
		//��ǰ�ڵ�Ϸ�
		
	}
	//�����յ��ʵ�ʵ���ʱ��
	int current_plan_length = current_plan.real_path.size();
	current_plan.real_path.back().real_end_time = get_real_end_time(current_plan.real_path.at(current_plan_length - 2),//ǰһ������
																		current_plan.real_path.back(), //��ǰ����
																		all_edge);
	//��ǰ·�����յ�
	auto &end_city = current_plan.real_path.back();
	//�յ㲻�Ϸ�
	if (rule_1(end_city) == false) {
		return false;
	}
	return true;
}
//������ļ����еķ���
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
//���һ���������ļ�
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
			rs << setw(8) << std::internal << "���� id";
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
//����ÿ�������Ĵ���
void get_cost(list<plan_node>&all_plan_node_global, const unordered_set<edge, edge_hash, edge_equal>&all_edge) {
	for (auto & one_plan : all_plan_node_global) {
		for (auto i = 1; i < one_plan.real_path.size(); i++) {
			one_plan.cost += all_edge.find(edge(one_plan.real_path.at(i - 1).name, one_plan.real_path.at(i).name))->length;
		}
		one_plan.cost += 200;
	}
}
//����ÿ���cost
void save_total_cost(double total_cost,int total_request) {
	ofstream rs("rs.txt", ios::app);
	rs << "###############################################" << endl;
	rs << "total cost:" << total_cost << endl;
	rs << "total request:"<<total_request << endl;
}
//��ӡ���Ž�
void save_optimization_result(const IloNumArray&vals) {
	int val_index = 0;//x���±�
	int request_index = 1;//����ı��
	for (auto one : all_plan_node_global) {
		if (vals[val_index] == 1) {
			save_one(one, request_index);
		}
		val_index++;
	}
}
//Ŀ�꺯��
void insert_object_function(IloExpr &obj, IloEnv &env, IloNumVarArray &x, IloModel &model) {
	int x_index = 0;
	for (auto one_plan : all_plan_node_global) {
		obj += x[x_index++] * one_plan.cost;
	}
	model.add(IloMinimize(env, obj));
}
//����Ҫ�ϲ�������
void insert_constraint_one_day(map<int, int>count_group, vector<request*> &all_request, IloEnv &env, IloNumVarArray &x, IloModel &model) {
	for (int row_index = 0; row_index < total_request; row_index++) {
		//����Ҫ�ϲ�������
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
//��Ҫ�ϲ��ĸ�ҹ����
void insert_constraint_two_day(map<int, int>count_group, vector<request*> &all_request, IloEnv &env, IloNumVarArray &x, IloModel &model) {
	for (auto one_count : count_group) {
		//��ҹ�����ֳ��˶��
		if (one_count.second > 1) {
			vector<int> one_row_coff(total_plan, 0);//�ϲ����ϵ��
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
//���ݸ�ҹ�����һ���ѡ����ȷ�ϵڶ��������
void get_second_real_request(vector<request*> &all_split_request_second_day, IloNumArray &vals) {
	vector<request*>second_request;
	//�����ڶ���ĸ�ҹ����
	for (int second_request_index = 0; second_request_index < all_split_request_second_day.size(); second_request_index++) {
		//�������з���
		int col_index = 0;
		for (auto one_plan : all_plan_node_global) {
			//���Ž��еķ���
			if (vals[col_index] == 1) {
				//���������е�����
				for (auto current_plan_city : one_plan.real_path) {
					for (auto current_city_in_request : current_plan_city.in_related_request) {
						//group id ��ͬ
						if (current_city_in_request->group_id == all_split_request_second_day.at(second_request_index)->group_id) {
							//��һ����յ���ڶ���������ͬ
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
	//һ��groupc���ڶ�����󣬵�ֻ������һ��
	for (auto one_re : all_request) {
		count_group[one_re->group_id]++;
	}
	insert_constraint_one_day(count_group, all_request, env, x, model);
	//�ϲ���ҹ���� ��ҹ����ֳɶ������Ҫcheck
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
	//���ݲ�ֵ�ѡ�� ���¸�ҹ�ڶ��������
	get_second_real_request(all_split_request_second_day, vals);
}
//���������±��
void reset_request_id(vector<request*> &all_request) {
	int request_index = 0;
	for (auto &one : all_request){
		one->id = request_index++;
	}
}
void test() {
	unordered_set<edge, edge_hash, edge_equal> all_edge = get_edge();//ͼ�����еı�
	vector<request*> all_request = get_request_one_day();//��������

	/*for (int i = 0; i < all_request.size(); i++) {
		all_request.at(i)->group_id = 1;
	}*/
	vector<request*> all_two_request = get_request_two_day();//��ҹ����
	vector<request*> all_split_request_first_day;//��ҹ�����һ��
	vector<request*> all_split_request_second_day;//��ҹ����ڶ���
	total_request_one_day = all_request.size();//������������
	total_request_two_day = all_two_request.size();//��ҹ��������
	//��ָ�ҹ����
	split(all_two_request, all_split_request_first_day, all_split_request_second_day, all_edge);
	total_request_two_day_after_splice = all_split_request_first_day.size();//��ҹ����ֽ������
	total_request = total_request_one_day + total_request_two_day_after_splice;//�ܵ�������Ŀ
	//�������е����������ֺ������
	all_request.insert(all_request.end(), all_split_request_first_day.begin(), all_split_request_first_day.end());
	//��������id
	reset_request_id(all_request);
	for (int i = 0; i < all_request.size(); i++) {
		start_with_one_request(all_request, all_edge, all_request.at(i), all_request.at(i)->start_time);
	}
	cout << "total plan " << all_plan_node_global.size() << endl;
	//����ÿ��·����cost
	get_cost(all_plan_node_global, all_edge);
	create_model(all_request, all_split_request_second_day);
	//�����ҹ��ֵĵڶ��������
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