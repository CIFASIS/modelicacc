model advection2D
  constant Integer N = 20;
  parameter Real ax;
  parameter Real ay;
  parameter Real r;
  parameter Real dx;
  parameter Real dy;
  Real u[N,N];

  initial algorithm

  ax := 1;
  ay := 1;
  r := 1;
  dx := 1;
  dy := 1;
  
  u[1,1] := 1; 

 equation
  
    // Origin equation
    der(u[1,1])=-ax*u[1,1]/dx - ay*u[1,1]/dy + r*(u[1,1]^2-u[1,1]^3);
    
    // Borders 
    for i in 2:N loop
      der(u[i,1])=-ax*u[i,1]/dx - ay*(u[i,1] - u[i-1,1])/dy + r*(u[i,1]^2-u[i,1]^3);
    end for;

    // Borders 
    for j in 2:N loop
      der(u[1,j])=-ax*(u[1,j] - u[1,j-1])/dx - ay*u[1,j]/dy + r*(u[1,j]^2-u[1,j]^3);
    end for;

    // Rest of the grid.
    for i in 2:N, j in 2:N loop
      der(u[i, j])=-ax*(u[i,j] - u[i,j-1])/dx - ay*(u[i,j] - u[i-1,j])/dy + r*(u[i,j]^2-u[i,j]^3);
	    end for;
	annotation(

	experiment(
		MMO_Description="Advection 2D",
		MMO_Solver=LIQSS_BDF,
		MMO_SymDiff=false,
		MMO_Period={100/5000},
		MMO_PartitionMethod=Metis,
		MMO_Output={u[N,N]},
		MMO_OutputType=CI_Sampled,
		Jacobian=Dense,
		MMO_BDF_PDepth=1,
		MMO_BDF_Max_Step=0,
		StartTime=0.0,
		StopTime=100,
		Tolerance={1e-3},
		AbsTolerance={1e-3}
	));
end advection2D;
