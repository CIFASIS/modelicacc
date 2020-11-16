model RLC
  constant Integer N = 5;
  Real A[N,N], B[N,N], C[N,N];
equation
	for i in 1:N, j in 1:N loop
		C[i,j] = i+j;
		A[i,j] + B[i,j] * B[i,j] + C[i,j]+C[1,1] = 7;
		A[i,j] - B[i,j] = 1;
	end for;
end RLC;
