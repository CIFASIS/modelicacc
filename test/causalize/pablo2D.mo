model AdvectionReaction2D
import file;
  parameter Real dT=1e-3;
  parameter Real ax=1,ay=1,r=1000;
  constant Integer N = 10, M=10;
  parameter Real dx=10/N;
  parameter Real dy=10/M;
  Real u[N,M](each fixed=false);
  Real reaction[N,M](each fixed=false);
initial equation
  for i in 1:5 loop
    u[1,i]=1;
  end for;
equation
  der(u[1,1])=(-u[1,1]*ax/dx)+(-u[1,1]*ay/dy) + reaction[1,1];
  for i in 2:N loop
    der(u[i,1])=(-u[i,1]*ax/dx)+(-u[i,1]+u[i-1,1])*ay/dy + reaction[i,1];
  end for;
  for i in 2:M loop
    der(u[1,i])=(-u[1,i]+u[1,i-1])*ax/dx+(-u[1,i]*ay/dy) + reaction[1,i];
  end for;
  for i in 2:N, j in 2:M loop
    der(u[i,j])=(-u[i,j]+u[i,j-1])*ax/dx+(-u[i,j]+u[i-1,j])*ay/dy + reaction[i,j];
  end for;
  for i in 1:N, j in 1:M loop
	  reaction[i,j] = r*u[i,j]^2*(1-u[i,j]);
  end for;
end AdvectionReaction2D;
