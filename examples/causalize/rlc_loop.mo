model RLC_loop
  Real u2;
  Real u1;
  Real uL;
  Real u3;
  Real i3;
  Real i2;
  Real u0;
  Real i0;
  Real i1;
  Real iL;
  parameter Real R1;
  parameter Real R2;
  parameter Real R3;
  parameter Real L;
equation
  u0 = sin(time);
  u1 = R1 * i1;
  u2 = R2 * i2;
  u3 = R3 * i3;
  uL = L * der(iL);
  u0 = u1 + u3;
  uL = u1 + u2;
  u3 = u2;
  i0 = i1 + iL;
  i1 = i2 + i3;
end RLC_loop;
