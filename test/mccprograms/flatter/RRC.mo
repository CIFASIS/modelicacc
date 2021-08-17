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

model RRC
  constant Integer N = 100;
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
end RRC;
