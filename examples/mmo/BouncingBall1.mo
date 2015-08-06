model BouncingBall1
  
  parameter Real g=9.81;
  parameter Real c=0.90;
  Real height(start=0);
  Real v(start=10);
  
  Real y1;
  Real y2;
  Real y3;
  
 equation
  der(height) = v+y3;
  der(v) = -g+y1;
  when {height<0} then	
    reinit(v, -c*v);
	reinit(height,0);
	y1 = 10;
  end when;
  
   when {v>10} then
   y3 = -10;
   y2 = height;
   reinit(height,15);
  end when;
  
  
end BouncingBall1;
