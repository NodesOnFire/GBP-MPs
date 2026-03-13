# GBP-MPs
A collection of compact Mathematical Programs for the Graph Burning Problem
#
```mermaid
graph TD;
    MPs["Mathematical Programs"] --> MILP["Mixed-Integer Linear Programs"];
    MPs["Mathematical Programs"] --> CSP["Constraint Satisfaction Problems"];
    MPs["Mathematical Programs"] --> ILP["Integer Linear Programs"];
    MPs["Mathematical Programs"] --> QUBO["Quadratic Unconstrained Binary Optimization problems"];
    MILP --> PROP-MILP;
    CSP --> COV-CSP;
    CSP --> GBP-IP;
    ILP --> COV-ILP;
    ILP --> GBP-ILP;
    ILP --> ILP-COV;
```
