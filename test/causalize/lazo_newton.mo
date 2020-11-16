model LAZO_NEWTON
  constant Integer N = 1;
  Real A[N],B[N],C[N];
equation
  for i in 1:N loop
		C[i] = i;
		A[i] * A[i] + B[i] * B[i] * B[i] + C[i] = 9;
		A[i] * A[i] * A[i] + B[i] * B[i] = 10;
	end for;
end LAZO_NEWTON;
