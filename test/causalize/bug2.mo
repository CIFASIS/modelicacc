model RLC
  constant Integer N = 1000000;
  Real A[N], B[N];
equation
	for i in 1:N loop
		A[i] + B[i] = 7;
	end for;
	for i in 1:N/2 loop
		A[i] - B[i] = 1;
		A[i+N/2] - B[i+N/2] = 3;
	end for;
end RLC;
