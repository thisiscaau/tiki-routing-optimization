#include<bits/stdc++.h>
using namespace std;
using namespace chrono;

class timer: high_resolution_clock {
    const time_point start_time;
public:
    timer(): start_time(now()) {}
    rep elapsed_time() const { return duration_cast<milliseconds>(now()-start_time).count(); } };

timer CLOCK;


const float inf = 1e9;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
// const float SCORE_CUR_TIME = 0;
// const float SCORE_REM_TIME = 0.1;
// const float SCORE_WEIGHT = 1000;
// const float SCORE_VOL = 0.9;

const float SCORE_CUR_TIME = 152;
const float SCORE_REM_TIME = 1;
const float SCORE_WEIGHT = 12;
const float SCORE_VOL = 10000;

const int MX = 75;

int convertHourToSec(string str) {
	int h = (str[0] - '0') * 10 + (str[1] - '0');
	int m = (str[3] - '0') * 10 + (str[4] - '0');
	int s = (str[6] - '0') * 10 + (str[7] - '0');
	return h * 3600 + m * 60 + s;	
}

string convertSecToHour(int sec) {
	int h = sec / 3600;
	int m = (sec - h * 3600) / 60;
	int s = sec - h * 3600 - m * 60;

	string res = "";
	if (h < 10) {
		res += '0';
	}
	res += to_string(h);
	res += ":";

	if (m < 10) {
		res += '0';
	}

	res += to_string(m);
	res += ":";

	if (s < 10) {
		res += '0';
	}

	res += to_string(s);

	return res;
}

struct Transport {
	int src, start, end; 
	float weight, vol, velo;

	Transport() {}
	Transport(int src, int start, int end, float weight, float vol, float velo) : src(src), start(start), end(end), weight(weight), vol(vol), velo(velo) {}
};

struct Goods {
	int src, des, load, unload, minGet, maxGet, minShip, maxShip;
	float vol, weight;

	Goods() {}
	Goods(int _src, int _des, float _weight, float _vol, int _load, int _unload, int _minGet, int _maxGet, int _minShip, int _maxShip) {
		src = _src;
		des = _des;
		weight = _weight;
		vol = _vol;
		load = _load;
		unload = _unload;
		minGet = _minGet;
		maxGet = _maxGet;
		minShip = _minShip;
		maxShip =  _maxShip;
	}
};

struct Event {
	int idHub, idGoods, startTime, endTime;
	Event() {}
	Event(int idHub, int idGoods, int startTime, int endTime) : idHub(idHub), idGoods(idGoods), startTime(startTime), endTime(endTime) {}
};

struct Hub_Visit {
	int hub,ops,arrive,leave;
	Hub_Visit() {}
	Hub_Visit(int hub, int ops, int arrive, int leave) : hub(hub), ops(ops), arrive(arrive), leave(leave) {}
};


int dist[1005][1005]; // distance between warehouses
int curTime[1005], curPos[1005]; // current time and pos of truck[i]
float curW[1005], curV[1005]; // current weight of truck[i]

vector<vector<Event>> allE; // store all events related to truck[i]
vector<Transport> Trans; // trucks' inf
vector<Goods> allG; // goods' info

int calTime(int id, int s, int e) {
	int t = ceil((float) 3600 * dist[s][e] / Trans[id].velo);	
	return t;
}

vector<vector<pair<int,int>>> paths; // <order_id,current_hub>

float simulate_path (int idT, vector<pair<int,int>> path){
	int cur_time = curTime[idT], end_time = Trans[idT].end;
	int cur_pos = curPos[idT];

	float cur_vol = 0; float cur_weight = 0; // current vol and weight
	int time_slack = 0;

	for (auto cargo : path){
		auto cur_cargo = allG[cargo.first]; int hub = cargo.second;
		
		if (cur_pos != hub){
			cur_time += calTime(idT,cur_pos,hub);
			cur_pos = hub;
		}

		if (hub == cur_cargo.src){
			if (cur_vol + cur_cargo.vol > Trans[idT].vol) return inf;
			if (cur_weight + cur_cargo.weight > Trans[idT].weight) return inf;
			
			// load cargo
			
			time_slack += max(0,cur_cargo.minGet - cur_time);

			if (cur_time < cur_cargo.minGet) {
				cur_time = cur_cargo.minGet;
			} else if (cur_time > cur_cargo.maxGet) {
				return inf;
			}

			time_slack += cur_cargo.maxGet - cur_time;
			cur_time += cur_cargo.load;
			cur_vol += cur_cargo.vol; cur_weight += cur_cargo.weight;
		}

		if (hub == cur_cargo.des){
			if (cur_time > cur_cargo.maxShip) return inf;

			// unload cargo
			time_slack += max(0,cur_cargo.minShip - cur_time);
			
			if (cur_time < cur_cargo.minShip){
				cur_time = cur_cargo.minShip;
			} else if (cur_time > cur_cargo.maxShip) {
				return inf;
			}

			time_slack += cur_cargo.maxShip - cur_time;
			cur_time += cur_cargo.unload;
			cur_vol -= cur_cargo.vol; cur_weight -= cur_cargo.weight;
		}

		if (cur_time > end_time) return inf;
		if (cur_time + calTime(idT,cur_pos,Trans[idT].src) > end_time) return inf;
	}

	if (cur_time + calTime(idT, cur_pos, Trans[idT].src) > end_time) return -inf;

	float score = (time_slack) * SCORE_REM_TIME + 
					(Trans[idT].weight - cur_weight) * SCORE_WEIGHT + 
					(Trans[idT].vol - cur_vol) * SCORE_VOL -
					cur_time * SCORE_CUR_TIME;

	return -score; 
}

