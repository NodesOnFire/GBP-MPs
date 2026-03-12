#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>
#include"gurobi_c++.h"
#include<algorithm>

using namespace std;
using namespace std::chrono;

vector<vector<int>> graph;
vector<vector<int>> distance_matrix;

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

bool validBurningSequence(vector<int> &s, int n){
    int counter = 0;
    for(int i=0;i<n;i++){
        int b = s.size();
        for(int j=0;j<b;j++){
            //if(distance_matrix[i][s[j]] <= b - (j+1)){
            if(distance_matrix[i][s[j]] <= j){
                counter += 1;
                break;
            }
        }
    }
    if (counter == n){
        return true;
    }else{
        return false;
    }
}

int main(int argc, char **argv) {
    int n,m;
    string path = argv[1];
    graph.clear();
    distance_matrix.clear();

    ifstream myfile(path);
    string line;
    if(myfile.is_open()){
        // Load graph
        getline(myfile, line);
        n = stoi(line);
        graph.resize(n, vector<int>(0));
        for(int i=0;i<n;i++){
            graph[i].push_back(i);
        }
        cout << "n: " << n << endl;
        getline(myfile, line);
        m = stoi(line);
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
    }
    auto start = high_resolution_clock::now();
    // Compute all-pairs shortest path
    distance_matrix = allPairsDistance();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    double time = duration.count();
    cout << endl <<"All-pairs shortest path running time: " << time << " miliseconds" << endl;
    vector<int> solution;
    vector<int> best_burning_sequence;
    int best_burning_sequence_size = numeric_limits<int>::max();
    // Set lower and upper bounds
    int low  = 0;
    int high = n;
    // Binary search
    start = high_resolution_clock::now();
    while(low <= high){
        int g = floor((high+low)/2);
        // Gurobi model
        try{
            GRBEnv env = GRBEnv(true);
            env.set("LogFile", "COV_ILP.log");
            env.set(GRB_IntParam_OutputFlag, 1);
            env.start();

            // Create an empty model
            GRBModel model = GRBModel(env);

            // Create variables
            vector<vector<GRBVar>> x;
            x.resize(n, vector<GRBVar>(g));
            for(int i=0;i<n;i++){
                for(int j=0;j<g;j++){
                    string varName = "x_" + to_string(i) + "_" + to_string(j);
                    x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
                }
            }
            // Objective function (22)
            GRBLinExpr objective_function = 0;
            for(int i=0;i<n;i++){
                objective_function += x[i][0];
            }
            model.setObjective(objective_function, GRB_MAXIMIZE);

            for(int j=1;j<g;j++){
                GRBLinExpr expr = 0;
                for(int i=0;i<n;i++){
                    expr += x[i][j];
                }
                model.addConstr(expr == 1);
            }

            for(int i=0;i<n;i++){
                GRBLinExpr expr = 0;
                for(int j=1;j<g;j++){
                    vector<int> j_neighbors;
                    for(int k=0;k<n;k++){
                        if(distance_matrix[i][k] <= j){
                            j_neighbors.push_back(k);
                        }
                    }
                    for(int k : j_neighbors){
                        expr += x[k][j];
                    }
                }
                model.addConstr(expr >= x[i][0]);
            }
            // Optimize
            model.optimize();

            // Optimal size
            int OPT = model.get(GRB_DoubleAttr_ObjVal);
            //cout << "Obj: " << OPT << endl;

            // Extract solution
            solution.resize(g);
            if (model.get(GRB_IntAttr_SolCount) > 0) {
                GRBVar* vars = NULL;
                double* values = NULL;
                string* names = NULL;
                int numVars = model.get(GRB_IntAttr_NumVars);
                vars = model.getVars();
                values = model.get(GRB_DoubleAttr_X, vars, numVars);
                names = model.get(GRB_StringAttr_VarName, vars, numVars);

                int k = 0;
                for(int i=0;i<n;i++){
                    for(int j=0;j<g;j++){
                        if(k%g!=0){
                            if(values[k] > 0.9){
                                solution[j] = i;
                            }
                        }else{
                            if(values[k] < 0.1){
                                solution[j] = i;
                            }
                        }
                        k++;
                    }
                }
            }
        }catch(GRBException e) {
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        }
        if(validBurningSequence(solution,n)){
            high = g-1;
            if(solution.size() <= best_burning_sequence_size){
                best_burning_sequence_size = solution.size();
                best_burning_sequence = solution;
            }
        }else{
            low = g+1;
        }
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    time = duration.count();
    cout << endl << "Total running time: " << time << " miliseconds" << endl;
    cout << "Best burning sequence: " << endl;
    cout << "[ ";
    for (int i=best_burning_sequence.size()-1;i>=0;i--) {
        if(i!=0){
            cout << best_burning_sequence[i] << ", ";
        }else{
            cout << best_burning_sequence[i] << " ]";
        }
    }
    cout << endl;
    cout << endl << "OPT: " << best_burning_sequence.size() << endl;
    cout << endl;
    return 0;
}