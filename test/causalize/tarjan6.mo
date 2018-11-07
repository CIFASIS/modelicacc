model RLC
  constant Integer N = 10;
  Real A[N], B[N];
equation
	for i in 1:N loop
		A[i] + B[i] = 2*i;
		A[i] - B[i] = 2;
	end for;
end RLC;
