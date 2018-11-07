model RLC
  constant Integer N = 10;
  Real A[N], B[N];
equation
	for i in 1:N loop
		B[i] = i;
	end for;
	for i in 1:N loop
		A[i] * A[i] + B[2] = 12;
	end for;
end RLC;
