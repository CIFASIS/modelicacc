within ;
model testWhenClause1

  Real x(start = 100);
  Real y(start = 10);
  Real y1;
  Real y2;
  Real y3;
 
 equation
  der(x) = 10*x - 5*y + y1;
  der(y) = 20*x*y - y + y2;
    
  when {x > 100, y > 15} then	
    y1 = 10;
	y2 = y3 - y;  
	
  end when;
  
  when {x < 100} then
   y3 = -10;
  end when;
  
end testWhenClause1;