vector<vector<Hub_Visit>> hub_log;

void path_to_event (int idT){
	// convert feasible path of orders into Event form
	vector<Event> res;

	if (paths[idT].size() == 0) return;

	// float eval = simulate_path(idT,paths[idT]);

	int &cur_time = curTime[idT], &cur_pos = curPos[idT];
	int cum_ops = 0; int arrive = cur_time;

	for (auto cargo : paths[idT]){
		auto cur_cargo = allG[cargo.first]; int hub = cargo.second;
		
		if (cur_pos != hub){
			// change hub
			hub_log[idT].push_back(Hub_Visit(cur_pos,cum_ops,arrive,cur_time));

			cur_time += calTime(idT,cur_pos,hub);
			cur_pos = hub; arrive = cur_time; cum_ops = 0; 
		}

		if (hub == cur_cargo.src){
			if (cur_time < cur_cargo.minGet) {
				cur_time = cur_cargo.minGet;
			} 
			
			int st = cur_time;
			cur_time += cur_cargo.load;
			int ed = cur_time;

			if (st > cur_cargo.maxGet){
				cout << "ngu" << endl;
			}
			res.push_back(Event(hub, cargo.first, st, ed));
			cum_ops++;
		}

		if (hub == cur_cargo.des){
			
			if (cur_time < cur_cargo.minShip){
				cur_time = cur_cargo.minShip;
			} 

			int st = cur_time;
			cur_time += cur_cargo.unload;
			int ed = cur_time;

			if (st > cur_cargo.maxShip){
				cout << "ngu" << endl;
			}
			res.push_back(Event(hub, cargo.first, st, ed));
			cum_ops++;
		}

	}
	hub_log[idT].push_back(Hub_Visit(cur_pos,cum_ops,arrive,cur_time));
	// return res;

	for (Event e : res) {
		allE[idT].push_back(e);
	}
}

pair<float,vector<pair<int,int>>> insertion_heuristic (int idT, int idG){
	vector<pair<int,int>> ori = paths[idT]; // save copy of original path
	vector<pair<int,int>> opt_path = ori;

	if (ori.size() == 0){
		vector<pair<int,int>> new_path;
		new_path.push_back(make_pair(idG,allG[idG].src));
		new_path.push_back(make_pair(idG,allG[idG].des));
		return make_pair(simulate_path(idT,new_path),new_path);
	}

	float cur_val = -1;

	for (int i = 1 ; i <= ori.size() + 1; i++){
		for (int j = i + 1 ; j <= ori.size() + 2 ; j++){
			vector<pair<int,int>> new_path;
			new_path = ori;

			new_path.insert(new_path.begin() + i - 1,make_pair(idG,allG[idG].src));
			new_path.insert(new_path.begin() + j - 1,make_pair(idG,allG[idG].des));

			float val = simulate_path(idT,new_path);
			if (val == inf) continue;
			if (val < cur_val || cur_val == -1){
				cur_val = val;
				opt_path = new_path;
			}
		}
	}

	if (cur_val == -1) cur_val = inf;

	return make_pair(cur_val,opt_path);
}

int hubs,cntTrans,cntGoods;
vector<pair<int,int>> vi; // processing order

void rollback(){
	for (int i = 1; i <= cntTrans; i++) {
		allE[i].clear();
		paths[i].clear();
		hub_log[i].clear();
	}

	for (int i = 1; i <= cntTrans; i++) {
		curW[i] = curV[i] = 0.0;
		curTime[i] = Trans[i].start;
		curPos[i] = Trans[i].src;
	}

	vi.clear();
}

