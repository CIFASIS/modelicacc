model bball2
  Real y(start = 1),vy;
  discrete Integer contact;
  parameter Real g = 9.81,m = 1,k = 100000,b = 30;
equation
  der(y) = vy;
  m * der(vy) = -m * g - contact * (k * y + b * vy);
algorithm
  when y > 0 then
      contact:=0;
  elsewhen y<0 then
      contact:=1;
  end when;
end bball2;

