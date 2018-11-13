model LAZO_NEWTON
  constant Integer N = 1;
  constant Integer M = 10;
  Real A[N],B[N],C[M];
equation
  for i in 1:M loop
		C[i] = i;
   end for;
  for i in 1:N loop
		A[i] * A[i] + B[i] * B[i] * B[i] + C[i] = 9;
		A[i] * A[i] * A[i] + B[i] * B[i] = 10;
	end for;
end LAZO_NEWTON;
