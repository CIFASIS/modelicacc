model OneDHeatTransferTI_FD_loop
  parameter Real L = 0.2;
  constant Integer N = 10;
  parameter Real T0 = 273.15;
  parameter Real TN = 330;
  parameter Real cp = 910 ;
  parameter Real lambda = 237 ;
  parameter Real rho = 2712 ;
  final parameter   Real dx = L / (N - 1);
  Real T[N] ;
  Real a,b;
initial algorithm
  for i in 1:N - 1 loop
    T[i] := T0;
  end for;
  T[N] := TN;
equation
  der(T[N]) = 0;
  for i in 2:N - 1 loop
    der(T[i]) = der(T[i-1]) + lambda * ((T[i + 1] - T[i]) / dx + ((-T[i]) + T[i - 1]) / dx) / cp / rho / dx;
  end for;
  der(T[1]) = lambda * ((T[2] - T[1]) / dx) / cp / rho / dx;
  a+b=T[1];
  a*3+b=T[1];
	annotation(

	experiment(
		MMO_Description="",
		MMO_Solver=DOPRI,
		MMO_Output={T[1]},
		StartTime=0.0,
		StopTime=1,
		Tolerance={1e-6},
		AbsTolerance={1e-6}
	));
end OneDHeatTransferTI_FD_loop;
