#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>

using namespace std;

enum Shader
{
	undecided,decided
};

struct clause_status {
	bool value;
	Shader status;
};

struct watch_pair {
	int w1, w2;
	bool status;
	watch_pair() {
		this->w1 = -1;
		this->w2 = -1;
		this->status = false;
	}
};

struct CNF_status {
	bool value;
	Shader status;
};

struct varset {
	int num;
	bool *values;
	Shader *shaders;
	varset(string s) {
		this->num = s.length();
		this->values = new bool[this->num];
		this->shaders = new Shader[this->num];
		for (int i = 0; i < this->num; i++) {
			if (s[i] == '1') {
				this->values[i] = true;
				this->shaders[i] = decided;
			}
			else {
				if (s[i]=='0') {
					this->values[i] = false;
					this->shaders[i] = decided;
				}
				else {
					this->values[i] = false;
					this->shaders[i] = undecided;
					cout << "This varset is wrong!" << endl;
				}
			}
		}
	}
	varset(int n) {
		if (n <= 0) {
			cout << "Length of varset must be positive!" << endl;
		}
		this->num = n;
		this->values = new bool[this->num];
		this->shaders = new Shader[this->num];
		for (int i = 0; i < this->num; i++) {
			this->values[i] = false;
			this->shaders[i] = undecided;
		}
	}

	void show() {
		for (int i = 0; i < this->num; i++) {
			if (this->shaders[i] == undecided) {
				cout << '-';
			}
			else {
				if (this->values[i] == true) {
					cout << '1';
				}
				else {
					cout << '0';
				}
			}
		}
		cout << endl;
	}
};

class clause {
public:
	int termnum;
	int* values;
	clause(string s,stringstream& strstream,int nliterals) {
		strstream.clear();
		strstream.str(s);
		int index;
		vector<int> temp;
		while (strstream>>index,index!=0)
		{
			temp.push_back(index);
		}
		this->termnum = temp.size();
		this->values = new int[this->termnum];
		for (int i = 0; i < this->termnum; i++) {
			this->values[i] = temp[i];
		}
	}

	bool int_to_boolean(int number, varset& vset) {
		if (number > 0) {
			return vset.values[abs(number) - 1];
		}
		else {
			if (number < 0) {
				return !vset.values[abs(number) - 1];
			}
			else {
				cout << "This clause is wrong!" << endl;
				return false;
			}
		}
	}

	clause_status evaluate(varset& vset) {
		bool undecided_exist = false;
		for (int i = 0; i < this->termnum; i++) {
			if (vset.shaders[abs(this->values[i])-1] == decided) {
				if (int_to_boolean(this->values[i],vset) == true) {
					return clause_status{ true,decided };
				}
			}
			else {
				undecided_exist = true;
			}
		}
		if (undecided_exist == true) {
			return clause_status{ false,undecided };
		}
		else {
			return clause_status{ false,decided };
		}
	}

	void show() {
		for (int i = 0; i < this->termnum; i++) {
			cout << this->values[i] << " ";
		}
		cout << endl;
	}
};

class CNF {
public:
	int nclause, nliteral;
	clause **clauses;
	watch_pair **watchpairs;

	CNF(string filepath) {
		ifstream file(filepath);
		string s;
		stringstream sstream;
		int clausen = 0;
		while (!file.eof()) {
			getline(file, s);
			if (s!="" && s[0] != 'c') {
				if (s[0] == 'p') {
					s = s.substr(6);
					sstream.str(s);
					sstream >> this->nliteral;
					sstream >> this->nclause;
					this->clauses = new clause *[nclause];
					this->watchpairs = new watch_pair *[nclause];
				}
				else {
					if (clausen < this->nclause) {
						this->clauses[clausen] = new clause(s, sstream, this->nliteral);
						this->watchpairs[clausen] = new watch_pair();
						clausen++;
						//cout << "\rParsering File... Number of clauses=" << clausen << flush << " Loading Status: " << 100 * clausen / this->nclause << "%";
					}
				}
			}
		}
		//initiate watchpair list
		for (int i = 0; i < this->nclause; i++) {
			if (this->clauses[i]->termnum < 2) {
				this->watchpairs[i]->w1 = 0;
				this->watchpairs[i]->w2 = 0;
				this->watchpairs[i]->status = false;
			}
			else {
				this->watchpairs[i]->w1 = 0;
				this->watchpairs[i]->w2 = 1;
				this->watchpairs[i]->status = false;
			}
		}

		//cout << endl;
		//cout << "CNF Loading finished!" << endl;
	}

