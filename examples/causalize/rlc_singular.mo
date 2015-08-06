model RLC_loop
  Real u0;
  Real u1;
  Real u2;
  Real uL;
  Real uC;
  Real i0;
  Real i1;
  Real i2;
  Real iL;
  Real iC;
  parameter Real R1;
  parameter Real R2;
  parameter Real C;
  parameter Real L;
equation
  u0 = sin(time);
  u1 = R1 * i1;
  u2 = R2 * i2;
  uL = L * der(iL);
  iC = C*der(uC);
  u0 = u1 + uL;
  uC = u1 + u2;
  uL = u2;
  i0 = i1 + iC;
  i1 = i2 + iL;
end RLC_loop;