bool used[1005]; // mark if vehicle is used

// DATA FOR GENETIC
struct route{
	int truck_id;
	vector<pair<int,int>> path; // <order_id,current_hub>
	route() {}
};

struct solution{
	vector<route> routes; // all truck routes
	int delivered; // number of orders
	int num_truck; // number of trucks
	float score;
	solution() {}
};

float evaluate_solution (solution a){
	float res = 0;
	res += 100000 * (a.delivered / cntGoods);
	res -= 1000 * (a.num_truck / cntTrans);
	return res;
}

vector<solution> chromosomes; // store solutions

// END DATA

// Solve for each strategies and format into "solution" struct
pair<int,int> Solve() {
	int cnt = 0;

	for (int i = 1 ; i <= cntTrans ; i++){
		used[i] = false; // mark used trucks
	}

	for (auto [_, i] : vi) {
		float tot = inf;
		int id;
		vector<pair<int,int>> opt_path;
		for (int j = 1; j <= cntTrans; j++) {
			auto query = insertion_heuristic(j,i);
			if (query.first < tot) {
				tot = query.first;
				opt_path = query.second;
				id = j;
			}
		}

		if (tot == inf) continue;

		cnt++;
		paths[id] = opt_path;
		used[id] = true;

		if (paths[id].size() >= MX) {
			simulate_path(id, paths[id]);
			path_to_event(id);
			paths[id].clear();
		}
	}

	for (int i = 1; i <= cntTrans; i++) {
		simulate_path(i, paths[i]);
		path_to_event(i);
	}

	int num_truck = 0;
	for (int i = 1; i <= cntTrans ; i++){
		if (used[i]) num_truck++;
	}

	// TURN INTO SOLUTION FORMAT
	solution res;
	res.delivered = cnt;
	res.num_truck = num_truck;
	res.score = evaluate_solution(res);

	for (int i = 1 ; i <= cntTrans ; i++){
		if (!used[i]) continue;
		route cur; cur.truck_id = i;
		cur.path = paths[i];
		res.routes.push_back(cur);
	}

	chromosomes.push_back(res);

	return make_pair(cnt,num_truck);
}

// GENETIC ALGORITHM STARTS HERE

vector<route> generate_solution (vector<route> used_routes, vector<int> rem){
	// param: used_routes, remaining requests
	// return: optimal route

	rollback();
	vector<route> res;
	shuffle(rem.begin(),rem.end(),rng);
	
	for (int i = 1 ; i <= cntTrans ; i++){
		used[i] = false;
	}

	for (auto cur : used_routes){
		paths[cur.truck_id] = cur.path;
		used[cur.truck_id] = true;
	}

	for (int i : rem){
		float tot = inf;
		int id;
		vector<pair<int,int>> opt_path;
		for (int j = 1; j <= cntTrans; j++) {
			auto query = insertion_heuristic(j,i); // insert request i into j current path
			if (query.first < tot) {
				tot = query.first;
				opt_path = query.second;
				id = j;
			}
		}

		if (tot == inf) continue;
		paths[id] = opt_path;
		used[id] = true;

		/*if (paths[id].size() >= MX) {
			simulate_path(id, paths[id]);
			path_to_event(id);
			paths[id].clear();
		}*/
	}

	for (int i = 1 ; i <= cntTrans ; i++){
		if (used[i] == false) continue;
		route cur; cur.truck_id = i;
		cur.path = paths[i];
		res.push_back(cur);
	}

	shuffle(res.begin(),res.end(),rng);
	return res;
}


bool transfered_requests[1005];
bool transfered_trucks[1005];

int random_range (int l, int r){
	int range = r - l + 1;
	int num = l + rng() % range;
	return num;
}

