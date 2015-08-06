within ;
model lotkaVoltera
  parameter Real g_r = 0.04 "Natural Growth for Rabbits";
  parameter Real d_rf = 0.0005 "Death Rate of Rabbits due to Foxes";
  parameter Real d_f = 0.09 "Natural Death Rate of Foxes";
  parameter Real g_fr = 0.1 "Efficiency in growing Foxes f Rabbits";
  Real rabbits(start = 700) "Rabbits with start population 700";
  Real foxes(start = 10) "Foxes with start population 10";
equation
  0 = -der(rabbits) + g_r*rabbits - d_rf*rabbits*foxes;
  der(foxes) = if (foxes>5 or rabbits > 500 and foxes < 20) then g_fr*d_rf*rabbits*foxes - d_f*foxes else 0;
  annotation (uses(Modelica(version="3.0")));
end lotkaVoltera;
