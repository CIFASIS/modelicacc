model AdvectionReaction3D
import file;
  parameter Real dT=1e-3;
  parameter Real ax=1,ay=1,az=1, r=1000;
  constant Integer N = 10, M=10, O = 10;
  parameter Real dx=10/N;
  parameter Real dy=10/M;
  parameter Real dz=10/O;
  Real u[N,M,O](each fixed=false);
  Real reaction[N,M,O](each fixed=false);
initial equation
  for i in 1:5 loop
    u[1,i,1]=1;
  end for;
equation
  // First volume
    der(u[1,1,1]) = (-u[1,1,1]*ax/dx) + (-u[1,1,1]*ay/dy) + (-u[1,1,1]*az/dz) + reaction[1,1,1];

  // X axis row
  for i in 2:N loop
    der(u[i,1,1]) = ((-u[i,1,1] + u[i-1,1,1])*ax/dx) + (-u[i,1,1])*ay/dy + (-u[i,1,1]*az/dz) + reaction[i,1,1];
  end for;

  // Y axis row
  for j in 2:M loop
    der(u[1,j,1]) = (-u[1,j,1]*ax/dz) + (-u[1,j,1] + u[1,j-1,1])*ay/dy + (-u[1,j,1]*az/dz) + reaction[1,j,1];
  end for;

  // Z axis row
  for k in 2:O loop
    der(u[1,1,k]) = (-u[1,1,k]*ax/dx) + (-u[1,1,k]*ay/dy) + ((-u[1,1,k] + u[1,1,k-1]) *az/dz) + reaction[1,1,k];
  end for;

  // XY plane 
  for i in 2:N, j in 2:M loop
    der(u[i,j,1]) = ((-u[i,j,1] + u[i-1,j,1]) *ax/dx) + (-u[i,j,1] + u[i,j-1,1])*ay/dy + (-u[i,j,1]*az/dz) + reaction[i,j,1];
  end for;

  // XZ plane 
  for i in 2:N, k in 2:O loop
    der(u[i,1,k]) = ((-u[i,1,k] + u[i-1,1,k]) *ax/dx) + (-u[i,1,k])*ay/dy + ((-u[i,1,k] + u[i,1,k-1])*az/dz) + reaction[i,1,k];
  end for;

  // YZ plane 
  for j in 2:M, k in 2:O loop
    der(u[1,j,k]) = (-u[1,j,k]*ax/dx) + ((-u[1,j,k]+u[1,j-1,k]))*ay/dy + ((-u[1,j,k] + u[1,j,k-1])*az/dz) + reaction[1,j,k];
  end for;

  // The common volumes
  for i in 2:N, j in 2:M, k in 2:O loop
    der(u[i,j,k]) = ((-u[i,j,k]+u[i-1,j,k])*ax/dx) + ((-u[i,j,k]+u[i,j-1,k]))*ay/dy + ((-u[i,j,k] + u[i,j,k-1])*az/dz) + reaction[i,j,k];
  end for;
  for i in 1:N, j in 1:M, k in 1:O loop
	  reaction[i,j,k] = r*u[i,j,k]^2*(1-u[i,j,k]);
  end for;
end AdvectionReaction3D;