solution crossing(solution par1, solution par2){
	solution res;
	// left & right cut of parent 2 (random)
	int lf = random_range(0,par2.routes.size() - 1);
	int rt = random_range(lf,par2.routes.size() - 1);

	for (int i = 1 ; i <= cntGoods ; i++){
		transfered_requests[i] = false;
	}

	for (int i = 1 ; i <= cntTrans ; i++){
		transfered_trucks[i] = false;
	}

	vector<route> new_routes;
	vector<route> cut_segment; // stay untouched
	for (int i = lf ; i <= rt ; i++){
		route cur = par2.routes[i];
		cut_segment.push_back(cur);
		transfered_trucks[cur.truck_id] = true; // mark truck

		for (auto req : cur.path){
			transfered_requests[req.first] = true; // mark requests
		}
	}

	vector<int> rem_requests; // unresolved requests
	for (auto cur : par1.routes){
		if (transfered_trucks[cur.truck_id] == true){
			// duplicated trucks
			for (auto req : cur.path){
				if (transfered_requests[req.first] == true) continue; // solved by transfered trucks
				rem_requests.push_back(req.first);
			}
		}
	}

	// remove duplicated requests in trucks that remains
	for (int i = 0 ; i < par1.routes.size() ; i++){
		auto cur = par1.routes[i];
		if (transfered_trucks[cur.truck_id] == true) continue;
		// clean up
		vector<pair<int,int>> tmp; // new path after deletion
		for (auto req : cur.path){
			if (transfered_requests[req.first] == true) continue;
			tmp.push_back(req);
		}
		cur.path = tmp;
		par1.routes[i] = cur;
	}


	// combining
	for (auto cur : cut_segment){
		new_routes.push_back(cur);
	}

	for (auto cur : par1.routes){
		if (transfered_trucks[cur.truck_id] == true) continue;
		new_routes.push_back(cur);
	}

	// mutation operator
	if (rng() % 2){
		// remove a random truck
		int idx = random_range(0,new_routes.size() - 1); // remove the idx-th truck
		set<int> add; 
		for (auto cur : new_routes[idx].path){
			add.insert(cur.first);
		}
		for (int req : add){
			rem_requests.push_back(req);
		}
		vector<route> upd;
		for (int i = 0 ; i < new_routes.size() ; i++){
			if (i == idx) continue;
			auto cur = new_routes[i];
			upd.push_back(cur);
		}
		new_routes = upd; add.clear(); upd.clear();
	}

	// dealing with unsettled requests
	shuffle(new_routes.begin(),new_routes.end(),rng);
	res.routes = generate_solution(new_routes,rem_requests);

	// wrap up
	res.num_truck = res.routes.size();
	res.delivered = 0;

	for (auto cur : res.routes){
		auto cur_path = cur.path;
		res.delivered += cur_path.size() / 2;
	}

	res.score = evaluate_solution(res);

	// return updated solution
	return res;
}

bool compare_solution (solution a, solution b){
	return a.score > b.score;
}

