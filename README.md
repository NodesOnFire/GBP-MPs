# GBP-MPs
A collection of compact Mathematical Programs for the Graph Burning Problem
#
```mermaid
graph TD;
    MPs["Mathematical Programs"] --> MILPs;
    MPs["Mathematical Programs"] --> CSPs;
    MPs["Mathematical Programs"] --> ILPs;
    MPs["Mathematical Programs"] --> QUBOs;
    MILPs --> PROP-MILP["PROP-MILP [1]"];
    CSP --> COV-CSP["COV-CSP [1]"];
    CSPs --> GBP-IP["GBP-IP [2]"];
    ILPs --> COV-ILP["COV-ILP [1]"];
    ILPs --> GBP-ILP["GBP-ILP [1]"];
    ILPs --> GBP-ILP-RG["GBP-ILP-RG [1]"];
    ILPs --> ILP-PROP["ILP-PROP [2]"];
    ILPs --> ILP-COV;
    QUBOs --> sQUBO["sQUBO [1]"];
    QUBOs --> uQUBO["uQUBO [1]"];
```
