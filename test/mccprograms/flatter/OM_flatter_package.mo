package GT_MODELS
  connector Pin
    Real v;
    flow Real i;
  end Pin;

  connector PositivePin
    Real v;
    flow Real i;
  end PositivePin;

  connector NegativePin
    Real v;
    flow Real i;
  end NegativePin;

  class Ground
    Pin p;
  equation
    p.v = 0;
  end Ground;

  class Resistor
    parameter Real R(start = 1.0);
    parameter Real T_ref = 300.15;
    parameter Real alpha = 0.0;
    Real v;
    Real i;
    PositivePin p;
    NegativePin n;
    Real LossPower;
    Real T_heatPort(start = 288.15);
    Real R_actual;
  equation
    R_actual = R * (1.0 + alpha * (T_heatPort - T_ref));
    v = R_actual * i;
    LossPower = v * i;
    T_heatPort = T_ref;
    v = p.v - n.v;
    0.0 = p.i + n.i;
    i = p.i;
  end Resistor;

  class Capacitor
    Real v(start = 0.0);
    Real i;
    PositivePin p;
    NegativePin n;
    parameter Real C(start = 1.0);
  equation
    i = C * der(v);
    v = p.v - n.v;
    0.0 = p.i + n.i;
    i = p.i;
  end Capacitor;

  class ConstantVoltage
    parameter Real V(start = 1.0);
    Real v;
    Real i;
    PositivePin p;
    NegativePin n;
  equation
    v = V;
    v = p.v - n.v;
    0.0 = p.i + n.i;
    i = p.i;
  end ConstantVoltage;

  model RLC
    constant Integer N = 100;
    Ground G;
    Resistor R[N];
    Capacitor C[N];
    ConstantVoltage S;
  equation
    connect(S.p, R[1].p);
    connect(S.n, G.p);
    for i in 1:N - 1 loop
      connect(R[i].n, R[i + 1].p);
    end for;
    for i in 1:N loop
      connect(C[i].p, R[i].n);
      connect(C[i].n, G.p);
    end for;
    annotation(
      experiment(StartTime = 0, StopTime = 10, Tolerance = 1e-10, Interval = 0.002),
      Documentation,
  __OpenModelica_commandLineOptions = "--matchingAlgorithm=PFPlusExt --indexReductionMethod=dynamicStateSelection -d=initialization,NLSanalyticJacobian",
  __OpenModelica_simulationFlags(lv = "LOG_STATS", s = "dassl"));
  end RLC;
  
model RRLC
  constant Integer N=100;
  Ground G;
  Resistor R[N];
  Capacitor C[N];
  ConstantVoltage S;
  equation
    connect(S.p,R[1].p);
    connect(S.n,G.p);
    connect(C[1].n, G.p);
    for i in 1:N-1 loop
      connect(R[i].n, R[i+1].p);
      connect(C[i+1].n, C[i].n);
    end for;
    for i in 1:N loop
      connect(C[i].p, R[i].n);
    end for;
  annotation(
      experiment(StartTime = 0, StopTime = 10, Tolerance = 1e-10, Interval = 0.002),
      __OpenModelica_commandLineOptions = "--matchingAlgorithm=PFPlusExt --indexReductionMethod=dynamicStateSelection -d=initialization,NLSanalyticJacobian",
  __OpenModelica_simulationFlags(lv = "LOG_STATS", s = "dassl"));end RRLC;

  model RCcell
    Resistor resistor;
    Resistor resistor1;
    Capacitor capacitor;
    Capacitor capacitor1;
    Pin l;
    Pin d;
    Pin r;
    Pin u;
  equation
    connect(resistor.n, capacitor.p);
    connect(resistor1.n, capacitor1.p);
    connect(resistor1.n, capacitor.p);
    connect(resistor.p, l);
    connect(capacitor.n, r);
    connect(resistor1.p, u);
    connect(capacitor1.n, d);
  end RCcell;

  model N2D
    constant Integer N = 10;
    constant Integer M = 10;
    RCcell Cell[N, M];
    ConstantVoltage S;
    Ground G;
  equation
    for i in 1:N - 1, j in 1:M - 1 loop
      connect(Cell[i, j].r, Cell[i, j + 1].l);
      connect(Cell[i, j].d, Cell[i + 1, j].u);
    end for;
    for i in 1:N loop
      connect(Cell[i, M].r, Cell[i, 1].l);
    end for;
    for j in 1:M loop
      connect(Cell[1, j].u, S.p);
      connect(Cell[N, j].d, S.n);
    end for;
    annotation(
      experiment(StartTime = 0, StopTime = 20, Tolerance = 1e-10, Interval = 0.004),
      __OpenModelica_commandLineOptions = "--matchingAlgorithm=PFPlusExt --indexReductionMethod=dynamicStateSelection -d=initialization,NLSanalyticJacobian",
  __OpenModelica_simulationFlags(lv = "LOG_STATS", s = "dassl"));
  end N2D;
end GT_MODELS;