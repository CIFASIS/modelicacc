// Simple Discontinuous model

model simpleDiscont
  Real x1;
  Real x2;
  Real x3;
  Real u;
 equation
 
  u - x3 = 1;
  der(x1) = if u>2 or x3<0.8  then 0 else x2;
  der(x3) = x2;
  der(x2) = 1-0.2*x2-x3;
  
end simpleDiscont;
