model RLC
  constant Integer N = 10;
  Real A[N], B[N];
equation
	for i in 1:N loop
		A[i] + B[i] = 7;
		A[i] - B[i] = 1;
	end for;
end RLC;
