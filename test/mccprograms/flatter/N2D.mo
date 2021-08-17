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
end N2D;
