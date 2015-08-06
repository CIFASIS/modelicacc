model bball
  Real y(start = 1),vy;
  Real f;
  parameter Real g = 9.81,m = 1,k = 100000,b = 30;
equation
  der(y) = vy;
  f = if y > 0 then 0 else k * y + b * vy;
  m * der(vy) = -m * g - f;
end bball;

