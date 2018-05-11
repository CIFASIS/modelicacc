model RLC
  constant Integer N = 1000000000;
  Real A[N], B[N];
equation
	A[1] = 5;
	for i in 1:N loop
		A[i] + B[i] = 12;
	end for;
	for i in 1:N-1 loop
		B[i+1] = 8;
	end for;
end RLC;
