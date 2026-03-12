#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<vector>
#include<limits>
#include<queue>
#include"gurobi_c++.h"
#include<algorithm>
#include<chrono>

using namespace std;
using namespace std::chrono;
vector<vector<int>> graph;

vector<vector<int>> allPairsDistance() {
    int n = graph.size();
    vector<vector<int>> distance(n, vector<int> (n, numeric_limits<int>::max()));
    for (int start=0 ; start<n ; start++){
        vector<bool> visited(n, false);
        queue<int> q;
        vector<int> d_local(graph.size(), numeric_limits<int>::max());
        d_local[start] = 0;
        visited[start] = true;
        q.push(start);
        while (!q.empty()) {
            int current = q.front();
            q.pop();
            for (int neighbor : graph[current]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                    d_local[neighbor] = d_local[current] + 1;
                }
            }
        }
        for(int i=0;i<n;i++){
                distance[start][i] = d_local[i];
                distance[i][start] = d_local[i];
        }
    }
    return distance;
}

vector<int> singleSourceShortestPathLength(int start){
    int n = graph.size();
    vector<int> distance(n,numeric_limits<int>::max());
    distance[start] = 0;
    vector<bool> visited(n, false);
    queue<int> q;
    visited[start] = true;
    q.push(start);
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        for(int neighbor : graph[current]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
                distance[neighbor] = distance[current] + 1;
            }
        }
    }
    return distance;
}

vector<int> kClosedNeighborhood(int start, int k){
    int n = graph.size();
    vector<int> k_closed_neighborhood;
    k_closed_neighborhood.push_back(start);
    vector<bool> visited(n, false);
    visited[start] = true;
    queue<int> q;
    q.push(start);
    int level = 0;
    int counter = 1;
    while(level<k & !q.empty()) {
        int current = q.front();
        q.pop();
        for(int neighbor : graph[current]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
                k_closed_neighborhood.push_back(neighbor);
            }
        }
        counter--;
        if(counter==0){
            counter = q.size();
            level++;
        }
    }
    return k_closed_neighborhood;
}

