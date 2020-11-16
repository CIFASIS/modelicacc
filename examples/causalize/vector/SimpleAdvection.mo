model SimpleAdvection "Basic thermal advection model with uniform speed"
  parameter Integer N = 10 "Number of nodes";
  parameter Real Tstart[N]=ones(N)*300
    "Start value of the temperature distribution";
  parameter Real L = 10 "Pipe length";
  final parameter Real l = L/N "Length of one volume";
  Real u = 1 "Fluid speed";
  Real Tin = 300 "Inlet temperature";
  Real T[N] "Node temperatures";
  Real Ttilde[N-1](start = Tstart[2:N], each fixed = true) "Temperature states";
  Real Tout;
equation
  for j in 1:N-1 loop
    der(Ttilde[j]) = u/l*(T[j]-T[j+1]);
  end for;
  T[1] = Tin;
  T[N] = Tout;
  for i in 1:N-1 loop 
    Ttilde[i] = T[i+1];
  end for;
  annotation (Documentation(info="<html>
<p>This models solves the temperature advection problem represented by the following PDEs by means of the finite volume method.</p>
<p><img src=\"modelica://ScalableTestSuite/Resources/Images/SimpleAdvection/eq_advection.png\"/></p>
<p><img src=\"modelica://ScalableTestSuite/Resources/Images/SimpleAdvection/bc_advection.png\"/></p>
<p>The boundary condition at the inlet Tin and the fluid speed u are specified by suitable binding equations.</p>
</html>"));
end SimpleAdvection;

