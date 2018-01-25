#include "edge.h"
#include "customize_hash.h"
//两个点之间只有一条边
class edge_hash {
public:
	size_t operator ()(const edge other) const {

		return hash_val(other.start_point, other.end_point);
	}

};

class edge_equal {
public:
	bool operator() (const edge other1,const edge other2)const {
		//return other1.id == other2.id;
		return (other1.start_point == other2.start_point)&&(other1.end_point == other2.end_point);
		
	}

};