int numberOfConnectedComponents(){
    int n = graph.size();
    vector<bool> visited(n, false);
    queue<int> q;
    int counter = 0;
    while(count(visited.begin(), visited.end(), true) < n){
        for(int i=0;i<n;i++){
            if(!visited[i]){
                q.push(i);
                visited[i] = true;
                break;
            }
        }
        while(!q.empty()) {
            int current = q.front();
            q.pop();
            for(int neighbor : graph[current]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        counter++;
    }
    return counter;
}

int main(int argc, char **argv) {
    string path = argv[1];
    int U  = stoi(argv[2]);

    ifstream myfile(path);
    string line;

    if(myfile.is_open()){
        // Load graph

        getline(myfile, line);
        int n = stoi(line);
        graph.clear();
        graph.resize(n, vector<int>(0));
        for(int i=0;i<n;i++){
            graph[i].push_back(i);
        }
        cout << "n: " << n << endl;
        getline(myfile, line);
        int m = stoi(line);
        cout << "m: " << m << endl;

        while(getline(myfile, line)) {
            istringstream iss(line);
            string s;
            int a,b;
            int i = 0;
            while(getline(iss, s, ' '))
            {
                if (i==0){
                    a = stoi(s) - 1;
                    i = i+1;
                }
                if (i==1){
                    b = stoi(s) - 1;
                }
            }
            graph[a].push_back(b);
            graph[b].push_back(a);
        }
        myfile.close();
        int number_of_connected_components = numberOfConnectedComponents();
        cout << endl;
        cout <<"---------------------------" << endl;
        cout << "graph loaded: " << path << endl;
        cout << "n: " << n << endl;
        cout << "m: " << m << endl;
        cout << "number of connected components: " << number_of_connected_components << endl;
        cout << endl;
        auto start_time = high_resolution_clock::now();
        // Load gurobi model
        // Create an environment
        start_time = high_resolution_clock::now();
        try{
            GRBEnv env = GRBEnv(true);
            env.set("LogFile", "PROP_MILP.log");
            env.set(GRB_IntParam_OutputFlag, 1);
            env.start();

            // Create an empty model
            GRBModel model = GRBModel(env);

            // Create variables
            vector<vector<GRBVar>> s;
            s.resize(n, vector<GRBVar>(U));
            for(int i=0;i<n;i++){
                for(int j=0;j<U;j++){
                    string varName = "s_" + to_string(i) + "_" + to_string(j);
                    s[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
                }
            }
            vector<vector<GRBVar>> b;
            b.resize(n, vector<GRBVar>(U));
            for(int i=0;i<n;i++){
                for(int j=0;j<U;j++){
                    string varName = "b_" + to_string(i) + "_" + to_string(j);
                    b[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
                }
            }
            vector<GRBVar> x;
            x.resize(U);
            for(int i=0;i<U;i++){
                string varName = "x_" + to_string(i);
                x[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
            }

            // Objective function (22)
            GRBLinExpr objective_function = 1;
            for(int i=0;i<U;i++){
                objective_function += x[i];
            }
            model.setObjective(objective_function, GRB_MINIMIZE);

            for(int i=0;i<n;i++){
                for(int j=0;j<U;j++){
                    model.addConstr(x[j] >= 1-b[i][j]);
                }
            }

            for(int i=0;i<n;i++){
                for(int j=0;j<U;j++){
                    GRBLinExpr expr = 0;
                    // Closed neighborhood
                    if(j>0){
                        expr += b[i][j-1];
                    }
                    for(int k : graph[i]){
                        if(j>0){
                            expr += b[k][j-1];
                        }
                    }
                    expr += s[i][j];
                    model.addConstr(b[i][j] <= expr);
                }
            }

            for(int j=0;j<U;j++){
                GRBLinExpr expr = 0;
                for(int i=0;i<n;i++){
                    expr += s[i][j];
                }
                model.addConstr(expr == 1);
            }

            // Optimize
            model.optimize();

            // Optimal size
            int OPT = model.get(GRB_DoubleAttr_ObjVal);
            cout << "OPT: " << OPT << endl;

            // Extract solution  
            int solution[OPT];
            if (model.get(GRB_IntAttr_SolCount) > 0) {
                GRBVar* vars = NULL;
                double* values = NULL;
                string* names = NULL;
                int numVars = model.get(GRB_IntAttr_NumVars);
                vars = model.getVars();
                values = model.get(GRB_DoubleAttr_X, vars, numVars);
                names = model.get(GRB_StringAttr_VarName, vars, numVars);
/*
                int k = 0;
                k = 0;
                for(int i=0;i<n;i++){
                    cout << "---- \n";
                    for(int j=0;j<U;j++){
                        bool valid = false;
                        for(int k=0;k<OPT;k++){
                            if(j%n==k){
                                valid = true;
                            }
                        }
                        if(valid){
                            cout << values[k] << " ";
                        }
                        k++;
                    }
                }

                k = 0;
                for(int i=0;i<n;i++){
                    cout << "---- \n";
                    for(int j=0;j<U;j++){
                        bool valid = false;
                        for(int k=0;k<OPT;k++){
                            if(j%n==k){
                                valid = true;
                            }
                        }
                        if(valid){
                            cout << names[k] << " ";
                        }
                        k++;
                    }
                }
                */

                int k = 0;
                k = 0;
                for(int i=0;i<n;i++){
                    for(int j=0;j<U;j++){
                        bool valid = false;
                        for(int k=0;k<OPT;k++){
                            if(j%n==k){
                                valid = true;
                            }
                        }
                        if(valid){
                            if(values[k] > 0.9){
                                solution[j] = i;
                            }
                        }
                        k++;
                    }
                }

                cout << "[ ";
                for (int i=0;i<=OPT-1;i++) {
                    if(i!=OPT-1){
                        cout << solution[i] << ", ";
                    }else{
                        cout << solution[i] << " ]";
                    }
                }
                cout << endl;
            }

            auto stop_time = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop_time - start_time);
            double time = duration.count();
            cout << "Total time: " << time << endl;
        }catch(GRBException e) {
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        }
    }
}