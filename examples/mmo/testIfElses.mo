within ;
model testIfElses1
  parameter Real period = 1e-2;
  Real x(start = 100);
  Real y(start = 10);
  Real z;
  Boolean pulse;
  Boolean temp_pulse;
equation
  der(x) = 10*x - 5*y;
  pulse = x > 5;
  temp_pulse = not pulse;
  der(y) = if (temp_pulse) then 20*x*y - y	else 0;
  der(z) = if (y<100 and sample(0,period)) then x else 10;
end testIfElses1;
