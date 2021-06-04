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
  connect(l, resistor.p);
  connect(capacitor.n, r);
  connect(resistor1.p, u);
  connect(capacitor1.n, d);
end RCcell;

model network2D
constant Integer N=4;
constant Integer M=5;
RCcell Cell[N,M];
ConstantVoltage S;
Ground G;
equation
for i in 1:N-1,j in 1:M-1 loop
  connect(Cell[i,j].r, Cell[i,j+1].l);
  connect(Cell[i,j].d, Cell[i+1,j].u);
end for;
for i in 1:N loop
  connect(Cell[i,M].r, Cell[i,1].l);
end for;
for j in 1:M loop
  connect(Cell[1,j].u,S.p);
  connect(Cell[N,j].d,S.n);
end for;
end network2D;
