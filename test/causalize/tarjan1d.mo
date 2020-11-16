model RLC
  constant Integer N = 10000000;
  Real A[N], B[N], C[N];
equation
	for i in 1:N loop
		C[i] = i;
	end for;
	for i in 1:N-1 loop
		A[i] + B[i+1] = 7;
	end for;
	for i in 1:N-1 loop
		A[i] - B[i+1] * B[i+1] + C[i+1] + i = 1;
	end for;
	A[N] = 4;
	B[1] = 3;
end RLC;
