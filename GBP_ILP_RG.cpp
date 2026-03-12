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
vector<bool> representative_vertices_bool;
int total_lazy_constraints;

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

class checkcoverage: public GRBCallback
{
    public:
        vector<vector<GRBVar>> vars;
        int n_, U_, rv_;
    checkcoverage(vector<vector<GRBVar>> xvars, int n, int U, int rv) {
        vars  = xvars;
        n_ = n;
        U_ = U;
        rv_ = rv;
    }
    protected:
    void callback() {
        try {
            if (where == GRB_CB_MIPSOL) {
                // Extract values
                vector<vector<double>> values(n_,vector<double>(U_));
                for(int i=0;i<n_;i++){
                    for(int j=0;j<U_;j++){
                        values[i][j] = getSolution(vars[i][j]);
                    }
                }
                // Get size of the extracted feasible solution
                int bks = 0;
                for(int j=0;j<U_;j++){
                    for(int i=0;i<n_;i++){
                        if(values[i][j] > 0.9){
                            bks++;
                            break;
                        }
                    }
                }
                // Extract feasible solution
                vector<int> solution(bks);
                for(int j=0;j<U_;j++){
                    for(int i=0;i<n_;i++){
                        if(values[i][j] > 0.9){
                            solution[j] = i;
                            break;
                        }
                    }
                }
                // Get uncovered vertices
                vector<bool> covered(n_,false);
                for(int i=0;i<bks;i++){
                    vector<int> N = kClosedNeighborhood(solution[i],i);
                    for(int v : N){
                        covered[v] = true;
                    }
                }
                int covered_vertices = count(covered.begin(), covered.end(), true);
                // The first selected vertex is the uncovered vertex with minimum label
                if(covered_vertices < n_){
                    vector<int> representative_vertices;
                    int some_uncovered_vertex;
                    for(int i=0;i<n_;i++){
                        if(!covered[i]){
                            some_uncovered_vertex = i;
                            break;
                        }
                    }
                    // update distance
                    vector<int> d = singleSourceShortestPathLength(some_uncovered_vertex);
                    vector<vector<int>> distance_matrix;
                    representative_vertices.push_back(some_uncovered_vertex);
                    representative_vertices_bool[some_uncovered_vertex] = true;
                    distance_matrix.push_back(d);
                    vector<int> distance = d;
                    // Select the remaining vertices
                    for(int i=0;i<rv_-1;i++){
                        int max_dist = 0;
                        int farthest_vertex = -1;
                        for(int j=0;j<n_;j++){
                            if(distance[j] > max_dist & !representative_vertices_bool[j]){
                                max_dist = distance[j];
                                farthest_vertex = j;
                            }                            
                        }
                        if(farthest_vertex!=-1){
                            representative_vertices.push_back(farthest_vertex);
                            representative_vertices_bool[farthest_vertex] = true;
                            d = singleSourceShortestPathLength(farthest_vertex);
                            distance_matrix.push_back(d);
                            // update distance
                            for(int j=0;j<n_;j++){
                                if(d[j] < distance[j]){
                                    distance[j] = d[j];
                                }
                            }   
                        }
                    }
                    // Add the new constraints
                    for(int i=0;i<representative_vertices.size();i++){
                        GRBLinExpr expr = 0;
                        for(int j=0;j<U_;j++){
                            vector<int> j_neighbors;
                            for(int k=0;k<n_;k++){
                                if(distance_matrix[i][k] <= j){
                                    j_neighbors.push_back(k);
                                }
                            }
                            for(int k : j_neighbors){
                                expr += vars[k][j];
                            }
                        }
                        addLazy(expr >= 1);
                    }
                    distance_matrix.clear();
                    total_lazy_constraints += representative_vertices.size();
                }
            }
        } catch (GRBException e) {
            cout << "Error number: " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        } catch (...) {
            cout << "Error during callback" << endl;
        }
    }
};

