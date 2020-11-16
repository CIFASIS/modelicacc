model AdvectionReaction
  "Model of an advection process with chemical reaction"
  parameter Integer N = 10 "Number of volumes";
  parameter Real mu = 1000 "Kinetic coefficient of the reaction";
  constant Real alpha = 0.5 "Parameter of the reaction model";
  Real u_in = 1 "Inlet concentration";
  Real u[N](each start = 0, each fixed = true)
    "Concentration at each volume outlet";
equation
  der(u[1]) = ((-u[1]) + 1)*N - mu*u[1]*(u[1] - alpha)*(u[1] - 1);
  for j in 2:N loop
    der(u[j]) = ((-u[j]) + u[j-1])*N - mu*u[j]*(u[j] - alpha)*(u[j] - 1);
  end for;
  annotation (Documentation(info="<html>
<p>This models solves the problem represented by the following PDE by means of the finite volume method, on a spatial domain of unit length and assuming unit velocity v.</p>
<p><img src=\"modelica://ScalableTestSuite/Resources/Images/AdvectionReaction/eq_advection_reaction.png\"/></p>
<p>If &mu; = 0, the model represent the transport of a certain chemical species in a fluid, similar to <a href = \"modelica://ScalableTestSuite.Thermal.Advection.Models.SimpleAdvection\">SimpleAdvection</a>. If mu is increased, a chemical reaction is added with two stable equilibria, one at u = 0 and one at u = 1, with an unstable equilibrium at u = &alpha;.</p>
<p> The chemical reaction sharpens the concentration wave front, which would be otherwise be smoothed out by the numerical diffusion effect of the finite volume method.</p>
<p>The boundary condition u_in at the inlet, i.e., u(0,t), is specified by suitable binding equations.</p>
</html>"));
end AdvectionReaction;

