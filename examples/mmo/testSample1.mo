within ;
model testSample1

  Real x(start = 100);
  Real y(start = 10);
  Real y1;
  Real y2;
  Real y3;
  parameter Real t1 = 0;
  parameter Real t2 = 1;
  parameter Real t3 = 2;
  parameter Real p1 = 1e-3;
  parameter Real p2 = 1e-4;
  
  
 equation
  der(x) = if (y > 10) then 10*x - 5*y + y1 else 10;
  der(y) = if (x > 100) then 20*x*y - y + y2 else 10;
  
  when {x > 100, y > 15, sample(0,0.1)} then	
    y1 = 10;
	y2 = y3 - y;  	
  end when;
  
  when {sample(0,1)} then
   y3 = -10;
  end when;
  
end testSample1;