int main() {
	ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

	cin >> hubs;

	for (int i = 1; i <= hubs; i++) 
		for (int j = 1; j <= hubs; j++) 
			cin >> dist[i][j];

	cin >> cntTrans; 

	Trans.resize(cntTrans + 1);
	allE.resize(cntTrans + 1);
	paths.resize(cntTrans + 1);
	hub_log.resize(cntTrans + 1);
 
	for (int i = 1; i <= cntTrans; i++) {
		curW[i] = curV[i] = 0.0;
		int src;
		cin >> src;
		curPos[i] = src;

		string str;

		cin >> str;
		int start = convertHourToSec(str);

		curTime[i] = start;

		cin >> str;
		int end = convertHourToSec(str);

		float weight; 
		cin >> weight;

		float vol;
		cin >> vol;

		float velo;
		cin >> velo;

		Trans[i] = Transport(src, start, end, weight, vol, velo);
	}

	cin >> cntGoods;

	allG.resize(cntGoods + 1);

	for (int i = 1; i <= cntGoods; i++) {
		int src, des, load, unload;
		float vol, weight;

		cin >> src >> des >> weight >> vol >> load >> unload;

		string str;

		cin >> str;
		int minGet = convertHourToSec(str);

		cin >> str;
		int maxGet = convertHourToSec(str);

		cin >> str;
		int minShip = convertHourToSec(str);

		cin >> str;
		int maxShip = convertHourToSec(str);

		allG[i] = Goods(src, des, weight, vol, load, unload, minGet, maxGet, minShip, maxShip);
		vi.push_back({maxGet, i});
	}

	sort(vi.begin(), vi.end());

	pair<int,int> cnt1 = Solve();

	pair<int,int> mx = cnt1;

	auto allE1 = allE;
	auto hub_log1 = hub_log;

// ------------ STRAT 2
	rollback();
	for (int i = 1; i <= cntGoods; i++) {
		vi.push_back({allG[i].minGet, i});
	}
	sort(vi.begin(), vi.end());

	pair<int,int> cnt2 = Solve();

	if ((cnt2.first > mx.first) || (cnt2.first == mx.first && cnt2.second < mx.second)) {
		mx = cnt2;
		swap(allE, allE1);
		swap(hub_log, hub_log1);
	}

// ------------ STRAT 3
	rollback();
	for (int i = 1; i <= cntGoods; i++) {
		vi.push_back({allG[i].minShip, i});
	}

	sort(vi.begin(), vi.end());

	pair<int,int> cnt3 = Solve();

	if ((cnt3.first > mx.first) || (cnt3.first == mx.first && cnt3.second < mx.second)) {
		mx = cnt3;
		swap(allE, allE1);
		swap(hub_log, hub_log1);
	}
// ------------ STRAT 4
	rollback();
	for (int i = 1; i <= cntGoods; i++) {
		vi.push_back({allG[i].maxShip, i});
	}
	sort(vi.begin(), vi.end());

	pair<int,int> cnt4 = Solve();

	if ((cnt4.first > mx.first) || (cnt4.first == mx.first && cnt4.second < mx.second)) {
		mx = cnt4;
		swap(allE, allE1);
		swap(hub_log, hub_log1);
	}

// ------------- STRAT 5
	rollback();

	for (int i = 1; i <= cntGoods; i++) {
		vi.push_back({ - (allG[i].vol / 1000 + allG[i].weight/0.9), i});
	}
	sort(vi.begin(), vi.end());

	pair<int,int> cnt5 = Solve();

	if ((cnt5.first > mx.first) || (cnt5.first == mx.first && cnt5.second < mx.second)) {
		mx = cnt5;
		swap(allE, allE1);
		swap(hub_log, hub_log1);
	}

//-------------STRAT 6

	rollback();

	for (int i = 1; i <= cntGoods; i++) {
		vi.push_back({ (allG[i].maxShip - allG[i].minGet), i});
	}
	sort(vi.begin(), vi.end());

	pair<int,int> cnt6 = Solve();

	if ((cnt6.first > mx.first) || (cnt6.first == mx.first && cnt6.second < mx.second)) {
		mx = cnt6;
		swap(allE, allE1);
		swap(hub_log, hub_log1);
	}

// ----------- Generating more chromosomes
	/*for (int i = 1 ; i <= 5 ; i++){
		rollback();
		shuffle(vi.begin(),vi.end(),rng);
		Solve();
	}*/


// RUN THE GENETIC ALGORITHM
	//cout << "chromo: " << chromosomes.size() << endl;
	sort(chromosomes.begin(),chromosomes.end(),compare_solution);

	while (CLOCK.elapsed_time() < 60000 * 4){
		int idx1,idx2;
		idx1 = random_range(0,chromosomes.size() - 1);
		idx2 = random_range(0,chromosomes.size() - 1);

		while (idx1 == idx2){
			idx2 = random_range(0,chromosomes.size() - 1);
		}

		solution par1 = chromosomes[idx1];
		solution par2 = chromosomes[idx2];

		solution child1,child2;
		child1 = crossing(par1,par2);
		child2 = crossing(par2,par1);
		chromosomes.push_back(child1); chromosomes.push_back(child2);
		sort(chromosomes.begin(),chromosomes.end(),compare_solution);
		chromosomes.pop_back(); chromosomes.pop_back();
	}

	solution FINAL = chromosomes[0];
// ----------- OUTPUT THE BEST RESULT -----------------
	rollback();

	for (auto cur_route : FINAL.routes){
		auto cur_path = cur_route.path;
		paths[cur_route.truck_id] = cur_path;
	}

	for (int i = 1; i <= cntTrans; i++) {
		simulate_path(i, paths[i]);
		path_to_event(i);
	}

	//cout << "delivered " << mx << '\n';

	// return 0;

	for (int i = 1; i <= cntTrans; i++){
		// going through each truck
		//cout << "Truck " << i << " " << allE[i].size() / 2 << endl;

		if (allE[i].size() == 0){
			auto cur = Trans[i];
			cout << cur.src << " " << 0 << " " << convertSecToHour(cur.start) << " " << convertSecToHour(cur.start) << endl;
			continue;
		}
		
		int total = hub_log[i].size();
		if (hub_log[i].back().hub != Trans[i].src){
			total++;
		}

		cout << total << endl;
		int idx = 0;
		for (auto cur : hub_log[i]){
			cout << cur.hub << " " << cur.ops << " " << convertSecToHour(cur.arrive) << " " << convertSecToHour(cur.leave) << endl;
			for (int _ = idx ; _ < idx + cur.ops ; _++){
				auto cur_event = allE[i][_];
				cout << cur_event.idGoods << " " << convertSecToHour(cur_event.startTime) << endl;
			}
			idx += cur.ops;
		}

		if (hub_log[i].back().hub != Trans[i].src){
			int cur_time = hub_log[i].back().leave;
			cur_time += calTime(i,hub_log[i].back().hub,Trans[i].src);
			// arriving depot
			cout << Trans[i].src << " " << 0 << " " << convertSecToHour(cur_time) << " " << convertSecToHour(cur_time) << endl;
		}
	}	

}
