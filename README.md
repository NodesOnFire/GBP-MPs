# GBP-MPs
A collection of compact Mathematical Programs for the Graph Burning Problem
#
```mermaid
graph TD;
    MP["Mathematical Programs"] --> MILP;
    MP["Mathematical Programs"] --> CSP;
    MP["Mathematical Programs"] --> ILP;
    MP["Mathematical Programs"] --> QUBO;
    MILP --> a["PROP-MILP [1]"];
    CSP --> b["COV-CSP [1]
               GBP-IP [3]"];
    ILP --> c["GBP-ILP [1]
               COV-ILP [1]
               ILP-PROP [2]
               ILP-CMCP [2]
               ILP-COV [2]"];
    QUBO --> d["sQUBO [1]
                uQUBO [1]"];
```
