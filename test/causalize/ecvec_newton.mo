model ECVEC_NEWTON
  constant Integer N = 10000;
  Real A[N], B[N];
equation
	for i in 1:N loop
	    B[i] = i;
      A[i] * A[i] - B[i] = 1;
  end for;
end ECVEC_SOL;
