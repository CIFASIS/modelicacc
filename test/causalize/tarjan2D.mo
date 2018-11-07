model RLC
  constant Integer N = 5;
  Real A[N,N], B[N,N];
equation
	for i in 1:N, j in 1:N loop
		A[i,j] + B[i,j] = 7;
		A[i,j] - B[i,j] = 1;
	end for;
end RLC;
