model LAZO_NEWTON_SOL
  function fsolve1
    input Real u_0;
    output Real y_0;
    output Real y_1;
   external "C"  fsolve1(u_0,y_0,y_1)    annotation(Library={"m","gsl","blas"},Include="#include \"LAZO_NEWTON.c\"");
  end fsolve1;
  constant Integer N=1;
  Real A[N];
  Real B[N];
  Real C[N];
equation
  for i in 1:N loop
	  C[i] = i;
	  (A[i],B[i]) = fsolve1(C[i]);
  end for;
end LAZO_NEWTON_SOL;