	CNF_status evaluate(varset& vset) {
		bool false_exist = false;
		for (int i = 0; i < this->nclause; i++) {
			clause_status cl_status = this->clauses[i]->evaluate(vset);
			if (cl_status.status == undecided) {
				return CNF_status{ false,undecided };
			}
			else {
				if (cl_status.value == false) {
					false_exist = true;
				}
			}
		}
		if (false_exist == true) {
			return CNF_status{ false,decided };
		}
		else {
			return CNF_status{ true,decided };
		}
	}

	void show() {
		for (int i = 0; i < this->nclause; i++) {
			(*this->clauses[i]).show();
		}
	}

	void set_simple_BCP(vector<int>& bcplist, varset& vset) {
		for (int i = 0; i < this->nclause; i++) {
			int n = 0, n_undecided = NAN;
			bool temp = false;
			for (int j = 0; j < this->clauses[i]->termnum; j++) {
				if (vset.shaders[abs(this->clauses[i]->values[j])-1] == decided) {
					n++;
					temp = temp || this->clauses[i]->int_to_boolean(this->clauses[i]->values[j], vset);
				}
				else {
					n_undecided = this->clauses[i]->values[j];
				}
			}
			if (n == this->clauses[i]->termnum - 1 && temp==false) {
				if (n_undecided > 0) {
					vset.values[abs(n_undecided)-1] = true;
				}
				else {
					if (n_undecided < 0) {
						vset.values[abs(n_undecided)-1] = false;
					}
					else {
						cout << "n_undecided=NAN!" << endl;
					}
				}
				vset.shaders[abs(n_undecided)-1] = decided;
				bcplist.push_back(abs(n_undecided)-1);

			}
		}
	}

	void cancle_BCP(vector<int>& bcplist, varset& vset) {
		for (int i = 0; i < bcplist.size(); i++) {
			vset.shaders[bcplist[i]] = undecided;
		}
	}

	pair<int, int> get_two_undecided_from_clause(int clindex,varset& vset) { //if find true return as found nothing
		int index[2], n = 0;
		for (int i = 0; i < this->clauses[clindex]->termnum; i++) {
			bool value = this->clauses[clindex]->int_to_boolean(this->clauses[clindex]->values[i], vset);
			if (value == true && vset.shaders[abs(this->clauses[clindex]->values[i]) - 1] == decided) {
				pair<int, int> p = make_pair(-1, -1);
				return p;
			}
			if (vset.shaders[abs(this->clauses[clindex]->values[i]) - 1] == undecided) {
				index[n] = i;
				n++;
			}
			if (n == 2) {
				pair<int, int> p = make_pair(index[0], index[1]);
				return p;
			}
		}
		if (n == 1) {
			pair<int, int> p = make_pair(index[0], -1);
			return p;
		}
		else {
			pair<int, int> p = make_pair(-1, -1);
			return p;
		}
	}

	void update_watchpairs_BCP_forward(vector<int>& bcplist,vector<int>& watchchangelist, varset& vset) {
		for (int i = 0; i < this->nclause; i++) {
			if (this->watchpairs[i]->status == false) {
				pair<int, int> pa = this->get_two_undecided_from_clause(i, vset);
				if (pa.first >= 0 && pa.second >= 0) {
					this->watchpairs[i]->w1 = pa.first;
					this->watchpairs[i]->w2 = pa.second;
				}
				else {
					if (pa.first >= 0) {
						int index = abs(this->clauses[i]->values[pa.first]) - 1;
						vset.shaders[index] = decided;
						if (this->clauses[i]->values[pa.first] > 0) {
							vset.values[index] = true;
						}
						else {
							vset.values[index] = false;
						}
						bcplist.push_back(index);
						//this->watchpairs[i]->status = true;
						//watchchangelist.push_back(i);
					}
					else {
						//this->watchpairs[i]->status = true;
						//watchchangelist.push_back(i);
					}
				}
			}
			
		}
	}

