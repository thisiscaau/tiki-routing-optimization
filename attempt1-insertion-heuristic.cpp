#include<bits/stdc++.h>
using namespace std;

const float inf = 1e9;
// const float SCORE_CUR_TIME = 0;
// const float SCORE_REM_TIME = 0.1;
// const float SCORE_WEIGHT = 1000;
// const float SCORE_VOL = 0.9;

const float SCORE_CUR_TIME = 152;
const float SCORE_REM_TIME = 1;
const float SCORE_WEIGHT = 12;
const float SCORE_VOL = 10000;

const int MX = 50;

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
			
			if (cur_time < cur_cargo.minGet) {
				cur_time = cur_cargo.minGet;
			} else if (cur_time > cur_cargo.maxGet) {
				return inf;
			}

			cur_time += cur_cargo.load;
			cur_vol += cur_cargo.vol; cur_weight += cur_cargo.weight;
		}

		if (hub == cur_cargo.des){
			if (cur_time > cur_cargo.maxShip) return inf;

			// unload cargo
			
			if (cur_time < cur_cargo.minShip){
				cur_time = cur_cargo.minShip;
			} else if (cur_time > cur_cargo.maxShip) {
				return inf;
			}

			cur_time += cur_cargo.unload;
			cur_vol -= cur_cargo.vol; cur_weight -= cur_cargo.weight;
		}

		if (cur_time > end_time) return inf;
		if (cur_time + calTime(idT,cur_pos,Trans[idT].src) > end_time) return inf;
	}

	if (cur_time + calTime(idT, cur_pos, Trans[idT].src) > end_time) return -inf;

	float score = (Trans[idT].end - cur_time) * SCORE_REM_TIME + 
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

int hubs,cntTrans;
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

int Solve() {
	int cnt = 0;

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

	return cnt;
}

int main() {
	ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

	// freopen("input.txt", "r", stdin);
	// freopen("output.txt", "w", stdout);

	int mx = 0;
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

	int cntGoods;
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

	int cnt1 = Solve();

	mx = cnt1;

	auto allE1 = allE;
	auto hub_log1 = hub_log;

// ------------ STRAT 2
	rollback();
	for (int i = 1; i <= cntGoods; i++) {
		vi.push_back({allG[i].minGet, i});
	}
	sort(vi.begin(), vi.end());

	int cnt2 = Solve();

	if (cnt2 > mx) {
		mx = cnt2;
		swap(allE, allE1);
	}

// ------------ STRAT 3
	rollback();
	for (int i = 1; i <= cntGoods; i++) {
		vi.push_back({allG[i].minShip, i});
	}

	sort(vi.begin(), vi.end());

	int cnt3 = Solve();

	if (cnt3 > mx) {
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

	int cnt4 = Solve();

	if (cnt4 > mx) {
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

	int cnt5 = Solve();

	if (cnt5 > mx) {
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

	int cnt6 = Solve();

	if (cnt6 > mx) {
		mx = cnt6;
		swap(allE, allE1);
		swap(hub_log, hub_log1);
	}

// ----------- OUTPUT
	swap(allE, allE1);
	swap(hub_log, hub_log1);

	// cout << mx << '\n';

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
