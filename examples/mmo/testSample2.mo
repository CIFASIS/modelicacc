within ;
model testSample2

  Real x(start = 100);
  Real y(start = 10);

  parameter Real t1 = 0;
  parameter Real t2 = 1;
  parameter Real t3 = 2;
  parameter Real p1 = 1e-3;
  parameter Real p2 = 1e-4;
  
  
 equation
  der(x) = if (y > 100) then y else 10;
  der(y) = if (x > 500) then x else 10;
  
  when {sample(0,0.1)} then	
    reinit(y,0);
	reinit(x,0);  	
  end when;
  
end testSample2;