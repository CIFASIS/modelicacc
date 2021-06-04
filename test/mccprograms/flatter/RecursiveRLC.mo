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

model Ground
  Pin p;  
equation
  p.v = 0;
end Ground;

model OnePort
  Real v;
  Real i;
  PositivePin p;
  NegativePin n;
equation
  v = p.v - n.v;
  0 = p.i + n.i;
  i = p.i;
end OnePort;

model ConditionalHeatPort
  parameter Real T=293.15;
  Real LossPower;
  Real T_heatPort;
equation
     T_heatPort = T;
end ConditionalHeatPort;

model Resistor
  parameter Real R(start=1);
  parameter Real T_ref=300.15;
  parameter Real alpha=0;
  Real v;
  Real i;
  PositivePin p;
  NegativePin n;
  Real R_actual;
  Real LossPower;
  Real T_heatPort;

equation
  T_heatPort = T_ref;
  v = p.v - n.v;
  0 = p.i + n.i;
  i = p.i;
  R_actual = R*(1 + alpha*(T_heatPort - T_ref));
  v = R_actual*i;
  LossPower = v*i;
end Resistor;

model Capacitor
  parameter Real C(start=1);
  Real v;
  Real i;
  PositivePin p;
  NegativePin n;

initial algorithm
  v := 0;

equation
  v = p.v - n.v;
  0 = p.i + n.i;
  i = p.i;
  i = C*der(v);
end Capacitor;

model ConstantVoltage
  parameter Real V(start=1);
  Real v;
  Real i;
  PositivePin p;
  NegativePin n;
equation
  v = p.v - n.v;
  0 = p.i + n.i;
  i = p.i;
  v = V;
end ConstantVoltage;

model RecursiveRLC
  constant Integer N=4;
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
end RecursiveRLC;