int main(int argc, char **argv) {
    string path = argv[1];
    int U  = stoi(argv[2]);
    int rv = U;
    int initial_rv = 2*U;
    total_lazy_constraints = initial_rv;

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
        cout << "U: " << U << endl;
        cout << "number of connected components: " << number_of_connected_components << endl;
        cout << "Initial representative vertices: " << initial_rv << endl;
        cout << endl;
        // Get rv representative vertices
        auto start_time = high_resolution_clock::now();
        vector<vector<int>> distance_matrix(1);
        vector<int> representative_vertices;
        representative_vertices_bool.resize(n,false);
        vector<int> distance;
        representative_vertices.push_back(0);
        representative_vertices_bool[0] = true;
        distance = singleSourceShortestPathLength(0);         
        distance_matrix[0] = distance;
        for(int i=0;i<initial_rv-1;i++){
            int max_dist = 0;
            int farthest_vertex = 0;
            for(int j=0;j<n;j++){
                if(distance[j] > max_dist){
                    max_dist = distance[j];
                    farthest_vertex = j;
                }
            }
            representative_vertices.push_back(farthest_vertex);
            representative_vertices_bool[farthest_vertex] = true;
            vector<int> d = singleSourceShortestPathLength(farthest_vertex);
            distance_matrix.push_back(d);
            // update distance
            for(int j=0;j<n;j++){
                if(d[j] < distance[j]){
                    distance[j] = d[j];
                }
            }
        }
        // Load gurobi model
        // Create an environment
        try{
            GRBEnv env = GRBEnv(true);
            env.set("LogFile", "GBP-ILP.log");
            env.set(GRB_IntParam_OutputFlag, 1);
            env.start();

            // Create an empty model
            GRBModel model = GRBModel(env);
            model.set(GRB_IntParam_LazyConstraints, 1);

            // Create variables
            vector<vector<GRBVar>> x;
            x.resize(n, vector<GRBVar>(U));
            for(int i=0;i<n;i++){
                for(int j=0;j<U;j++){
                    string varName = "x_" + to_string(i) + "_" + to_string(j);
                    x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
                }
            }
            // Objective function (22)
            GRBLinExpr objective_function = 0;
            for(int i=0;i<n;i++){
                for(int j=0;j<U;j++){
                    objective_function += x[i][j];
                }
            }
            model.setObjective(objective_function, GRB_MINIMIZE);

            // Constraints (23)
            for(int j=1;j<U;j++){
                GRBLinExpr expr1 = 0;
                GRBLinExpr expr2 = 0;
                for(int i=0;i<n;i++){
                    expr1 += x[i][j];
                    expr2 += x[i][j-1];
                }
                model.addConstr(expr1 <= expr2);
            }
            for(int j=0;j<U;j++){
                GRBLinExpr expr = 0;
                for(int i=0;i<n;i++){
                    expr += x[i][j];
                }
                model.addConstr(expr <= 1);
            }

            // Constraints (24)
            for(int i=0;i<initial_rv;i++){
                GRBLinExpr expr = 0;
                for(int j=0;j<U;j++){
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
                model.addConstr(expr >= 1);
            }
            distance_matrix.clear();

            // Set callback function
            checkcoverage cc = checkcoverage(x, n, U, rv);
            model.setCallback(&cc);

            // Optimize
            model.optimize();

            // Optimal size
            int OPT = model.get(GRB_DoubleAttr_ObjVal);
            cout << "Obj: " << OPT << endl;

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

                int k = 0;
                for(int i=0;i<n;i++){
                    for(int j=0;j<U;j++){
                        if(values[k] > 0.9){
                            solution[j] = i;
                        }
                        k++;
                    }
                }
                cout << "[ ";
                for (int i=OPT-1;i>=0;i--) {
                    if(i!=0){
                        cout << solution[i] << ", ";
                    }else{
                        cout << solution[i] << " ]";
                    }
                }
                cout << endl;
            }

            // Validate solution
            vector<bool> covered(n);
            for(int i=0;i<n;i++){
                covered[i] = false;
            }
            if (model.get(GRB_IntAttr_SolCount) > 0) {
                for(int i=0;i<OPT;i++){
                    vector<int> N = kClosedNeighborhood(solution[i],i);
                    for(int v : N){
                        covered[v] = true;
                    }
                }
            }
            int covered_vertices = count(covered.begin(), covered.end(), true);
            auto stop_time = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop_time - start_time);
            double time = duration.count();
            cout << "Time: " << time << endl;
            cout << "Total lazy constraints: " << total_lazy_constraints << endl;
            //cout << endl << "Number of covered vertices: " << covered_vertices << endl;
            if(covered_vertices<n){
                cout<<"WARNING: this is not a burning sequence"<<endl;
            }
        }catch(GRBException e) {
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        }
    }
}