	void update_watchpairs_BCP_backward(vector<int>& bcplist, vector<int>& watchchangelist, varset& vset) {
		for (int i = 0; i < bcplist.size(); i++) {
			vset.shaders[bcplist[i]] = undecided;
		}
		for (int i = 0; i < watchchangelist.size(); i++) {
			this->watchpairs[watchchangelist[i]]->status = false;
		}
	}

	bool DPLL_branch_search(varset& vset,int i,bool set,bool trace) {
		vset.shaders[i] = decided;
		vset.values[i] = set;
		if (trace) {
			cout << "switch to";
			vset.show();
		}
		vector<int> bcp_list,watchchangelist;
		this->update_watchpairs_BCP_forward(bcp_list, watchchangelist, vset);
		//this->set_simple_BCP(bcp_list, vset);
		if (trace) {
			cout << "forced decision:";
			for (int i = 0; i < bcp_list.size(); i++) {
				cout << " " << bcp_list[i] + 1 << "=" << vset.values[bcp_list[i]] << ",";
			}
			cout << endl;
			///*
			cout << "watchlist change:";
			for (int i = 0; i < watchchangelist.size(); i++) {
				cout << " " << watchchangelist[i] + 1 << "=" << this->watchpairs[watchchangelist[i]]->status << ",";
			}
			cout << endl;
			cout << "Watchlist:";
			for (int i = 0; i < this->nclause; i++) {
				cout << this->watchpairs[i]->status;
			}
			cout << endl;//*/
			if (bcp_list.size() > 0) {
				cout << "switch to";
				vset.show();
			}
		}
		if (i == vset.num - 1) {
			CNF_status cnfstatus = (this->evaluate(vset));
			if (cnfstatus.status == undecided) {
				cout << "Wrong evaluation!" << endl;
			}
			if (trace) {
				cout << "The following node has been searched:" << endl;
				vset.show();
				cout << "The result is:" << cnfstatus.value << endl;
			}
			if (cnfstatus.value == false) {
				vset.shaders[i] = undecided;
				//this->cancle_BCP(bcp_list, vset);
				this->update_watchpairs_BCP_backward(bcp_list, watchchangelist, vset);
			}
			return cnfstatus.value;
		}
		if (vset.shaders[i + 1] == undecided) {
			bool flag=this->DPLL_branch_search(vset, i + 1, false,trace);
			if (flag == true) {
				return true;
			}
			else {
				flag = this->DPLL_branch_search(vset, i + 1, true,trace);
				if (flag == true) {
					return true;
				}
				else {
					vset.shaders[i] = undecided;
					//this->cancle_BCP(bcp_list, vset);
					this->update_watchpairs_BCP_backward(bcp_list, watchchangelist, vset);
					return false;
				}
			}
			
			

		}
		else {
			bool flag = this->DPLL_branch_search(vset, i + 1, vset.values[i + 1], trace);
			if (flag == true) {
				return true;
			}
			else {
				vset.shaders[i] = undecided;
				//this->cancle_BCP(bcp_list, vset);
				this->update_watchpairs_BCP_backward(bcp_list, watchchangelist, vset);
				return false;
			}
		}
	}

	bool DPLL_search(varset& vset,bool trace) {
		bool flag = this->DPLL_branch_search(vset, 0, false, trace);
		if (flag == true) {
			return true;
		}
		flag = this->DPLL_branch_search(vset, 0, true, trace);
		if (flag == true) {
			return true;
		}
		//cout << "Unsatisfiable!" << endl;
		return false;
	}
};