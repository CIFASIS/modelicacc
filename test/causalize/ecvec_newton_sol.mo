model ECVEC_NEWTON_SOL
  function fsolve1
    input Real u_0;
    output Real y_0;
   external "C" y_0 = fsolve1(u_0)    annotation(Library={"m","gsl","blas"},Include="#include \"ECVEC.c\"");
  end fsolve1;
  constant Integer N=10000;
  Real A[N];
  Real B[N];
equation
  for i in 1:N loop
    B[i] = i;
  end for;
  for i in 1:N loop
    A[i] = fsolve1(B[i]);
  end for;
end ECVEC_NEWTON_SOL;
