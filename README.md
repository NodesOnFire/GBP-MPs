# GBP-MPs
A collection of compact Mathematical Programs for the Graph Burning Problem
#
```mermaid
graph TD;
    MPs["Mathematical Programs"] --> MILPs;
    MPs["Mathematical Programs"] --> CSPs;
    MPs["Mathematical Programs"] --> ILPs;
    MPs["Mathematical Programs"] --> QUBOs;
    MILP --> PROP-MILP["[1]"];
    CSP --> COV-CSP;
    CSP --> GBP-IP;
    ILP --> COV-ILP;
    ILP --> GBP-ILP;
    ILP --> GBP-ILP-RG;
    ILP --> ILP-COV;
    QUBOs --> sQUBO;
    QUBOs --> uQUBO;
```
