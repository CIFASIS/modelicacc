model RLC
  constant Integer N = 5;
  Real A[N,N,N,N], B[N,N,N,N];
equation
	for i in 1:N, j in 1:N, k in 1:N, q in 1:N loop
		A[i,j,k,q] + B[i,j,k,q] = 7;
		A[i,j,k,q] - B[i,j,k,q] = 1;
	end for;
end RLC;
